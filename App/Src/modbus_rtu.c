/**
 * @file    modbus_rtu.c
 * @brief   Modbus RTU Engine — frame işleme, CRC doğrulama, FC yönlendirme
 *
 * İşlem akışı:
 *   1. Frame uzunluk kontrolü (min 4 byte)
 *   2. CRC-16 doğrulama
 *   3. Slave address kontrolü
 *   4. Fonksiyon kodu dispatch (FC03, FC06, FC16)
 *   5. Broadcast ise (addr=0) yanıt gönderilmez
 */
#include "modbus_rtu.h"
#include "modbus_defs.h"
#include "modbus_crc.h"
#include "modbus_handlers.h"
#include <string.h>

/* ══════════════════════════════════════════════════════════════════════
 *  Init
 * ══════════════════════════════════════════════════════════════════════ */
void Modbus_Init(Modbus_Handle_t *handle, BMS_State_t *bmsState, uint8_t slaveAddress)
{
    memset(handle, 0, sizeof(Modbus_Handle_t));
    handle->slaveAddress = slaveAddress;
    handle->bmsState = bmsState;
}

/* ══════════════════════════════════════════════════════════════════════
 *  Frame İşleme
 * ══════════════════════════════════════════════════════════════════════ */
bool Modbus_ProcessFrame(Modbus_Handle_t *handle,
                         const uint8_t *frame, uint16_t frameLen)
{
    handle->responseLength = 0;

    /* ── 1. Uzunluk kontrolü ──────────────────────────────────────── */
    if (frameLen < MODBUS_MIN_FRAME_SIZE)
    {
        handle->rxErrorCount++;
        if (handle->bmsState)
            handle->bmsState->comm_err_count++;
        return false;
    }

    /* ── 2. CRC doğrulama ─────────────────────────────────────────── */
    if (!Modbus_CheckCRC(frame, frameLen))
    {
        handle->rxErrorCount++;
        if (handle->bmsState)
            handle->bmsState->comm_err_count++;
        return false;
    }

    /* ── 3. Slave address kontrolü ────────────────────────────────── */
    uint8_t frameAddr = frame[0];
    bool isBroadcast = (frameAddr == MODBUS_BROADCAST_ADDRESS);

    if (!isBroadcast && frameAddr != handle->slaveAddress)
    {
        /* Bize ait değil → sessiz kal */
        return false;
    }

    /* ── 4. Geçerli frame sayacı ──────────────────────────────────── */
    handle->rxValidCount++;
    if (handle->bmsState)
        handle->bmsState->comm_rx_count++;

    /* ── 5. Fonksiyon kodu dispatch ───────────────────────────────── */
    uint8_t fc = frame[1];

    switch (fc)
    {
        case MODBUS_FC_READ_HOLDING:
            Modbus_HandleFC03(frame, frameLen,
                              handle->responseBuffer,
                              &handle->responseLength);
            break;

        case MODBUS_FC_WRITE_SINGLE:
            Modbus_HandleFC06(frame, frameLen,
                              handle->responseBuffer,
                              &handle->responseLength);
            break;

        case MODBUS_FC_WRITE_MULTIPLE:
            Modbus_HandleFC16(frame, frameLen,
                              handle->responseBuffer,
                              &handle->responseLength);
            break;

        default:
        {
            /* Desteklenmeyen FC → Exception 01 */
            handle->responseBuffer[0] = frameAddr;
            handle->responseBuffer[1] = fc | 0x80;
            handle->responseBuffer[2] = MODBUS_EX_ILLEGAL_FUNCTION;
            Modbus_AppendCRC(handle->responseBuffer, 3);
            handle->responseLength = 5;
            handle->exceptionCount++;
            break;
        }
    }

    /* ── 6. Broadcast ise yanıt gönderme ──────────────────────────── */
    if (isBroadcast)
    {
        handle->responseLength = 0;
        return false;
    }

    return (handle->responseLength > 0);
}

/* ══════════════════════════════════════════════════════════════════════
 *  Slave Address Değiştirme
 * ══════════════════════════════════════════════════════════════════════ */
void Modbus_SetSlaveAddress(Modbus_Handle_t *handle, uint8_t addr)
{
    if (addr >= 1 && addr <= 247)
        handle->slaveAddress = addr;
}
