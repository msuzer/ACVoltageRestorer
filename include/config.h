#pragma once

// --- GENERAL INCLUDES ---
#include <Arduino.h>
#include <stdint.h>

// --- SYSTEM CONFIG ---
constexpr uint32_t SWITCHING_FREQ_HZ       = 20000;     // 20 kHz
constexpr uint16_t DEAD_TIME_NS            = 800;       // 800 ns hardware dead-time

// --- PIN MAPPINGS ---
constexpr uint8_t PIN_USER_LED            = 12;

constexpr uint8_t PIN_I2C_SDA             = 21;
constexpr uint8_t PIN_I2C_SCL             = 22;

constexpr uint8_t PIN_PWM_S1_S3_HI        = 19;        // To M26LS31 / TLP350 Board 1
constexpr uint8_t PIN_PWM_S1_S3_LO        = 18;
constexpr uint8_t PIN_PWM_S2_S4_HI        = 17;        // To M26LS31 / TLP350 Board 2
constexpr uint8_t PIN_PWM_S2_S4_LO        = 16;

constexpr uint8_t PIN_ZCD_POS             = 39;        // From PhaseMon Opto 1 (Positive Half-Cycle)
constexpr uint8_t PIN_ZCD_NEG             = 36;        // From PhaseMon Opto 2 (Negative Half-Cycle)

constexpr uint8_t PIN_EMERGENCY_SHUTDOWN_B1     = 25;        // CNY17 Shutdown output to Driver Board1
constexpr uint8_t PIN_EMERGENCY_SHUTDOWN_B2     = 26;        // CNY17 Shutdown output to Driver Board2
constexpr uint8_t PIN_EMERGENCY_SHUTDOWN_B3     = 27;        // CNY17 Shutdown output to Driver Board3 // not used
constexpr uint8_t PIN_EMERGENCY_SHUTDOWN_B4     = 14;        // CNY17 Shutdown output to Driver Board4 // not used

constexpr uint8_t PIN_SAFETY_FAULT  = 23;        // Wired-AND +15V Interlock Chain Input

constexpr uint8_t PIN_USER_SET_POINT      = 34;        // User set point input, ADC1_CH6
constexpr uint8_t PIN_VOLTAGE_SENSE_INPUT = 32;        // Voltage sense input, ADC1_CH4
