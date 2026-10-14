/**
 * @file    register_map.h
 * @brief   Modbus Register Haritası — adres tanımları ve erişim API
 */
#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H

#include <stdint.h>
#include "bms_state.h"

/* ══════════════════════════════════════════════════════════════════════
 *  Register Adres Tanımları
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Pack Bilgileri (0x0000–0x000F) ──────────────────────────────── */
#define REG_PACK_VOLTAGE        0x0000
#define REG_PACK_VOLTAGE_H      0x0001
#define REG_PACK_CURRENT        0x0002
#define REG_PACK_CURRENT_H      0x0003
#define REG_SOC                 0x0004
#define REG_SOH                 0x0005
#define REG_DESIGN_CAPACITY     0x0006
#define REG_REMAINING_CAPACITY  0x0007
#define REG_CYCLE_COUNT         0x0008
#define REG_CELL_COUNT          0x0009
#define REG_MIN_CELL_VOLTAGE    0x000A
#define REG_MAX_CELL_VOLTAGE    0x000B
#define REG_DELTA_CELL_VOLTAGE  0x000C

/* ── Hücre Gerilimleri (0x0010–0x001F) ───────────────────────────── */
#define REG_CELL_01_VOLTAGE     0x0010
#define REG_CELL_16_VOLTAGE     0x001F

/* ── Sıcaklıklar (0x0020–0x0027) ─────────────────────────────────── */
#define REG_TEMP_1              0x0020
#define REG_TEMP_2              0x0021
#define REG_TEMP_3              0x0022
#define REG_TEMP_4              0x0023
#define REG_TEMP_MIN            0x0024
#define REG_TEMP_MAX            0x0025
#define REG_TEMP_AVG            0x0026

/* ── MOS / Kontrol (0x0030–0x003F) ───────────────────────────────── */
#define REG_CHARGE_MOS          0x0030
#define REG_DISCHARGE_MOS       0x0031
#define REG_BALANCE_ENABLE      0x0032
#define REG_BALANCE_STATUS      0x0033
#define REG_PROTECTION_STATUS   0x0034

/* ── Alarm / Fault (0x0040–0x004F) ───────────────────────────────── */
#define REG_FAULT_FLAGS         0x0040
#define REG_ALARM_FLAGS         0x0041
#define REG_FAULT_CLEAR         0x0042
#define REG_OV_THRESHOLD        0x0043
#define REG_UV_THRESHOLD        0x0044
#define REG_OC_CHARGE_THRESH    0x0045
#define REG_OC_DISCHARGE_THRESH 0x0046
#define REG_OT_THRESHOLD        0x0047
#define REG_UT_THRESHOLD        0x0048

/* ── Simülasyon Kontrol (0x0050–0x005F) ──────────────────────────── */
#define REG_SIM_ENABLE          0x0050
#define REG_SIM_MODE            0x0051
#define REG_SIM_SPEED           0x0052
#define REG_SIM_INITIAL_SOC     0x0053
#define REG_SIM_LOAD_CURRENT    0x0054
#define REG_SIM_RESET           0x0055

/* ── Sistem / Test (0x0060–0x006F) ───────────────────────────────── */
#define REG_FW_VERSION_MAJOR    0x0060
#define REG_FW_VERSION_MINOR    0x0061
#define REG_SLAVE_ADDRESS       0x0062
#define REG_BAUD_RATE_CODE      0x0063
#define REG_UPTIME_SECONDS_L    0x0064
#define REG_UPTIME_SECONDS_H    0x0065
#define REG_COMM_RX_COUNT       0x0066
#define REG_COMM_ERR_COUNT      0x0067
#define REG_ECHO_REGISTER       0x0068
#define REG_SYSTEM_RESET        0x0069

/* ── Toplam adres aralığı ────────────────────────────────────────── */
#define REG_MAX_ADDRESS         0x0069

/* ══════════════════════════════════════════════════════════════════════
 *  Durum Kodları
 * ══════════════════════════════════════════════════════════════════════ */
typedef enum {
    REGMAP_OK = 0,
    REGMAP_ILLEGAL_ADDRESS,
    REGMAP_ILLEGAL_VALUE,
    REGMAP_READ_ONLY,
    REGMAP_WRITE_ONLY,
    REGMAP_SLAVE_FAILURE
} RegMap_Status_t;

/* ══════════════════════════════════════════════════════════════════════
 *  API
 * ══════════════════════════════════════════════════════════════════════ */

/**
 * @brief  Register map'e BMS state referansını ata
 */
void RegMap_Init(BMS_State_t *state);

/**
 * @brief  Tek register oku
 */
RegMap_Status_t RegMap_Read(uint16_t addr, uint16_t *value);

/**
 * @brief  Tek register yaz
 */
RegMap_Status_t RegMap_Write(uint16_t addr, uint16_t value);

/**
 * @brief  Çoklu register oku
 */
RegMap_Status_t RegMap_ReadMultiple(uint16_t startAddr, uint16_t count,
                                    uint16_t *values);

/**
 * @brief  Çoklu register yaz
 */
RegMap_Status_t RegMap_WriteMultiple(uint16_t startAddr, uint16_t count,
                                     const uint16_t *values);

#endif /* REGISTER_MAP_H */
