/**
 * @file    modbus_defs.h
 * @brief   Modbus RTU sabitleri ve tanımları
 */
#ifndef MODBUS_DEFS_H
#define MODBUS_DEFS_H

#include <stdint.h>

/* ── Fonksiyon Kodları ─────────────────────────────────────────────── */
#define MODBUS_FC_READ_HOLDING      0x03
#define MODBUS_FC_WRITE_SINGLE      0x06
#define MODBUS_FC_WRITE_MULTIPLE    0x10

/* ── Exception Kodları ─────────────────────────────────────────────── */
#define MODBUS_EX_ILLEGAL_FUNCTION  0x01
#define MODBUS_EX_ILLEGAL_ADDRESS   0x02
#define MODBUS_EX_ILLEGAL_VALUE     0x03
#define MODBUS_EX_SLAVE_FAILURE     0x04

/* ── Limitler ──────────────────────────────────────────────────────── */
#define MODBUS_MAX_ADU_SIZE         256
#define MODBUS_FC03_MAX_QUANTITY    125
#define MODBUS_FC16_MAX_QUANTITY    123
#define MODBUS_BROADCAST_ADDRESS    0x00
#define MODBUS_MIN_FRAME_SIZE       4      /* Addr + FC + CRC(2) */

#endif /* MODBUS_DEFS_H */
