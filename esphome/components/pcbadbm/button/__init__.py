import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_RESTART,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ENTITY_CATEGORY_CONFIG,
    ICON_RESTART,
    ICON_RESTART_ALERT,
)
from .. import CONF_PCBADBM_ID, PCBADBMComponent, pcbadbm_ns

ResetSystemButton = pcbadbm_ns.class_("ResetSystemButton", button.Button)
ResetHistoryButton = pcbadbm_ns.class_("ResetHistoryButton", button.Button)
ResetMinMaxButton = pcbadbm_ns.class_("ResetMinMaxButton", button.Button)
ResetInterruptButton = pcbadbm_ns.class_("ResetInterruptButton", button.Button)

CONF_RESET_SYSTEM = "reset_system"
CONF_RESET_HISTORY = "reset_history"
CONF_RESET_MINMAX = "reset_minmax"
CONF_RESET_INTERRUPT = "reset_interrupt"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_PCBADBM_ID): cv.use_id(PCBADBMComponent),
    cv.Optional(CONF_RESET_SYSTEM): button.button_schema(
        ResetSystemButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_RESTART_ALERT,
    ),
    cv.Optional(CONF_RESET_HISTORY): button.button_schema(
        ResetHistoryButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon=ICON_RESTART,
    ),
    cv.Optional(CONF_RESET_MINMAX): button.button_schema(
        ResetMinMaxButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon=ICON_RESTART,
    ),
    cv.Optional(CONF_RESET_INTERRUPT): button.button_schema(
        ResetInterruptButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon=ICON_RESTART,
    ),
}


async def to_code(config):
    pcbadbm_component = await cg.get_variable(config[CONF_PCBADBM_ID])
    if system_reset_config := config.get(CONF_RESET_SYSTEM):
        b = await button.new_button(system_reset_config)
        await cg.register_parented(b, config[CONF_PCBADBM_ID])
        cg.add(pcbadbm_component.set_reset_system_button(b))
    if history_reset_config := config.get(CONF_RESET_HISTORY):
        b = await button.new_button(history_reset_config)
        await cg.register_parented(b, config[CONF_PCBADBM_ID])
        cg.add(pcbadbm_component.set_reset_history_button(b))
    if minmax_reset_config := config.get(CONF_RESET_MINMAX):
        b = await button.new_button(minmax_reset_config)
        await cg.register_parented(b, config[CONF_PCBADBM_ID])
        cg.add(pcbadbm_component.set_reset_minmax_button(b))
    if interrupt_reset_config := config.get(CONF_RESET_INTERRUPT):
        b = await button.new_button(interrupt_reset_config)
        await cg.register_parented(b, config[CONF_PCBADBM_ID])
        cg.add(pcbadbm_component.set_reset_interrupt_button(b))
