#pragma once
#include <Arduino.h>

void initPWMDriver();
void setConverterDuty(float dutyCycle, bool positiveHalfCycle);
void emergencyStopPWM();