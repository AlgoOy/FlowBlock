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

typedef struct uart_rx_ctl_cfg_t {
    UART_HandleTypeDef  *ptHandleUart;
} uart_rx_ctl_cfg_t;

typedef struct uart_rx_ctl_t {
    uint8_t             chState;
    size_t              tPrePos;
    uart_rx_ctl_cfg_t   tCFG;
} uart_rx_ctl_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_tx;

ARM_NONNULL(1)
extern
bool uart_transmit_by_dma(uint8_t *const pchData, const uint16_t hwSizeInByte);

ARM_NONNULL(1)
extern
bool uart_receive_by_dma(uint8_t *const pchData, const uint16_t hwSizeInByte);

ARM_NONNULL(1)
extern
void dma_tx_cpl_callback(DMA_HandleTypeDef *hdma);

ARM_NONNULL(1)
extern
void uart_tx_cpl_callback(UART_HandleTypeDef *hdma);

#ifdef __cplusplus
}
#endif

#endif