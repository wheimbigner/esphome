#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/number/number.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/select/select.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/light/light_traits.h"

namespace esphome {
namespace vesync {

  enum VesyncTimerIndex : uint8_t {
    TIMER_FAN = 0,
    TIMER_FILTER = 1,
  };

  enum class WorkMode : uint8_t {
    MANUAL = 0,
    SLEEP = 1,
    AUTO = 2,
    POLLEN = 3,
    UNSET = 255,
    UNKNOWN = 254 // Assuming 254 as 'unknown' placeholder
  };

  struct VesyncTimer {
    const std::string name;
    bool active;
    uint32_t time;
    uint32_t start_time;
    std::function<void()> func;
  };

  class vesync : public Component,  public uart::UARTDevice {
    public:
      float get_setup_priority() const override { return setup_priority::DATA; }
      void setup() override;
      void loop() override;
      void dump_config() override;
      void send_command_(std::vector<uint8_t> data);
      void set_vesyncPowerSwitch(switch_::Switch *vesyncPowerSwitch) { vesyncPowerSwitch_ = vesyncPowerSwitch; }
      void set_vesyncFanSpeed(number::Number *vesyncFanSpeed) { vesyncFanSpeed_ = vesyncFanSpeed; }
      void set_vesyncFanTimer(number::Number *vesyncFanTimer) { vesyncFanTimer_ = vesyncFanTimer; }
      void set_vesyncMode(select::Select *vesyncMode) { vesyncMode_ = vesyncMode; }
      void set_vesyncSelectThing(select::Select *vesyncSelectThing) { vesyncSelectThing_ = vesyncSelectThing; }
      void set_vesyncMcuVersion(text_sensor::TextSensor *vesyncMcuVersion) { vesyncMcuVersion_ = vesyncMcuVersion; }
      void set_vesyncFilterSwitch(switch_::Switch *vesyncFilterSwitch) { vesyncFilterSwitch_ = vesyncFilterSwitch; }
      void set_vesyncChildlock(switch_::Switch *vesyncChildlock) { vesyncChildlock_ = vesyncChildlock; }
      void set_vesyncNightlight(light::LightOutput *vesyncNightlight) { vesyncNightlight_ = vesyncNightlight; }

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
      number::Number *vesyncFanTimer_{nullptr};
      select::Select *vesyncMode_{nullptr};
      select::Select *vesyncSelectThing_{nullptr};
      text_sensor::TextSensor *vesyncMcuVersion_{nullptr};
      switch_::Switch *vesyncFilterSwitch_{nullptr};
      switch_::Switch *vesyncChildlock_{nullptr};
      light::LightOutput *vesyncNightlight_{nullptr};

      void start_timer_(VesyncTimerIndex timer_index);
      bool cancel_timer_(VesyncTimerIndex timer_index);
      /// returns true if the specified timer is active/running
      bool timer_active_(VesyncTimerIndex timer_index);
      /// time is converted to milliseconds (ms) for set_timeout()
      void set_timer_duration_(VesyncTimerIndex timer_index, uint32_t time);
      /// returns time in milliseconds (ms)
      uint32_t timer_duration_(VesyncTimerIndex timer_index);
      std::function<void()> timer_cbf_(VesyncTimerIndex timer_index);

      void fan_timer_callback_();
      void filter_timer_callback_();

      std::vector<VesyncTimer> timer_{};

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

  class vesyncFanTimer : public Component, public number::Number {
    public:
      void set_parent(vesync *parent) { this->parent_ = parent; }
    protected:
      void control(float state) override;
      vesync *parent_;
  };
  
  class vesyncMode : public select::Select, public Component {
    public:
      void set_parent(vesync *parent) { this->parent_ = parent; }
    protected:
      void control(const std::string &value) override;
      vesync *parent_;
  };

  class vesyncFilterSwitch : public Component, public switch_::Switch {
    public:
      void set_parent(vesync *parent) { this->parent_ = parent; }
    protected:
      void write_state(bool state) override;
      vesync *parent_;
  };

  class vesyncChildlock : public Component, public switch_::Switch {
    public:
      void set_parent(vesync *parent) { this->parent_ = parent; }
    protected:
      void write_state(bool state) override;
      vesync *parent_;
  };

  class vesyncNightlight : public light::LightOutput, public Component {
    public:
      void set_parent(vesync *parent) { this->parent_ = parent; }
    protected:
      void write_state(light::LightState *state) override;
      vesync *parent_;
  };

  class vesyncSelectThing : public select::Select, public Component {
 public:
//  void set_template(std::function<optional<std::string>()> &&f) { this->f_ = f; }
  void set_parent(vesync *parent) { this->parent_ = parent; }
  void setup() override;
//  void dump_config() override;
//  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  void control(const std::string &value) override;
//  optional<std::function<optional<std::string>()>> f_;
  vesync *parent_;
};
}  // namespace vesync
}  // namespace esphome
