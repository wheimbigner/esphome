#include "reset_minmax_button.h"

namespace esphome {
namespace pcbadbm {

void ResetMinMaxButton::press_action() { this->parent_->write_byte(PCBADBM_REGISTER_RESET, 0b00000010); }

}  // namespace pcbadbm
}  // namespace esphome
