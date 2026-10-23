#pragma once
#include <Arduino.h>

// Starts the background sampling task; must be called once before the other getters are used.
void initADCSensors();
float getUserSetPointDuty();  // Latest filtered, normalized target (0.0 to 1.0), non-blocking
float getMeasuredVoltage();   // Latest filtered measured voltage in volts, non-blocking