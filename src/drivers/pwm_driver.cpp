#include "pwm_driver.h"
#include "config.h"
#include "driver/mcpwm.h"

// Helper to convert nanoseconds to MCPWM clock ticks (ESP32 APB clock is 160MHz, scaled by timer prescaler)
// Prescaler set such that timer resolution is 1us or 0.1us. Let's calculate ticks for dead-time.
// With timer resolution, 1 tick = 12.5ns (at 80MHz timer clock). 800ns / 12.5ns = 64 ticks.
#define NS_TO_TICKS(ns) ((ns * 80) / 1000) 

void initPWMDriver() {
    // ----------------------------------------------------
    // 1. Configure Unit 0 for Board 1 (Pins 19 & 18: S1 / S3)
    // ----------------------------------------------------
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PIN_PWM_S1_S3_HI);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PIN_PWM_S1_S3_LO);

    mcpwm_config_t pwm_config_b1;
    pwm_config_b1.frequency = SWITCHING_FREQ_HZ; // 20 kHz
    pwm_config_b1.cmpr_a = 0.0;                  // Initial duty 0%
    pwm_config_b1.cmpr_b = 0.0;
    pwm_config_b1.counter_mode = MCPWM_UP_COUNTER;
    pwm_config_b1.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config_b1);

    // Enable hardware dead-time for Board 1 (Complementary output with 800ns delay)
    uint32_t dead_time_ticks = NS_TO_TICKS(DEAD_TIME_NS);
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, dead_time_ticks, dead_time_ticks);


    // ----------------------------------------------------
    // 2. Configure Unit 1 for Board 2 (Pins 17 & 16: S2 / S4)
    // ----------------------------------------------------
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, PIN_PWM_S2_S4_HI);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0B, PIN_PWM_S2_S4_LO);

    mcpwm_config_t pwm_config_b2;
    pwm_config_b2.frequency = SWITCHING_FREQ_HZ; // 20 kHz
    pwm_config_b2.cmpr_a = 0.0;
    pwm_config_b2.cmpr_b = 0.0;
    pwm_config_b2.counter_mode = MCPWM_UP_COUNTER;
    pwm_config_b2.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config_b2);

    // Enable hardware dead-time for Board 2
    mcpwm_deadtime_enable(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, dead_time_ticks, dead_time_ticks);

    // Start with all outputs safely zeroed out
    emergencyStopPWM();
}

/**
 * Implements Hagemeyer Voltage-Polarity Commutation:
 * - When v_i > 0 (Positive Half-Cycle): 
 *     Board 2 ($S_2/S_4$) is held static ON (100%), while Board 1 ($S_1/S_3$) handles PWM.
 * - When v_i < 0 (Negative Half-Cycle): 
 *     Board 1 ($S_1/S_3$) is held static ON (100%), while Board 2 ($S_2/S_4$) handles PWM.
 */
void setConverterDuty(float dutyCycle, bool positiveHalfCycle) {
    // If positive: S3/S4 = ON, S1/S2 = PWM
    // If negative: S1/S2 = ON, S3/S4 = PWM

    // Clamp duty cycle between 0.0 and 1.0
    if (dutyCycle < 0.0f) dutyCycle = 0.0f;
    if (dutyCycle > 1.0f) dutyCycle = 1.0f;

    if (positiveHalfCycle) {
        // --- POSITIVE HALF-CYCLE ---
        // Board 2 (Shunt pair S2/S4) stays static ON to provide freewheeling/bypass paths
        mcpwm_set_signal_high(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_A); 
        mcpwm_set_signal_low(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_B);

        // Board 1 (Series pair S1/S3) actively switches at the requested duty cycle
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, dutyCycle * 100.0f);
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    } 
    else {
        // --- NEGATIVE HALF-CYCLE ---
        // Board 1 (Series pair S1/S3) stays static ON
        mcpwm_set_signal_high(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);

        // Board 2 (Shunt pair S2/S4) actively switches at the requested duty cycle
        mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_A, dutyCycle * 100.0f);
        mcpwm_set_duty_type(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    }
}

// Immediately forces all PWM output channels LOW to protect the power stage during faults
void emergencyStopPWM() {
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    mcpwm_set_signal_low(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_A);
    mcpwm_set_signal_low(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_B);
}