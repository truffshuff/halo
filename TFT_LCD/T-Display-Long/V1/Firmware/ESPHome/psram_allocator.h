/**
 * PSRAM Allocator for ESPHome
 *
 * Provides utilities for forcing allocations to PSRAM (external SPIRAM)
 * instead of internal heap on ESP32-S3 devices.
 *
 * This is particularly useful for weather sensors and other components
 * that create many temporary string buffers, which can exhaust internal heap.
 *
 * Usage in ESPHome lambda:
 *   char* buffer = psram_alloc<char>(1024);  // Allocate in PSRAM
 *   snprintf(buffer, 1024, "Temperature: %.2f", temp);
 *   psram_free(buffer);
 */

#pragma once

#include <cstdlib>
#include <cstring>
#include "esp_heap_caps.h"
#include "esp_log.h"

namespace esphome {
namespace psram {

static const char* TAG = "psram_alloc";

/**
 * Allocate memory in PSRAM (external SPIRAM)
 *
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory in PSRAM, or nullptr on failure
 */
template<typename T>
inline T* psram_alloc(size_t count) {
    size_t size = count * sizeof(T);
    void* ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);

    if (ptr) {
        ESP_LOGV(TAG, "Allocated %zu bytes in PSRAM at %p", size, ptr);
        return static_cast<T*>(ptr);
    } else {
        ESP_LOGE(TAG, "Failed to allocate %zu bytes in PSRAM", size);
        return nullptr;
    }
}

/**
 * Allocate memory in PSRAM with alignment
 *
 * @param size Number of bytes to allocate
 * @param alignment Alignment requirement (must be power of 2)
 * @return Pointer to aligned memory in PSRAM, or nullptr on failure
 */
template<typename T>
inline T* psram_alloc_aligned(size_t count, size_t alignment) {
    size_t size = count * sizeof(T);
    void* ptr = heap_caps_aligned_alloc(alignment, size, MALLOC_CAP_SPIRAM);

    if (ptr) {
        ESP_LOGV(TAG, "Allocated %zu bytes (aligned %zu) in PSRAM at %p", size, alignment, ptr);
        return static_cast<T*>(ptr);
    } else {
        ESP_LOGE(TAG, "Failed to allocate %zu bytes (aligned %zu) in PSRAM", size, alignment);
        return nullptr;
    }
}

/**
 * Free PSRAM-allocated memory
 *
 * @param ptr Pointer to memory allocated with psram_alloc
 */
template<typename T>
inline void psram_free(T* ptr) {
    if (ptr) {
        ESP_LOGV(TAG, "Freeing PSRAM at %p", ptr);
        heap_caps_free(ptr);
    }
}

/**
 * Allocate and zero-initialize memory in PSRAM
 *
 * @param count Number of elements to allocate
 * @return Pointer to zero-initialized memory in PSRAM, or nullptr on failure
 */
template<typename T>
inline T* psram_calloc(size_t count) {
    void* ptr = heap_caps_calloc(count, sizeof(T), MALLOC_CAP_SPIRAM);

    if (ptr) {
        ESP_LOGV(TAG, "Allocated and zeroed %zu bytes in PSRAM at %p", count * sizeof(T), ptr);
        return static_cast<T*>(ptr);
    } else {
        ESP_LOGE(TAG, "Failed to allocate %zu bytes in PSRAM", count * sizeof(T));
        return nullptr;
    }
}

/**
 * Duplicate a string in PSRAM
 *
 * @param str String to duplicate
 * @return Pointer to duplicated string in PSRAM, or nullptr on failure
 */
inline char* psram_strdup(const char* str) {
    if (!str) return nullptr;

    size_t len = strlen(str) + 1;
    char* dup = psram_alloc<char>(len);

    if (dup) {
        memcpy(dup, str, len);
        ESP_LOGV(TAG, "Duplicated string (%zu bytes) in PSRAM at %p", len, dup);
    }

    return dup;
}

/**
 * RAII wrapper for PSRAM allocations
 * Automatically frees memory when going out of scope
 *
 * Usage:
 *   PSRAMBuffer<char> buffer(1024);
 *   snprintf(buffer.get(), buffer.size(), "Temperature: %.2f", temp);
 */
template<typename T>
class PSRAMBuffer {
public:
    PSRAMBuffer(size_t count) : count_(count), ptr_(nullptr) {
        ptr_ = psram_alloc<T>(count);
    }

    ~PSRAMBuffer() {
        if (ptr_) {
            psram_free(ptr_);
            ptr_ = nullptr;
        }
    }

    // No copying
    PSRAMBuffer(const PSRAMBuffer&) = delete;
    PSRAMBuffer& operator=(const PSRAMBuffer&) = delete;

    // Allow moving
    PSRAMBuffer(PSRAMBuffer&& other) : count_(other.count_), ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    PSRAMBuffer& operator=(PSRAMBuffer&& other) {
        if (this != &other) {
            if (ptr_) psram_free(ptr_);
            count_ = other.count_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    T* get() { return ptr_; }
    const T* get() const { return ptr_; }
    size_t size() const { return count_; }
    bool valid() const { return ptr_ != nullptr; }

    operator bool() const { return valid(); }
    T& operator[](size_t index) { return ptr_[index]; }
    const T& operator[](size_t index) const { return ptr_[index]; }

private:
    size_t count_;
    T* ptr_;
};

/**
 * Get PSRAM memory statistics
 */
struct PSRAMStats {
    size_t total_size;
    size_t free_size;
    size_t largest_free_block;
    size_t minimum_free_size;
};

inline PSRAMStats get_psram_stats() {
    PSRAMStats stats;
    stats.total_size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    stats.free_size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    stats.largest_free_block = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    stats.minimum_free_size = heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);
    return stats;
}

inline void log_psram_stats() {
    PSRAMStats stats = get_psram_stats();
    ESP_LOGI(TAG, "PSRAM Stats:");
    ESP_LOGI(TAG, "  Total: %zu bytes (%.2f MB)", stats.total_size, stats.total_size / 1048576.0);
    ESP_LOGI(TAG, "  Free: %zu bytes (%.2f MB)", stats.free_size, stats.free_size / 1048576.0);
    ESP_LOGI(TAG, "  Largest block: %zu bytes (%.2f KB)", stats.largest_free_block, stats.largest_free_block / 1024.0);
    ESP_LOGI(TAG, "  Min free ever: %zu bytes (%.2f KB)", stats.minimum_free_size, stats.minimum_free_size / 1024.0);
}

/**
 * Get internal heap memory statistics
 */
struct InternalHeapStats {
    size_t total_size;
    size_t free_size;
    size_t largest_free_block;
    size_t minimum_free_size;
};

inline InternalHeapStats get_internal_heap_stats() {
    InternalHeapStats stats;
    stats.total_size = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    stats.free_size = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    stats.largest_free_block = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
    stats.minimum_free_size = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
    return stats;
}

inline void log_internal_heap_stats() {
    InternalHeapStats stats = get_internal_heap_stats();
    ESP_LOGI(TAG, "Internal Heap Stats:");
    ESP_LOGI(TAG, "  Total: %zu bytes (%.2f KB)", stats.total_size, stats.total_size / 1024.0);
    ESP_LOGI(TAG, "  Free: %zu bytes (%.2f KB)", stats.free_size, stats.free_size / 1024.0);
    ESP_LOGI(TAG, "  Largest block: %zu bytes (%.2f KB)", stats.largest_free_block, stats.largest_free_block / 1024.0);
    ESP_LOGI(TAG, "  Min free ever: %zu bytes (%.2f KB)", stats.minimum_free_size, stats.minimum_free_size / 1024.0);
}

inline void log_memory_summary() {
    ESP_LOGI(TAG, "=== Memory Summary ===");
    log_internal_heap_stats();
    log_psram_stats();
}

} // namespace psram
} // namespace esphome

// Convenience macros for use in ESPHome lambdas
#define PSRAM_ALLOC(type, count) esphome::psram::psram_alloc<type>(count)
#define PSRAM_FREE(ptr) esphome::psram::psram_free(ptr)
#define PSRAM_STRDUP(str) esphome::psram::psram_strdup(str)
#define PSRAM_BUFFER(type, count) esphome::psram::PSRAMBuffer<type>(count)
#define LOG_MEMORY_STATS() esphome::psram::log_memory_summary()
