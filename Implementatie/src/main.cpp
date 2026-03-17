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

#include "Arduino.h"
// Stel het minimale logniveau in vóór de include, anders gebruikt SerialDebug
// de standaard (WARNING)
#define DEBUG_INITIAL_LEVEL DEBUG_LEVEL_VERBOSE
#define ARDUINOTRACE_ENABLE 0
#include <SerialDebug.h>
#ifdef MOCK_SENSORS
#include <ArduinoTrace.h>
#endif
#include <Bounce2.h>
#include "config.h"

// Bounce2 object voor de handmatige override-knop; globaal zodat zowel setup()
// als loop() er toegang toe hebben
Bounce button;

// Variabelen om wachttijd tussen inlezen sensoren te kunnen regelen
long laatste_check_ms = 0;

// Variabelen om duurtijd van water geven te kunnen regelen
long pomp_start_ms = 0;
int pomp_duur_ms = 0;

// Variabele om status van de waterpomp aan te geven, dit is nodig om te kunnen
// controlleren of de waterpomp gestopt moet worden
bool pomp_actief = false;

// Forward declarations
Vochtigheid berekenCategorieCapactieveBHV(int sensorwaarde);
Vochtigheid berekenCategorieResistieveBVH(int sensorwaarde);
Vochtigheid berekenSamengesteldeCategorie(Vochtigheid categorieResistieveBVH, Vochtigheid categorieCapacitieveBVH);
const String vochtigheidsNiveauNaarString(Vochtigheid niveau);

/**
 * Bepaal de temperatuur via de LM35 analoge sensor.
 * Conversie: 10mV per °C, zie LM35 sensor manual bron.
 * Geeft de temperatuur in °C terug.
 */
int leesTemperatuur() {
#ifdef MOCK_SENSORS
  TRACE();
  int temperatureSensorMv = MOCK_TEMP_MV;
  DUMP(temperatureSensorMv);
#else
  int temperatureSensorMv = analogReadMilliVolts(TEMP_SENSOR);
  debugV("temperatureSensorMv = %d", temperatureSensorMv);
#endif
  int tempC = temperatureSensorMv / 10;
#ifdef MOCK_SENSORS
  DUMP(tempC);
#else
  debugV("tempC = %d", tempC);
#endif
  return tempC;
}

/**
 * Bepaal de juiste sensorwaarde voor de capacitieve bodemvochtigheidssensor.
 */
int leesCapacitieveBVHSensor() {
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

/**
 * Bepaal de juiste sensorwaarde voor de resistieve bodemvochtigheidssensor.
 */
int leesResistieveBVHSensor() {
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

/* MOCK functies om sensoren te bypassen */
int leesTemperatuur_MOCK() {
  // Return random waarde tussen 0 en 30 °C
  return random(0, 30);
}

int leesCapacitieveBVHSensor_MOCK() {
  // Return random waarde tussen 0 en 4095
  return random(0, 4095);
}

int leesResistieveBVHSensor_MOCK() {
  // Return random waarde tussen 0 en 4095
  return random(0, 4095);
}

/**
 * Bepaal de categorie van de capacitieve bodemvochtigheidssensor voor de gemeten sensorwaarde.
 * We gebruiken hierbij de configuratie uit onze calibratie.  Per categorie checken we of de waarde
 * tussen de MIN en de MAX valt.
 * Return type: Vochtigheid (uit configuratiebestand)
 */
Vochtigheid berekenCategorieCapactieveBHV(int sensorwaarde) {
  if (sensorwaarde >= CAPACITIEVE_SENSOR_DROOG_INTERVAL_MIN && sensorwaarde <= CAPACITIEVE_SENSOR_DROOG_INTERVAL_MAX) {
    return DROOG;
  }
  if (sensorwaarde >= CAPACITIEVE_SENSOR_VOCHTIG_INTERVAL_MIN && sensorwaarde <= CAPACITIEVE_SENSOR_VOCHTIG_INTERVAL_MAX) {
    return VOCHTIG;
  }
  return NAT;
}

/**
 * Bepaal de categorie van de resistieve bodemvochtigheidssensor voor de gemeten sensorwaarde.
 * We gebruiken hierbij de configuratie uit onze calibratie.  Per categorie checken we of de waarde
 * tussen de MIN en de MAX valt.
 * Return type: Vochtigheid (uit configuratiebestand)
 */
Vochtigheid berekenCategorieResistieveBVH(int sensorwaarde) {
  if (sensorwaarde >= RESISTIEVE_SENSOR_DROOG_INTERVAL_MIN && sensorwaarde <= RESISTIEVE_SENSOR_DROOG_INTERVAL_MAX) {
    return DROOG;
  }
  if (sensorwaarde >= RESISTIEVE_SENSOR_VOCHTIG_INTERVAL_MIN && sensorwaarde <= RESISTIEVE_SENSOR_VOCHTIG_INTERVAL_MAX) {
    return VOCHTIG;
  }
  return NAT;
}

/**
 * Bereken de samengestelde categorie voor beide bodemvochtigheidssensoren.
 * Strategie: het droogste niveau wint altijd.
 * Redenering: te droog is een groter risico voor de plant dan te nat.
 * Return type: Vochtigheid (uit configuratiebestand)
 */
Vochtigheid berekenSamengesteldeCategorie(Vochtigheid categorieResistieveBVH, Vochtigheid categorieCapacitieveBVH) {
  if (categorieResistieveBVH == DROOG || categorieCapacitieveBVH == DROOG) {
    return DROOG;
  }
  if (categorieResistieveBVH == VOCHTIG || categorieCapacitieveBVH == VOCHTIG) {
    return VOCHTIG;
  }
  return NAT;
}

/**
 * Zet de waterpomp aan voor een bepaalde tijd.
 * Bevat GEEN DELAY. De duurtijd wordt bijgehouden via globale variabelen.
 * De loop() controleert via millis() of de duurtijd verstreken is.
 */
void zetWaterpompAan(int duurtijd) {
  debugI("Pump ON");
  digitalWrite(PUMP_RELAY_PIN, HIGH);

  pomp_start_ms = millis();
  pomp_duur_ms = duurtijd;
  pomp_actief = true;
}

/**
 * Zet de waterpomp uit.
 * Reset de variabelen die door zetWaterpompAan() zijn ingesteld.
 */
void zetWaterpompUit() {
  debugI("Pump OFF");
  digitalWrite(PUMP_RELAY_PIN, LOW);

  pomp_start_ms = 0;
  pomp_duur_ms = 0;
  pomp_actief = false;
}

/**
 * Deze functie bevat alle code voor het uitlezen van de sensoren en om de waterpomp indien nodig aan te zetten.
 * Het uitzetten van de waterpomp gebeurt niet hier maar in de loop() functie na controle of er voldoende tijd verstreken is.
 */
void leesSensorenEnGeefWaterIndienNodig() {
  int capacitieve_bvh_waarde = leesCapacitieveBVHSensor();
  int resistieve_bvh_waarde = leesResistieveBVHSensor();
  int temperatuur = leesTemperatuur();

  // Bepaal individuele categoriën en samengestelde categorie
  Vochtigheid categorieCapacitieveBVH = berekenCategorieCapactieveBHV(capacitieve_bvh_waarde);
  Vochtigheid categorieResistieveBVH = berekenCategorieResistieveBVH(resistieve_bvh_waarde);
  Vochtigheid categorie = berekenSamengesteldeCategorie(categorieCapacitieveBVH, categorieResistieveBVH);

  debugV("capacitiveValue = %5d | categorieCapacitief: %s", capacitieve_bvh_waarde,
         vochtigheidsNiveauNaarString(categorieCapacitieveBVH).c_str());
  debugV("resistiveValue = %5d | categorieResistief: %s", resistieve_bvh_waarde,
         vochtigheidsNiveauNaarString(categorieResistieveBVH).c_str());

  // Log het vochtigheidsniveau
  if (categorie == DROOG) {
    debugW("Grond is droog (capacitief=%d, resistief=%d)", capacitieve_bvh_waarde,
           resistieve_bvh_waarde);
  } else if (categorie == VOCHTIG) {
    debugI("Grond is vochtig (capacitief=%d, resistief=%d)", capacitieve_bvh_waarde,
           resistieve_bvh_waarde);
  } else {
    debugI("Grond is nat (capacitief=%d, resistief=%d)", capacitieve_bvh_waarde,
           resistieve_bvh_waarde);
  }

  // Log de temperatuurstatus
  if (temperatuur > TEMPERATUUR_NORMAAL_GRENS_C) {
    debugI("Temperatuur normaal: %dC", temperatuur);
  } else if (temperatuur > TEMPERATUUR_TE_KOUD_GRENS_C) {
    debugW("Temperatuur laag: %dC", temperatuur);
  } else {
    debugW("Temperatuur te koud: %dC", temperatuur);
  }

  // Beslis over water geven: alleen als de grond droog is en de pomp niet loopt
  if (categorie == DROOG && !pomp_actief) {
    int duur = GEEN_WATER;
    if (temperatuur > TEMPERATUUR_NORMAAL_GRENS_C) {
      duur = WATER_GEEF_DUUR_NORMAAL_MS;
    } else if (temperatuur > TEMPERATUUR_TE_KOUD_GRENS_C) {
      duur = WATER_GEEF_DUUR_KORT_MS;
    }
    // Te koud (duur == GEEN_WATER): geen water geven
    debugI("Pomp duurtijd: %dms", duur);
    if (duur > GEEN_WATER) {
      zetWaterpompAan(duur);
    }
  }
}

const String vochtigheidsNiveauNaarString(Vochtigheid niveau) {
  switch (niveau) {
  case NAT:
    return "NAT";
  case VOCHTIG:
    return "VOCHTIG";
  case DROOG:
    return "DROOG";
  }
  debugE("Fout met de toString van Vochtigheid");
  return "ONBEKEND";
}

void setup() {
  Serial.begin(9600);
  debugI("Setup gestart");
  // Zet de relay-pin als uitgang zodat we de pomp kunnen aansturen
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  // Koppel de override-knop aan het Bounce2-object; INPUT_PULLUP zodat de knop
  // actief-laag werkt (verbinding naar GND bij indrukken)
  button.attach(MANUAL_BUTTON_PIN, INPUT_PULLUP);
  button.interval(25); // 25ms debounce-tijd
  debugSetLevel(DEBUG_LEVEL_VERBOSE);
  // Zet de attenuatie van de temperatuursensor-pin op 0dB (bereik 0-1.1V)
  analogSetPinAttenuation(TEMP_SENSOR, ADC_0db);
}

void loop() {
  // We hebben huidige millis nodig om de verschillende processen te controleren (water geven / stoppen)
  long huidigeMillis = millis();

  button.update();

  // Handmatige override: fell() vuurt alleen bij de neergaande flank (één keer
  // per druk), dankzij Bounce2 wordt contact-dender uitgefilterd
  if (button.fell() && !pomp_actief) {
    debugI("Handmatige override knop ingedrukt");
    zetWaterpompAan(WATER_GEEF_DUUR_MANUEEL_MS);
  }

  // Controleer of de waterpomp uitgezet moet worden en roep zetWaterpompUit() aan indien nodig
  if (pomp_actief && (huidigeMillis - pomp_start_ms >= pomp_duur_ms)) {
    zetWaterpompUit();
  }

  // Controleer of sensoren ingelezen moeten worden en roep leesSensorenEnGeefWaterIndienNodig() aan indien nodig
  if (huidigeMillis - laatste_check_ms >= SENSOR_INLEES_INTERVAL_MS) {
    laatste_check_ms = huidigeMillis;
    leesSensorenEnGeefWaterIndienNodig();
  }

  // Verwerk inkomende seriële debug-commando's
  debugHandle();
}
