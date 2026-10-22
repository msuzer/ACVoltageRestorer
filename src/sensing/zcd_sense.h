#pragma once
#include <Arduino.h>

enum GridState {
    GRID_ZERO_CROSSING_DEADBAND = 0,
    GRID_POSITIVE_HALF_CYCLE,
    GRID_NEGATIVE_HALF_CYCLE
};

void initZCD();
GridState getGridState();