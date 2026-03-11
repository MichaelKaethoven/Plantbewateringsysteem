/*
  BRONNEN:
  Canvas cursus - 16/02/2026 - https://canvas.kdg.be/courses/55542
  LM35 temperature sensor manual - 16/02/2026 -
  https://wiki.dfrobot.com/DFRobot_LM35_Linear_Temperature_Sensor__SKU_DFR0023_
  ADC pin attenuation - 16/02/2026 -
  https://deepbluembedded.com/esp32-adc-tutorial-read-analog-voltage-arduino/
  Capacitieve bodemvochtigheidssensor - 16/02/2026 -
  https://canvas.kdg.be/courses/55542/pages/bodemvochtigheidssensoren-capacitieve-bvh-sensor?module_item_id=1357791
  The static keyword - 16/02/2026 -
  https://ccrma.stanford.edu/~fgeorg/250a/lab2/arduino-0019/reference/Static.html#:~:text=The%20static%20keyword%20is%20used,their%20data%20between%20function%20calls.
  Using buildflags - 10/03/2026 -
  https://docs.platformio.org/en/stable/projectconf/sections/env/options/build/build_flags.html
  Using C preprocessors - 10/03/2026 -
  https://www.tutorialspoint.com/cprogramming/c_preprocessors.htm
*/

#include <Arduino.h>
// Stel het minimale logniveau in vóór de include, anders gebruikt SerialDebug
// de standaard (WARNING)
#define DEBUG_INITIAL_LEVEL DEBUG_LEVEL_VERBOSE
#define ARDUINOTRACE_ENABLE 0
#include <SerialDebug.h>
#ifdef MOCK_SENSORS
#include <ArduinoTrace.h>
#endif
#include <config.h>
#include <logic.h>
#include <functionDeclarations.h>

void setup() {
  Serial.begin(9600);
  debugI("Setup started");
  // Zet de relay-pin als uitgang zodat we de pomp kunnen aansturen
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  debugSetLevel(DEBUG_LEVEL_VERBOSE);
  // Zet de attenuatie van de temperatuursensor-pin op 0dB (bereik 0-1.1V)
  analogSetPinAttenuation(TEMP_SENSOR, ADC_0db);
}

void loop() {
  // Statics die hetzelfde blijven tijdens de loops, maar in loop-scope
  // aangezien de rest van het programma dit niet moet zien
  static bool pumpActive = false;
  static int pumpStartMs = 0;
  static int pumpDurationMs = 0;
  static int lastCheckMs = 0;

  int nowMs = (int)millis();

  // Lees de sensoren uit op een vast interval
  if (nowMs - lastCheckMs >= PUMP_CHECK_INTERVAL_MS) {
    lastCheckMs = nowMs;

    // Lees de ruwe ADC-waarden van beide sensoren uit
    int capacitiveValue = getCapacitiveSensorValue();
    int resistiveValue = getResistiveSensorValue();

    // Zet de ruwe waarden om naar een vochtigheidsniveau per sensor
    MoistureLevel capacitiveLevel = getCapacitiveMoistureLevel(capacitiveValue);
    MoistureLevel resistiveLevel = getResistiveMoistureLevel(resistiveValue);
    debugV("capacitiveValue = %5d | capacitiveLevel: %s", capacitiveValue,
           moistureLevelToString(capacitiveLevel));
    debugV("resistiveValue = %5d | resistiveLevel: %s", resistiveValue,
           moistureLevelToString(resistiveLevel));
    // Combineer beide niveaus: het droogste niveau wint
    MoistureLevel moistureLevel =
        selectMoistureLevel(capacitiveLevel, resistiveLevel);

    float tempC = getTemperatureFromSensor();

    // Log het vochtigheidsniveau
    if (moistureLevel == DRY) {
      debugW("Ground is dry (capacitive=%d, resistive=%d)", capacitiveValue,
             resistiveValue);
    } else if (moistureLevel == MOIST) {
      debugI("Ground is moist (capacitive=%d, resistive=%d)", capacitiveValue,
             resistiveValue);
    } else {
      debugI("Ground is wet (capacitive=%d, resistive=%d)", capacitiveValue,
             resistiveValue);
    }

    // Log de temperatuurstatus
    if (tempC > TEMP_PUMP_THRESHOLD_C) {
      debugI("Temperature normal: %.1fC", tempC);
    } else if (tempC > TEMP_COLD_CUTOFF_C) {
      debugW("Temperature low: %.1fC", tempC);
    } else {
      debugW("Temperature too cold: %.1fC", tempC);
    }

    // Start de pomp alleen als de grond droog is en de pomp nog niet loopt
    if (!pumpActive) {
      pumpDurationMs = getPumpDurationMs(moistureLevel, tempC);
      debugI("Pump duration: %dms", pumpDurationMs);

      if (pumpDurationMs > 0) {
        turnPumpOn();
        pumpActive = true;
        pumpStartMs = nowMs;
      }
    }
  }

  // Zet de pomp uit zodra de ingestelde duur verstreken is
  // (buiten het interval-blok zodat de timing precies klopt)
  if (pumpActive && (nowMs - pumpStartMs >= pumpDurationMs)) {
    turnPumpOff();
    pumpActive = false;
  }

  // Verwerk inkomende seriële debug-commando's
  debugHandle();
}

// Leest de temperatuursensor uit en geeft de waarde terug in graden Celsius
float getTemperatureFromSensor() {
#ifdef MOCK_SENSORS
  TRACE();
  int temperatureSensorMv = MOCK_TEMP_MV;
  DUMP(temperatureSensorMv);
#else
  int temperatureSensorMv = analogReadMilliVolts(TEMP_SENSOR);
  debugV("temperatureSensorMv = %d", temperatureSensorMv);
#endif
  // Conversie: 10mV per graden Celsius, zie LM35 sensor manual bron
  float tempC = temperatureSensorMv / 10.0f;
#ifdef MOCK_SENSORS
  DUMP(tempC);
#else
  debugV("tempC = %.1f", tempC);
#endif
  return tempC;
}

// Leest de ruwe ADC-waarde van de resistieve bodemvochtigheidssensor uit
int getResistiveSensorValue() {
#ifdef MOCK_SENSORS
  TRACE();
  int sensorValue = MOCK_RESISTIVE_VALUE;
  DUMP(sensorValue);
  return sensorValue;
#else
  int sensorValue = analogRead(RESISTIVE_SENSOR);
  debugV("resistiveSensorValue = %d", sensorValue);
  return sensorValue;
#endif
}

// Leest de ruwe ADC-waarde van de capacitieve bodemvochtigheidssensor uit
int getCapacitiveSensorValue() {
#ifdef MOCK_SENSORS
  TRACE();
  int sensorValue = MOCK_CAPACITIVE_VALUE;
  DUMP(sensorValue);
  return sensorValue;
#else
  int sensorValue = analogRead(CAPACITIVE_SENSOR);
  debugV("capacitiveSensorValue = %d", sensorValue);
  return sensorValue;
#endif
}

// Stuurt een signaal naar de relay om de pomp aan te zetten
void turnPumpOn() {
  debugI("Pump ON");
  digitalWrite(PUMP_RELAY_PIN, HIGH);
}

// Stuurt een signaal naar de relay om de pomp uit te zetten
void turnPumpOff() {
  debugI("Pump OFF");
  digitalWrite(PUMP_RELAY_PIN, LOW);
}
