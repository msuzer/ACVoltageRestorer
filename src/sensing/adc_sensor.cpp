#include "adc_sensor.h"
#include "config.h"

void initADCSensors() {
    // Configure ADC pins as inputs
    pinMode(PIN_USER_SET_POINT, INPUT);
    pinMode(PIN_VOLTAGE_SENSE_INPUT, INPUT);

    // Set 11dB attenuation globally or per-pin to allow full 0-3.3V range measurement
    analogSetPinAttenuation(PIN_USER_SET_POINT, ADC_11db);
    analogSetPinAttenuation(PIN_VOLTAGE_SENSE_INPUT, ADC_11db);
}

/**
 * Reads the user setpoint trimpot with a rolling average filter,
 * returning a normalized duty cycle float between 0.0 and 1.0.
 */
float getUserSetPointDuty() {
    uint32_t raw_sum = 0;
    const uint8_t samples = 16;

    for (uint8_t i = 0; i < samples; i++) {
        raw_sum += analogRead(PIN_USER_SET_POINT); // Built-in Arduino ESP32 read
    }
    uint32_t raw_avg = raw_sum / samples;

    // Convert raw ADC (0-4095 for 12-bit) to normalized float (0.0 to 1.0)
    float normalizedDuty = (float)raw_avg / 4095.0f;

    // Safety clamp
    if (normalizedDuty < 0.0f) normalizedDuty = 0.0f;
    if (normalizedDuty > 1.0f) normalizedDuty = 1.0f;

    return normalizedDuty;
}

/**
 * Placeholder for your future external DC-proportional voltage sensor (0-3.3V).
 * Uses analogReadMilliVolts for built-in factory-calibrated millivolt outputs.
 */
float getMeasuredVoltage() {
    uint32_t millivolts = analogReadMilliVolts(PIN_VOLTAGE_SENSE_INPUT);
    
    // Return as volts 
    return (float)millivolts / 1000.0f; 
}