# Halo v1 — Memory Architecture

Why memory is placed where it is on this board, and what will break if you move it.

Read this before changing any buffer size, `sdkconfig_option`, `psram:` setting, or
allocation in a lambda. Several values here look wasteful and are not.

Baseline: **ESPHome 2026.9.0**, ESP-IDF 5.5.x, native `esp-idf` toolchain.

---

## 1. The one rule

> **Internal SRAM is scarce. PSRAM is not. Anything that does not *require* internal SRAM
> belongs in PSRAM.**

The ESP32-S3 has ~512 KB of internal SRAM. How much of that reaches the heap depends on which
stacks are linked in — read the actual figure from the `Free Heap` diagnostic sensor on your
device rather than trusting a number in a document. It has 8 MB of PSRAM, of which this
firmware uses a small fraction.

The metric that matters is **minimum free internal heap during normal operation**, not idle
free heap. Peak pressure happens at specific, predictable moments (§6), and those are what
cause crashes.

---

## 2. What must stay in internal SRAM

Never move these. ESP-IDF, the WiFi driver, or the hardware require them.

| Consumer | Why it cannot go to PSRAM |
|---|---|
| **lwIP pbufs / TCP buffers** | The lwIP memory pools are not PSRAM-capable in this configuration. Every TCP window and send buffer byte is internal DRAM. |
| **WiFi driver RX/TX buffers** | Fed to DMA. `CONFIG_SPIRAM_TRY_ALLOCATE_WIFI_LWIP` was tried here and **reverted** — it made free internal heap *worse*, because the driver then had to bounce through internal buffers anyway. |
| **ISR stacks and any code/data touched while the flash cache is disabled** | PSRAM is behind the same cache as flash. A cache miss inside an ISR that runs during a flash write is a crash. |
| **DMA descriptors** | Must be DMA-capable memory. |
| **Small, hot allocations (< 512 bytes)** | Kept internal on purpose via `CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL`. PSRAM access latency would show up on hot paths. |

`CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL: "32768"` holds 32 KB of internal DRAM back so that
PSRAM-eligible allocations can never starve the ones in this table. It is a guard rail, not
waste — do not lower it to "free" memory.

---

## 3. What lives in PSRAM

| Consumer | Approx. size | How it gets there |
|---|---|---|
| **LVGL draw buffer** | 115.2 KB (exact: 180×640/2×2) | `lvgl: buffer_size: 50%` → ESPHome allocates 1/2-screen buffers straight from PSRAM (§4) |
| **LVGL object/widget heap** | varies | ESPHome's `lv_malloc_core` uses `MALLOC_CAP_SPIRAM \| MALLOC_CAP_8BIT` first, internal as fallback |
| **NimBLE stack** | ~40 KB | `CONFIG_BT_NIMBLE_MEM_ALLOC_MODE_EXTERNAL: "y"` (only with `ble_improv.yaml`) |
| **Bluedroid host tables** | not yet measured | `esp32_ble: use_psram: true` (→ `BT_BLE_DYNAMIC_ENV_MEMORY`) plus a manual `CONFIG_BT_ALLOCATION_FROM_SPIRAM_FIRST: "y"` (only with `ble_esphome.yaml`). ESPHome sets the second one only on the original ESP32; on the S3 it must be set by hand — see §10 |
| **mbedTLS contexts** | ~50 KB per TLS session | `CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC: "y"`. Without this, `mbedtls_ssl_setup()` / `mbedtls_ctr_drbg_seed()` fail with `-0x7F00` / `-0x0001` against a ~28 KB internal heap. |
| **ArduinoJson forecast pools** | up to ~40 KB transiently | explicit `ArduinoJson::Allocator` subclass using `heap_caps_malloc(..., MALLOC_CAP_SPIRAM)` (§5) |
| **HTTP response body string** | 32 KB reserved | `std::string::reserve(32768)` ≥ the ALWAYSINTERNAL threshold, so it lands in PSRAM |
| **`online_image` download buffer** | 64 KB | ESPHome's `RAMAllocator` defaults to *external first, internal fallback* |
| **Decoded printer cover image** | ~39 KB (140×140 RGB565) | same `RAMAllocator` default |
| **AQI history ring buffer** | ~2.3 KB | explicit `heap_caps_malloc(..., MALLOC_CAP_SPIRAM)` in `airq_history.yaml`, allocated lazily |
| **`.bss` segment** | varies | `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY: "y"` |
| **Instructions and rodata** | — | `execute_from_psram: true` (`CONFIG_SPIRAM_FETCH_INSTRUCTIONS` + `CONFIG_SPIRAM_RODATA`) — also what allows the display to keep drawing during an OTA flash write |

---

## 4. The LVGL draw buffer — the biggest single lever

This deserves its own section because the mechanism is not obvious and the old configuration
had it backwards.

ESPHome quantises `buffer_size:` to a fraction of the screen
(`esphome/components/lvgl/__init__.py`):

| `buffer_size:` | fraction used | bytes at 180×640×16 bpp |
|---|---|---|
| ≥ 75 % | 1/1 | 230.4 KB |
| ≥ 37.5 % | 1/2 | 115.2 KB |
| > 19 % | 1/4 | 57.6 KB |
| otherwise | 1/8 | 28.8 KB |

Then it chooses the heap (`lvgl_esphome.cpp`, `LvglComponent::setup`):

```cpp
// for small buffers, try to allocate in internal memory first to improve performance
if (this->buffer_frac_ >= MIN_BUFFER_FRAC / 2)   // frac >= 4, i.e. 1/4 or 1/8
    buffer = lv_alloc_draw_buf(buf_bytes, true);  // INTERNAL first
if (buffer == nullptr)
    buffer = lv_alloc_draw_buf(buf_bytes, false); // PSRAM
```

So **a 1/4 or 1/8 buffer is deliberately placed in internal DRAM; a 1/1 or 1/2 buffer goes
straight to PSRAM.**

The previous setting of `30%` quantised to 1/4 and therefore put **~57.6 KB in internal
DRAM** — despite a comment claiming `CONFIG_SPIRAM_USE_MALLOC` sent it to PSRAM. It did not;
that sdkconfig option affects `malloc()`, while LVGL calls `heap_caps_aligned_alloc()` with
explicit capabilities.

**Current setting: `buffer_size: 50%`** → 1/2 → 115.2 KB allocated from PSRAM, returning
~57.6 KB of internal DRAM to the heap.

Trade-off accepted: rendering now reads and writes PSRAM. Mitigating factors — octal PSRAM at
80 MHz, `execute_from_psram` already enabled, and ESPHome 2026.9.0 added PSRAM DMA for
external SPI buffers (esphome#18699) so the flush path stays on DMA. The display's 5 MHz QSPI
data rate dominates frame time either way.

**If the display tears, stutters, or fails to allocate:** set `buffer_size: 30%` and expect
~57.6 KB less free internal heap. **This change requires physical-device testing.**

---

## 5. PSRAM-aware allocation in lambdas

### ArduinoJson

ArduinoJson 7 keeps its growing data pool in a *separate* allocation reached through an
`Allocator`. The `JsonDocument` object itself is only ~32 bytes.

The previous pattern was:

```cpp
void* json_mem = heap_caps_malloc(sizeof(JsonDocument), MALLOC_CAP_SPIRAM);
JsonDocument *doc = new (json_mem) JsonDocument;   // ~32 bytes in PSRAM
...
doc->~JsonDocument();
heap_caps_free(json_mem);
```

This placed only those 32 bytes in PSRAM. The pool — the part that grows to tens of
kilobytes — went through the default allocator, and stayed out of internal DRAM **only
because `CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL` happens to be 512**. It worked by accident,
and every early return needed a matching manual teardown.

The current pattern makes it explicit and gives the document normal RAII:

```cpp
struct PsramJsonAllocator : ArduinoJson::Allocator {
  void *allocate(size_t size) override { return heap_caps_malloc(size, MALLOC_CAP_SPIRAM); }
  void deallocate(void *ptr) override { heap_caps_free(ptr); }
  void *reallocate(void *ptr, size_t n) override { return heap_caps_realloc(ptr, n, MALLOC_CAP_SPIRAM); }
};
static PsramJsonAllocator psram_json_alloc;
JsonDocument doc(&psram_json_alloc);
```

Use this pattern for any new JSON parsing of Home Assistant payloads.

### Raw buffers

`heap_caps_malloc(size, MALLOC_CAP_SPIRAM)` with a matching `heap_caps_free`. See
`features/airq/airq_history.yaml` for a worked example including re-allocation on size
change.

### Avoid per-tick allocation

Anything inside a 1 s or 100 ms interval must not allocate. The page-rotation lambda used to
build a `std::vector<int>` and a `std::vector<std::string>` on **every tick** — roughly 16
allocate/free pairs per second for the entire uptime of the device. It now uses fixed stack
arrays of `const char *` and allocates nothing. Prefer `static char buf[N]` + `snprintf`,
which the sensor `on_value` lambdas already do.

---

## 6. Known peak-memory scenarios

Idle free heap is not the number to watch. These are the moments that actually fail:

1. **Boot, ~0–30 s.** WiFi stack, BLE stack (if enabled), LVGL buffer, fonts and all
   component `setup()` calls land at once. `execute_from_psram` and `.bss` in PSRAM exist to
   flatten this peak.
2. **TLS handshake to Home Assistant.** ~50 KB across `mbedtls_ssl_setup()` and
   `mbedtls_ctr_drbg_seed()`. This is the failure that `CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC`
   was added to fix; symptoms are `-0x7F00` / `-0x0001`.
3. **Hourly forecast fetch.** 32 KB response string + ArduinoJson pool + 24 forecast entries
   written into global arrays, all while the BLE scanner is paused. Watchdog is at 30 s for
   this path.
4. **Printer cover image download.** 64 KB download buffer + ~39 KB decoded image. Since
   ESPHome 2026.9.0 (esphome#18488) the image **decoder stays allocated for the component's
   lifetime** instead of being freed after decode — a new, persistent cost in this release.
   Both buffers are PSRAM-resident via `RAMAllocator`.
5. **Bluetooth-proxy advertisement bursts** (only with `ble_improv.yaml`). Fills the API send
   path; this is what `tcp_send_buffer` and `max_send_queue` exist to absorb.
6. **OTA.** Firmware write plus an active API connection. `ota_in_progress` suppresses page
   rotation and pauses BLE scanning to keep the peak down.

---

## 7. Tunables, in the order to try them

If free internal heap gets tight, change these one at a time and measure.

| Setting | Where | Current | Internal-RAM effect |
|---|---|---|---|
| `network: tcp_send_buffer` | `system/esphome_core.yaml` | 65535 | **Largest tunable.** Try 32768, then 16384. Watch for "TCP buffer" warnings. |
| `CONFIG_LWIP_TCP_WND_DEFAULT` | `system/esphome_core.yaml` | 65535 | Receive window. Sized for the ~20 KB forecast JSON; 32768 is likely still fine. |
| `api: max_send_queue` | `Halo-v1-Core.yaml` | 16 | 2 KB per slot → ~32 KB ceiling, retained at high-water mark since 2026.9.0. |
| `api: max_connections` | `Halo-v1-Core.yaml` | 8 | Bounds concurrent per-connection buffers. ESPHome's ESP32 default is 5. |
| `lvgl: buffer_size` | `Halo-v1-Core.yaml` | 50% | 30% moves ~57.6 KB **back into** internal DRAM. Only go this way for display problems. |
| `ble_stub.yaml` instead of `ble_improv.yaml` | package list | — | Removes the whole BLE stack (~40 KB, mostly PSRAM) and its API traffic. |
| `weather_hourly.yaml` removed | package list | enabled | Removes the largest forecast path and its 8 UI pages. |

**Do not** lower `CONFIG_SPIRAM_MALLOC_RESERVE_INTERNAL` or raise
`CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL` to chase free heap — the first removes the guard rail
protecting DMA/ISR allocations, the second pushes medium allocations back into internal DRAM.

---

## 8. Known hazards

- **`CONFIG_SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY: "y"`** permits task stacks in PSRAM. It is
  kept because removing it would push stacks back into internal DRAM. The hazard: a task
  whose stack lives in PSRAM must never run while the flash cache is disabled (flash writes,
  NVS commits). Nothing in the current configuration creates such a task. **Re-check this if
  you add a native component that spawns its own FreeRTOS task.**

- **`enable_high_performance` must stay `false`.** It is auto-enabled by components that want
  throughput (`speaker/media_player`, `sendspin`). Because `psram: ignore_not_found: false`
  makes ESPHome treat PSRAM as guaranteed, turning it on would apply **512 KB TCP windows and
  512-deep lwIP mailboxes** — far more internal DRAM than this board can spare. It is pinned
  to `false` explicitly so adding a future component cannot silently enable it.

- **Fonts are flash, not RAM.** ESPHome emits glyph bitmaps as `const` arrays in flash and
  LVGL reads them in place. Removing a font size saves flash and build time, not heap. The
  README's historical "~200 KB, largest memory consumer" line referred to flash.

---

## 9. Diagnosing a memory problem

Enable `features/diagnostics/diagnostics.yaml` (it is commented out in `Halo-v1.yaml` and
enabled in the device file). It exposes, all at 60 s:

| Entity | `heap_caps_*` call | Read it for |
|---|---|---|
| **Min Free Heap Ever** | `get_minimum_free_size(MALLOC_CAP_INTERNAL)` | **the number that matters** — worst case since boot |
| Free Heap | `get_free_size(MALLOC_CAP_INTERNAL)` | current internal headroom |
| Largest Free Block | `get_largest_free_block(MALLOC_CAP_INTERNAL)` | fragmentation |
| Heap Fragmentation | `(free − largest) / free × 100` | rising over days ⇒ a churn source |
| DMA Capable Heap Free / Largest Block | `MALLOC_CAP_DMA` | WiFi/display allocation failures |
| PSRAM Free | `MALLOC_CAP_SPIRAM` | PSRAM exhaustion (unlikely) |
| Total Free Heap (All) | `MALLOC_CAP_8BIT` | internal + PSRAM combined |

Method:

1. Record **Min Free Heap Ever** and **Heap Fragmentation** after 24 h of normal operation.
2. Reproduce the peak scenario from §6 that you suspect and watch **Free Heap** during it.
3. Rising fragmentation with stable free heap points at repeated allocate/free of similar
   sizes — look for allocation inside an interval or a script that runs often.
4. Falling **Min Free Heap Ever** with stable fragmentation points at a leak or a newly
   retained buffer.
5. `ESP_LOGI` around a suspect block with
   `heap_caps_get_free_size(MALLOC_CAP_INTERNAL)` before and after. `weather_base.yaml`
   already does this around the forecast fetch.

---

## 10. Measurement status

No before/after device measurements are recorded here. The changes in the 2026.09
modernization were validated by `esphome config` against ESPHome 2026.9.0 and by static
analysis of the ESPHome 2026.9.0 allocator source; **they have not been run on hardware.**

To populate a baseline, record from the diagnostics entities above:

| Metric | Before | After |
|---|---:|---:|
| Free internal heap | not measured | **102,551 – 105,363 B** |
| Free heap, all caps (internal + PSRAM) | not measured | **5,191,127 – 5,211,335 B** |
| Min free internal heap ever | not measured | **~63 KB** (NimBLE; see below) |
| Largest free internal block | not measured | not measured |
| Heap fragmentation % | not measured | **69.7 %** (72 s uptime) |
| Firmware size | not measured | **2,374,128 B** |

The "after" figures are from serial logs on device `halo-v1-79e1f8` running the
2026.09 build (`weather:1122` logs internal and total free heap before each hourly
fetch). There is no "before" column because the pre-change firmware was never
instrumented this way — do not read the blanks as zero.

**What this supports:** ~102 KB free internal heap is consistent with the LVGL draw
buffer living in PSRAM. Had it remained internal at the old `buffer_size: 30%`
(1/4 screen = 57.6 KB), free internal would sit nearer 45–48 KB. That is corroboration,
not proof — an A/B at `30%` on the same device would settle it.

**`Min Free Heap Ever`** (`heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL)`) —
the number that matters, because idle free heap does not tell you whether a peak
scenario from §6 nearly exhausted internal RAM:

| Reading | Value | Conditions |
|---|---:|---|
| `halo-v1-79e1f8`, NimBLE | 66,484 B | 72 s uptime — boot transient only, before any weather refresh |
| one unit, NimBLE (`ble_improv.yaml`) | ~63 KB | 2026-09-22; uptime not recorded |
| same unit, Bluedroid (`ble_esphome.yaml`) | ~40 KB | 2026-09-22; before the Bluedroid PSRAM options existed |

**BLE stack comparison.** Swapping NimBLE for ESPHome's native Bluedroid stack cost
~23 KB of minimum free internal heap. Two things make that larger than the stacks
alone: the NimBLE build keeps its host in PSRAM (`NIMBLE_MEM_ALLOC_MODE_EXTERNAL`)
while the Bluedroid package had no equivalent, and `bluetooth_proxy` defaults to 3
connection slots where `ble_improv.yaml` runs 1. `ble_esphome.yaml` now sets
`esp32_ble: use_psram: true` and `CONFIG_BT_ALLOCATION_FROM_SPIRAM_FIRST` to address
the first; the recovered amount is **not yet measured**. ESPHome 2026.9.0 applies
`SPIRAM_FIRST` only on the original ESP32, citing a missing Kconfig symbol on
BLE-only chips — but in ESP-IDF 5.5.5 the symbol depends only on
`BT_BLUEDROID_ENABLED` and the allocator (`osi/allocator.h`) is chip-independent.

Re-read all three after 24 h of normal operation.

The expected direction of change is **+~57.6 KB internal heap** from the LVGL buffer move,
plus a reduction in fragmentation from removing the per-second vector churn in the page
rotation lambda. Both are predictions from source inspection, not measurements.
