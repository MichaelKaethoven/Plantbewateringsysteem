#include "logic.h"

// Geeft het droogste van twee vochtigheidsniveaus terug
MoistureLevel selectMoistureLevel(MoistureLevel a, MoistureLevel b) {
  if (a == DRY || b == DRY) {
    return DRY;
  }
  if (a == MOIST || b == MOIST) {
    return MOIST;
  }
  return WET;
}

// Zet de ruwe ADC-waarde van de capacitieve sensor om naar een
// vochtigheidsniveau
MoistureLevel getCapacitiveMoistureLevel(int value) {
  if (value >= CAPACITIVE_DRY_MIN && value <= CAPACITIVE_DRY_MAX) {
    return DRY;
  }
  if (value >= CAPACITIVE_MOIST_MIN && value <= CAPACITIVE_MOIST_MAX) {
    return MOIST;
  }
  return WET;
}

// Zet de ruwe ADC-waarde van de resistieve sensor om naar een
// vochtigheidsniveau
MoistureLevel getResistiveMoistureLevel(int value) {
  if (value >= RESISTIVE_DRY_MIN && value <= RESISTIVE_DRY_MAX) {
    return DRY;
  }
  if (value >= RESISTIVE_MOIST_MIN && value <= RESISTIVE_MOIST_MAX) {
    return MOIST;
  }
  return WET;
}

int getPumpDurationMs(MoistureLevel moistureLevel, float tempC) {
  if (moistureLevel != DRY) {
    return 0;
  }
  if (tempC > TEMP_PUMP_THRESHOLD_C) {
    return PUMP_ON_DURATION_MS;
  }
  if (tempC > TEMP_COLD_CUTOFF_C) {
    return PUMP_ON_DURATION_SHORT_MS;
  }
  return 0; // Te koud: geen water geven
}

const char *moistureLevelToString(MoistureLevel level) {
  switch (level) {
  case WET:
    return "WET";
  case MOIST:
    return "MOIST";
  case DRY:
    return "DRY";
  }
  return "UNKNOWN";
}
