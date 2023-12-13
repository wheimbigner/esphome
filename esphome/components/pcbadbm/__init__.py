import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID, CONF_FILTER

DEPENDENCIES = ["i2c"]
CODEOWNERS = ["@wheimbigner"]
MULTI_CONF = False

pcbadbm_ns = cg.esphome_ns.namespace("pcbadbm")
PCBADBMComponent = pcbadbm_ns.class_("PCBADBMComponent", cg.Component, i2c.I2CDevice)

CONF_PCBADBM_ID = "pcbadbm_id"

PCBADBMFilter = pcbadbm_ns.enum("PCBADBMFilter")
FILTER_OPTIONS = {
    "NONE": PCBADBMFilter.PCBADBM_FILTER_NONE,
    "A-WEIGHTING": PCBADBMFilter.PCBADBM_FILTER_A_WEIGHTING,
    "C-WEIGHTING": PCBADBMFilter.PCBADBM_FILTER_C_WEIGHTING,
    "RESERVED": PCBADBMFilter.PCBADBM_FILTER_RESERVED,
}

PCBADBMComponent = pcbadbm_ns.class_(
    "PCBADBMComponent", cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PCBADBMComponent),
            cv.Optional(CONF_FILTER, default="A-WEIGHTING"): cv.enum(
                FILTER_OPTIONS, upper=True
            ),
        }
    )
    .extend(cv.polling_component_schema("125ms"))
    .extend(i2c.i2c_device_schema(0x48))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_filter(config[CONF_FILTER]))