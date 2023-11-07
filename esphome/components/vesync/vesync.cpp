#include "vesync.h"
#include "esphome/core/log.h"

namespace esphome {
  namespace vesync {

  static const char *TAG = "vesync";

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
    return byte == 0xA5;

  if (index == 1)
    return (byte == 0x12) || (byte == 0x22);
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
    return byte == 0x00;
  }

  // start (16bit) + length (16bit) + DATA (payload_length-2 bytes) + checksum (16bit)
  uint8_t total_size = 6 + payload_length;

  if (index < total_size)
    return true;

  return {};
}

void vesync::parse_data_() {
  // todo: publish states here for any data we get from MCU

  /* todo
      uint16_t pm_1_0_std_concentration = this->get_16_bit_uint_(4);
      uint16_t pm_2_5_std_concentration = this->get_16_bit_uint_(6);
      uint16_t pm_10_0_std_concentration = this->get_16_bit_uint_(8);

      ESP_LOGD(TAG,
               "Got PM1.0 Concentration: %u µg/m^3, PM2.5 Concentration %u µg/m^3, PM10.0 Concentration: %u µg/m^3",
               pm_1_0_concentration, pm_2_5_concentration, pm_10_0_concentration);

      if (this->pm_1_0_std_sensor_ != nullptr)
        this->pm_1_0_std_sensor_->publish_state(pm_1_0_std_concentration);
      if (this->pm_2_5_std_sensor_ != nullptr)
        this->pm_2_5_std_sensor_->publish_state(pm_2_5_std_concentration);
      if (this->pm_10_0_std_sensor_ != nullptr)
        this->pm_10_0_std_sensor_->publish_state(pm_10_0_std_concentration);
 */
}


/* void vesync::handle_char_(uint8_t c) {
  if (c == '\r')
    return;
  if (c == '\n') {
    std::string s(this->rx_message_.begin(), this->rx_message_.end());
    if (this->the_text_ != nullptr)
      this->the_text_->publish_state(s);
    if (this->the_sensor_ != nullptr)
      this->the_sensor_->publish_state(parse_number<float>(s).value_or(0));
    if (this->the_binsensor_ != nullptr)
      this->the_binsensor_->publish_state(s == "ON");
    this->rx_message_.clear();
    return;
  }
  this->rx_message_.push_back(c);
} */

void vesync::dump_config() {
//  LOG_TEXT_SENSOR("", "The Text Sensor", this->the_text_);
//  LOG_SENSOR("", "The Sensor", this->vesyncPowerSwitch_);
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
    0x00, // The rest of the command's bytes
    0xA0,
    0x00,
    state ? 0x01 : 0x00 // State byte: 0x01 for ON, 0x00 for OFF
  };

  this->parent_->send_command_(data);
// strictly speaking we shouldn't publish state until we get a response from the device
  this->publish_state(state);
}

void vesyncFanSpeed::setup() {
  auto traits = this->get_traits();
  traits.set_min_value(0);
  traits.set_max_value(4);
  traits.set_step(1.0); // Set step size to 1 to enforce integer increments
  this->set_traits(traits);
}

void vesyncFanSpeed::write_state(float state) override {
  int rounded_state = static_cast<int>(std::round(state));
  if (rounded_state < 0) rounded_state = 0;
  if (rounded_state > 4) rounded_state = 4;
    // Since the step is 1, state should always be an integer value.

  std::vector<uint8_t> data = {
    0x60,
    0xA2,
    0x00,
    0x00,
    0x01,
    static_cast<uint8_t>(rounded_state)
  };

  this->parent_->send_command_(data);

    // Finally, call the parent class's method to actually set the value
    this->publish_state(rounded_state);
  }
/*
void vesync::write_binary(bool state) {
  this->write_str(ONOFF(state));
}

void UARTDemo::ping() {
  this->write_str("PING");
}

void UARTDemo::write_float(float state) {
  this->write_str(to_string(state).c_str());
}

void UARTDemoBOutput::dump_config() {
  LOG_BINARY_OUTPUT(this);
}

void UARTDemoBOutput::write_state(bool state) {
  this->parent_->write_binary(state);
}

void UARTDemoFOutput::dump_config() {
  LOG_FLOAT_OUTPUT(this);
}

void UARTDemoFOutput::write_state(float state) {
  this->parent_->write_float(state);
}

void UARTDemoSwitch::dump_config() {
  LOG_SWITCH("", "UART Demo Switch", this);
}

void UARTDemoSwitch::write_state(bool state) {
  this->parent_->write_binary(state);
  this->publish_state(state);
}

void UARTDemoButton::dump_config() {
  LOG_BUTTON("", "UART Demo Button", this);
}

void UARTDemoButton::press_action() {
  this->parent_->ping();
}
*/

}  // namespace uart_demo
}  // namespace esphome
