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

*/

#include <Arduino.h>
#include <config.h>
#include <functionDeclarations.h>

#include <Arduino.h>

void setup() {
  Serial.begin(9600);
  pinMode(PUMP_RELAY_PIN, OUTPUT);

  analogSetPinAttenuation(TEMP_SENSOR, ADC_0db);
}

void loop() {
  // statics die hetzelfde blijven tijdens de loops, maar in loop-scope
  // aangezien de rest van het programma dit niet moet zien
  static bool pumpActive = false;
  static int pumpStartMs = 0;
  static int lastCheckMs = 0;

  int nowMs = (int)millis();

  if (nowMs - lastCheckMs >= PUMP_CHECK_INTERVAL_MS) {
    lastCheckMs = nowMs;

    int capacitiveValue = getCapacitiveSensorValue();
    int resistiveValue = getResistiveSensorValue();
    float tempC = getTemperatureFromSensor() - 3;
    Serial.print("Temp:");
    Serial.println(tempC);

    // "droog" wordt gemeten wanneer de sensor een waarde heeft tussen DRY_MIN
    // en DRY_MAX
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
  Serial.print("Resistive val: ");
  Serial.println(sensorValue);

  return sensorValue;
}

int getCapacitiveSensorValue() {
  int sensorValue = analogRead(CAPACITIVE_SENSOR);
  Serial.print("Capacitive val: ");
  Serial.println(sensorValue);
  return sensorValue;
}

// Stuurt een signaal naar de relay om de pomp aan te zetten
void turnPumpOn() { digitalWrite(PUMP_RELAY_PIN, HIGH); }

// Stuurt een signaal naar de relay om de pomp uit te zetten
void turnPumpOff() { digitalWrite(PUMP_RELAY_PIN, LOW); }
