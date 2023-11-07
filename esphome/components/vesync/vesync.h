#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/number/number.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace vesync {

class vesync : public Component,  public uart::UARTDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;
  void loop() override;
  void dump_config() override;
  void send_command_(std::vector<uint8_t> data);

 protected:
  optional<bool> check_byte_();
  void parse_data_();
  uint32_t last_update_{0};
  uint8_t data_[64];
  uint8_t data_index_{0};
  uint32_t last_transmission_{0};

  std::vector<uint8_t> rx_message_;
};

class vesyncPowerSwitch : public Component, public switch_::Switch {
 public:
  void set_parent(vesync *parent) { this->parent_ = parent; }
  protected:
    void write_state(bool state) override;
  vesync *parent_;
};

class vesyncFanSpeed : public Component, public number::Number {
 public:
  void set_parent(vesync *parent) { this->parent_ = parent; }
  void setup() override;
  protected:
    void write_state(float state) override;
  vesync *parent_;
}
}  // namespace vesync
}  // namespace esphome
