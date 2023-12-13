#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
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

/// This class implements support for the PCB Artists dB meter i2c sensor.
class PCBADBMComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_decibels_sensor(sensor::Sensor *decibels_sensor) { decibels_sensor_ = decibels_sensor; }
  void set_decibels_max_sensor(sensor::Sensor *decibels_max_sensor) { decibels_max_sensor_ = decibels_max_sensor; }
  void set_decibels_min_sensor(sensor::Sensor *decibels_min_sensor) { decibels_min_sensor_ = decibels_min_sensor; }
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
  void reset_interrupt_();
  void reset_max_min_();
  void reset_system_();
  void reset_history_();

  PCBADBMFilter filter_{PCBADBM_FILTER_A_WEIGHTING};
  bool interrupt_enable_{false};
  PCBADBMInterruptType interrupt_type_{PCBADBM_INTERRUPT_TYPE_HISTORY};
  sensor::Sensor *decibels_sensor_{nullptr};
  sensor::Sensor *decibels_max_sensor_{nullptr};
  sensor::Sensor *decibels_min_sensor_{nullptr};

  enum ErrorCode {
    NONE = 0,
    COMMUNICATION_FAILED,
  } error_code_{NONE};
};

}  // namespace pcbadbm
}  // namespace esphome
