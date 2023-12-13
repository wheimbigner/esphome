#include "reset_system_button.h"

namespace esphome {
namespace pcbadbm {

void ResetSystemButton::press_action() { this->parent_->write_byte(PCBADBM_REGISTER_RESET, 0b00001000); }

}  // namespace pcbadbm
}  // namespace esphome
