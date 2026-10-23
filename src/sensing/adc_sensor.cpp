#include "adc_sensor.h"

#include "config.h"

// Background task periodically samples both channels with a moving-average filter
// so consumers always read an already-fresh value instead of blocking on multiple analogReads.
namespace {
constexpr uint8_t FILTER_WINDOW = 16;
constexpr TickType_t SAMPLE_PERIOD_TICKS = pdMS_TO_TICKS(2);  // 500 Hz sampling

// Incremental moving-average filter over one ADC pin; readRaw() picks counts vs. calibrated mV.
class FilteredAdcChannel {
 public:
  void begin(uint8_t pin, bool useMillivolts) {
    pin_ = pin;
    useMillivolts_ = useMillivolts;
    for (uint8_t i = 0; i < FILTER_WINDOW; i++) {
      uint16_t raw = readRaw();
      history_[i] = raw;
      sum_ += raw;
    }
  }

  void sample() {
    uint16_t raw = readRaw();
    sum_ += raw - history_[index_];
    history_[index_] = raw;
    index_ = (index_ + 1) % FILTER_WINDOW;
  }

  float average() const {
    return (float)(sum_ / FILTER_WINDOW);
  }

 private:
  uint16_t readRaw() const {
    return useMillivolts_ ? (uint16_t)analogReadMilliVolts(pin_) : analogRead(pin_);
  }

  uint8_t pin_ = 0;
  bool useMillivolts_ = false;
  uint16_t history_[FILTER_WINDOW] = {0};
  uint32_t sum_ = 0;
  uint8_t index_ = 0;
};

portMUX_TYPE adcMux = portMUX_INITIALIZER_UNLOCKED;

volatile float userSetPointDuty = 0.0f;
volatile float measuredVoltage = 0.0f;

FilteredAdcChannel setPointChannel;
FilteredAdcChannel voltageChannel;

void adcSamplingTask(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  while (true) {
    setPointChannel.sample();
    voltageChannel.sample();

    float normalizedDuty = constrain(setPointChannel.average() / 4095.0f, 0.0f, 1.0f);
    float voltage = voltageChannel.average() / 1000.0f;

    portENTER_CRITICAL(&adcMux);
    userSetPointDuty = normalizedDuty;
    measuredVoltage = voltage;
    portEXIT_CRITICAL(&adcMux);

    vTaskDelayUntil(&lastWakeTime, SAMPLE_PERIOD_TICKS);
  }
}
}  // namespace

void initADCSensors() {
  // Configure ADC pins as inputs
  pinMode(PIN_USER_SET_POINT, INPUT);
  pinMode(PIN_VOLTAGE_SENSE_INPUT, INPUT);

  // Set 11dB attenuation globally or per-pin to allow full 0-3.3V range measurement
  analogSetPinAttenuation(PIN_USER_SET_POINT, ADC_11db);
  analogSetPinAttenuation(PIN_VOLTAGE_SENSE_INPUT, ADC_11db);

  // Prime the filter histories with real samples so the first readings aren't biased toward 0
  setPointChannel.begin(PIN_USER_SET_POINT, false);
  voltageChannel.begin(PIN_VOLTAGE_SENSE_INPUT, true);
  userSetPointDuty = constrain(setPointChannel.average() / 4095.0f, 0.0f, 1.0f);
  measuredVoltage = voltageChannel.average() / 1000.0f;

  // Pin to Core 0 alongside the heartbeat task, keeping Core 1's real-time loop free of ADC I/O
  xTaskCreatePinnedToCore(
      adcSamplingTask,
      "ADCSamplingTask",
      2048,
      NULL,
      1,
      NULL,
      0);
}

/**
 * Returns the latest filtered, normalized duty cycle (0.0 to 1.0).
 * Non-blocking: reads a value kept fresh by the background sampling task.
 */
float getUserSetPointDuty() {
  portENTER_CRITICAL(&adcMux);
  float value = userSetPointDuty;
  portEXIT_CRITICAL(&adcMux);
  return value;
}

/**
 * Returns the latest filtered measured voltage in volts.
 * Non-blocking: reads a value kept fresh by the background sampling task.
 */
float getMeasuredVoltage() {
  portENTER_CRITICAL(&adcMux);
  float value = measuredVoltage;
  portEXIT_CRITICAL(&adcMux);
  return value;
}