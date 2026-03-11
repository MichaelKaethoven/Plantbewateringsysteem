#include <unity.h>
#include "logic.h"

void setUp(void) {}
void tearDown(void) {}

// ── selectMoistureLevel ────────────────────────────────────────────────────

void test_select_both_dry_returns_dry(void) {
  TEST_ASSERT_EQUAL(DRY, selectMoistureLevel(DRY, DRY));
}

void test_select_dry_and_wet_returns_dry(void) {
  TEST_ASSERT_EQUAL(DRY, selectMoistureLevel(DRY, WET));
  TEST_ASSERT_EQUAL(DRY, selectMoistureLevel(WET, DRY));
}

void test_select_dry_and_moist_returns_dry(void) {
  TEST_ASSERT_EQUAL(DRY, selectMoistureLevel(DRY, MOIST));
  TEST_ASSERT_EQUAL(DRY, selectMoistureLevel(MOIST, DRY));
}

void test_select_moist_and_wet_returns_moist(void) {
  TEST_ASSERT_EQUAL(MOIST, selectMoistureLevel(MOIST, WET));
  TEST_ASSERT_EQUAL(MOIST, selectMoistureLevel(WET, MOIST));
}

void test_select_both_moist_returns_moist(void) {
  TEST_ASSERT_EQUAL(MOIST, selectMoistureLevel(MOIST, MOIST));
}

void test_select_both_wet_returns_wet(void) {
  TEST_ASSERT_EQUAL(WET, selectMoistureLevel(WET, WET));
}

// ── getCapacitiveMoistureLevel ─────────────────────────────────────────────

void test_capacitive_dry_min_boundary(void) {
  TEST_ASSERT_EQUAL(DRY, getCapacitiveMoistureLevel(2800));
}

void test_capacitive_dry_max_boundary(void) {
  TEST_ASSERT_EQUAL(DRY, getCapacitiveMoistureLevel(3000));
}

void test_capacitive_dry_mid(void) {
  TEST_ASSERT_EQUAL(DRY, getCapacitiveMoistureLevel(2900));
}

void test_capacitive_moist_min_boundary(void) {
  TEST_ASSERT_EQUAL(MOIST, getCapacitiveMoistureLevel(2400));
}

void test_capacitive_moist_max_boundary(void) {
  TEST_ASSERT_EQUAL(MOIST, getCapacitiveMoistureLevel(2799));
}

void test_capacitive_moist_mid(void) {
  TEST_ASSERT_EQUAL(MOIST, getCapacitiveMoistureLevel(2600));
}

void test_capacitive_wet_mid(void) {
  TEST_ASSERT_EQUAL(WET, getCapacitiveMoistureLevel(1800));
}

void test_capacitive_wet_min_boundary(void) {
  TEST_ASSERT_EQUAL(WET, getCapacitiveMoistureLevel(1400));
}

// ── getResistiveMoistureLevel ──────────────────────────────────────────────

void test_resistive_dry_min_boundary(void) {
  TEST_ASSERT_EQUAL(DRY, getResistiveMoistureLevel(0));
}

void test_resistive_dry_max_boundary(void) {
  TEST_ASSERT_EQUAL(DRY, getResistiveMoistureLevel(250));
}

void test_resistive_dry_mid(void) {
  TEST_ASSERT_EQUAL(DRY, getResistiveMoistureLevel(100));
}

void test_resistive_moist_min_boundary(void) {
  TEST_ASSERT_EQUAL(MOIST, getResistiveMoistureLevel(251));
}

void test_resistive_moist_max_boundary(void) {
  TEST_ASSERT_EQUAL(MOIST, getResistiveMoistureLevel(1500));
}

void test_resistive_moist_mid(void) {
  TEST_ASSERT_EQUAL(MOIST, getResistiveMoistureLevel(800));
}

void test_resistive_wet_min_boundary(void) {
  TEST_ASSERT_EQUAL(WET, getResistiveMoistureLevel(1501));
}

void test_resistive_wet_mid(void) {
  TEST_ASSERT_EQUAL(WET, getResistiveMoistureLevel(2000));
}

// ── getPumpDurationMs ──────────────────────────────────────────────────────

// Grond is niet droog → pomp blijft altijd uit
void test_pump_off_when_moist(void) {
  TEST_ASSERT_EQUAL(0, getPumpDurationMs(MOIST, 30.0f));
}

void test_pump_off_when_wet(void) {
  TEST_ASSERT_EQUAL(0, getPumpDurationMs(WET, 30.0f));
}

// Grond is droog maar te koud → geen water geven
void test_pump_off_when_dry_too_cold(void) {
  TEST_ASSERT_EQUAL(0, getPumpDurationMs(DRY, 3.0f));
}

void test_pump_off_at_cold_cutoff_boundary(void) {
  TEST_ASSERT_EQUAL(0, getPumpDurationMs(DRY, 5.0f));
}

// Grond is droog, temperatuur laag maar toegestaan → korte pompduur
void test_pump_short_when_dry_and_cool(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_SHORT_MS, getPumpDurationMs(DRY, 15.0f));
}

void test_pump_short_just_above_cold_cutoff(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_SHORT_MS, getPumpDurationMs(DRY, 5.1f));
}

void test_pump_short_at_pump_threshold_boundary(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_SHORT_MS, getPumpDurationMs(DRY, 25.0f));
}

// Grond is droog, temperatuur normaal → volledige pompduur
void test_pump_full_when_dry_and_warm(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_MS, getPumpDurationMs(DRY, 26.0f));
}

void test_pump_full_just_above_threshold(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_MS, getPumpDurationMs(DRY, 25.1f));
}

void test_pump_full_at_high_temp(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_MS, getPumpDurationMs(DRY, 35.0f));
}

// ── 27 scenario-integratie tests ──────────────────────────────────────────
// Representatieve sensorwaarden per toestand
#define CAP_DRY_VAL   2900
#define CAP_MOIST_VAL 2600
#define CAP_WET_VAL   1800
#define RES_DRY_VAL   100
#define RES_MOIST_VAL 800
#define RES_WET_VAL   2000
#define TEMP_LOW_C    3.0f
#define TEMP_NORM_C   15.0f
#define TEMP_HIGH_C   30.0f

// Hulpfunctie: keten van sensorruwwaarden naar pompduur
static int pumpDurationFromRaw(int capRaw, int resRaw, float tempC) {
  MoistureLevel cap = getCapacitiveMoistureLevel(capRaw);
  MoistureLevel res = getResistiveMoistureLevel(resRaw);
  MoistureLevel combined = selectMoistureLevel(cap, res);
  return getPumpDurationMs(combined, tempC);
}

// ── cap DRY ────────────────────────────────────────────────────────────────
void test_scenario_cap_dry_res_dry_temp_low(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_DRY_VAL, RES_DRY_VAL, TEMP_LOW_C));
}
void test_scenario_cap_dry_res_dry_temp_normal(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_SHORT_MS, pumpDurationFromRaw(CAP_DRY_VAL, RES_DRY_VAL, TEMP_NORM_C));
}
void test_scenario_cap_dry_res_dry_temp_high(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_MS, pumpDurationFromRaw(CAP_DRY_VAL, RES_DRY_VAL, TEMP_HIGH_C));
}

void test_scenario_cap_dry_res_moist_temp_low(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_DRY_VAL, RES_MOIST_VAL, TEMP_LOW_C));
}
void test_scenario_cap_dry_res_moist_temp_normal(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_SHORT_MS, pumpDurationFromRaw(CAP_DRY_VAL, RES_MOIST_VAL, TEMP_NORM_C));
}
void test_scenario_cap_dry_res_moist_temp_high(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_MS, pumpDurationFromRaw(CAP_DRY_VAL, RES_MOIST_VAL, TEMP_HIGH_C));
}

void test_scenario_cap_dry_res_wet_temp_low(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_DRY_VAL, RES_WET_VAL, TEMP_LOW_C));
}
void test_scenario_cap_dry_res_wet_temp_normal(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_SHORT_MS, pumpDurationFromRaw(CAP_DRY_VAL, RES_WET_VAL, TEMP_NORM_C));
}
void test_scenario_cap_dry_res_wet_temp_high(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_MS, pumpDurationFromRaw(CAP_DRY_VAL, RES_WET_VAL, TEMP_HIGH_C));
}

// ── cap MOIST ──────────────────────────────────────────────────────────────
void test_scenario_cap_moist_res_dry_temp_low(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_MOIST_VAL, RES_DRY_VAL, TEMP_LOW_C));
}
void test_scenario_cap_moist_res_dry_temp_normal(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_SHORT_MS, pumpDurationFromRaw(CAP_MOIST_VAL, RES_DRY_VAL, TEMP_NORM_C));
}
void test_scenario_cap_moist_res_dry_temp_high(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_MS, pumpDurationFromRaw(CAP_MOIST_VAL, RES_DRY_VAL, TEMP_HIGH_C));
}

void test_scenario_cap_moist_res_moist_temp_low(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_MOIST_VAL, RES_MOIST_VAL, TEMP_LOW_C));
}
void test_scenario_cap_moist_res_moist_temp_normal(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_MOIST_VAL, RES_MOIST_VAL, TEMP_NORM_C));
}
void test_scenario_cap_moist_res_moist_temp_high(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_MOIST_VAL, RES_MOIST_VAL, TEMP_HIGH_C));
}

void test_scenario_cap_moist_res_wet_temp_low(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_MOIST_VAL, RES_WET_VAL, TEMP_LOW_C));
}
void test_scenario_cap_moist_res_wet_temp_normal(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_MOIST_VAL, RES_WET_VAL, TEMP_NORM_C));
}
void test_scenario_cap_moist_res_wet_temp_high(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_MOIST_VAL, RES_WET_VAL, TEMP_HIGH_C));
}

// ── cap WET ────────────────────────────────────────────────────────────────
void test_scenario_cap_wet_res_dry_temp_low(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_WET_VAL, RES_DRY_VAL, TEMP_LOW_C));
}
void test_scenario_cap_wet_res_dry_temp_normal(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_SHORT_MS, pumpDurationFromRaw(CAP_WET_VAL, RES_DRY_VAL, TEMP_NORM_C));
}
void test_scenario_cap_wet_res_dry_temp_high(void) {
  TEST_ASSERT_EQUAL(PUMP_ON_DURATION_MS, pumpDurationFromRaw(CAP_WET_VAL, RES_DRY_VAL, TEMP_HIGH_C));
}

void test_scenario_cap_wet_res_moist_temp_low(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_WET_VAL, RES_MOIST_VAL, TEMP_LOW_C));
}
void test_scenario_cap_wet_res_moist_temp_normal(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_WET_VAL, RES_MOIST_VAL, TEMP_NORM_C));
}
void test_scenario_cap_wet_res_moist_temp_high(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_WET_VAL, RES_MOIST_VAL, TEMP_HIGH_C));
}

void test_scenario_cap_wet_res_wet_temp_low(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_WET_VAL, RES_WET_VAL, TEMP_LOW_C));
}
void test_scenario_cap_wet_res_wet_temp_normal(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_WET_VAL, RES_WET_VAL, TEMP_NORM_C));
}
void test_scenario_cap_wet_res_wet_temp_high(void) {
  TEST_ASSERT_EQUAL(0, pumpDurationFromRaw(CAP_WET_VAL, RES_WET_VAL, TEMP_HIGH_C));
}

// ── moistureLevelToString ──────────────────────────────────────────────────

void test_to_string_dry(void) {
  TEST_ASSERT_EQUAL_STRING("DRY", moistureLevelToString(DRY));
}

void test_to_string_moist(void) {
  TEST_ASSERT_EQUAL_STRING("MOIST", moistureLevelToString(MOIST));
}

void test_to_string_wet(void) {
  TEST_ASSERT_EQUAL_STRING("WET", moistureLevelToString(WET));
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_select_both_dry_returns_dry);
  RUN_TEST(test_select_dry_and_wet_returns_dry);
  RUN_TEST(test_select_dry_and_moist_returns_dry);
  RUN_TEST(test_select_moist_and_wet_returns_moist);
  RUN_TEST(test_select_both_moist_returns_moist);
  RUN_TEST(test_select_both_wet_returns_wet);

  RUN_TEST(test_capacitive_dry_min_boundary);
  RUN_TEST(test_capacitive_dry_max_boundary);
  RUN_TEST(test_capacitive_dry_mid);
  RUN_TEST(test_capacitive_moist_min_boundary);
  RUN_TEST(test_capacitive_moist_max_boundary);
  RUN_TEST(test_capacitive_moist_mid);
  RUN_TEST(test_capacitive_wet_mid);
  RUN_TEST(test_capacitive_wet_min_boundary);

  RUN_TEST(test_resistive_dry_min_boundary);
  RUN_TEST(test_resistive_dry_max_boundary);
  RUN_TEST(test_resistive_dry_mid);
  RUN_TEST(test_resistive_moist_min_boundary);
  RUN_TEST(test_resistive_moist_max_boundary);
  RUN_TEST(test_resistive_moist_mid);
  RUN_TEST(test_resistive_wet_min_boundary);
  RUN_TEST(test_resistive_wet_mid);

  RUN_TEST(test_pump_off_when_moist);
  RUN_TEST(test_pump_off_when_wet);
  RUN_TEST(test_pump_off_when_dry_too_cold);
  RUN_TEST(test_pump_off_at_cold_cutoff_boundary);
  RUN_TEST(test_pump_short_when_dry_and_cool);
  RUN_TEST(test_pump_short_just_above_cold_cutoff);
  RUN_TEST(test_pump_short_at_pump_threshold_boundary);
  RUN_TEST(test_pump_full_when_dry_and_warm);
  RUN_TEST(test_pump_full_just_above_threshold);
  RUN_TEST(test_pump_full_at_high_temp);

  RUN_TEST(test_scenario_cap_dry_res_dry_temp_low);
  RUN_TEST(test_scenario_cap_dry_res_dry_temp_normal);
  RUN_TEST(test_scenario_cap_dry_res_dry_temp_high);
  RUN_TEST(test_scenario_cap_dry_res_moist_temp_low);
  RUN_TEST(test_scenario_cap_dry_res_moist_temp_normal);
  RUN_TEST(test_scenario_cap_dry_res_moist_temp_high);
  RUN_TEST(test_scenario_cap_dry_res_wet_temp_low);
  RUN_TEST(test_scenario_cap_dry_res_wet_temp_normal);
  RUN_TEST(test_scenario_cap_dry_res_wet_temp_high);
  RUN_TEST(test_scenario_cap_moist_res_dry_temp_low);
  RUN_TEST(test_scenario_cap_moist_res_dry_temp_normal);
  RUN_TEST(test_scenario_cap_moist_res_dry_temp_high);
  RUN_TEST(test_scenario_cap_moist_res_moist_temp_low);
  RUN_TEST(test_scenario_cap_moist_res_moist_temp_normal);
  RUN_TEST(test_scenario_cap_moist_res_moist_temp_high);
  RUN_TEST(test_scenario_cap_moist_res_wet_temp_low);
  RUN_TEST(test_scenario_cap_moist_res_wet_temp_normal);
  RUN_TEST(test_scenario_cap_moist_res_wet_temp_high);
  RUN_TEST(test_scenario_cap_wet_res_dry_temp_low);
  RUN_TEST(test_scenario_cap_wet_res_dry_temp_normal);
  RUN_TEST(test_scenario_cap_wet_res_dry_temp_high);
  RUN_TEST(test_scenario_cap_wet_res_moist_temp_low);
  RUN_TEST(test_scenario_cap_wet_res_moist_temp_normal);
  RUN_TEST(test_scenario_cap_wet_res_moist_temp_high);
  RUN_TEST(test_scenario_cap_wet_res_wet_temp_low);
  RUN_TEST(test_scenario_cap_wet_res_wet_temp_normal);
  RUN_TEST(test_scenario_cap_wet_res_wet_temp_high);

  RUN_TEST(test_to_string_dry);
  RUN_TEST(test_to_string_moist);
  RUN_TEST(test_to_string_wet);

  return UNITY_END();
}
