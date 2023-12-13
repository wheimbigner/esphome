#include "reset_history_button.h"

namespace esphome {
namespace pcbadbm {

void ResetHistoryButton::press_action() { this->parent_->reset_history(); }

}  // namespace pcbadbm
}  // namespace esphome
