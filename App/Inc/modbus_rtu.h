/**
 * @file    modbus_rtu.h
 * @brief   Modbus RTU Engine — frame parse, CRC check, FC dispatch
 */
#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <stdint.h>
#include <stdbool.h>
#include "bms_state.h"

#define MODBUS_RTU_BUF_SIZE  256

typedef struct {
    /* Konfigürasyon */
    uint8_t              slaveAddress;

    /* İstatistikler */
    uint32_t             rxValidCount;
    uint32_t             rxErrorCount;
    uint32_t             exceptionCount;

    /* Yanıt buffer (non-blocking TX için handle içinde kalıcı) */
    uint8_t              responseBuffer[MODBUS_RTU_BUF_SIZE];
    uint16_t             responseLength;

    /* BMS state referansı (istatistik güncelleme için) */
    BMS_State_t         *bmsState;
} Modbus_Handle_t;

/**
 * @brief  Modbus engine'i başlat
 */
void Modbus_Init(Modbus_Handle_t *handle, BMS_State_t *bmsState, uint8_t slaveAddress);

/**
 * @brief  Gelen frame'i işle — CRC doğrula, slave adresi kontrol et, FC dispatch et
 * @param  handle       Modbus handle
 * @param  frame        Gelen ham frame (DMA buffer'dan)
 * @param  frameLen     Frame uzunluğu
 * @return true = yanıt oluşturuldu (responseBuffer'da), false = sessiz reddetme
 */
bool Modbus_ProcessFrame(Modbus_Handle_t *handle,
                         const uint8_t *frame, uint16_t frameLen);

/**
 * @brief  Slave adresini runtime'da değiştir
 */
void Modbus_SetSlaveAddress(Modbus_Handle_t *handle, uint8_t addr);

#endif /* MODBUS_RTU_H */
