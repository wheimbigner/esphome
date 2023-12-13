#pragma once

#include "esphome/components/button/button.h"
#include "../pcbadbm.h"

namespace esphome {
namespace pcbadbm {

class ResetHistoryButton : public button::Button, public Parented<PCBADBMComponent> {
 public:
  ResetHistoryButton() = default;

 protected:
  void press_action() override;
};

}  // namespace pcbadbm
}  // namespace esphome
