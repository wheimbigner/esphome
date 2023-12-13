#include "pcbadbm.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pcbadbm {

static const char *const TAG = "pcbadbm.sensor";

static const uint8_t PCBADBM_REGISTER_VERSION = 0x00; // Bit 7:4=HW version, 3:0=SW version
static const uint8_t PCBADBM_REGISTER_ID3 = 0x01; // 4 bit device uid
static const uint8_t PCBADBM_REGISTER_ID2 = 0x02;
static const uint8_t PCBADBM_REGISTER_ID1 = 0x03;
static const uint8_t PCBADBM_REGISTER_ID0 = 0x04;
static const uint8_t PCBADBM_REGISTER_SCRATCH = 0x05; // r/w, default value 0xAA
static const uint8_t PCBADBM_REGISTER_CONTROL = 0x06;
// bit 7:5 = reserved. Always use 0.
// bit 4 = interrupt type (1 = max/min; 0 = history 90% full)
// bit 3 = enable interrupt operation
// bit 2:1 = filter: none/A-weighting/C-weighting/reserved
// bit 0 = power down. Set bit 3 of reset to trigger reset.
static const uint8_t PCBADBM_REGISTER_TAVG_HIGH = 0x07; // default 0x03
static const uint8_t PCBADBM_REGISTER_TAVG_LOW = 0x08; // default 0xE8
// 1,000ms for slow intensity measurement, 125ms for fast mode
// TAVG_HIGH must be written first, as writing low activates changes
static const uint8_t PCBADBM_REGISTER_RESET = 0x09; // write only
// bit 7:4 = reserved
// bit 3 = system reset. restore settings to defaults.
// bit 2 = clear history
// bit 1 = clear max/min
// bit 0 = clear interrupt
static const uint8_t PCBADBM_REGISTER_DECIBEL = 0x0A; // read only
// Latest sound intensity value in decibels, averaged over the last Tavg time period.
// The decibel reading is only valid after about 1 second of module power-up.
static const uint8_t PCBADBM_REGISTER_MIN_DECIBEL = 0x0B; // read only
// Minimum value of decibel reading captured since power-up or manual reset of MIN/MAX registers.
static const uint8_t PCBADBM_REGISTER_MAX_DECIBEL = 0x0C; // read only
// Maximum value of decibel reading captured since power-up or manual reset of MIN/MAX registers.
static const uint8_t PCBADBM_REGISTER_THR_MIN = 0x0D; // read-write. Default 45
// Minimum decibel value (threshold) under which interrupt should be asserted.
static const uint8_t PCBADBM_REGISTER_THR_MAX = 0x0E; // read-write. Default 85
// Maximum decibel value (threshold) above which interrupt should be asserted.
static const uint8_t PCBADBM_REGISTER_DBHISTORY_0 = 0x14; // read-only, continues on through 0x77 for 0..99
// History of decibel readings, 0 = newest, 99 = oldest.

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
  uint16_t Tavg = 0x03E8; // 1000ms; to-do: make configurable. shouldn't need to write here regardless, it's default.
  if (!this->write_byte(PCBADBM_REGISTER_TAVG_HIGH, (Tavg >> 8) & 0xFF)) {
    this->mark_failed();
    return;
  }
  if (!this->write_byte(PCBADBM_REGISTER_TAVG_LOW, Tavg & 0xFF)) {
    this->mark_failed();
    return;
  }
}

void PCBADBMComponent::reset_system_() {
  this->write_byte(PCBADBM_REGISTER_RESET, 0b00001000);
}

void PCBADBMComponent::reset_history_() {
  this->write_byte(PCBADBM_REGISTER_RESET, 0b00000100);
}

void PCBADBMComponent::reset_max_min_() {
  this->write_byte(PCBADBM_REGISTER_RESET, 0b00000010);
}

void PCBADBMComponent::reset_interrupt_() {
  this->write_byte(PCBADBM_REGISTER_RESET, 0b00000001);
}

void PCBADBMComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "PCBADBM:");
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
