
#pragma once

#pragma region PIN DEFINITIONS

// GPIO-pinnummers van de aangesloten sensoren en de relay
const int TEMP_SENSOR = 36;       // A0
const int CAPACITIVE_SENSOR = 39; // A1
const int RESISTIVE_SENSOR = 34;  // A2
const int PUMP_RELAY_PIN = 25;    // D2

// Handmatige override-knop: sluit een externe drukknop aan op GPIO 27 (D3).
// GPIO 27 is een RTC GPIO op de ESP32 en ondersteunt ext0 wakeup.
const int WAKEUP_BUTTON_PIN = 27; // D3

#pragma endregion

// ── Deep sleep
// ──────────────────────────────────────────────────────────────── Slaaptijd
// tussen twee metingen: 60 seconden (1 meting per minuut). Eenheid voor
// esp_sleep_enable_timer_wakeup() is microseconden.
const int DEEP_SLEEP_DUUR_S = 60;
const uint64_t DEEP_SLEEP_DUUR_US = (uint64_t)DEEP_SLEEP_DUUR_S * 1000000ULL;

// ── GPS-sensor
// ──────────────────────────────────────────────────────────────── Aangesloten
// op UART1 van de ESP32 (HardwareSerial poort 1). GPS TX → GPIO 16 (ESP32 RX),
// GPS RX → GPIO 17 (ESP32 TX). De GPS-module kan NIET als wakeup-trigger
// dienen; hij wordt enkel uitgelezen wanneer de microcontroller al wakker is
// (timer of knop).
const int GPS_RX_PIN = 16;
const int GPS_TX_PIN = 17;
const int GPS_BAUD = 9600;

// Maximale wachttijd voor een GPS-fix per wakeup.
// Bij een cold start kan een fix minuten duren; 5 s is een compromis tussen
// nauwkeurigheid en stroomverbruik. De laatste bekende positie (RTC-geheugen)
// wordt gebruikt als er geen nieuwe fix is.
const int GPS_READ_TIMEOUT_MS = 5000;

// ── WiFi
// ──────────────────────────────────────────────────────────────────────
// Initiële verbindingstijd in setup(): 10 s dekt een cold-start scan + auth.
// Herverbindingstijd in loop(): beperkt tot 5 s zoals gespecificeerd in het
// flowchart (één herpoging, daarna connectiefout loggen).
const int WIFI_VERBINDING_TIMEOUT_MS = 10000;

// ── NTP
// ───────────────────────────────────────────────────────────────────────
// België: UTC+1 (winter/CET). DST voegt nog 1 h toe (CEST = UTC+2).
// configTime(gmtOffset_sec, daylightOffset_sec, server)
#define NTP_SERVER "pool.ntp.org"
const long NTP_TIJDZONE_OFFSET_S = 3600; // UTC+1
const int NTP_ZOMERTIJD_OFFSET_S = 3600; // +1 h tijdens zomertijd
const int NTP_TIMEOUT_MS = 5000;

// ── Temperatuur
// ───────────────────────────────────────────────────────────────
const int TEMPERATUUR_NORMAAL_GRENS_C = 25;
const int TEMPERATUUR_TE_KOUD_GRENS_C = 5;

// ── Vochtigheid en pompstatus
// ─────────────────────────────────────────────────
enum Vochtigheid { NAT, VOCHTIG, DROOG };

// WaterGeefStatus: gebruikt in leesSensorenEnGeefWaterIndienNodig() als
// int-waarde (GEEN_WATER = 0 als neutrale duurtijd).
enum PompStatus { GEEN_WATER_GEVEN, WATER_GEVEN };

// ── Pomp-duurtijden (ms)
// ──────────────────────────────────────────────────────
const int WATER_GEEF_DUUR_NORMAAL_MS = 5000;  // temp > 25 °C
const int WATER_GEEF_DUUR_KORT_MS = 2500;     // 5 °C < temp ≤ 25 °C
const int WATER_GEEF_DUUR_KOUD_MS = 1000;     // temp ≤ 5 °C (minimale gift)
const int WATER_GEEF_DUUR_MANUEEL_MS = 10000; // handmatige override

// ── Sensorintervallen (ADC 0–4095, gesloten: [min, max]) ─────────────────────

// Resistieve sensor (hogere waarde = natter)
const int RESISTIEVE_SENSOR_DROOG_INTERVAL_MIN = 0;
const int RESISTIEVE_SENSOR_DROOG_INTERVAL_MAX = 376;
const int RESISTIEVE_SENSOR_VOCHTIG_INTERVAL_MIN =
    RESISTIEVE_SENSOR_DROOG_INTERVAL_MAX + 1;
const int RESISTIEVE_SENSOR_VOCHTIG_INTERVAL_MAX = 743;
// Capacitieve sensor (omgekeerd: hogere waarde = droger)
const int CAPACITIEVE_SENSOR_DROOG_INTERVAL_MIN = 2800;
const int CAPACITIEVE_SENSOR_DROOG_INTERVAL_MAX = 4095;
const int CAPACITIEVE_SENSOR_VOCHTIG_INTERVAL_MIN = 2000;
const int CAPACITIEVE_SENSOR_VOCHTIG_INTERVAL_MAX =
    CAPACITIEVE_SENSOR_DROOG_INTERVAL_MIN - 1;
#pragma region MOCK
// Nep-sensorwaarden voor testen zonder fysiek bord (alleen actief bij
// MOCK_SENSORS build flag)
#ifdef MOCK_SENSORS

// Capacitieve sensor: MOCK_CAPACITIVE_DRY / MOCK_CAPACITIVE_MOIST /
// MOCK_CAPACITIVE_WET (standaard)
#ifdef MOCK_CAPACITIVE_DRY
const int MOCK_CAPACITIVE_VALUE = 2900; // DROOG range (2800–4095)
#elif defined(MOCK_CAPACITIVE_MOIST)
const int MOCK_CAPACITIVE_VALUE = 2400; // VOCHTIG range (2000–2799)
#else // MOCK_CAPACITIVE_WET
const int MOCK_CAPACITIVE_VALUE = 1700; // NAT range (0–1999)
#endif

// Resistieve sensor: MOCK_RESISTIVE_DRY / MOCK_RESISTIVE_MOIST /
// MOCK_RESISTIVE_WET (standaard)
#ifdef MOCK_RESISTIVE_DRY
const int MOCK_RESISTIVE_VALUE = 188; // DROOG range (0–376)
#elif defined(MOCK_RESISTIVE_MOIST)
const int MOCK_RESISTIVE_VALUE = 560; // VOCHTIG range (377–743)
#else // MOCK_RESISTIVE_WET
const int MOCK_RESISTIVE_VALUE = 870; // NAT range (744–4095)
#endif

// Temperatuursensor in millivolt
#ifdef MOCK_TEMP_HIGH
const int MOCK_TEMP_MV = 260; // 26 °C — boven TEMPERATUUR_NORMAAL_GRENS_C
#elif defined(MOCK_TEMP_NORMAL)
const int MOCK_TEMP_MV = 150; // 15 °C — tussen de twee grenzen
#else // MOCK_TEMP_LOW
const int MOCK_TEMP_MV = 30; //  3 °C — onder TEMPERATUUR_TE_KOUD_GRENS_C
#endif

// Mock GPS-coördinaten (KDG campus Antwerpen)
const double MOCK_GPS_LAT = 51.2194;
const double MOCK_GPS_LNG = 4.4025;
const bool MOCK_GPS_FIX = true;

#endif // MOCK_SENSORS
#pragma endregion
