#include "reset_system_button.h"

namespace esphome {
namespace pcbadbm {

void ResetSystemButton::press_action() { this->parent_->reset_system(); }

}  // namespace pcbadbm
}  // namespace esphome
