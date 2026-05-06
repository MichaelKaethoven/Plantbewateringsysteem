#pragma once
#include "config.h"

Vochtigheid berekenCategorieCapactieveBHV(int sensorwaarde);
Vochtigheid berekenCategorieResistieveBVH(int sensorwaarde);
Vochtigheid berekenSamengesteldeCategorie(Vochtigheid cap, Vochtigheid res);
const char *vochtigheidsNiveauNaarString(Vochtigheid niveau);
int berekenPompDuurMs(Vochtigheid vochtigheid, int temperatuurC);
