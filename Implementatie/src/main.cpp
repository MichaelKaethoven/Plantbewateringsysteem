/*
  BRONNEN:
  Canvas cursus - 16/02/2026 - https://canvas.kdg.be/courses/55542
  LM35 temperature sensor manual - 16/02/2026 -
  https://wiki.dfrobot.com/DFRobot_LM35_Linear_Temperature_Sensor__SKU_DFR0023_
  ADC pin attenuation - 16/02/2026 -
  https://deepbluembedded.com/esp32-adc-tutorial-read-analog-voltage-arduino/
  Capacitieve bodemvochtigheidssensor - 16/02/2026 -
  https://canvas.kdg.be/courses/55542/pages/bodemvochtigheidssensoren-capacitieve-bvh-sensor?module_item_id=1357791


*/

#include <Arduino.h>
#include <config.h>
#include <functionDeclarations.h>

#include <Arduino.h>

void setup() {
  Serial.begin(9600);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  // Geef aan dat de ESP32 op de TEMP_SENSOR pin een lage voltage mag verwachten
  // (100-950mV) zie pin attenuation bron
  analogSetPinAttenuation(TEMP_SENSOR, ADC_0db);
}

void loop() {
  // statics die hetzelfde blijven tijdens de loops
  static bool pumpActive = false;
  static int pumpStartMs = 0;
  static int lastCheckMs = 0;

  int nowMs = (int)millis();

  if (nowMs - lastCheckMs >= PUMP_CHECK_INTERVAL_MS) {
    lastCheckMs = nowMs;

    int capacitiveValue = getCapacitiveSensorValue();
    int resistiveValue = getResistiveSensorValue();
    float tempC = getTemperatureFromSensor();
    Serial.print("Temp:");
    Serial.println(tempC);

    bool capacitiveDry = (capacitiveValue >= CAPACITIVE_DRY_MIN &&
                          capacitiveValue <= CAPACITIVE_DRY_MAX);
    bool resistiveDry = (resistiveValue >= RESISTIVE_DRY_MIN &&
                         resistiveValue <= RESISTIVE_DRY_MAX);

    bool shouldRunPump =
        capacitiveDry || resistiveDry || (tempC > TEMP_PUMP_THRESHOLD_C);

    if (shouldRunPump && !pumpActive) {
      turnPumpOn();
      pumpActive = true;
      pumpStartMs = nowMs;
    }
  }

  if (pumpActive && (nowMs - pumpStartMs >= PUMP_ON_DURATION_MS)) {
    turnPumpOff();
    pumpActive = false;
  }
}

float getTemperatureFromSensor() {
  int temperatureSensorMv = analogReadMilliVolts(TEMP_SENSOR);
  // Conversie: 10mV per graden Celsius, zie LM35 sensor manual bron
  float tempC = temperatureSensorMv / 10.0f;
  return tempC;
}

int getResistiveSensorValue() {
  int sensorValue = analogRead(RESISTIVE_SENSOR);
  return sensorValue;
}

int getCapacitiveSensorValue() {
  int sensorValue = analogRead(CAPACITIVE_SENSOR);
  return sensorValue;
}

// Stuurt een signaal naar de relay om de pomp aan te zetten
void turnPumpOn() { digitalWrite(PUMP_RELAY_PIN, HIGH); }

// Stuurt een signaal naar de relay om de pomp uit te zetten
void turnPumpOff() { digitalWrite(PUMP_RELAY_PIN, LOW); }
