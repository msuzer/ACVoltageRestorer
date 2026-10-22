#include "safety/safety_interlock.h"
#include "config.h"
#include "drivers/pwm_driver.h"

void initSafetyInterlock() {
    // Configure wired-AND fault input (expects external pull-up on main control board)
    pinMode(PIN_SAFETY_FAULT, INPUT_PULLUP);

    // Configure individual emergency shutdown outputs to driver boards
    pinMode(PIN_EMERGENCY_SHUTDOWN_B1, OUTPUT);
    pinMode(PIN_EMERGENCY_SHUTDOWN_B2, OUTPUT);
    pinMode(PIN_EMERGENCY_SHUTDOWN_B3, OUTPUT);
    pinMode(PIN_EMERGENCY_SHUTDOWN_B4, OUTPUT);

    // Default state: Active LOW (CNY17 enabled, no emergency shutdown asserted)
    digitalWrite(PIN_EMERGENCY_SHUTDOWN_B1, LOW);
    digitalWrite(PIN_EMERGENCY_SHUTDOWN_B2, LOW);
    digitalWrite(PIN_EMERGENCY_SHUTDOWN_B3, LOW);
    digitalWrite(PIN_EMERGENCY_SHUTDOWN_B4, LOW);
}

/**
 * Checks the wired-AND +15V safety chain.
 * PIN_SAFETY_FAULT going HIGH means the chain is broken (+15V lost on a board).
 */
bool checkSystemHealth() {
    bool faultDetected = (digitalRead(PIN_SAFETY_FAULT) == HIGH);

    if (faultDetected) {
        // 1. Immediately kill all PWM outputs via MCPWM hardware
        emergencyStopPWM();

        // 2. Assert hardware shutdown lines across all driver board CNY17s
        digitalWrite(PIN_EMERGENCY_SHUTDOWN_B1, HIGH);
        digitalWrite(PIN_EMERGENCY_SHUTDOWN_B2, HIGH);
        digitalWrite(PIN_EMERGENCY_SHUTDOWN_B3, HIGH);
        digitalWrite(PIN_EMERGENCY_SHUTDOWN_B4, HIGH);

        return false; // System is unhealthy
    }

    return true; // System is healthy
}