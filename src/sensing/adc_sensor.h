#pragma once
#include <Arduino.h>

void initADCSensors();
float getUserSetPointDuty(); // Returns normalized target between 0.0 and 1.0
float getMeasuredVoltage();  // Placeholder for future external DC-proportional sensor