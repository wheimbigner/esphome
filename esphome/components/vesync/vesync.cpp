#include "vesync.h"
#include "esphome/core/log.h"

namespace esphome {
  namespace vesync {

  static const char *TAG = "vesync";

  void vesync::set_vesyncPowerSwitch(switch_::Switch *vesyncPowerSwitch) {
    vesyncPowerSwitch_ = vesyncPowerSwitch;
  }

  void vesync::set_vesyncFanSpeed(number::Number *vesyncFanSpeed) {
    vesyncFanSpeed_ = vesyncFanSpeed;
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
    if (this->data_[0] != 0xA5) {
      return;
    }
    if (this->data_[1] != 0x22) {
      ESP_LOGE(TAG, "rejecting unhandled 2nd byte style (%#02x not 0x22)", this->data_[1]);
      return;
    }
    if (this->data_[6] != 0x01) {
      ESP_LOGE(TAG, "rejecting unhandled 6th byte style (%#02x not 0x01 - possibly unknown MCU variant!)", this->data_[6]);
      return;
    }
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
//      ESP_LOGE(TAG, "fan_level: %d", fan_level)
// original firmware prints this out but it's a separate function that references stack depth, that I need to decode still
    } else {
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


}  // namespace vesync
}  // namespace esphome
