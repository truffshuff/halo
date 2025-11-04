import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from pathlib import Path
import shutil

CODEOWNERS = ["@truffshuff"]
DEPENDENCIES = ["api"]
AUTO_LOAD = []

nimble_proxy_ns = cg.esphome_ns.namespace("nimble_proxy")
NimBLEProxy = nimble_proxy_ns.class_("NimBLEProxy", cg.Component)

CONF_ACTIVE = "active"
CONF_MAX_CONNECTIONS = "max_connections"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(NimBLEProxy),
        cv.Optional(CONF_ACTIVE, default=True): cv.boolean,
        cv.Optional(CONF_MAX_CONNECTIONS, default=3): cv.int_range(min=1, max=9),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    import os

    # Define USE_BLUETOOTH_PROXY to enable bluetooth proxy API types
    cg.add_define("USE_BLUETOOTH_PROXY")

    # Copy the stub bluetooth_proxy header to the build directory
    # This is needed because ESPHome's API component includes it
    component_dir = Path(__file__).parent
    stub_header_src = component_dir / "esphome" / "components" / "bluetooth_proxy" / "bluetooth_proxy.h"

    # Get the actual build directory from CORE
    from esphome import cpp_generator as cpp
    from esphome.core import CORE
    build_dir = Path(CORE.relative_build_path("src"))
    stub_header_dst = build_dir / "esphome" / "components" / "bluetooth_proxy"

    # Ensure the destination directory exists and copy the header
    stub_header_dst.mkdir(parents=True, exist_ok=True)
    if stub_header_src.exists():
        shutil.copy2(str(stub_header_src), str(stub_header_dst / "bluetooth_proxy.h"))

    # Provide sane defaults for API compile-time constants if not set
    cg.add_build_flag("-DBLUETOOTH_PROXY_ADVERTISEMENT_BATCH_SIZE=5")
    cg.add_build_flag("-DBLUETOOTH_PROXY_MAX_CONNECTIONS=%d" % config[CONF_MAX_CONNECTIONS])

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_active(config[CONF_ACTIVE]))
    cg.add(var.set_max_connections(config[CONF_MAX_CONNECTIONS]))

    # No library dependency needed - using ESP-IDF native NimBLE
