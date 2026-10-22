#include "zcd_sense.h"
#include "config.h"

// State tracking variables
static GridState currentGridState = GRID_ZERO_CROSSING_DEADBAND;
static uint32_t lastStateChangeMicros = 0;

// 8.5 ms lockout window (50Hz half-cycle = 10ms total. 8.5ms lockout ignores mid-cycle spikes)
const uint32_t LOCKOUT_DURATION_US = 8500; 

void initZCD() {
    // Configure ZCD pins as inputs (External pull-ups expected from main control board)
    pinMode(PIN_ZCD_POS, INPUT);
    pinMode(PIN_ZCD_NEG, INPUT);
    
    currentGridState = GRID_ZERO_CROSSING_DEADBAND;
    lastStateChangeMicros = micros();
}

/**
 * Reads ZCD optical pins and applies the 8.5ms minimum half-cycle noise lockout.
 * - PIN_ZCD_POS goes LOW during Positive Half-Cycle (v_i > +V_th)
 * - PIN_ZCD_NEG goes LOW during Negative Half-Cycle (v_i < -V_th)
 * - Both HIGH means we are in the Zero-Crossing Deadband window near 0V.
 */
GridState getGridState() {
    bool posActive = (digitalRead(PIN_ZCD_POS) == LOW);
    bool negActive = (digitalRead(PIN_ZCD_NEG) == LOW);
    uint32_t now = micros();

    // If a lockout period is active since the last state change, maintain the current state
    // to immunize the converter against grid notches and appliance noise spikes.
    if ((now - lastStateChangeMicros) < LOCKOUT_DURATION_US) {
        return currentGridState;
    }

    // Evaluate incoming grid state after lockout expires
    GridState newState = GRID_ZERO_CROSSING_DEADBAND;

    if (posActive && !negActive) {
        newState = GRID_POSITIVE_HALF_CYCLE;
    } 
    else if (negActive && !posActive) {
        newState = GRID_NEGATIVE_HALF_CYCLE;
    } 
    else {
        // Both high or conflicting -> Safe Zero-Crossing Deadband
        newState = GRID_ZERO_CROSSING_DEADBAND;
    }

    // If state has transitioned, update the lockout timestamp timer
    if (newState != currentGridState) {
        currentGridState = newState;
        lastStateChangeMicros = now;
    }

    return currentGridState;
}