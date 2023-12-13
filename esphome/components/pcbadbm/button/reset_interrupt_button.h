#pragma once

#include "esphome/components/button/button.h"
#include "../pcbadbm.h"

namespace esphome {
namespace pcbadbm {

class ResetSystemButton : public button::Button, public Parented<PCBADBMComponent> {
 public:
  ResetSystemButton() = default;

 protected:
  void press_action() override;
};

}  // namespace pcbadbm
}  // namespace esphome
