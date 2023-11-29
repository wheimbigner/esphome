import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import uart, binary_sensor, button, output, sensor, switch, text_sensor, number, select, light
from esphome.const import CONF_ID, CONF_STATE, DEVICE_CLASS_VOLTAGE, ICON_FLASH, UNIT_VOLT

MULTI_CONF = False
DEPENDENCIES = ['uart']
AUTO_LOAD = ['binary_sensor', 'button', 'output', 'sensor', 'switch', 'text_sensor', 'number', 'select', 'light']

vesync_ns = cg.esphome_ns.namespace('vesync')

vesync = vesync_ns.class_('vesync', cg.Component, uart.UARTDevice)
vesyncPowerSwitch = vesync_ns.class_('vesyncPowerSwitch', switch.Switch, cg.Component)
vesyncFanSpeed = vesync_ns.class_('vesyncFanSpeed', number.Number, cg.Component)
vesyncFanTimer = vesync_ns.class_('vesyncFanTimer', number.Number, cg.Component)
vesyncMode = vesync_ns.class_('vesyncMode', select.Select, cg.Component)
vesyncSelectThing = vesync_ns.class_('vesyncSelectThing', select.Select, cg.Component)
vesyncFilterSwitch = vesync_ns.class_('vesyncFilterSwitch', switch.Switch, cg.Component)
vesyncChildlock = vesync_ns.class_('vesyncChildlock', switch.Switch, cg.Component)
vesyncNightlight = vesync_ns.class_('vesyncNightlight', light.LightOutput, cg.Component)

CONF_POWER_SWITCH = "power_switch"
CONF_FAN_SPEED = "fan_speed"
CONF_FAN_TIMER = "fan_timer"
CONF_MODE = "mode"
CONF_SELECTTHING = "selectthing"
CONF_MCU_VERSION = "mcu_version"
CONF_FILTER_SWITCH = "filter_switch"
CONF_CHILDLOCK = "childlock"
CONF_NIGHTLIGHT = "nightlight"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(vesync),
    cv.Required(CONF_POWER_SWITCH): switch.SWITCH_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncPowerSwitch)}),
    cv.Required(CONF_FAN_SPEED): number.NUMBER_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncFanSpeed)}),
    cv.Required(CONF_FAN_TIMER): number.NUMBER_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncFanTimer)}),
    cv.Required(CONF_MODE): select.SELECT_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncMode)}),
    cv.Required(CONF_SELECTTHING): select.SELECT_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncSelectThing)}),
    cv.Required(CONF_MCU_VERSION): text_sensor.text_sensor_schema(text_sensor.TextSensor),
    cv.Required(CONF_FILTER_SWITCH): switch.SWITCH_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncFilterSwitch)}),
    cv.Required(CONF_CHILDLOCK): switch.SWITCH_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncChildlock)}),
    cv.Optional(CONF_NIGHTLIGHT): light.BRIGHTNESS_ONLY_LIGHT_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncNightlight)}),
}).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_POWER_SWITCH in config:
        sw = await switch.new_switch(config[CONF_POWER_SWITCH])
        cg.add(var.set_vesyncPowerSwitch(sw))
        cg.add(sw.set_parent(var))

    if CONF_FAN_SPEED in config:
        sw = await number.new_number(config[CONF_FAN_SPEED], min_value = 0.0, max_value = 3.0, step = 1.0)
        cg.add(var.set_vesyncFanSpeed(sw))
        cg.add(sw.set_parent(var))

    if CONF_FAN_TIMER in config:
        sw = await number.new_number(config[CONF_FAN_TIMER], min_value = 0.0, max_value = 4294967295.0, step = 1.0)
        cg.add(var.set_vesyncFanTimer(sw))
        cg.add(sw.set_parent(var))
    
    if CONF_MODE in config:
        # Note: Other models also have ..., "Auto", "Pollen"
        sw = await select.new_select(config[CONF_MODE], options = ["Manual", "Sleep"])
        cg.add(var.set_vesyncMode(sw))
        cg.add(sw.set_parent(var))
    
    if CONF_FILTER_SWITCH in config:
        sw = await switch.new_switch(config[CONF_FILTER_SWITCH])
        cg.add(var.set_vesyncFilterSwitch(sw))
        cg.add(sw.set_parent(var))

    if CONF_CHILDLOCK in config:
        sw = await switch.new_switch(config[CONF_CHILDLOCK])
        cg.add(var.set_vesyncChildlock(sw))
        cg.add(sw.set_parent(var))

    if CONF_NIGHTLIGHT in config:
        sw = await light.new_light(config[CONF_NIGHTLIGHT])
        cg.add(var.set_vesyncNightlight(sw))
        cg.add(sw.set_parent(var))

    if CONF_MCU_VERSION in config:
        sw = await text_sensor.new_text_sensor(config[CONF_MCU_VERSION])
        cg.add(var.set_vesyncMcuVersion(sw))

    if CONF_SELECTTHING in config:
        sw = await select.new_select(config[CONF_SELECTTHING], options = ["NONE", "wifi0", "wifi1", "wifi2", "wifi3", 
            "mode0", "mode1", "mode2", "mode3", "settimer0", "settimer60", "settimermax", "setfilterstate0", "setfilterstate1"])
        cg.add(var.set_vesyncSelectThing(sw))
        cg.add(sw.set_parent(var))