/**
 * @file    modbus_handlers.h
 * @brief   Modbus FC handler fonksiyonları
 */
#ifndef MODBUS_HANDLERS_H
#define MODBUS_HANDLERS_H

#include <stdint.h>

/**
 * @brief  FC03 — Read Holding Registers
 * @param  request       Gelen frame (CRC dahil)
 * @param  requestLen    Frame uzunluğu
 * @param  response      Yanıt buffer
 * @param  responseLen   [out] Yanıt uzunluğu
 */
void Modbus_HandleFC03(const uint8_t *request, uint16_t requestLen,
                       uint8_t *response, uint16_t *responseLen);

/**
 * @brief  FC06 — Write Single Register
 */
void Modbus_HandleFC06(const uint8_t *request, uint16_t requestLen,
                       uint8_t *response, uint16_t *responseLen);

/**
 * @brief  FC16 — Write Multiple Registers
 */
void Modbus_HandleFC16(const uint8_t *request, uint16_t requestLen,
                       uint8_t *response, uint16_t *responseLen);

#endif /* MODBUS_HANDLERS_H */
