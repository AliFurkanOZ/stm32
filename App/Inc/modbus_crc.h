/**
 * @file    modbus_crc.h
 * @brief   CRC-16/Modbus hesaplama
 */
#ifndef MODBUS_CRC_H
#define MODBUS_CRC_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief  CRC-16/Modbus hesapla
 * @param  data  Veri buffer
 * @param  length  Veri uzunluğu
 * @return CRC-16 değeri (little-endian: CRC_LO | CRC_HI<<8)
 */
uint16_t Modbus_CRC16(const uint8_t *data, uint16_t length);

/**
 * @brief  Frame'in CRC'sini doğrula
 * @param  frame   Tam frame (veri + son 2 byte CRC)
 * @param  length  Tam frame uzunluğu (CRC dahil)
 * @return true = CRC doğru, false = hatalı
 */
bool Modbus_CheckCRC(const uint8_t *frame, uint16_t length);

/**
 * @brief  Frame'in sonuna CRC ekle
 * @param  frame      Veri buffer (son 2 byte CRC için boş olmalı)
 * @param  dataLength Sadece veri uzunluğu (CRC hariç)
 */
void Modbus_AppendCRC(uint8_t *frame, uint16_t dataLength);

#endif /* MODBUS_CRC_H */
