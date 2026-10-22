#include "adc_sensor.h"
#include "config.h"
#include "esp_adc_cal.h"

// ADC Characterization structure for calibrated millivolt readings
static esp_adc_cal_characteristics_t adc_chars;

void initADCSensors() {
    // Configure ADC pins as inputs
    pinMode(PIN_USER_SET_POINT, INPUT);
    pinMode(PIN_VOLTAGE_SENSE_INPUT, INPUT);

    // Configure ADC width and attenuation for 0-3.3V range
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // Pin 34 (User Setpoint)
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11); // Pin 32 (Voltage Sense)

    // Characterize ADC using factory calibration values stored in ESP32 eFuse
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
}

/**
 * Reads the user setpoint trimpot with a rolling average filter to eliminate noise,
 * returning a normalized duty cycle float between 0.0 and 1.0.
 */
float getUserSetPointDuty() {
    uint32_t raw_sum = 0;
    const uint8_t samples = 16;

    for (uint8_t i = 0; i < samples; i++) {
        raw_sum += adc1_get_raw(ADC1_CHANNEL_6);
        // tiny delay or yield if needed, but ADC read is fast
    }
    uint32_t raw_avg = raw_sum / samples;

    // Convert raw ADC (0-4095) to normalized float (0.0 to 1.0)
    float normalizedDuty = (float)raw_avg / 4095.0f;

    // Safety clamp
    if (normalizedDuty < 0.0f) normalizedDuty = 0.0f;
    if (normalizedDuty > 1.0f) normalizedDuty = 1.0f;

    return normalizedDuty;
}

/**
 * Placeholder for your future external DC-proportional voltage sensor (0-3.3V).
 * Converts raw ADC counts into calibrated millivolts.
 */
float getMeasuredVoltage() {
    uint32_t raw_adc = adc1_get_raw(ADC1_CHANNEL_4);
    uint32_t millivolts = esp_adc_cal_raw_to_voltage(raw_adc, &adc_chars);
    
    // Return as volts (e.g., scale up based on your sensor's transfer function later)
    return (float)millivolts / 1000.0f; 
}