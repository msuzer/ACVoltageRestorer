#include "zcd_sense.h"

#include "config.h"

// State tracking variables
static GridState currentGridState = GRID_ZERO_CROSSING_DEADBAND;
static uint32_t lastStateChangeMicros = 0;

// 8.5 ms lockout window (50Hz half-cycle = 10ms total. 8.5ms lockout ignores mid-cycle spikes)
const uint32_t LOCKOUT_DURATION_US = 8500;

// A candidate new state must repeat this many consecutive polls before being accepted,
// so a single noise glitch right as the lockout reopens can't latch a wrong state for 8.5ms.
const uint8_t DEBOUNCE_CONFIRM_SAMPLES = 2;
static GridState pendingState = GRID_ZERO_CROSSING_DEADBAND;
static uint8_t pendingCount = 0;

void initZCD() {
  // Configure ZCD pins as inputs (External pull-ups expected from main control board)
  pinMode(PIN_ZCD_POS, INPUT);
  pinMode(PIN_ZCD_NEG, INPUT);

  currentGridState = GRID_ZERO_CROSSING_DEADBAND;
  lastStateChangeMicros = micros();
  pendingState = GRID_ZERO_CROSSING_DEADBAND;
  pendingCount = 0;
}

/**
 * Reads ZCD optical pins and applies the 8.5ms minimum half-cycle noise lockout.
 * - PIN_ZCD_POS goes LOW during Positive Half-Cycle (v_i > +V_th)
 * - PIN_ZCD_NEG goes LOW during Negative Half-Cycle (v_i < -V_th)
 * - Both HIGH means we are in the Zero-Crossing Deadband window near 0V.
 * Lockout/debounce only guard resuming active PWM; entering the safe deadband is immediate.
 */
GridState getGridState() {
  bool posActive = (digitalRead(PIN_ZCD_POS) == LOW);
  bool negActive = (digitalRead(PIN_ZCD_NEG) == LOW);
  uint32_t now = micros();

  // Evaluate incoming grid state from the raw pin reading
  GridState rawState = GRID_ZERO_CROSSING_DEADBAND;

  if (posActive && !negActive) {
    rawState = GRID_POSITIVE_HALF_CYCLE;
  } else if (negActive && !posActive) {
    rawState = GRID_NEGATIVE_HALF_CYCLE;
  } else {
    // Both high or conflicting -> Safe Zero-Crossing Deadband
    rawState = GRID_ZERO_CROSSING_DEADBAND;
  }

  // Entering the safe freewheeling state disables active PWM, so it's accepted immediately --
  // unlike resuming active switching, reacting fast here can't create a switching hazard.
  if (rawState == GRID_ZERO_CROSSING_DEADBAND && currentGridState != GRID_ZERO_CROSSING_DEADBAND) {
    currentGridState = GRID_ZERO_CROSSING_DEADBAND;
    lastStateChangeMicros = now;
    pendingCount = 0;
    return currentGridState;
  }

  // If a lockout period is active since the last state change, maintain the current state
  // to immunize the converter against grid notches and appliance noise spikes.
  if ((now - lastStateChangeMicros) < LOCKOUT_DURATION_US) {
    return currentGridState;
  }

  if (rawState == currentGridState) {
    // Reading agrees with where we already are; nothing pending to confirm.
    pendingCount = 0;
    return currentGridState;
  }

  // Reading disagrees with current state: require it to repeat before committing.
  if (rawState == pendingState) {
    pendingCount++;
  } else {
    pendingState = rawState;
    pendingCount = 1;
  }

  if (pendingCount >= DEBOUNCE_CONFIRM_SAMPLES) {
    currentGridState = rawState;
    lastStateChangeMicros = now;
    pendingCount = 0;
  }

  return currentGridState;
}
