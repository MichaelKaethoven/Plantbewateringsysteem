#pragma once

#include "config.h"

MoistureLevel selectMoistureLevel(MoistureLevel a, MoistureLevel b);
MoistureLevel getCapacitiveMoistureLevel(int value);
MoistureLevel getResistiveMoistureLevel(int value);
const char *moistureLevelToString(MoistureLevel level);

// Geeft de pompduur in ms terug op basis van vochtigheid en temperatuur.
// Geeft 0 terug als de grond niet droog is of als het te koud is.
int getPumpDurationMs(MoistureLevel moistureLevel, float tempC);
