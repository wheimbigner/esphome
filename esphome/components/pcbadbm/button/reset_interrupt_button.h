#pragma once

#include "esphome/components/button/button.h"
#include "../pcbadbm.h"

namespace esphome {
namespace pcbadbm {

class ResetInterruptButton : public button::Button, public Parented<PCBADBMComponent> {
 public:
  ResetInterruptButton() = default;

 protected:
  void press_action() override;
};

}  // namespace pcbadbm
}  // namespace esphome
