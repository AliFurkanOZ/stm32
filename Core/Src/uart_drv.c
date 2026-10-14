#include "uart_drv.h"
#include <string.h>

extern UART_HandleTypeDef huart1;

void UART_Init(void)
{

}

void UART_SendString(char *str)
{
    HAL_UART_Transmit(&huart1,
                      (uint8_t *)str,
                      strlen(str),
                      HAL_MAX_DELAY);
}
