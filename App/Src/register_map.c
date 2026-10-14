/**
 * @file    register_map.c
 * @brief   Register Map — Modbus adresi ↔ BMS state eşlemesi
 *
 * Her register adresi bir uint16_t pointer'a eşlenir.
 * R/W izinleri bir permission tablosu ile kontrol edilir.
 */
#include "register_map.h"
#include <string.h>

/* ── Modül değişkenleri ───────────────────────────────────────────── */
static BMS_State_t *pState = (void *)0;

/* ══════════════════════════════════════════════════════════════════════
 *  İzin Tablosu
 * ══════════════════════════════════════════════════════════════════════ */
typedef enum {
    PERM_R,     /* Salt okunur            */
    PERM_RW,    /* Okunur/yazılır         */
    PERM_W,     /* Sadece yazılır (trigger) */
    PERM_NONE   /* Geçersiz/reserved       */
} RegPerm_t;

/**
 * @brief  Adrese göre izin ver
 */
static RegPerm_t getPermission(uint16_t addr)
{
    /* Pack bilgileri — hepsi salt okunur */
    if (addr >= REG_PACK_VOLTAGE && addr <= REG_DELTA_CELL_VOLTAGE)
        return PERM_R;

    /* Hücre gerilimleri — salt okunur */
    if (addr >= REG_CELL_01_VOLTAGE && addr <= REG_CELL_16_VOLTAGE)
        return PERM_R;

    /* Sıcaklıklar — salt okunur */
    if (addr >= REG_TEMP_1 && addr <= REG_TEMP_AVG)
        return PERM_R;

    /* MOS / Kontrol */
    if (addr >= REG_CHARGE_MOS && addr <= REG_BALANCE_ENABLE)
        return PERM_RW;
    if (addr == REG_BALANCE_STATUS || addr == REG_PROTECTION_STATUS)
        return PERM_R;

    /* Alarm / Fault */
    if (addr == REG_FAULT_FLAGS || addr == REG_ALARM_FLAGS)
        return PERM_R;
    if (addr == REG_FAULT_CLEAR)
        return PERM_W;
    if (addr >= REG_OV_THRESHOLD && addr <= REG_UT_THRESHOLD)
        return PERM_RW;

    /* Simülasyon */
    if (addr >= REG_SIM_ENABLE && addr <= REG_SIM_LOAD_CURRENT)
        return PERM_RW;
    if (addr == REG_SIM_RESET)
        return PERM_W;

    /* Sistem */
    if (addr == REG_FW_VERSION_MAJOR || addr == REG_FW_VERSION_MINOR)
        return PERM_R;
    if (addr == REG_SLAVE_ADDRESS || addr == REG_BAUD_RATE_CODE)
        return PERM_RW;
    if (addr == REG_UPTIME_SECONDS_L || addr == REG_UPTIME_SECONDS_H)
        return PERM_R;
    if (addr == REG_COMM_RX_COUNT || addr == REG_COMM_ERR_COUNT)
        return PERM_R;
    if (addr == REG_ECHO_REGISTER)
        return PERM_RW;
    if (addr == REG_SYSTEM_RESET)
        return PERM_W;

    return PERM_NONE;  /* Geçersiz adres */
}

/* ══════════════════════════════════════════════════════════════════════
 *  Adres → Pointer Eşleme
 * ══════════════════════════════════════════════════════════════════════ */
static uint16_t *getPointer(uint16_t addr)
{
    if (pState == (void *)0) return (void *)0;

    switch (addr)
    {
        /* ── Pack bilgileri ───────────────────────────── */
        case REG_PACK_VOLTAGE:        return &pState->pack_voltage_mV;
        case REG_PACK_VOLTAGE_H:      return &pState->pack_voltage_mV_H;
        case REG_PACK_CURRENT:        return (uint16_t *)&pState->pack_current_mA;
        case REG_PACK_CURRENT_H:      return (uint16_t *)&pState->pack_current_mA_H;
        case REG_SOC:                 return &pState->soc_permille;
        case REG_SOH:                 return &pState->soh_permille;
        case REG_DESIGN_CAPACITY:     return &pState->design_capacity_mAh;
        case REG_REMAINING_CAPACITY:  return &pState->remaining_capacity_mAh;
        case REG_CYCLE_COUNT:         return &pState->cycle_count;
        case REG_CELL_COUNT:          return &pState->cell_count;
        case REG_MIN_CELL_VOLTAGE:    return &pState->min_cell_voltage_mV;
        case REG_MAX_CELL_VOLTAGE:    return &pState->max_cell_voltage_mV;
        case REG_DELTA_CELL_VOLTAGE:  return &pState->delta_cell_voltage_mV;

        /* ── Sıcaklıklar ─────────────────────────────── */
        case REG_TEMP_1:              return (uint16_t *)&pState->temperature_dC[0];
        case REG_TEMP_2:              return (uint16_t *)&pState->temperature_dC[1];
        case REG_TEMP_3:              return (uint16_t *)&pState->temperature_dC[2];
        case REG_TEMP_4:              return (uint16_t *)&pState->temperature_dC[3];
        case REG_TEMP_MIN:            return (uint16_t *)&pState->temp_min_dC;
        case REG_TEMP_MAX:            return (uint16_t *)&pState->temp_max_dC;
        case REG_TEMP_AVG:            return (uint16_t *)&pState->temp_avg_dC;

        /* ── MOS / Kontrol ────────────────────────────── */
        case REG_CHARGE_MOS:          return &pState->charge_mos;
        case REG_DISCHARGE_MOS:       return &pState->discharge_mos;
        case REG_BALANCE_ENABLE:      return &pState->balance_enable;
        case REG_BALANCE_STATUS:      return &pState->balance_status;
        case REG_PROTECTION_STATUS:   return &pState->protection_status;

        /* ── Alarm / Fault ────────────────────────────── */
        case REG_FAULT_FLAGS:         return &pState->fault_flags;
        case REG_ALARM_FLAGS:         return &pState->alarm_flags;
        case REG_FAULT_CLEAR:         return &pState->fault_flags; /* yazınca temizler */
        case REG_OV_THRESHOLD:        return &pState->ov_threshold_mV;
        case REG_UV_THRESHOLD:        return &pState->uv_threshold_mV;
        case REG_OC_CHARGE_THRESH:    return &pState->oc_charge_mA;
        case REG_OC_DISCHARGE_THRESH: return &pState->oc_discharge_mA;
        case REG_OT_THRESHOLD:        return (uint16_t *)&pState->ot_threshold_dC;
        case REG_UT_THRESHOLD:        return (uint16_t *)&pState->ut_threshold_dC;

        /* ── Simülasyon ───────────────────────────────── */
        case REG_SIM_ENABLE:          return &pState->sim_enable;
        case REG_SIM_MODE:            return &pState->sim_mode;
        case REG_SIM_SPEED:           return &pState->sim_speed;
        case REG_SIM_INITIAL_SOC:     return &pState->sim_initial_soc;
        case REG_SIM_LOAD_CURRENT:    return &pState->sim_load_current_mA;
        case REG_SIM_RESET:           return &pState->sim_enable; /* yazınca reset */

        /* ── Sistem ───────────────────────────────────── */
        case REG_FW_VERSION_MAJOR:    return &pState->fw_version_major;
        case REG_FW_VERSION_MINOR:    return &pState->fw_version_minor;
        case REG_SLAVE_ADDRESS:       return &pState->slave_address;
        case REG_BAUD_RATE_CODE:      return &pState->baud_rate_code;
        case REG_UPTIME_SECONDS_L:    return &pState->uptime_seconds_L;
        case REG_UPTIME_SECONDS_H:    return &pState->uptime_seconds_H;
        case REG_COMM_RX_COUNT:       return &pState->comm_rx_count;
        case REG_COMM_ERR_COUNT:      return &pState->comm_err_count;
        case REG_ECHO_REGISTER:       return &pState->echo_register;
        case REG_SYSTEM_RESET:        return &pState->echo_register; /* placeholder */

        default: break;
    }

    /* Hücre gerilimleri (0x0010–0x001F) */
    if (addr >= REG_CELL_01_VOLTAGE && addr <= REG_CELL_16_VOLTAGE)
    {
        uint16_t idx = addr - REG_CELL_01_VOLTAGE;
        if (idx < BMS_MAX_CELLS)
            return &pState->cell_voltage_mV[idx];
    }

    return (void *)0;
}

/* ══════════════════════════════════════════════════════════════════════
 *  Public API
 * ══════════════════════════════════════════════════════════════════════ */

void RegMap_Init(BMS_State_t *state)
{
    pState = state;
}

RegMap_Status_t RegMap_Read(uint16_t addr, uint16_t *value)
{
    RegPerm_t perm = getPermission(addr);
    if (perm == PERM_NONE)
        return REGMAP_ILLEGAL_ADDRESS;
    if (perm == PERM_W)
        return REGMAP_ILLEGAL_ADDRESS; /* Sadece yazılır → okunamaz */

    uint16_t *ptr = getPointer(addr);
    if (ptr == (void *)0)
        return REGMAP_ILLEGAL_ADDRESS;

    *value = *ptr;
    return REGMAP_OK;
}

RegMap_Status_t RegMap_Write(uint16_t addr, uint16_t value)
{
    RegPerm_t perm = getPermission(addr);
    if (perm == PERM_NONE)
        return REGMAP_ILLEGAL_ADDRESS;
    if (perm == PERM_R)
        return REGMAP_READ_ONLY;

    uint16_t *ptr = getPointer(addr);
    if (ptr == (void *)0)
        return REGMAP_ILLEGAL_ADDRESS;

    /* Özel trigger register'lar */
    if (addr == REG_FAULT_CLEAR)
    {
        pState->fault_flags = 0;
        pState->alarm_flags = 0;
        return REGMAP_OK;
    }

    if (addr == REG_SIM_RESET)
    {
        /* Simülasyonu sıfırla */
        pState->sim_enable = 0;
        pState->soc_permille = pState->sim_initial_soc;
        return REGMAP_OK;
    }

    if (addr == REG_SYSTEM_RESET)
    {
        /* Yazılım reset — NVIC_SystemReset() çağrılabilir */
        /* Şimdilik sadece değer yazılır */
        return REGMAP_OK;
    }

    /* Slave address aralık kontrolü */
    if (addr == REG_SLAVE_ADDRESS)
    {
        if (value < 1 || value > 247)
            return REGMAP_ILLEGAL_VALUE;
    }

    *ptr = value;
    return REGMAP_OK;
}

RegMap_Status_t RegMap_ReadMultiple(uint16_t startAddr, uint16_t count,
                                    uint16_t *values)
{
    for (uint16_t i = 0; i < count; i++)
    {
        uint16_t addr = startAddr + i;
        RegMap_Status_t status = RegMap_Read(addr, &values[i]);

        /* "Gap" adresleri (reserved) 0 olarak oku */
        if (status == REGMAP_ILLEGAL_ADDRESS)
        {
            /* Adres haritanın içinde mi kontrol et */
            if (addr <= REG_MAX_ADDRESS)
            {
                values[i] = 0;  /* Reserved alanlar 0 döner */
            }
            else
            {
                return REGMAP_ILLEGAL_ADDRESS;
            }
        }
        else if (status != REGMAP_OK)
        {
            return status;
        }
    }
    return REGMAP_OK;
}

RegMap_Status_t RegMap_WriteMultiple(uint16_t startAddr, uint16_t count,
                                     const uint16_t *values)
{
    /* İlk önce tüm adreslerin yazılabilir olduğunu kontrol et */
    for (uint16_t i = 0; i < count; i++)
    {
        uint16_t addr = startAddr + i;
        RegPerm_t perm = getPermission(addr);
        if (perm == PERM_NONE)
            return REGMAP_ILLEGAL_ADDRESS;
        if (perm == PERM_R)
            return REGMAP_READ_ONLY;
    }

    /* Hepsini yaz */
    for (uint16_t i = 0; i < count; i++)
    {
        RegMap_Status_t status = RegMap_Write(startAddr + i, values[i]);
        if (status != REGMAP_OK)
            return status;
    }
    return REGMAP_OK;
}
