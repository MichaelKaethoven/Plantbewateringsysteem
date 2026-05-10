/*
  Gebruik:
    WiFiMulti voegt beide netwerken toe en verbindt automatisch
    met het beschikbare netwerk (thuis of campus).
    Geen handmatige omschakeling nodig; beide netwerken kunnen
    gewoon tegelijk ingesteld blijven.

  Bron WiFiMulti:
    https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifimulti.html
*/

#pragma once

const char *WIFI_SSID_THUIS = "jouw_thuis_netwerk";
const char *WIFI_PASSWORD_THUIS = "jouw_thuis_wachtwoord";

const char *WIFI_SSID_CAMPUS = "jouw_campus_netwerk";
const char *WIFI_PASSWORD_CAMPUS = "jouw_campus_wachtwoord";

const char *GOOGLE_SCRIPT_DEPLOYMENT_ID = "jouw_script_deployment_id";
