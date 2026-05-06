#include <unity.h>
#include "logica.h"

void setUp(void) {}
void tearDown(void) {}

// ── berekenSamengesteldeCategorie ──────────────────────────────────────────

void test_samengesteld_beide_droog_geeft_droog(void) {
  TEST_ASSERT_EQUAL(DROOG, berekenSamengesteldeCategorie(DROOG, DROOG));
}

void test_samengesteld_droog_en_nat_geeft_droog(void) {
  TEST_ASSERT_EQUAL(DROOG, berekenSamengesteldeCategorie(DROOG, NAT));
  TEST_ASSERT_EQUAL(DROOG, berekenSamengesteldeCategorie(NAT, DROOG));
}

void test_samengesteld_droog_en_vochtig_geeft_droog(void) {
  TEST_ASSERT_EQUAL(DROOG, berekenSamengesteldeCategorie(DROOG, VOCHTIG));
  TEST_ASSERT_EQUAL(DROOG, berekenSamengesteldeCategorie(VOCHTIG, DROOG));
}

void test_samengesteld_vochtig_en_nat_geeft_vochtig(void) {
  TEST_ASSERT_EQUAL(VOCHTIG, berekenSamengesteldeCategorie(VOCHTIG, NAT));
  TEST_ASSERT_EQUAL(VOCHTIG, berekenSamengesteldeCategorie(NAT, VOCHTIG));
}

void test_samengesteld_beide_vochtig_geeft_vochtig(void) {
  TEST_ASSERT_EQUAL(VOCHTIG, berekenSamengesteldeCategorie(VOCHTIG, VOCHTIG));
}

void test_samengesteld_beide_nat_geeft_nat(void) {
  TEST_ASSERT_EQUAL(NAT, berekenSamengesteldeCategorie(NAT, NAT));
}

// ── berekenCategorieCapactieveBHV ──────────────────────────────────────────
// DROOG: 2800–4095 | VOCHTIG: 2000–2799 | NAT: 0–1999

void test_capacitief_droog_ondergrens(void) {
  TEST_ASSERT_EQUAL(DROOG, berekenCategorieCapactieveBHV(2800));
}

void test_capacitief_droog_bovengrens(void) {
  TEST_ASSERT_EQUAL(DROOG, berekenCategorieCapactieveBHV(4095));
}

void test_capacitief_droog_midden(void) {
  TEST_ASSERT_EQUAL(DROOG, berekenCategorieCapactieveBHV(2900));
}

void test_capacitief_vochtig_ondergrens(void) {
  TEST_ASSERT_EQUAL(VOCHTIG, berekenCategorieCapactieveBHV(2000));
}

void test_capacitief_vochtig_bovengrens(void) {
  TEST_ASSERT_EQUAL(VOCHTIG, berekenCategorieCapactieveBHV(2799));
}

void test_capacitief_vochtig_midden(void) {
  TEST_ASSERT_EQUAL(VOCHTIG, berekenCategorieCapactieveBHV(2400));
}

void test_capacitief_nat_bovengrens(void) {
  TEST_ASSERT_EQUAL(NAT, berekenCategorieCapactieveBHV(1999));
}

void test_capacitief_nat_midden(void) {
  TEST_ASSERT_EQUAL(NAT, berekenCategorieCapactieveBHV(1000));
}

// ── berekenCategorieResistieveBVH ──────────────────────────────────────────
// DROOG: 0–376 | VOCHTIG: 377–743 | NAT: 744–4095

void test_resistief_droog_ondergrens(void) {
  TEST_ASSERT_EQUAL(DROOG, berekenCategorieResistieveBVH(0));
}

void test_resistief_droog_bovengrens(void) {
  TEST_ASSERT_EQUAL(DROOG, berekenCategorieResistieveBVH(376));
}

void test_resistief_droog_midden(void) {
  TEST_ASSERT_EQUAL(DROOG, berekenCategorieResistieveBVH(188));
}

void test_resistief_vochtig_ondergrens(void) {
  TEST_ASSERT_EQUAL(VOCHTIG, berekenCategorieResistieveBVH(377));
}

void test_resistief_vochtig_bovengrens(void) {
  TEST_ASSERT_EQUAL(VOCHTIG, berekenCategorieResistieveBVH(743));
}

void test_resistief_vochtig_midden(void) {
  TEST_ASSERT_EQUAL(VOCHTIG, berekenCategorieResistieveBVH(560));
}

void test_resistief_nat_ondergrens(void) {
  TEST_ASSERT_EQUAL(NAT, berekenCategorieResistieveBVH(744));
}

void test_resistief_nat_midden(void) {
  TEST_ASSERT_EQUAL(NAT, berekenCategorieResistieveBVH(870));
}

// ── berekenPompDuurMs ──────────────────────────────────────────────────────

void test_pomp_uit_bij_vochtig(void) {
  TEST_ASSERT_EQUAL(0, berekenPompDuurMs(VOCHTIG, 30));
}

void test_pomp_uit_bij_nat(void) {
  TEST_ASSERT_EQUAL(0, berekenPompDuurMs(NAT, 30));
}

void test_pomp_koud_bij_droog_en_erg_koud(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KOUD_MS, berekenPompDuurMs(DROOG, 3));
}

void test_pomp_koud_op_grenstemperatuur(void) {
  // temp == TEMPERATUUR_TE_KOUD_GRENS_C (5) → valt in de koud-tak (niet >)
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KOUD_MS, berekenPompDuurMs(DROOG, 5));
}

void test_pomp_kort_net_boven_koude_grens(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KORT_MS, berekenPompDuurMs(DROOG, 6));
}

void test_pomp_kort_bij_droog_en_koel(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KORT_MS, berekenPompDuurMs(DROOG, 15));
}

void test_pomp_kort_op_normaalgrens(void) {
  // temp == TEMPERATUUR_NORMAAL_GRENS_C (25) → valt in de kort-tak (niet >)
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KORT_MS, berekenPompDuurMs(DROOG, 25));
}

void test_pomp_normaal_net_boven_normaalgrens(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_NORMAAL_MS, berekenPompDuurMs(DROOG, 26));
}

void test_pomp_normaal_bij_droog_en_warm(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_NORMAAL_MS, berekenPompDuurMs(DROOG, 30));
}

// ── 27 scenario-integratie tests ──────────────────────────────────────────

#define CAP_DROOG_VAL   2900
#define CAP_VOCHTIG_VAL 2400
#define CAP_NAT_VAL     1800
#define RES_DROOG_VAL   188
#define RES_VOCHTIG_VAL 560
#define RES_NAT_VAL     870
#define TEMP_LAAG_C     3
#define TEMP_NORM_C     15
#define TEMP_HOOG_C     30

static int pompDuurVanRuweWaarden(int capRaw, int resRaw, int tempC) {
  Vochtigheid cap = berekenCategorieCapactieveBHV(capRaw);
  Vochtigheid res = berekenCategorieResistieveBVH(resRaw);
  Vochtigheid samengesteld = berekenSamengesteldeCategorie(cap, res);
  return berekenPompDuurMs(samengesteld, tempC);
}

// ── cap DROOG ──────────────────────────────────────────────────────────────
void test_scenario_cap_droog_res_droog_temp_laag(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KOUD_MS, pompDuurVanRuweWaarden(CAP_DROOG_VAL, RES_DROOG_VAL, TEMP_LAAG_C));
}
void test_scenario_cap_droog_res_droog_temp_normaal(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KORT_MS, pompDuurVanRuweWaarden(CAP_DROOG_VAL, RES_DROOG_VAL, TEMP_NORM_C));
}
void test_scenario_cap_droog_res_droog_temp_hoog(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_NORMAAL_MS, pompDuurVanRuweWaarden(CAP_DROOG_VAL, RES_DROOG_VAL, TEMP_HOOG_C));
}

void test_scenario_cap_droog_res_vochtig_temp_laag(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KOUD_MS, pompDuurVanRuweWaarden(CAP_DROOG_VAL, RES_VOCHTIG_VAL, TEMP_LAAG_C));
}
void test_scenario_cap_droog_res_vochtig_temp_normaal(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KORT_MS, pompDuurVanRuweWaarden(CAP_DROOG_VAL, RES_VOCHTIG_VAL, TEMP_NORM_C));
}
void test_scenario_cap_droog_res_vochtig_temp_hoog(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_NORMAAL_MS, pompDuurVanRuweWaarden(CAP_DROOG_VAL, RES_VOCHTIG_VAL, TEMP_HOOG_C));
}

void test_scenario_cap_droog_res_nat_temp_laag(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KOUD_MS, pompDuurVanRuweWaarden(CAP_DROOG_VAL, RES_NAT_VAL, TEMP_LAAG_C));
}
void test_scenario_cap_droog_res_nat_temp_normaal(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KORT_MS, pompDuurVanRuweWaarden(CAP_DROOG_VAL, RES_NAT_VAL, TEMP_NORM_C));
}
void test_scenario_cap_droog_res_nat_temp_hoog(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_NORMAAL_MS, pompDuurVanRuweWaarden(CAP_DROOG_VAL, RES_NAT_VAL, TEMP_HOOG_C));
}

// ── cap VOCHTIG ────────────────────────────────────────────────────────────
void test_scenario_cap_vochtig_res_droog_temp_laag(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KOUD_MS, pompDuurVanRuweWaarden(CAP_VOCHTIG_VAL, RES_DROOG_VAL, TEMP_LAAG_C));
}
void test_scenario_cap_vochtig_res_droog_temp_normaal(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KORT_MS, pompDuurVanRuweWaarden(CAP_VOCHTIG_VAL, RES_DROOG_VAL, TEMP_NORM_C));
}
void test_scenario_cap_vochtig_res_droog_temp_hoog(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_NORMAAL_MS, pompDuurVanRuweWaarden(CAP_VOCHTIG_VAL, RES_DROOG_VAL, TEMP_HOOG_C));
}

void test_scenario_cap_vochtig_res_vochtig_temp_laag(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_VOCHTIG_VAL, RES_VOCHTIG_VAL, TEMP_LAAG_C));
}
void test_scenario_cap_vochtig_res_vochtig_temp_normaal(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_VOCHTIG_VAL, RES_VOCHTIG_VAL, TEMP_NORM_C));
}
void test_scenario_cap_vochtig_res_vochtig_temp_hoog(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_VOCHTIG_VAL, RES_VOCHTIG_VAL, TEMP_HOOG_C));
}

void test_scenario_cap_vochtig_res_nat_temp_laag(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_VOCHTIG_VAL, RES_NAT_VAL, TEMP_LAAG_C));
}
void test_scenario_cap_vochtig_res_nat_temp_normaal(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_VOCHTIG_VAL, RES_NAT_VAL, TEMP_NORM_C));
}
void test_scenario_cap_vochtig_res_nat_temp_hoog(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_VOCHTIG_VAL, RES_NAT_VAL, TEMP_HOOG_C));
}

// ── cap NAT ────────────────────────────────────────────────────────────────
void test_scenario_cap_nat_res_droog_temp_laag(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KOUD_MS, pompDuurVanRuweWaarden(CAP_NAT_VAL, RES_DROOG_VAL, TEMP_LAAG_C));
}
void test_scenario_cap_nat_res_droog_temp_normaal(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_KORT_MS, pompDuurVanRuweWaarden(CAP_NAT_VAL, RES_DROOG_VAL, TEMP_NORM_C));
}
void test_scenario_cap_nat_res_droog_temp_hoog(void) {
  TEST_ASSERT_EQUAL(WATER_GEEF_DUUR_NORMAAL_MS, pompDuurVanRuweWaarden(CAP_NAT_VAL, RES_DROOG_VAL, TEMP_HOOG_C));
}

void test_scenario_cap_nat_res_vochtig_temp_laag(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_NAT_VAL, RES_VOCHTIG_VAL, TEMP_LAAG_C));
}
void test_scenario_cap_nat_res_vochtig_temp_normaal(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_NAT_VAL, RES_VOCHTIG_VAL, TEMP_NORM_C));
}
void test_scenario_cap_nat_res_vochtig_temp_hoog(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_NAT_VAL, RES_VOCHTIG_VAL, TEMP_HOOG_C));
}

void test_scenario_cap_nat_res_nat_temp_laag(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_NAT_VAL, RES_NAT_VAL, TEMP_LAAG_C));
}
void test_scenario_cap_nat_res_nat_temp_normaal(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_NAT_VAL, RES_NAT_VAL, TEMP_NORM_C));
}
void test_scenario_cap_nat_res_nat_temp_hoog(void) {
  TEST_ASSERT_EQUAL(0, pompDuurVanRuweWaarden(CAP_NAT_VAL, RES_NAT_VAL, TEMP_HOOG_C));
}

// ── vochtigheidsNiveauNaarString ───────────────────────────────────────────

void test_naar_string_nat(void) {
  TEST_ASSERT_EQUAL_STRING("NAT", vochtigheidsNiveauNaarString(NAT));
}

void test_naar_string_vochtig(void) {
  TEST_ASSERT_EQUAL_STRING("VOCHTIG", vochtigheidsNiveauNaarString(VOCHTIG));
}

void test_naar_string_droog(void) {
  TEST_ASSERT_EQUAL_STRING("DROOG", vochtigheidsNiveauNaarString(DROOG));
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_samengesteld_beide_droog_geeft_droog);
  RUN_TEST(test_samengesteld_droog_en_nat_geeft_droog);
  RUN_TEST(test_samengesteld_droog_en_vochtig_geeft_droog);
  RUN_TEST(test_samengesteld_vochtig_en_nat_geeft_vochtig);
  RUN_TEST(test_samengesteld_beide_vochtig_geeft_vochtig);
  RUN_TEST(test_samengesteld_beide_nat_geeft_nat);

  RUN_TEST(test_capacitief_droog_ondergrens);
  RUN_TEST(test_capacitief_droog_bovengrens);
  RUN_TEST(test_capacitief_droog_midden);
  RUN_TEST(test_capacitief_vochtig_ondergrens);
  RUN_TEST(test_capacitief_vochtig_bovengrens);
  RUN_TEST(test_capacitief_vochtig_midden);
  RUN_TEST(test_capacitief_nat_bovengrens);
  RUN_TEST(test_capacitief_nat_midden);

  RUN_TEST(test_resistief_droog_ondergrens);
  RUN_TEST(test_resistief_droog_bovengrens);
  RUN_TEST(test_resistief_droog_midden);
  RUN_TEST(test_resistief_vochtig_ondergrens);
  RUN_TEST(test_resistief_vochtig_bovengrens);
  RUN_TEST(test_resistief_vochtig_midden);
  RUN_TEST(test_resistief_nat_ondergrens);
  RUN_TEST(test_resistief_nat_midden);

  RUN_TEST(test_pomp_uit_bij_vochtig);
  RUN_TEST(test_pomp_uit_bij_nat);
  RUN_TEST(test_pomp_koud_bij_droog_en_erg_koud);
  RUN_TEST(test_pomp_koud_op_grenstemperatuur);
  RUN_TEST(test_pomp_kort_net_boven_koude_grens);
  RUN_TEST(test_pomp_kort_bij_droog_en_koel);
  RUN_TEST(test_pomp_kort_op_normaalgrens);
  RUN_TEST(test_pomp_normaal_net_boven_normaalgrens);
  RUN_TEST(test_pomp_normaal_bij_droog_en_warm);

  RUN_TEST(test_scenario_cap_droog_res_droog_temp_laag);
  RUN_TEST(test_scenario_cap_droog_res_droog_temp_normaal);
  RUN_TEST(test_scenario_cap_droog_res_droog_temp_hoog);
  RUN_TEST(test_scenario_cap_droog_res_vochtig_temp_laag);
  RUN_TEST(test_scenario_cap_droog_res_vochtig_temp_normaal);
  RUN_TEST(test_scenario_cap_droog_res_vochtig_temp_hoog);
  RUN_TEST(test_scenario_cap_droog_res_nat_temp_laag);
  RUN_TEST(test_scenario_cap_droog_res_nat_temp_normaal);
  RUN_TEST(test_scenario_cap_droog_res_nat_temp_hoog);
  RUN_TEST(test_scenario_cap_vochtig_res_droog_temp_laag);
  RUN_TEST(test_scenario_cap_vochtig_res_droog_temp_normaal);
  RUN_TEST(test_scenario_cap_vochtig_res_droog_temp_hoog);
  RUN_TEST(test_scenario_cap_vochtig_res_vochtig_temp_laag);
  RUN_TEST(test_scenario_cap_vochtig_res_vochtig_temp_normaal);
  RUN_TEST(test_scenario_cap_vochtig_res_vochtig_temp_hoog);
  RUN_TEST(test_scenario_cap_vochtig_res_nat_temp_laag);
  RUN_TEST(test_scenario_cap_vochtig_res_nat_temp_normaal);
  RUN_TEST(test_scenario_cap_vochtig_res_nat_temp_hoog);
  RUN_TEST(test_scenario_cap_nat_res_droog_temp_laag);
  RUN_TEST(test_scenario_cap_nat_res_droog_temp_normaal);
  RUN_TEST(test_scenario_cap_nat_res_droog_temp_hoog);
  RUN_TEST(test_scenario_cap_nat_res_vochtig_temp_laag);
  RUN_TEST(test_scenario_cap_nat_res_vochtig_temp_normaal);
  RUN_TEST(test_scenario_cap_nat_res_vochtig_temp_hoog);
  RUN_TEST(test_scenario_cap_nat_res_nat_temp_laag);
  RUN_TEST(test_scenario_cap_nat_res_nat_temp_normaal);
  RUN_TEST(test_scenario_cap_nat_res_nat_temp_hoog);

  RUN_TEST(test_naar_string_nat);
  RUN_TEST(test_naar_string_vochtig);
  RUN_TEST(test_naar_string_droog);

  return UNITY_END();
}
