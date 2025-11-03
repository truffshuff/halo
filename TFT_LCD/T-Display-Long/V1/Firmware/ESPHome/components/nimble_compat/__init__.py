import esphome.codegen as cg

CODEOWNERS = ["@truffshuff"]
AUTO_LOAD = []

nimble_compat_ns = cg.esphome_ns.namespace("nimble_compat")


def to_code(config):
    # This component doesn't generate any code, it just provides header compatibility
    pass
