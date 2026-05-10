#include "logica.h"
#include <cmath>

Vochtigheid berekenCategorieCapactieveBHV(int sensorwaarde) {
  if (sensorwaarde >= CAPACITIEVE_SENSOR_DROOG_INTERVAL_MIN &&
      sensorwaarde <= CAPACITIEVE_SENSOR_DROOG_INTERVAL_MAX) {
    return DROOG;
  }
  if (sensorwaarde >= CAPACITIEVE_SENSOR_VOCHTIG_INTERVAL_MIN &&
      sensorwaarde <= CAPACITIEVE_SENSOR_VOCHTIG_INTERVAL_MAX) {
    return VOCHTIG;
  }
  return NAT;
}

Vochtigheid berekenCategorieResistieveBVH(int sensorwaarde) {
  if (sensorwaarde >= RESISTIEVE_SENSOR_DROOG_INTERVAL_MIN &&
      sensorwaarde <= RESISTIEVE_SENSOR_DROOG_INTERVAL_MAX) {
    return DROOG;
  }
  if (sensorwaarde >= RESISTIEVE_SENSOR_VOCHTIG_INTERVAL_MIN &&
      sensorwaarde <= RESISTIEVE_SENSOR_VOCHTIG_INTERVAL_MAX) {
    return VOCHTIG;
  }
  return NAT;
}

Vochtigheid berekenSamengesteldeCategorie(Vochtigheid cap, Vochtigheid res) {
  if (cap == DROOG || res == DROOG) {
    return DROOG;
  }
  if (cap == VOCHTIG || res == VOCHTIG) {
    return VOCHTIG;
  }
  return NAT;
}

const char *vochtigheidsNiveauNaarString(Vochtigheid niveau) {
  switch (niveau) {
  case NAT:
    return "NAT";
  case VOCHTIG:
    return "VOCHTIG";
  case DROOG:
    return "DROOG";
  }
  return "ONBEKEND";
}

/*
Code geschreven met Claude Code om afstand te berekenen t.o.v. vorige fix
*/
double berekenAfstandMeter(double lat1, double lng1, double lat2, double lng2) {
  const double R = 6371000.0;
  double dLat = (lat2 - lat1) * M_PI / 180.0;
  double dLng = (lng2 - lng1) * M_PI / 180.0;
  double a = sin(dLat / 2) * sin(dLat / 2) + cos(lat1 * M_PI / 180.0) *
                                                 cos(lat2 * M_PI / 180.0) *
                                                 sin(dLng / 2) * sin(dLng / 2);
  return R * 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
}

int berekenPompDuurMs(Vochtigheid vochtigheid, int temperatuurC) {
  if (vochtigheid != DROOG) {
    return 0;
  }
  if (temperatuurC > TEMPERATUUR_NORMAAL_GRENS_C) {
    return WATER_GEEF_DUUR_NORMAAL_MS;
  }
  if (temperatuurC > TEMPERATUUR_TE_KOUD_GRENS_C) {
    return WATER_GEEF_DUUR_KORT_MS;
  }
  return WATER_GEEF_DUUR_KOUD_MS;
}
