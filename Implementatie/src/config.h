#pragma region PIN DEFINITIONS

// GPIO-pinnummers van de aangesloten sensoren en de relay
const int TEMP_SENSOR = 36;       // A0
const int CAPACITIVE_SENSOR = 39; // A1
const int RESISTIVE_SENSOR = 34;  // A2
const int PUMP_RELAY_PIN = 25;    // D2

#pragma endregion

#pragma region CONSTANTS

// ADC-drempelwaarden van de capacitieve bodemvochtigheidssensor (hogere waarde
// = droger)
const int CAPACITIVE_DRY_MAX = 3000;
const int CAPACITIVE_DRY_MIN = 2800;

const int CAPACITIVE_MOIST_MAX = 2799;
const int CAPACITIVE_MOIST_MIN = 2400;

const int CAPACITIVE_WET_MAX = 2399;
const int CAPACITIVE_WET_MIN = 1400;

// ADC-drempelwaarden van de resistieve bodemvochtigheidssensor (lagere waarde =
// droger)
const int RESISTIVE_DRY_MIN = 0;
const int RESISTIVE_DRY_MAX = 250;

const int RESISTIVE_MOIST_MIN = 251;
const int RESISTIVE_MOIST_MAX = 1500;

const int RESISTIVE_WET_MIN = 1501;
const int RESISTIVE_WET_MAX = 3000;

// Temperatuurgrens waarboven de pomp normaal lang loopt (in °C)
const int TEMP_PUMP_THRESHOLD_C = 25;
// Temperatuurgrens waaronder de pomp helemaal niet loopt (te koud voor de
// plant)
const int TEMP_COLD_CUTOFF_C = 5;
// Hoe lang de pomp aan blijft bij normale temperatuur (in ms)
const int PUMP_ON_DURATION_MS = 2000;
// Hoe lang de pomp aan blijft bij lage maar toegestane temperatuur (in ms)
const int PUMP_ON_DURATION_SHORT_MS = 1000;
// Hoe vaak de sensoren uitgelezen worden (in ms)
const int PUMP_CHECK_INTERVAL_MS = 1000;

#pragma endregion

// De drie mogelijke vochtigheidsniveaus van de grond
enum MoistureLevel { WET, MOIST, DRY };

#pragma region MOCK
// Nep-sensorwaarden voor testen zonder fysiek bord (alleen actief bij
// MOCK_SENSORS build flag)
#ifdef MOCK_SENSORS
// Bodemvochtigheidssensor: droog of nat afhankelijk van MOCK_DRY/MOCK_WET build
// flag
#ifdef MOCK_DRY
const int MOCK_CAPACITIVE_VALUE = 2900; // DRY range (2800-3000)
const int MOCK_RESISTIVE_VALUE = 100;   // DRY range (0-250)
#else                                   // MOCK_WET
const int MOCK_CAPACITIVE_VALUE = 1800; // WET range (1400-2399)
const int MOCK_RESISTIVE_VALUE = 2000;  // WET range (1501-3000)
#endif
// Temperatuursensor in millivolt: hoog, normaal of laag afhankelijk van build
// flag
#ifdef MOCK_TEMP_HIGH
const int MOCK_TEMP_MV = 260; // 26°C, boven TEMP_PUMP_THRESHOLD_C
#elif defined(MOCK_TEMP_NORMAL)
const int MOCK_TEMP_MV =
    150; // 15°C, tussen TEMP_COLD_CUTOFF_C en TEMP_PUMP_THRESHOLD_C
#else // MOCK_TEMP_LOW
const int MOCK_TEMP_MV = 30; // 3°C, onder TEMP_COLD_CUTOFF_C
#endif
#endif
#pragma endregion