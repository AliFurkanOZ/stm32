/**
 * @file    bms_state.c
 * @brief   BMS State Store — varsayılan değerler ve istatistik hesaplama
 */
#include "bms_state.h"
#include <string.h>

/* ── Varsayılan Değerlerle Başlat ─────────────────────────────────── */
void BMS_State_Init(BMS_State_t *state)
{
    memset(state, 0, sizeof(BMS_State_t));

    /* Pack bilgileri — tipik 4S Li-ion */
    state->cell_count = 4;
    state->pack_voltage_mV = 14800;   /* 4 × 3700 mV */
    state->pack_current_mA = 0;
    state->soc_permille = 850;        /* %85.0 */
    state->soh_permille = 1000;       /* %100.0 */
    state->design_capacity_mAh = 5000;
    state->remaining_capacity_mAh = 4250;
    state->cycle_count = 0;

    /* Hücre gerilimleri */
    state->cell_voltage_mV[0] = 3700;
    state->cell_voltage_mV[1] = 3700;
    state->cell_voltage_mV[2] = 3700;
    state->cell_voltage_mV[3] = 3700;

    /* Sıcaklıklar — 25.0°C */
    state->temperature_dC[0] = 250;
    state->temperature_dC[1] = 252;
    state->temperature_dC[2] = 248;
    state->temperature_dC[3] = 251;

    /* MOS — açık */
    state->charge_mos = 1;
    state->discharge_mos = 1;
    state->balance_enable = 0;
    state->balance_status = 0;
    state->protection_status = 0;

    /* Alarm eşikleri */
    state->ov_threshold_mV = 4250;
    state->uv_threshold_mV = 2800;
    state->oc_charge_mA = 10000;
    state->oc_discharge_mA = 15000;
    state->ot_threshold_dC = 600;     /* 60.0°C */
    state->ut_threshold_dC = -100;    /* -10.0°C */

    /* Simülasyon — başlangıçta durmuş */
    state->sim_enable = 0;
    state->sim_mode = 0;              /* Statik */
    state->sim_speed = 1;
    state->sim_initial_soc = 850;
    state->sim_load_current_mA = 1000;

    /* Sistem */
    state->fw_version_major = 1;
    state->fw_version_minor = 0;
    state->slave_address = 1;
    state->baud_rate_code = 0;        /* 9600 */
    state->echo_register = 0;

    /* İstatistikleri güncelle */
    BMS_State_UpdateCellStats(state);
    BMS_State_UpdateTempStats(state);
}

/* ── Hücre İstatistikleri ─────────────────────────────────────────── */
void BMS_State_UpdateCellStats(BMS_State_t *state)
{
    if (state->cell_count == 0) return;

    uint16_t minV = 0xFFFF;
    uint16_t maxV = 0;

    for (uint16_t i = 0; i < state->cell_count && i < BMS_MAX_CELLS; i++)
    {
        if (state->cell_voltage_mV[i] < minV)
            minV = state->cell_voltage_mV[i];
        if (state->cell_voltage_mV[i] > maxV)
            maxV = state->cell_voltage_mV[i];
    }

    state->min_cell_voltage_mV = minV;
    state->max_cell_voltage_mV = maxV;
    state->delta_cell_voltage_mV = maxV - minV;
}

/* ── Sıcaklık İstatistikleri ──────────────────────────────────────── */
void BMS_State_UpdateTempStats(BMS_State_t *state)
{
    int16_t minT = 32767;
    int16_t maxT = -32768;
    int32_t sum = 0;
    uint16_t count = BMS_MAX_TEMPS;

    for (uint16_t i = 0; i < count; i++)
    {
        if (state->temperature_dC[i] < minT)
            minT = state->temperature_dC[i];
        if (state->temperature_dC[i] > maxT)
            maxT = state->temperature_dC[i];
        sum += state->temperature_dC[i];
    }

    state->temp_min_dC = minT;
    state->temp_max_dC = maxT;
    state->temp_avg_dC = (int16_t)(sum / count);
}
