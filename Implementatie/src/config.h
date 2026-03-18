/*
VERWERKING KLASSIKALE FEEDBACK
- Ik heb de klassikale feedback bekeken. Echter moest ik niets meer aanpassen.
*/

#pragma region PIN DEFINITIONS

// GPIO-pinnummers van de aangesloten sensoren en de relay
const int TEMP_SENSOR = 36;       // A0
const int CAPACITIVE_SENSOR = 39; // A1
const int RESISTIVE_SENSOR = 34;  // A2
const int PUMP_RELAY_PIN = 25;    // D2
const int MANUAL_BUTTON_PIN = 27; // D3

#pragma endregion

// Wachttijd tussen 2 opeenvolgende inlezingen van sensoren (in ms).
// Ingesteld op 5s voor testdoeleinden; in productie kan dit verhoogd worden
// (bv. elke 30 minuten) om energie te besparen
const int SENSOR_INLEES_INTERVAL_MS = 5000;

// Temperatuur schakelwaarden (in °C)
// Waarboven de pomp normaal lang loopt
const int TEMPERATUUR_NORMAAL_GRENS_C = 25;
// Waaronder de pomp helemaal niet loopt (te koud voor de plant)
const int TEMPERATUUR_TE_KOUD_GRENS_C = 5;

// Categoriën vochtigheid: de drie mogelijke vochtigheidsniveaus van de grond
enum Vochtigheid { NAT, VOCHTIG, DROOG };

// Statussen water geven (geen water, wél water)
enum WaterGeefStatus { GEEN_WATER, WEL_WATER };

// Duurtijden water geven (in milliseconden)
// Hoe lang de pomp aan blijft bij normale temperatuur
// Waarde afkomstig uit de flowchart die in de klas is opgesteld
const int WATER_GEEF_DUUR_NORMAAL_MS = 2000;
// Hoe lang de pomp aan blijft bij lage maar toegestane temperatuur
// Waarde afkomstig uit de flowchart die in de klas is opgesteld
const int WATER_GEEF_DUUR_KORT_MS = 1000;
// Hoe lang de pomp aan blijft bij een handmatige override.
// Afzonderlijk instelbaar zodat de gebruiker per druk op de knop een vaste,
// van de automatische cycli afwijkende hoeveelheid water kan geven
const int WATER_GEEF_DUUR_MANUEEL_MS = 3000;

// Intervallen voor BVH waarden
// Opgelet, we definiëren de intervallen als gesloten: [min, max]
// Minimum- en maximumwaarde voor de resistieve vochtigheidssensor om de
// interpretatie "DROOG" te krijgen
const int RESISTIEVE_SENSOR_DROOG_INTERVAL_MIN = 0;
const int RESISTIEVE_SENSOR_DROOG_INTERVAL_MAX = 376;

// Minimum- en maximumwaarde voor de resistieve vochtigheidssensor om de
// interpretatie "VOCHTIG" te krijgen
const int RESISTIEVE_SENSOR_VOCHTIG_INTERVAL_MIN =
    RESISTIEVE_SENSOR_DROOG_INTERVAL_MAX + 1;
const int RESISTIEVE_SENSOR_VOCHTIG_INTERVAL_MAX = 743;

// Minimum- en maximumwaarde voor de resistieve vochtigheidssensor om de
// interpretatie "NAT" te krijgen
const int RESISTIEVE_SENSOR_NAT_INTERVAL_MIN =
    RESISTIEVE_SENSOR_VOCHTIG_INTERVAL_MAX + 1;
const int RESISTIEVE_SENSOR_NAT_INTERVAL_MAX = 4095;

// Minimum- en maximumwaarde voor de capacitieve vochtigheidssensor om de
// interpretatie "DROOG" te krijgen
const int CAPACITIEVE_SENSOR_DROOG_INTERVAL_MIN = 2800;
const int CAPACITIEVE_SENSOR_DROOG_INTERVAL_MAX = 4095;

// Minimum- en maximumwaarde voor de capacitieve vochtigheidssensor om de
// interpretatie "VOCHTIG" te krijgen
const int CAPACITIEVE_SENSOR_VOCHTIG_INTERVAL_MIN = 2000;
const int CAPACITIEVE_SENSOR_VOCHTIG_INTERVAL_MAX =
    CAPACITIEVE_SENSOR_DROOG_INTERVAL_MIN - 1;

// Minimum- en maximumwaarde voor de capacitieve vochtigheidssensor om de
// interpretatie "NAT" te krijgen
const int CAPACITIEVE_SENSOR_NAT_INTERVAL_MIN = 0;
const int CAPACITIEVE_SENSOR_NAT_INTERVAL_MAX =
    CAPACITIEVE_SENSOR_VOCHTIG_INTERVAL_MIN - 1;

#pragma region MOCK
// Nep-sensorwaarden voor testen zonder fysiek bord (alleen actief bij
// MOCK_SENSORS build flag)
#ifdef MOCK_SENSORS

// Capacitieve sensor: MOCK_CAPACITIVE_DRY / MOCK_CAPACITIVE_MOIST /
// MOCK_CAPACITIVE_WET (standaard)
#ifdef MOCK_CAPACITIVE_DRY
const int MOCK_CAPACITIVE_VALUE = 2900; // DROOG range (2800-3150)
#elif defined(MOCK_CAPACITIVE_MOIST)
const int MOCK_CAPACITIVE_VALUE = 2400; // VOCHTIG range (2000-2799)
#else // MOCK_CAPACITIVE_WET
const int MOCK_CAPACITIVE_VALUE = 1700; // NAT range (1420-1999)
#endif

// Resistieve sensor: MOCK_RESISTIVE_DRY / MOCK_RESISTIVE_MOIST /
// MOCK_RESISTIVE_WET (standaard)
#ifdef MOCK_RESISTIVE_DRY
const int MOCK_RESISTIVE_VALUE = 188; // DROOG range (0-376)
#elif defined(MOCK_RESISTIVE_MOIST)
const int MOCK_RESISTIVE_VALUE = 560; // VOCHTIG range (377-743)
#else // MOCK_RESISTIVE_WET
const int MOCK_RESISTIVE_VALUE = 870; // NAT range (744-999)
#endif

// Temperatuursensor in millivolt: hoog, normaal of laag afhankelijk van build
// flag
#ifdef MOCK_TEMP_HIGH
const int MOCK_TEMP_MV = 260; // 26°C, boven TEMPERATUUR_NORMAAL_GRENS_C
#elif defined(MOCK_TEMP_NORMAL)
const int MOCK_TEMP_MV = 150; // 15°C, tussen TEMPERATUUR_TE_KOUD_GRENS_C en
                              // TEMPERATUUR_NORMAAL_GRENS_C
#else // MOCK_TEMP_LOW
const int MOCK_TEMP_MV = 30; // 3°C, onder TEMPERATUUR_TE_KOUD_GRENS_C
#endif
#endif
#pragma endregion
