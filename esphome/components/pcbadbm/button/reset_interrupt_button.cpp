#include "reset_interrupt_button.h"

namespace esphome {
namespace pcbadbm {

void ResetInterruptButton::press_action() { this->parent_->write_byte(PCBADBM_REGISTER_RESET, 0b00000001); }

}  // namespace pcbadbm
}  // namespace esphome
