import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@truffshuff"]
AUTO_LOAD = []
DEPENDENCIES = []

nimble_compat_ns = cg.esphome_ns.namespace("nimble_compat")

CONFIG_SCHEMA = cv.Schema({})


async def to_code(config):
    # Add include paths for compatibility headers
    cg.add_build_flag("-I" + str(cg.RawExpression("$PROJECT_DIR/.esphome/external_components/nimble_compat")))
    cg.add_build_flag("-DCONFIG_BT_NIMBLE_ENABLED=1")
    
    # Note: The headers in this component directory will be available
    # to include from ESPHome BLE components
    pass
