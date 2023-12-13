#include "reset_history_button.h"
#include "../pcbadbm.h"

namespace esphome {
namespace pcbadbm {

void ResetHistoryButton::press_action() { this->parent_->write_byte(PCBADBM_REGISTER_RESET, 0b00000100); }

}  // namespace pcbadbm
}  // namespace esphome
