#pragma once

#include "esphome/components/button/button.h"
#include "../pcbadbm.h"

namespace esphome {
namespace pcbadbm {

class ResetMinMaxButton : public button::Button, public Parented<PCBADBMComponent> {
 public:
  ResetMinMaxButton() = default;

 protected:
  void press_action() override;
};

}  // namespace pcbadbm
}  // namespace esphome
