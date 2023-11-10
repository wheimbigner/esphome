#include "vesync.h"
#include "esphome/core/log.h"


/*
Commands to handle:
  purifier_uart_query_device_status: 0x4061
         purifier_uart_set_wifi_led: 0xA129
      purifier_uart_set_power_state: 0xA000 // DONE
             purifier_uart_set_mode: 0xA5E0 - for sleep mode on/off
            purifier_uart_set_level: 0xA260 // DONE
    purifier_uart_set_display_state: 0xA105 - for turning display on/off
     purifier_uart_set_display_mode: 0xA5E1 // Not sure what this does - it might be for the "auto-off-at-night" on some purifier models, but not the 200S
            purifier_uart_set_timer: 0xA264
            purifier_uart_get_timer: 0xA265
     purifier_uart_set_filter_state: 0xA5E2
        purifier_uart_set_childlock: 0xD100
       purifier_uart_set_nightlight: 0xA003
         purifier_uart_reset_filter: 0xA5E4
           purifier_uart_reboot_mcu: 0xD101
purifier_uart_set_production_status: 0xD004
purifier_uart_set_production_result: 0xD005
         purifier_uart_test_command: 0xD007

ACKs to handle:
purifier_device_status_ack_and_report: 0x4061
purifier_uart_ack_test?????:           0xA265 c.f. DAT_40148864
purifier_uart_set_timer_ack:           0xA264
purifier_uart_set_level_ack:           0xA260

Functionality to do:
Our component should query device status on initialization
Need to incorporate persistent tracking of the filter's lifespan

Additional components/types needed:
Timer (since MCU I believe bakes that inside when pressing the timer button on the purifier)
Light/nightlight status
Child-Lock status
Enable-display status

*/
namespace esphome {
  namespace vesync {

  static const char *TAG = "vesync";

  void vesync::set_vesyncPowerSwitch(switch_::Switch *vesyncPowerSwitch) {
    vesyncPowerSwitch_ = vesyncPowerSwitch;
  }

  void vesync::set_vesyncFanSpeed(number::Number *vesyncFanSpeed) {
    vesyncFanSpeed_ = vesyncFanSpeed;
  }

  void vesync::set_vesyncFanTimer(number::Number *vesyncFanTimer) {
    vesyncFanTimer_ = vesyncFanTimer;
  }

  vesync::vesync() {}
  vesync::vesync(const std::string &name) {
    // The `name` is needed to set timers up, hence non-default constructor
    // replaces `set_name()` method previously existed
    this->name_ = name;
//    this->timer_.push_back({this->name_ + "_purifier", false, 0, 0, std::bind(&vesync::purifier_timer_callback_, this)});
//    this->timer_.push_back({this->name_ + "_filter_quality", false, 0, 0, std::bind(&vesync::filter_quality_timer_callback, this)});
  }

  void vesync::setup() {
  }

  void vesync::loop() {
    const uint32_t now = millis();
    if (now - this->last_transmission_ >= 200) {
      // last transmission too long ago. Reset RX index.
      this->data_index_ = 0;
    }
    this->last_transmission_ = now;
    while (this->available() != 0) {
      this->read_byte(&this->data_[this->data_index_]);
      auto check = this->check_byte_();
      if (!check.has_value()) {
        // finished
        this->parse_data_();
        this->data_index_ = 0;
        this->last_update_ = now;
      } else if (!*check) {
        // wrong data
        this->data_index_ = 0;
        ESP_LOGE(TAG, "wrong data, resetting index");
      } else {
        // next byte
        this->data_index_++;
      }
    }
  }

  optional<bool> vesync::check_byte_() {
    uint8_t index = this->data_index_;
    uint8_t byte = this->data_[index];

    if (index == 0)
      return (byte == 0xA5);

    if (index == 1)
      return ((byte == 0x12) || (byte == 0x22));
  // I think 0x12 = message from MCU, 0x22 = message TO MCU, but MCU seems to send 0x12 sometimes
    if (index == 2)
      return true;

    uint8_t payload_length = uint8_t(this->data_[3]);
  //this->get_16_bit_uint_(3);
  //payload length might be uint16 not 8, but a payload > 32 bytes never happens
    if (index == 3) {
      if (payload_length < 4) {
        ESP_LOGW(TAG, "vesync payload length too short! %d", payload_length);
        return false;
      }
      return true;
    }
    if (index == 4) {
      return (byte == 0x00);
    }

    uint8_t total_size = 5 + payload_length;

    if (index < total_size)
      return true;

    return {};
  }

  void vesync::parse_data_() {
    uint8_t index = this->data_index_;
    uint8_t trivial_version;
    uint8_t minor_version;
    uint8_t major_version;
    uint8_t power_state;
    WorkMode work_mode;
    uint8_t manual_level;
    uint8_t display_state;
    uint8_t display_config;
    uint8_t display_mode;
    uint8_t filter_state;
    uint8_t childlock;
    uint8_t nightlight;
    if (this->data_[6] != 0x01) {
      ESP_LOGE(TAG, "rejecting unhandled 6th byte style (%#02x not 0x01 - possibly unknown MCU variant!)", this->data_[6]);
      return;
    }
    if ((this->data_[0] == 0xA5) && (this->data_[1] == 0x22)) { // receiving a command
      if ((this->data_[7] == 0x60) && (this->data_[8] == 0x40)) {
        // idk what data_9 is for; it seems to always be 0
        // todo, add logging here if we see data_[9] not be 0
        trivial_version = this->data_[10];
        minor_version = this->data_[11];
        major_version = this->data_[12];
        power_state = this->data_[13];
        switch (this->data_[14]) {
          case 0: work_mode = WorkMode::MANUAL; break;
          case 1: work_mode = WorkMode::SLEEP; break;
          case 2: work_mode = WorkMode::AUTO; break;
          case 3: work_mode = WorkMode::POLLEN; break;
          case 255: work_mode = WorkMode::UNSET; break;
          default: work_mode = WorkMode::UNKNOWN; break;
        }
        manual_level = this->data_[15];
        display_state = this->data_[16];
        display_config = this->data_[17];
        display_mode = this->data_[18];
        filter_state = this->data_[19];
        childlock = this->data_[20];
        nightlight = this->data_[21];
        ESP_LOGE(TAG, "Got 0x60 0x40 device status update:\n"
                      "mcu_ver: V%d.%d.%02d\n"
                      "power_state: %d\n"
                      "work_mode: %d\n"
                      "manual_level: %d\n"
                      "display_state: %d\n"
                      "display_config: %d\n"
                      "display_mode: %d\n"
                      "filter_state: %d\n"
                      "child_lock: %d\n"
                      "nightlight: %d",
                      major_version, minor_version, trivial_version, power_state,
                      static_cast<uint8_t>(work_mode), manual_level, display_state,
                      display_config, display_mode, filter_state, childlock, nightlight);
        if (this->vesyncPowerSwitch_ != nullptr)
          this->vesyncPowerSwitch_->publish_state(power_state);
        if (this->vesyncFanSpeed_ != nullptr)
          this->vesyncFanSpeed_->publish_state(manual_level);
        // TBD: If the device is being turned off, do we cancel the timer?
        //      ESP_LOGE(TAG, "fan_level: %d", fan_level)
        // original firmware prints this out but it's a separate function that references stack depth, that I need to decode still
      }
    } else if ((this->data_[0] == 0xA5) && (this->data_[1] == 0x12)) { // receiving command acknowledgement
      if ((this->data_[7] == 0x61) && (this->data_[8] == 0x40)) { // purifier_device_status_ack_and_report
      } else if ((this->data_[7] == 0x60) && (this->data_[8] == 0xA2)) { // purifier_uart_set_level_ack
      } else if ((this->data_[7] == 0x64) && (this->data_[8] == 0xA2)) { // purifier_uart_set_timer_ack
      } else if ((this->data_[7] == 0x65) && (this->data_[8] == 0xA2)) { // purifier_uart_get_timer_ack_and_report
        // bytes 10-13 are the number of seconds remaining on the timer
        // bytes 14-17 are the number of seconds the timer was set for
      }

    } 
    else {
      ESP_LOGE(TAG, "magic command check failed!");
    }

  }

  void vesync::dump_config() {
    LOG_SWITCH("", "Power Switch", this->vesyncPowerSwitch_);
    LOG_NUMBER("", "Fan Speed", this->vesyncFanSpeed_)
  }

  void vesync::send_command_(std::vector<uint8_t> data) {
    // Command format: 0xA5, 0x22, command, length, checksum, ...data...
    std::vector<uint8_t> command;
    command.push_back(0xA5); // Start of command
    command.push_back(0x22); // Command type or ID
    command.push_back(0x0C); // unknown purpose, possibly nonce/sequential command#
    command.push_back(0x00); // Placeholder for length of following data, will be set later
    command.push_back(0x00); // Unknown purpose, but appears to always be 0
    command.push_back(0x00); // Placeholder for checksum, will be set later
    command.push_back(0x01); // Unknown purporse, appears to always be 1

    // Append actual data to the command
    command.insert(command.end(), data.begin(), data.end());

    // Set the length byte to the size of data following it (not including checksum placeholder)
    command[3] = command.size() - 6;

    // Calculate checksum (excluding the checksum byte itself)
    uint8_t checksum = 0x00;
    for (size_t i = 0; i < command.size(); i++) {
      checksum += command[i];
    }
    checksum = ~checksum;

    // Set the checksum in the command
    command[5] = checksum;

    // Send all bytes
    for (uint8_t byte : command) {
      this->parent_->write_byte(byte);
    }
  }

  void vesyncPowerSwitch::write_state(bool state) {
    std::vector<uint8_t> data = {
      0x00, 0xA0, 0x00,
      static_cast<uint8_t>(state ? 0x01 : 0x00) // State byte: 0x01 for ON, 0x00 for OFF
    };

    this->parent_->send_command_(data);
  // strictly speaking we shouldn't publish state until we get a response from the device
    this->publish_state(state);
  }

  void vesyncFanSpeed::control(float state) {
    int rounded_state = static_cast<int>(std::round(state));
    if (rounded_state < 0) rounded_state = 0;
    if (rounded_state > 4) rounded_state = 4;
      // Since the step is 1, state should always be an integer value.

    std::vector<uint8_t> data = {
      0x60, 0xA2, 0x00, 0x00, 0x01,
      static_cast<uint8_t>(rounded_state)
    };

    this->parent_->send_command_(data);

      // Finally, call the parent class's method to actually set the value
    this->publish_state(rounded_state);
  }

void vesync::start_timer_(const VesyncTimerIndex timer_index) {
  if (this->timer_duration_(timer_index) > 0) {
    this->set_timeout(this->timer_[timer_index].name, this->timer_duration_(timer_index),
                      this->timer_cbf_(timer_index));
    this->timer_[timer_index].start_time = millis();
    this->timer_[timer_index].active = true;
  }
  ESP_LOGVV(TAG, "Timer %u started for %u sec", static_cast<size_t>(timer_index),
            this->timer_duration_(timer_index) / 1000);
}

bool vesync::cancel_timer_(const VesyncTimerIndex timer_index) {
  this->timer_[timer_index].active = false;
  return this->cancel_timeout(this->timer_[timer_index].name);
}

bool vesync::timer_active_(const VesyncTimerIndex timer_index) { return this->timer_[timer_index].active; }

void vesync::set_timer_duration_(const VesyncTimerIndex timer_index, const uint32_t time) {
  this->timer_[timer_index].time = 1000 * time;
}

uint32_t vesync::timer_duration_(const VesyncTimerIndex timer_index) { return this->timer_[timer_index].time; }

std::function<void()> vesync::timer_cbf_(const VesyncTimerIndex timer_index) {
  return this->timer_[timer_index].func;
}

void vesync::fan_timer_callback_() {
  this->timer_[vesync::TIMER_FAN].active = false;
  ESP_LOGVV(TAG, "Fan timer expired");
  // I don't think we need to necessarily need to do much here?
}

void vesync::filter_timer_callback_() {
  this->timer_[vesync::TIMER_FILTER].active = false;
  ESP_LOGVV(TAG, "Filter timer expired");
}

void vesyncSelectThing::setup() {
  if (this->f_.has_value())
    return;

  std::string value;
  ESP_LOGD(TAG, "Setting up Template Select");
  value = this->at(0).value();
  this->publish_state(value);
}

  void vesyncSelectThing::control(std::string &value) {
    // "wifi0", "wifi1", "wifi2", "wifi3", 
    //        "mode0", "mode1", "mode2", "mode3", "settimer0", "settimer60", "settimermax", "setfilterstate0", "setfilterstate1"] 
    // wifi modes:
    // slow_blink: 29:A1:00:02:F4:01:F4:01:00
    // fast_blink: 29:A1:00:02:7D:00:7D:00:00
    // stop_blink: 29:A1:00:01:7D:00:7D:00:00
    // Command structure: 0x29, 0xA1, 0x00, (param_1 is 0, 1, or 2), [word]param_2, [word] param_3, [char] param_4
    // successful wifi selfcheck callback = 1 0 0 0
    // failed wifi selfcheck callback = 0 0 0 0
    // param_1 color, param_2 on time, param_3 off time, param_4 unknown?

    if (value == "wifi0") {
      std::vector<uint8_t> data = { 0x29, 0xA1, 0x00, 0x02, 0xF4, 0x01, 0xF4, 0x01, 0x00 };
      this->parent_->send_command_(data);
    }
    if (value == "wifi1") {
      std::vector<uint8_t> data = { 0x29, 0xA1, 0x00, 0x02, 0x7D, 0x00, 0x7D, 0x00, 0x00 };
      this->parent_->send_command_(data);
    }
    if (value == "wifi2") {
      std::vector<uint8_t> data = { 0x29, 0xA1, 0x00, 0x02, 0xF4, 0x01, 0xF4, 0x01, 0x00 };
      this->parent_->send_command_(data);
    }
    if (value == "wifi3") {
      std::vector<uint8_t> data = { 0x29, 0xA1, 0x00, 0x01, 0xDC, 0x05, 0x7D, 0x00, 0x00 };
      this->parent_->send_command_(data);
    }
    // Finally, call the parent class's method to actually set the value
    this->publish_state(rounded_state);
  }


}  // namespace vesync
}  // namespace esphome
