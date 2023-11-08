#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/number/number.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace vesync {

enum class WorkMode : uint8_t {
  MANUAL = 0,
  SLEEP = 1,
  AUTO = 2,
  POLLEN = 3,
  UNSET = 255,
  UNKNOWN = 254 // Assuming 254 as 'unknown' placeholder
};

class vesync : public Component,  public uart::UARTDevice {
  public:
    float get_setup_priority() const override { return setup_priority::DATA; }
    void setup() override;
    void loop() override;
    void dump_config() override;
    void send_command_(std::vector<uint8_t> data);
    void set_vesyncPowerSwitch(switch_::Switch *vesyncPowerSwitch);
    void set_vesyncFanSpeed(number::Number *vesyncFanSpeed);

  protected:
    optional<bool> check_byte_();
    void parse_data_();
    uint32_t last_update_{0};
    uint8_t data_[64];
    uint8_t data_index_{0};
    uint32_t last_transmission_{0};

    std::vector<uint8_t> rx_message_;
    switch_::Switch *vesyncPowerSwitch_{nullptr};
    number::Number *vesyncFanSpeed_{nullptr};
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
  protected:
    void control(float state) override;
    vesync *parent_;
};
}  // namespace vesync
}  // namespace esphome
