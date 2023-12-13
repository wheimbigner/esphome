import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_PRESSURE,
    CONF_TEMPERATURE,
    DEVICE_CLASS_SOUND_PRESSURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_DECIBEL,
    CONF_FILTER,
)
from . import CONF_PCBADBM_ID, PCBADBMComponent

DEPENDENCIES = ["i2c"]

pcbadbm_ns = cg.esphome_ns.namespace("pcbadbm")

PCBADBMComponent = pcbadbm_ns.class_(
    "PCBADBMComponent", cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_PCBADBM_ID): cv.use_id(PCBADBMComponent),
            cv.Optional("decibels"): sensor.sensor_schema(
                unit_of_measurement=UNIT_DECIBEL,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_SOUND_PRESSURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("decibels_max"): sensor.sensor_schema(
                unit_of_measurement=UNIT_DECIBEL,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_SOUND_PRESSURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("decibels_min"): sensor.sensor_schema(
                unit_of_measurement=UNIT_DECIBEL,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_SOUND_PRESSURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
)


async def to_code(config):
#    var = cg.new_Pvariable(config[CONF_ID])
    pcbadbm_component = await cg.get_variable(config[CONF_PCBADBM_ID])
#    await cg.register_component(var, config)
#    await i2c.register_i2c_device(var, config)

    if decibels_config := config.get("decibels"):
        sens = await sensor.new_sensor(decibels_config)
        cg.add(pcbadbm_component.set_decibels_sensor(sens))
    if decibels_max_config := config.get("decibels_max"):
        sens = await sensor.new_sensor(decibels_max_config)
        cg.add(pcbadbm_component.set_decibels_max_sensor(sens))
    if decibels_min_config := config.get("decibels_min"):
        sens = await sensor.new_sensor(decibels_min_config)
        cg.add(pcbadbm_component.set_decibels_min_sensor(sens))
