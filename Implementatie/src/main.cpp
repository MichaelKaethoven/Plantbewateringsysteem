/*
  BRONNEN:
  Canvas cursus - 16/02/2026 - https://canvas.kdg.be/courses/55542
  LM35 temperature sensor manual - 16/02/2026 -
    https://wiki.dfrobot.com/DFRobot_LM35_Linear_Temperature_Sensor__SKU_DFR0023_
  ADC pin attenuation - 16/02/2026 -
    https://deepbluembedded.com/esp32-adc-tutorial-read-analog-voltage-arduino/
  Capacitieve bodemvochtigheidssensor - 16/02/2026 -
    https://canvas.kdg.be/courses/55542/pages/bodemvochtigheidssensoren-capacitieve-bvh-sensor
  The static keyword - 16/02/2026 -
    https://ccrma.stanford.edu/~fgeorg/250a/lab2/arduino-0019/reference/Static.html
  Using build flags - 10/03/2026 -
    https://docs.platformio.org/en/stable/projectconf/sections/env/options/build/build_flags.html
  Using C preprocessors - 10/03/2026 -
    https://www.tutorialspoint.com/cprogramming/c_preprocessors.htm
  ESP32 Deep Sleep (Arduino) - 20/04/2026 -
    https://docs.espressif.com/projects/arduino-esp32/en/latest/api/deepsleep.html
  ESP32 Sleep Modes (IDF) - 20/04/2026 -
    https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
  TinyGPS++ library - 20/04/2026 -
    https://github.com/mikalhart/TinyGPSPlus
  ESP32 WiFi Arduino API - 27/04/2026 -
    https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html
  ESP32 WiFiMulti - 27/04/2026 -
    https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifimulti.html
  Google Apps Script Web App - 27/04/2026 -
    https://developers.google.com/apps-script/guides/web
  ESP32 HTTPClient - 27/04/2026 -
    https://docs.espressif.com/projects/arduino-esp32/en/latest/api/httpclient.html
  ESP32 NTP / configTime - 27/04/2026 -
    https://docs.espressif.com/projects/arduino-esp32/en/latest/api/time.html
  Claude Code hulp met debugging en code structuur - https://claude.ai -
  06/05/2026
*/

/*
  Er zijn tests geschreven in ./test/test_logica/test_main.cpp
  Deze kan je runnen met het commando:
  & "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" test -e native

*/
/*
  VERWERKING KLASSIKALE FEEDBACK (deel 1)
  - Structuur herwerkt naar vereiste opbouw
  - true/false vervangen door Enum voor pompstatus
  - Bounce2 toegevoegd voor debouncing van de knop
*/

#include "Arduino.h"

// Stel het minimale logniveau in vóór de include, anders gebruikt
// SerialDebug de standaard (WARNING).
#define DEBUG_INITIAL_LEVEL DEBUG_LEVEL_VERBOSE
#define ARDUINOTRACE_ENABLE 0
#include <SerialDebug.h>

#ifdef MOCK_SENSORS
#include <ArduinoTrace.h>
#endif

#include <HTTPClient.h>
#include <TinyGPSPlus.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <esp_sleep.h>
#include <time.h>

#include "config.h"
#include "logica.h"
#include "secrets.h"

// ── WiFi globalen
// WiFiMulti houdt een lijst van netwerken bij en kiest bij run() automatisch
// het beschikbare netwerk. Zo is er geen handmatige omschakeling nodig tussen
// thuis en campus.
WiFiMulti wifiMulti;

// ── GPS globalen
// HardwareSerial poort 1 (UART1): GPIO16=RX, GPIO17=TX. Alleen aangemaakt bij
// echte hardware; in mock-modus is er geen fysieke poort.
#ifndef MOCK_SENSORS
HardwareSerial gpsSerial(1);
#endif

TinyGPSPlus gps;

// RTC-geheugen: bewaard tijdens deep sleep, gereset bij harde reset/power-on.
// Slaat de laatste bekende GPS-positie op zodat die beschikbaar blijft als er
// bij een volgende wakeup geen nieuwe fix verkregen wordt.
RTC_DATA_ATTR double rtcLaatsteLatitude = 0.0;
RTC_DATA_ATTR double rtcLaatsteLongitude = 0.0;
RTC_DATA_ATTR bool rtcHeeftGpsFix = false;
RTC_DATA_ATTR int rtcLaatsteCap = 0;
RTC_DATA_ATTR int rtcLaatsteRes = 0;
RTC_DATA_ATTR int rtcLaatsteTemp = 0;
RTC_DATA_ATTR Vochtigheid rtcLaatsteCategorie = NAT;

// ── Meetresultaten
// Gevuld door de
// sensorleesfuncties, uitgelezen door stuurNaarGoogleSheets(). Deep sleep reset
// het RAM bij elke wakeup, dus de beginwaarden zijn altijd 0.
int meting_temp = 0;
int meting_cap_bvh = 0;
int meting_res_bvh = 0;
Vochtigheid meting_categorie = NAT;
int meting_pomp_duur_s = 0;          // 0 = geen water gegeven
char meting_datum_tijdstip[22] = ""; // "DD-MM-YYYY_HH:MM:SS"

// ── Loop-state (per wakeup, automatisch gereset door deep sleep)
bool positieGewijzigd = false; // true als nieuwe fix >50 m van vorige fix
esp_sleep_wakeup_cause_t wakeup_reden;
PompStatus pompStatus = GEEN_WATER_GEVEN;
unsigned long starttijd_waterpomp_ms = 0;
int duurtijd_waterpomp_ms = 0;
bool sensoren_uitgelezen = false;
bool gps_uitgelezen = false;
bool data_stap_klaar = false;
unsigned long wifi_start_ms = 0;

// ── Forward declarations
void verbindMetWifi();
void verbreekWifi();
void haalNTPTijdOp();
String bouwSheetsUrl();
void stuurNaarGoogleSheets();
int leesTemperatuur();
int leesCapacitieveBVHSensor();
int leesResistieveBVHSensor();
void leesGPS();
void leesSensorsEnBeslisWatering();
void zetWaterpompUit();
void gaInDeepSleep();

// ═════════════════════════════════════════════════════════════════════════════
// WIFI
// ═════════════════════════════════════════════════════════════════════════════

/**
 * Registreer thuis- en campusnetwerk bij WiFiMulti en wacht op verbinding.
 *
 * WiFiMulti scant alle beschikbare netwerken en verbindt met het netwerk
 * met het sterkste signaal dat in de lijst staat. Zo is er geen handmatige
 * omschakeling nodig: beide netwerken staan altijd ingesteld.
 *
 * Busywait zonder delay() — consistent met de rest van de codebase.
 * Geeft true terug bij verbinding, false bij timeout.
 */
void verbindMetWifi() {
  wifiMulti.addAP(WIFI_SSID_THUIS, WIFI_PASSWORD_THUIS);
  wifiMulti.addAP(WIFI_SSID_CAMPUS, WIFI_PASSWORD_CAMPUS);
  wifiMulti.addAP(WIFI_SSID_WERK, WIFI_PASSWORD_WERK);
  wifiMulti.run();
  debugI("WiFi: verbinding gestart");
}

/**
 * Verbreek de WiFi-verbinding voor het ingaan van deep sleep.
 * Expliciete disconnect is netter dan de radio gewoon te laten uitvallen;
 * de AP logt een correcte disconnect i.p.v. een time-out.
 */
void verbreekWifi() {
  WiFi.disconnect(true);
  debugI("WiFi verbroken");
}

// ═════════════════════════════════════════════════════════════════════════════
// SENSOR UITLEZEN
// ═════════════════════════════════════════════════════════════════════════════

/**
 * Bepaal de temperatuur via de LM35 analoge sensor.
 * Conversie: 10 mV per °C (zie LM35 sensor manual in BRONNEN).
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
 * Lees de ruwe ADC-waarde van de capacitieve bodemvochtigheidssensor.
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
 * Lees de ruwe ADC-waarde van de resistieve bodemvochtigheidssensor.
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

/**
 * Lees de GPS-sensor en sla de coördinaten op in RTC-geheugen.
 * Bij geen fix binnen GPS_READ_TIMEOUT_MS wordt de vorige positie bewaard.
 */
void leesGPS() {
#ifdef MOCK_SENSORS
  TRACE();
  rtcLaatsteLatitude = MOCK_GPS_LAT;
  rtcLaatsteLongitude = MOCK_GPS_LNG;
  rtcHeeftGpsFix = MOCK_GPS_FIX;
  debugI("Mock GPS: lat=%.6f, lng=%.6f", rtcLaatsteLatitude,
         rtcLaatsteLongitude);
#else
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  debugI("GPS: wacht op fix (max %d ms)", GPS_READ_TIMEOUT_MS);

  unsigned long start = millis();
  while ((long)(millis() - start) < GPS_READ_TIMEOUT_MS) {
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }
    if (gps.location.isUpdated() && gps.satellites.isValid()) {
      break;
    }
  }

  debugV("GPS: chars=%lu, zinnen=%lu, fouten=%lu", gps.charsProcessed(),
         gps.sentencesWithFix(), gps.failedChecksum());

  if (gps.location.isValid()) {
    if (rtcHeeftGpsFix) {
      double afstand =
          berekenAfstandMeter(rtcLaatsteLatitude, rtcLaatsteLongitude,
                              gps.location.lat(), gps.location.lng());
      positieGewijzigd = afstand > GPS_POSITIE_WIJZIGING_DREMPEL_M;
      debugI("GPS: afstand t.o.v. vorige fix: %.1f m%s", afstand,
             positieGewijzigd ? " — POSITIE GEWIJZIGD" : "");
    }
    rtcLaatsteLatitude = gps.location.lat();
    rtcLaatsteLongitude = gps.location.lng();
    rtcHeeftGpsFix = true;
    debugI("GPS fix: lat=%.6f, lng=%.6f, satellieten=%d, HDOP=%.1f",
           rtcLaatsteLatitude, rtcLaatsteLongitude, gps.satellites.value(),
           gps.hdop.hdop());
  } else {
    rtcHeeftGpsFix = false;
    debugW("GPS: geen fix binnen timeout (satellieten=%d) — laatste bekende "
           "positie bewaard",
           gps.satellites.value());
    debugV("GPS: laatste positie: lat=%.6f, lng=%.6f", rtcLaatsteLatitude,
           rtcLaatsteLongitude);
  }
#endif
}

// ═════════════════════════════════════════════════════════════════════════════
// POMPBESTURING
// ═════════════════════════════════════════════════════════════════════════════

/**
 * Zet de waterpomp onmiddellijk uit en stel de pompstatus in op
 * GEEN_WATER_GEVEN.
 */
void zetWaterpompUit() {
  debugI("Pomp UIT");
  digitalWrite(PUMP_RELAY_PIN, LOW);
}

// ═════════════════════════════════════════════════════════════════════════════
// SENSOR + WATER BESLISSING
// ═════════════════════════════════════════════════════════════════════════════

/**
 * Lees alle sensoren, bepaal de vochtigheid en temperatuur, en start de
 * waterpomp non-blocking als de grond droog is.
 *
 * Non-blocking: de pomp wordt gestart (GPIO HIGH + starttijd vastgelegd) maar
 * niet hier gewacht. De pompstatus wordt afgehandeld in loop() via de
 * pomp-uitzet-check (stap 3).
 */
void leesSensorsEnBeslisWatering() {
  int capacitieve_bvh_waarde = leesCapacitieveBVHSensor();
  int resistieve_bvh_waarde = leesResistieveBVHSensor();
  int temperatuur = leesTemperatuur();

  Vochtigheid categorieCapacitief =
      berekenCategorieCapactieveBHV(capacitieve_bvh_waarde);
  Vochtigheid categorieResistief =
      berekenCategorieResistieveBVH(resistieve_bvh_waarde);
  Vochtigheid categorie =
      berekenSamengesteldeCategorie(categorieCapacitief, categorieResistief);

  meting_temp = temperatuur;
  meting_cap_bvh = capacitieve_bvh_waarde;
  meting_res_bvh = resistieve_bvh_waarde;
  meting_categorie = categorie;

  rtcLaatsteTemp      = meting_temp;
  rtcLaatsteCap       = meting_cap_bvh;
  rtcLaatsteRes       = meting_res_bvh;
  rtcLaatsteCategorie = meting_categorie;

  debugV("capacitiveValue = %5d | categorie: %s", capacitieve_bvh_waarde,
         vochtigheidsNiveauNaarString(categorieCapacitief));
  debugV("resistiveValue  = %5d | categorie: %s", resistieve_bvh_waarde,
         vochtigheidsNiveauNaarString(categorieResistief));
  debugV("temperatuur = %d C", temperatuur);

  int duur = berekenPompDuurMs(categorie, temperatuur);
  if (duur == 0) {
    debugI("Grond is %s — geen water nodig",
           vochtigheidsNiveauNaarString(categorie));
    return;
  }

  debugI("Grond is DROOG — pomp AAN voor %d ms", duur);
  duurtijd_waterpomp_ms = duur;
  meting_pomp_duur_s = duur / 1000;
  starttijd_waterpomp_ms = millis();
  pompStatus = WATER_GEVEN;
  digitalWrite(PUMP_RELAY_PIN, HIGH);
}

// ═════════════════════════════════════════════════════════════════════════════
// DATA DOORSTUREN
// ═════════════════════════════════════════════════════════════════════════════

/**
 * Haal de huidige datum en tijd op via NTP en sla ze gecombineerd op in
 * meting_datum_tijdstip voor gebruik in de Sheets-URL.
 *
 * configTime() stelt de interne systeemklok in via SNTP.
 * getLocalTime() wacht tot de klok gesynchroniseerd is (max NTP_TIMEOUT_MS).
 * België: UTC+1 (CET) + 1 h DST (CEST) → offsets in config.h.
 */
void haalNTPTijdOp() {
  configTime(NTP_TIJDZONE_OFFSET_S, NTP_ZOMERTIJD_OFFSET_S, NTP_SERVER);
  struct tm tijdinfo;
  if (!getLocalTime(&tijdinfo, NTP_TIMEOUT_MS)) {
    debugW("NTP: geen tijdsynchronisatie binnen %d ms", NTP_TIMEOUT_MS);
    strncpy(meting_datum_tijdstip, "00-00-0000 00:00:00",
            sizeof(meting_datum_tijdstip));
    return;
  }
  strftime(meting_datum_tijdstip, sizeof(meting_datum_tijdstip),
           "%d-%m-%Y %H:%M:%S", &tijdinfo);
  debugI("NTP: %s", meting_datum_tijdstip);
}

/**
 * Bouw de Google Apps Script URL met alle meetwaarden als query-parameters.
 *
 * Verwachte Apps Script doGet(e)-parameters:
 *   datum_tijdstip, temperatuur, cap_bvh, res_bvh, categorie,
 *   water_s, lat, lng, gps_fix
 *
 * Kolons in datum_tijdstip worden vervangen door %3A (URL-codering).
 */
String bouwSheetsUrl() {
  String datum_tijdstipEncoded = String(meting_datum_tijdstip);
  datum_tijdstipEncoded.replace(" ", "%20");
  datum_tijdstipEncoded.replace(":", "%3A");

  String url = "https://script.google.com/macros/s/";
  url += GOOGLE_SCRIPT_DEPLOYMENT_ID;
  url += "/exec";
  url += "?datum_tijdstip=" + datum_tijdstipEncoded;
  url += "&temperatuur=" + String(meting_temp);
  url += "&cap_bvh=" + String(meting_cap_bvh);
  url += "&res_bvh=" + String(meting_res_bvh);
  url += "&categorie=";
  url += vochtigheidsNiveauNaarString(meting_categorie);
  url += "&water_s=" + String(meting_pomp_duur_s);
  url += "&lat=" + String(rtcLaatsteLatitude, 6);
  url += "&lng=" + String(rtcLaatsteLongitude, 6);
  url += "&gps_fix=" + String(rtcHeeftGpsFix ? 1 : 0);
  url += "&positie_gewijzigd=" + String(positieGewijzigd ? 1 : 0);
  return url;
}

/**
 * Stuur de meetgegevens naar Google Sheets via HTTPS GET.
 */
void stuurNaarGoogleSheets() {
  String url = bouwSheetsUrl();
  debugI("Sheets: stuur naar %s", url.c_str());

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String body = http.getString();
    debugI("Sheets: rij toegevoegd (HTTP %d) — respons: %s", httpCode,
           body.c_str());
  } else {
    String body = http.getString();
    debugE("Sheets: fout (HTTP %d) — respons: %s", httpCode, body.c_str());
  }
  http.end();
}

// ═════════════════════════════════════════════════════════════════════════════
// DEEP SLEEP
// ═════════════════════════════════════════════════════════════════════════════

/**
 * Configureer de wakeup-bronnen en start deep sleep.
 *
 *  - Timer:  DEEP_SLEEP_DUUR_US microseconden (= 60 s → 1 meting per minuut).
 *  - EXT0:   WAKEUP_BUTTON_PIN op LOW-niveau → handmatige override.
 */
void gaInDeepSleep() {
  verbreekWifi();
  debugI("Deep sleep voor %d s", DEEP_SLEEP_DUUR_S);

  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DUUR_US);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKEUP_BUTTON_PIN, 0);

  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  debugSetLevel(DEBUG_LEVEL_VERBOSE);

  // Zorg dat de pomp UIT staat bij elke wakeup (veiligheid: voorkomt dat de
  // pomp aan blijft als de ESP32 onverwacht reset tijdens het pompen)
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY_PIN, LOW);

  // INPUT_PULLUP: knop is active-low (verbinding naar GND bij indrukken)
  pinMode(WAKEUP_BUTTON_PIN, INPUT_PULLUP);

  // 0 dB attenuatie voor de temperatuursensor: bereik 0–1.1 V (LM35 max ~1 V)
  analogSetPinAttenuation(TEMP_SENSOR, ADC_0db);

  wakeup_reden = esp_sleep_get_wakeup_cause();
  debugI("Wakeup: oorzaak=%d", (int)wakeup_reden);

  // ── Bij knopwakeup: herstel laatste sensorwaarden uit RTC-geheugen ──────────
  // De sensoren worden bij een knopdrukvering niet uitgelezen (opdrachtvereiste:
  // de pomp draait onmiddellijk zonder sensorcheck). Zonder dit herstel zouden
  // meting_cap_bvh / meting_res_bvh / meting_temp als 0 naar Sheets en Grafana
  // gaan, waardoor het dashboard plotse nulwaarden toont die niet overeenkomen
  // met de werkelijke grondtoestand en grafieken vervormen.
  if (wakeup_reden == ESP_SLEEP_WAKEUP_EXT0) {
    meting_cap_bvh   = rtcLaatsteCap;
    meting_res_bvh   = rtcLaatsteRes;
    meting_temp      = rtcLaatsteTemp;
    meting_categorie = rtcLaatsteCategorie;
  }

  // WiFi-scan gestart; loop() bewaakt de verbinding en timeout.
  verbindMetWifi();
  wifi_start_ms = millis();
}

void loop() {
  // ── 1. Panic button: waterpomp manueel starten ────────────────────────────
  // Enkel bij EXT0-wakeup en als de pomp nog niet loopt (geen herstart).
  if (pompStatus == GEEN_WATER_GEVEN && wakeup_reden == ESP_SLEEP_WAKEUP_EXT0) {
    debugI("Panic button: pomp AAN voor %d ms", WATER_GEEF_DUUR_MANUEEL_MS);
    duurtijd_waterpomp_ms = WATER_GEEF_DUUR_MANUEEL_MS;
    meting_pomp_duur_s =
        WATER_GEEF_DUUR_MANUEEL_MS /
        1000; // voor makkelijk leesbare tekst in de Google Sheets
    starttijd_waterpomp_ms = millis();
    pompStatus = WATER_GEVEN;
    digitalWrite(PUMP_RELAY_PIN, HIGH);
  }

  // ── 2. GPS uitlezen (1x per wakeup, vóór pompaansturing) ────────────────
  // GPS wordt uitgelezen vóór de sensoren zodat de blocking fix-wacht
  // gegarandeerd plaatsvindt terwijl de pomp nog UIT is.
  if (!gps_uitgelezen) {
    leesGPS();
    gps_uitgelezen = true;
  }

  // ── 3. Sensoren uitlezen en watering beslissen (enkel bij timer-wakeup) ───
  // Sensorwaarden worden slechts 1x per wakeup uitgelezen
  // (sensoren_uitgelezen). ESP_SLEEP_WAKEUP_UNDEFINED = eerste opstart:
  // behandeld als timer-wakeup.
  if (!sensoren_uitgelezen && pompStatus == GEEN_WATER_GEVEN &&
      wakeup_reden != ESP_SLEEP_WAKEUP_EXT0) {
    leesSensorsEnBeslisWatering();
    sensoren_uitgelezen = true;
  }

  // ── 4. Waterpomp uitzetten als de duurtijd verstreken is ──────────────────
  if (pompStatus == WATER_GEVEN &&
      (long)(millis() - starttijd_waterpomp_ms) >= duurtijd_waterpomp_ms) {
    zetWaterpompUit();
    pompStatus = GEEN_WATER_GEVEN;
  }

  // ── 5. Data doorsturen naar Google Sheets (1x per wakeup, vereist WiFi) ──
  if (!data_stap_klaar) {
    if (WiFi.status() == WL_CONNECTED) {
      debugI("WiFi verbonden: SSID=%s, IP=%s", WiFi.SSID().c_str(),
             WiFi.localIP().toString().c_str());
      haalNTPTijdOp();
      stuurNaarGoogleSheets();
      data_stap_klaar = true;
    } else if ((long)(millis() - wifi_start_ms) >= WIFI_VERBINDING_TIMEOUT_MS) {
      debugE("WiFi: geen verbinding na %d ms — data niet doorgestuurd",
             WIFI_VERBINDING_TIMEOUT_MS);
      data_stap_klaar = true;
    } else {
      wifiMulti.run();
    }
  }

  // ── 6. Deep sleep als de pomp niet actief is én data is doorgestuurd ────
  // data_stap_klaar wordt ook true gezet bij WiFi-timeout, zodat het systeem
  // nooit onbepaald wakker blijft wachten op een verbinding.
  if (pompStatus == GEEN_WATER_GEVEN && data_stap_klaar) {
    gaInDeepSleep();
  }
}
