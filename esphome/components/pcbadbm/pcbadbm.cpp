#include "pcbadbm.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pcbadbm {

static const char *const TAG = "pcbadbm.sensor";

inline uint16_t combine_bytes(uint8_t msb, uint8_t lsb) { return ((msb & 0xFF) << 8) | (lsb & 0xFF); }

static const char *filter_to_str(PCBADBMFilter filter) {
  switch (filter) {
    case PCBADBM_FILTER_NONE:
      return "None";
    case PCBADBM_FILTER_A_WEIGHTING:
      return "A-weighting";
    case PCBADBM_FILTER_C_WEIGHTING:
      return "C-weighting";
    case PCBADBM_FILTER_RESERVED:
      return "Reserved";
    default:
      return "UNKNOWN";
  }
}

void PCBADBMComponent::setup() {
  // Wait until the module has had a couple of seconds to power-up.
  delay(2);
  ESP_LOGCONFIG(TAG, "Setting up PCBA dBm...");
  uint8_t chip_version = 0;
  ESP_LOGCONFIG(TAG, "Reading chip version...");
  if (!this->read_byte(PCBADBM_REGISTER_VERSION, &chip_version)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  ESP_LOGCONFIG(TAG, "Chip version: %d", chip_version);
  ESP_LOGCONFIG(TAG, "Reading chip ID...");
  uint8_t chip_id3 = 0;
  if (!this->read_byte(PCBADBM_REGISTER_ID3, &chip_id3)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  uint8_t chip_id2 = 0;
  if (!this->read_byte(PCBADBM_REGISTER_ID2, &chip_id2)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  uint8_t chip_id1 = 0;
  if (!this->read_byte(PCBADBM_REGISTER_ID1, &chip_id1)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  uint8_t chip_id0 = 0;
  if (!this->read_byte(PCBADBM_REGISTER_ID0, &chip_id0)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  uint32_t chip_id = (((chip_id3 & 0xFF) << 24) | ((chip_id2 & 0xFF) << 16) | ((chip_id1 & 0xFF) << 8) | (chip_id0 & 0xFF));
  ESP_LOGCONFIG(TAG, "Chip ID: %08X", chip_id);

  ESP_LOGCONFIG(TAG, "Resetting system...");
  // Send a soft reset.
  if (!this->write_byte(PCBADBM_REGISTER_RESET, 0b00001111)) {
    this->mark_failed();
    return;
  }
  // Wait until the module has had a couple of seconds to power-up.
  delay(2);

  uint8_t config_register = 0b00000000;
  config_register |= (this->interrupt_type_ & 0b1) << 4;
  config_register |= (this->interrupt_enable_ & 0b1) << 3;
  config_register |= (this->filter_ & 0b11) << 1;

  ESP_LOGCONFIG(TAG, "Setting configuration register to 0x%02X...", config_register);
  if (!this->write_byte(PCBADBM_REGISTER_CONTROL, config_register)) {
    this->mark_failed();
    return;
  }

  ESP_LOGCONFIG(TAG, "Setting Tavg to 1000ms...");
  uint16_t Tavg = 1000; // or 1000ms; to-do: make configurable. shouldn't need to write here regardless, it's default.
  if (!this->write_byte(PCBADBM_REGISTER_TAVG_HIGH, (Tavg >> 8) & 0xFF)) {
    this->mark_failed();
    return;
  }
  if (!this->write_byte(PCBADBM_REGISTER_TAVG_LOW, Tavg & 0xFF)) {
    this->mark_failed();
    return;
  }
}

void PCBADBMComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "PCBADBM:");
  ESP_LOGCONFIG(TAG, "  Version 2");
  LOG_I2C_DEVICE(this);
  switch (this->error_code_) {
    case COMMUNICATION_FAILED:
      ESP_LOGE(TAG, "Communication with PCBADBM failed!");
      break;
    case NONE:
    default:
      break;
  }
  ESP_LOGCONFIG(TAG, "  Filter: %s", filter_to_str(this->filter_));
  LOG_UPDATE_INTERVAL(this);

  LOG_SENSOR("  ", "Decibels", this->decibels_sensor_);
  LOG_SENSOR("  ", "Decibels (Max)", this->decibels_max_sensor_);
  LOG_SENSOR("  ", "Decibels (Min)", this->decibels_min_sensor_);
}
float PCBADBMComponent::get_setup_priority() const { return setup_priority::DATA; }

void PCBADBMComponent::update() {
//  static int invocationCount = 0;

  // Increment the count each time the function is called
//  invocationCount++;

//  if (invocationCount % 8 == 0) { // change from 1 to 8 for slow mode
//    invocationCount = 0;
  // Enable sensor
    ESP_LOGV(TAG, "Sending conversion request...");

    uint32_t meas_time = 2;

    this->set_timeout("data", meas_time, [this]() {
      uint8_t dbm = this->read_decibels_();
      uint8_t dbm_min = this->read_min_decibels_();
      uint8_t dbm_max = this->read_max_decibels_();

      ESP_LOGD(TAG, "Got dBm=%3d min=%3d max=%3d", dbm, dbm_min, dbm_max);
      if (this->decibels_sensor_ != nullptr)
        this->decibels_sensor_->publish_state(dbm);
      if (this->decibels_max_sensor_ != nullptr)
        this->decibels_max_sensor_->publish_state(dbm_max);
      if (this->decibels_min_sensor_ != nullptr)
        this->decibels_min_sensor_->publish_state(dbm_min);
    });
//  }
}

uint8_t PCBADBMComponent::read_decibels_() {
  uint8_t data = 0;
  this->read_byte(PCBADBM_REGISTER_DECIBEL, &data);
  return data;
}

uint8_t PCBADBMComponent::read_min_decibels_() {
  uint8_t data = 0;
  this->read_byte(PCBADBM_REGISTER_MIN_DECIBEL, &data);
  return data;
}

uint8_t PCBADBMComponent::read_max_decibels_() {
  uint8_t data = 0;
  this->read_byte(PCBADBM_REGISTER_MAX_DECIBEL, &data);
  return data;
}

}  // namespace pcbadbm
}  // namespace esphome
