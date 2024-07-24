#ifndef __BLOCK_HARDWARE_H__
#define __BLOCK_HARDWARE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_tx;

extern
void dma_tx_cpl_callback(DMA_HandleTypeDef *hdma);

#ifdef __cplusplus
}
#endif

#endif