import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

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
    # Define USE_BLUETOOTH_PROXY to enable bluetooth proxy API types
    cg.add_define("USE_BLUETOOTH_PROXY")

    # Add the component directory to the include path so our stub header
    # at bluetooth_proxy_shim.h can be found
    # This will be used by creating a symlink or copying during build
    from pathlib import Path
    import shutil
    import os
    from esphome.core import CORE

    # Get component directory
    component_dir = Path(__file__).parent

    # Get build source directory
    build_src_dir = Path(CORE.relative_build_path("src"))

    # Create the bluetooth_proxy directory in build
    bt_proxy_dir = build_src_dir / "esphome" / "components" / "bluetooth_proxy"
    bt_proxy_dir.mkdir(parents=True, exist_ok=True)

    # Copy the stub header from the nested location
    src_header = component_dir / "esphome" / "components" / "bluetooth_proxy" / "bluetooth_proxy.h"
    dst_header = bt_proxy_dir / "bluetooth_proxy.h"

    if src_header.exists():
        shutil.copy2(str(src_header), str(dst_header))
        print(f"Copied BT proxy stub header from {src_header} to {dst_header}")
    else:
        print(f"WARNING: Stub header not found at {src_header}")

    # Provide sane defaults for API compile-time constants if not set
    cg.add_build_flag("-DBLUETOOTH_PROXY_ADVERTISEMENT_BATCH_SIZE=5")
    cg.add_build_flag("-DBLUETOOTH_PROXY_MAX_CONNECTIONS=%d" % config[CONF_MAX_CONNECTIONS])

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_active(config[CONF_ACTIVE]))
    cg.add(var.set_max_connections(config[CONF_MAX_CONNECTIONS]))

    # No library dependency needed - using ESP-IDF native NimBLE
