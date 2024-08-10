#ifndef __BLOCK_HARDWARE_H__
#define __BLOCK_HARDWARE_H__

/*============================ INCLUDES ======================================*/

#include <stdbool.h>

#include "stm32l4xx_hal.h"

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_tx;

ARM_NONNULL(1)
extern
bool uart_transmit_by_dma(const uint8_t *pchData, size_t tSizeInByte);

extern
void dma_tx_cpl_callback(DMA_HandleTypeDef *hdma);

extern
void uart_tx_cpl_callback(UART_HandleTypeDef *hdma);

#ifdef __cplusplus
}
#endif

#endif