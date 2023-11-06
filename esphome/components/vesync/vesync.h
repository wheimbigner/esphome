#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
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
  void send_command_onoff(bool state);

/*  void set_the_text(text_sensor::TextSensor *text_sensor) { the_text_ = text_sensor; }
  void set_the_sensor(sensor::Sensor *sensor) { the_sensor_ = sensor; }
  void set_the_binsensor(binary_sensor::BinarySensor *sensor) { the_binsensor_ = sensor; } */

 protected:
//  switch::Switch *vesyncPowerSwitch_{nullptr};
/*  text_sensor::TextSensor *the_text_{nullptr};
  sensor::Sensor *the_sensor_{nullptr};
  binary_sensor::BinarySensor *the_binsensor_{nullptr}; */
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


/*
class UARTDemoBOutput : public Component, public output::BinaryOutput {
 public:
  void dump_config() override;
  void set_parent(UARTDemo *parent) { this->parent_ = parent; }
 protected:
  void write_state(bool state) override;
  UARTDemo *parent_;
};

class UARTDemoFOutput : public Component, public output::FloatOutput {
 public:
  void dump_config() override;
  void set_parent(UARTDemo *parent) { this->parent_ = parent; }
 protected:
  void write_state(float state) override;
  UARTDemo *parent_;
};

class UARTDemoSwitch : public Component, public switch_::Switch {
 public:
  void dump_config() override;
  void set_parent(UARTDemo *parent) { this->parent_ = parent; }
 protected:
  void write_state(bool state) override;
  UARTDemo *parent_;
};

class UARTDemoButton : public Component, public button::Button {
 public:
  void dump_config() override;
  void set_parent(UARTDemo *parent) { this->parent_ = parent; }
 protected:
  void press_action() override;
  UARTDemo *parent_;
};
*/
}  // namespace vesync
}  // namespace esphome
