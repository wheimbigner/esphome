#include "reset_minmax_button.h"

namespace esphome {
namespace pcbadbm {

void ResetMinMaxButton::press_action() { this->parent_->reset_minmax(); }

}  // namespace pcbadbm
}  // namespace esphome
