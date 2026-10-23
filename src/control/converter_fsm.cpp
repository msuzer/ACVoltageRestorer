#include "converter_fsm.h"

#include "drivers/pwm_driver.h"
#include "sensing/adc_sensor.h"
#include "sensing/zcd_sense.h"

// Applied duty cycle only moves when the setpoint changes by more than the deadband
static float appliedDutyCycle = -1.0f;
const float DUTY_CHANGE_THRESHOLD = 0.01f;  // Ignore setpoint changes smaller than 1%

void initConverterFSM() {
  initADCSensors();
}

void updateConverterFSM() {
  GridState gridState = getGridState();

  // For testing phase, we can set a fixed target duty cycle (e.g., 0.5 for 55V -> 27.5V test)
  // static float targetDutyCycle = 0.5f;
  // Read live user setpoint trimpot (0.0 to 1.0)
  float rawDutyCycle = getUserSetPointDuty();

  // Only accept the new setpoint (and log it) if it moved meaningfully
  if (abs(rawDutyCycle - appliedDutyCycle) >= DUTY_CHANGE_THRESHOLD) {
    appliedDutyCycle = rawDutyCycle;
    Serial.printf("[Trimpot] New Target Duty: %.1f%% (Value: %.3f)\n",
                  appliedDutyCycle * 100.0f, appliedDutyCycle);
  }
  float targetDutyCycle = appliedDutyCycle;

  switch (gridState) {
    case GRID_ZERO_CROSSING_DEADBAND:
      // Safe Freewheeling State near 0V S1/S3 OFF, S2/S4 ON
      // Turn off switching legs to allow inductor current to circulate safely.
      setConverterDuty(0.0f, true);
      break;

    case GRID_POSITIVE_HALF_CYCLE:
      // v_i > 0: Board 2 static ON, Board 1 PWMs at target duty cycle
      // S3/S4 ON, S1/S2 PWM
      setConverterDuty(targetDutyCycle, true);
      break;

    case GRID_NEGATIVE_HALF_CYCLE:
      // v_i < 0: Board 1 static ON, Board 2 PWMs at target duty cycle
      // S1/S2 ON, S3/S4 PWM
      setConverterDuty(targetDutyCycle, false);
      break;
  }
}