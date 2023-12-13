#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace pcbadbm {

enum PCBADBMFilter {
  PCBADBM_FILTER_NONE = 0b00,
  PCBADBM_FILTER_A_WEIGHTING = 0b01,
  PCBADBM_FILTER_C_WEIGHTING = 0b10,
  PCBADBM_FILTER_RESERVED = 0b11,
};

enum PCBADBMInterruptType {
  PCBADBM_INTERRUPT_TYPE_HISTORY = 0b00,
  PCBADBM_INTERRUPT_TYPE_MAX_MIN = 0b01,
};

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


/// This class implements support for the PCB Artists dB meter i2c sensor.
class PCBADBMComponent : public PollingComponent, public i2c::I2CDevice {
#ifdef USE_BUTTON
  SUB_BUTTON(reset_history)
  SUB_BUTTON(reset_interrupt)
  SUB_BUTTON(reset_minmax)
  SUB_BUTTON(reset_system)
#endif
#ifdef USE_SENSOR
  SUB_SENSOR(decibels)
  SUB_SENSOR(decibels_max)
  SUB_SENSOR(decibels_min)
#endif
 public:
  void set_filter(PCBADBMFilter filter) { filter_ = filter; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;

 protected:
  /// Read the temperature value and store the calculated ambient temperature in t_fine.
  uint8_t read_decibels_();
  uint8_t read_min_decibels_();
  uint8_t read_max_decibels_();
/*  void set_filter_(PCBADBMFilter filter);
  void set_interrupt_enable_(bool enable);
  void set_interrupt_type_(PCBADBMInterruptType type); */

  PCBADBMFilter filter_{PCBADBM_FILTER_A_WEIGHTING};
  bool interrupt_enable_{false};
  PCBADBMInterruptType interrupt_type_{PCBADBM_INTERRUPT_TYPE_HISTORY};
  enum ErrorCode {
    NONE = 0,
    COMMUNICATION_FAILED,
  } error_code_{NONE};
};

}  // namespace pcbadbm
}  // namespace esphome
