/**
 * @file    modbus_handlers.c
 * @brief   Modbus FC03/FC06/FC16 handler implementasyonu
 *
 * Her handler:
 *  1) Request'i parse eder
 *  2) register_map üzerinden okuma/yazma yapar
 *  3) Normal response veya exception response oluşturur
 *  4) CRC ekler
 */
#include "modbus_handlers.h"
#include "modbus_defs.h"
#include "modbus_crc.h"
#include "register_map.h"
#include <string.h>

/* ══════════════════════════════════════════════════════════════════════
 *  Yardımcı: Exception Response Oluştur
 * ══════════════════════════════════════════════════════════════════════ */
static void buildException(uint8_t slaveAddr, uint8_t fc,
                           uint8_t exCode,
                           uint8_t *response, uint16_t *responseLen)
{
    response[0] = slaveAddr;
    response[1] = fc | 0x80;       /* FC + 0x80 = exception */
    response[2] = exCode;
    Modbus_AppendCRC(response, 3);
    *responseLen = 5;              /* addr + fc|0x80 + ex + crc(2) */
}

/**
 * @brief  RegMap_Status_t → Modbus exception kodu dönüşümü
 */
static uint8_t statusToException(RegMap_Status_t status)
{
    switch (status)
    {
        case REGMAP_ILLEGAL_ADDRESS:  return MODBUS_EX_ILLEGAL_ADDRESS;
        case REGMAP_ILLEGAL_VALUE:    return MODBUS_EX_ILLEGAL_VALUE;
        case REGMAP_READ_ONLY:        return MODBUS_EX_ILLEGAL_VALUE;
        case REGMAP_WRITE_ONLY:       return MODBUS_EX_ILLEGAL_ADDRESS;
        case REGMAP_SLAVE_FAILURE:    return MODBUS_EX_SLAVE_FAILURE;
        default:                      return MODBUS_EX_SLAVE_FAILURE;
    }
}

/* ══════════════════════════════════════════════════════════════════════
 *  FC03 — Read Holding Registers
 *
 *  Request:  [Addr][0x03][StartH][StartL][QtyH][QtyL][CRC_L][CRC_H]
 *  Response: [Addr][0x03][ByteCount][ValH1][ValL1]...[CRC_L][CRC_H]
 * ══════════════════════════════════════════════════════════════════════ */
void Modbus_HandleFC03(const uint8_t *request, uint16_t requestLen,
                       uint8_t *response, uint16_t *responseLen)
{
    uint8_t  slaveAddr = request[0];
    uint16_t startAddr = ((uint16_t)request[2] << 8) | request[3];
    uint16_t quantity  = ((uint16_t)request[4] << 8) | request[5];

    /* Quantity sınır kontrolü */
    if (quantity == 0 || quantity > MODBUS_FC03_MAX_QUANTITY)
    {
        buildException(slaveAddr, MODBUS_FC_READ_HOLDING,
                       MODBUS_EX_ILLEGAL_VALUE, response, responseLen);
        return;
    }

    /* Register'ları oku */
    uint16_t values[MODBUS_FC03_MAX_QUANTITY];
    RegMap_Status_t status = RegMap_ReadMultiple(startAddr, quantity, values);

    if (status != REGMAP_OK)
    {
        buildException(slaveAddr, MODBUS_FC_READ_HOLDING,
                       statusToException(status), response, responseLen);
        return;
    }

    /* Normal response oluştur */
    uint8_t byteCount = (uint8_t)(quantity * 2);
    response[0] = slaveAddr;
    response[1] = MODBUS_FC_READ_HOLDING;
    response[2] = byteCount;

    for (uint16_t i = 0; i < quantity; i++)
    {
        response[3 + i * 2]     = (uint8_t)(values[i] >> 8);   /* HI */
        response[3 + i * 2 + 1] = (uint8_t)(values[i] & 0xFF); /* LO */
    }

    uint16_t dataLen = 3 + byteCount;
    Modbus_AppendCRC(response, dataLen);
    *responseLen = dataLen + 2;
}

/* ══════════════════════════════════════════════════════════════════════
 *  FC06 — Write Single Register
 *
 *  Request:  [Addr][0x06][RegH][RegL][ValH][ValL][CRC_L][CRC_H]
 *  Response: Echo (aynı frame aynen geri döner)
 * ══════════════════════════════════════════════════════════════════════ */
void Modbus_HandleFC06(const uint8_t *request, uint16_t requestLen,
                       uint8_t *response, uint16_t *responseLen)
{
    uint8_t  slaveAddr = request[0];
    uint16_t regAddr   = ((uint16_t)request[2] << 8) | request[3];
    uint16_t value     = ((uint16_t)request[4] << 8) | request[5];

    /* Register'a yaz */
    RegMap_Status_t status = RegMap_Write(regAddr, value);

    if (status != REGMAP_OK)
    {
        buildException(slaveAddr, MODBUS_FC_WRITE_SINGLE,
                       statusToException(status), response, responseLen);
        return;
    }

    /* Echo response — request'in aynısını geri gönder */
    response[0] = slaveAddr;
    response[1] = MODBUS_FC_WRITE_SINGLE;
    response[2] = (uint8_t)(regAddr >> 8);
    response[3] = (uint8_t)(regAddr & 0xFF);
    response[4] = (uint8_t)(value >> 8);
    response[5] = (uint8_t)(value & 0xFF);
    Modbus_AppendCRC(response, 6);
    *responseLen = 8;
}

/* ══════════════════════════════════════════════════════════════════════
 *  FC16 — Write Multiple Registers
 *
 *  Request:  [Addr][0x10][StartH][StartL][QtyH][QtyL][ByteCnt][V1H][V1L]...[CRC]
 *  Response: [Addr][0x10][StartH][StartL][QtyH][QtyL][CRC_L][CRC_H]
 * ══════════════════════════════════════════════════════════════════════ */
void Modbus_HandleFC16(const uint8_t *request, uint16_t requestLen,
                       uint8_t *response, uint16_t *responseLen)
{
    uint8_t  slaveAddr = request[0];
    uint16_t startAddr = ((uint16_t)request[2] << 8) | request[3];
    uint16_t quantity  = ((uint16_t)request[4] << 8) | request[5];
    uint8_t  byteCount = request[6];

    /* Quantity sınır kontrolü */
    if (quantity == 0 || quantity > MODBUS_FC16_MAX_QUANTITY)
    {
        buildException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE,
                       MODBUS_EX_ILLEGAL_VALUE, response, responseLen);
        return;
    }

    /* Byte count tutarlılık kontrolü */
    if (byteCount != quantity * 2)
    {
        buildException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE,
                       MODBUS_EX_ILLEGAL_VALUE, response, responseLen);
        return;
    }

    /* Frame uzunluk kontrolü: addr(1) + fc(1) + start(2) + qty(2) + bytecnt(1) + data + crc(2) */
    uint16_t expectedLen = 7 + byteCount + 2;
    if (requestLen < expectedLen)
    {
        buildException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE,
                       MODBUS_EX_ILLEGAL_VALUE, response, responseLen);
        return;
    }

    /* Değerleri ayıkla */
    uint16_t values[MODBUS_FC16_MAX_QUANTITY];
    for (uint16_t i = 0; i < quantity; i++)
    {
        values[i] = ((uint16_t)request[7 + i * 2] << 8) |
                     request[7 + i * 2 + 1];
    }

    /* Register'lara yaz */
    RegMap_Status_t status = RegMap_WriteMultiple(startAddr, quantity, values);

    if (status != REGMAP_OK)
    {
        buildException(slaveAddr, MODBUS_FC_WRITE_MULTIPLE,
                       statusToException(status), response, responseLen);
        return;
    }

    /* Normal response */
    response[0] = slaveAddr;
    response[1] = MODBUS_FC_WRITE_MULTIPLE;
    response[2] = (uint8_t)(startAddr >> 8);
    response[3] = (uint8_t)(startAddr & 0xFF);
    response[4] = (uint8_t)(quantity >> 8);
    response[5] = (uint8_t)(quantity & 0xFF);
    Modbus_AppendCRC(response, 6);
    *responseLen = 8;
}
