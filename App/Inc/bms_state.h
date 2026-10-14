/**
 * @file    bms_state.h
 * @brief   BMS State Store — tüm BMS verilerini RAM'de tutan merkezi veri yapısı
 */
#ifndef BMS_STATE_H
#define BMS_STATE_H

#include <stdint.h>

#define BMS_MAX_CELLS   16
#define BMS_MAX_TEMPS   4

typedef struct {
    /* ── Pack bilgileri ──────────────────────────────── */
    uint16_t pack_voltage_mV;         /* Toplam pack gerilimi (mV)       */
    uint16_t pack_voltage_mV_H;       /* Üst word (>65535 mV için)       */
    int16_t  pack_current_mA;         /* Pack akımı (+şarj, -deşarj)     */
    int16_t  pack_current_mA_H;       /* Üst word                        */
    uint16_t soc_permille;            /* 0–1000 (0.1% çözünürlük)        */
    uint16_t soh_permille;            /* 0–1000                          */
    uint16_t design_capacity_mAh;     /* Tasarım kapasitesi              */
    uint16_t remaining_capacity_mAh;  /* Kalan kapasite                  */
    uint16_t cycle_count;             /* Şarj/deşarj döngü sayısı        */
    uint16_t cell_count;              /* Aktif hücre sayısı               */
    uint16_t min_cell_voltage_mV;     /* En düşük hücre gerilimi         */
    uint16_t max_cell_voltage_mV;     /* En yüksek hücre gerilimi        */
    uint16_t delta_cell_voltage_mV;   /* Max-Min fark                    */

    /* ── Hücre gerilimleri ────────────────────────────── */
    uint16_t cell_voltage_mV[BMS_MAX_CELLS];

    /* ── Sıcaklıklar ─────────────────────────────────── */
    int16_t  temperature_dC[BMS_MAX_TEMPS]; /* 0.1°C (25.3°C → 253) */
    int16_t  temp_min_dC;
    int16_t  temp_max_dC;
    int16_t  temp_avg_dC;

    /* ── MOS / Kontrol ───────────────────────────────── */
    uint16_t charge_mos;              /* 0=OFF, 1=ON                     */
    uint16_t discharge_mos;           /* 0=OFF, 1=ON                     */
    uint16_t balance_enable;          /* Bitmask                         */
    uint16_t balance_status;          /* Aktif dengeleme durumu           */
    uint16_t protection_status;       /* Koruma durumu                   */

    /* ── Alarm / Fault ───────────────────────────────── */
    uint16_t fault_flags;             /* Bitmask                         */
    uint16_t alarm_flags;             /* Bitmask                         */
    uint16_t ov_threshold_mV;         /* Aşırı gerilim eşiği (4250)      */
    uint16_t uv_threshold_mV;         /* Düşük gerilim eşiği (2800)      */
    uint16_t oc_charge_mA;            /* Aşırı şarj akımı (10000)        */
    uint16_t oc_discharge_mA;         /* Aşırı deşarj akımı (15000)      */
    int16_t  ot_threshold_dC;         /* Aşırı sıcaklık (600 = 60.0°C)   */
    int16_t  ut_threshold_dC;         /* Düşük sıcaklık (-100 = -10.0°C)  */

    /* ── Simülasyon kontrol ──────────────────────────── */
    uint16_t sim_enable;              /* 0=Durdur, 1=Çalıştır            */
    uint16_t sim_mode;                /* 0=Statik, 1=Deşarj, 2=Şarj      */
    uint16_t sim_speed;               /* Hız çarpanı (1=normal)           */
    uint16_t sim_initial_soc;         /* Başlangıç SOC                   */
    uint16_t sim_load_current_mA;     /* Simüle edilen yük akımı         */

    /* ── Sistem ──────────────────────────────────────── */
    uint16_t fw_version_major;
    uint16_t fw_version_minor;
    uint16_t slave_address;
    uint16_t baud_rate_code;          /* 0=9600, 1=19200, 2=38400, 3=115200 */
    uint16_t uptime_seconds_L;
    uint16_t uptime_seconds_H;
    uint16_t comm_rx_count;
    uint16_t comm_err_count;
    uint16_t echo_register;           /* Test: yazılan değer okunur      */
} BMS_State_t;

/**
 * @brief  BMS state'i varsayılan değerlerle başlat
 */
void BMS_State_Init(BMS_State_t *state);

/**
 * @brief  Min/max/delta cell voltage hesapla
 */
void BMS_State_UpdateCellStats(BMS_State_t *state);

/**
 * @brief  Sıcaklık min/max/avg hesapla
 */
void BMS_State_UpdateTempStats(BMS_State_t *state);

#endif /* BMS_STATE_H */
