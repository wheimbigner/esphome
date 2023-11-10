import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import uart, binary_sensor, button, output, sensor, switch, text_sensor, number, select
from esphome.const import CONF_ID, CONF_STATE, DEVICE_CLASS_VOLTAGE, ICON_FLASH, UNIT_VOLT

MULTI_CONF = False
DEPENDENCIES = ['uart']
AUTO_LOAD = ['binary_sensor', 'button', 'output', 'sensor', 'switch', 'text_sensor', 'number', 'select']

vesync_ns = cg.esphome_ns.namespace('vesync')

vesync = vesync_ns.class_('vesync', cg.Component, uart.UARTDevice)
vesyncPowerSwitch = vesync_ns.class_('vesyncPowerSwitch', switch.Switch, cg.Component)
vesyncFanSpeed = vesync_ns.class_('vesyncFanSpeed', number.Number, cg.Component)
vesyncFanTimer = vesync_ns.class_('vesyncFanTimer', number.Number, cg.Component)
vesyncSelectThing = vesync_ns.class_('vesyncSelectThing', select.Select, cg.Component)

CONF_POWER_SWITCH = "power_switch"
CONF_FAN_SPEED = "fan_speed"
CONF_FAN_TIMER = "fan_timer"
CONF_SELECTTHING = "selectthing"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(vesync),
    cv.Required(CONF_POWER_SWITCH): switch.SWITCH_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncPowerSwitch)}),
    cv.Required(CONF_FAN_SPEED): number.NUMBER_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncFanSpeed)}),
    cv.Required(CONF_FAN_TIMER): number.NUMBER_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncFanTimer)}),
    cv.Required(CONF_SELECTTHING): select.SELECT_SCHEMA.extend({cv.GenerateID(): cv.declare_id(vesyncSelectThing)})
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
        sw = await number.new_number(config[CONF_FAN_SPEED], min_value = 0.0, max_value = 4.0, step = 1.0)
        cg.add(var.set_vesyncFanSpeed(sw))
        cg.add(sw.set_parent(var))

    if CONF_FAN_TIMER in config:
        sw = await number.new_number(config[CONF_FAN_TIMER], min_value = 0.0, max_value = 4294967295.0, step = 1.0)
        cg.add(var.set_vesyncFanTimer(sw))
        cg.add(sw.set_parent(var))
    
    if CONF_SELECTTHING in config:
        sw = await select.new_select(config[CONF_SELECTTHING], options = ["NONE", "wifi0", "wifi1", "wifi2", "wifi3", 
            "mode0", "mode1", "mode2", "mode3", "settimer0", "settimer60", "settimermax", "setfilterstate0", "setfilterstate1"])
        cg.add(var.set_vesyncSelectThing(sw))
        cg.add(sw.set_parent(var))