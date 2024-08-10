/*============================ INCLUDES ======================================*/

#include <assert.h>

#include "hardware.h"
#include "byte_fifo.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
#undef this
#define this (*ptThis)

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

extern byte_fifo_t g_tByteFifoCB;

ARM_NONNULL(1)
bool uart_transmit_by_dma(const uint8_t *pchData, size_t tSizeInByte) {
    assert(NULL != pchData);
    
    if (tSizeInByte > UINT16_MAX) {
        return false;
    }
    
    if (HAL_OK == HAL_UART_Transmit_DMA(&huart1, pchData, tSizeInByte)) {
        return true;
    }
    return false;
}

void dma_tx_cpl_callback(DMA_HandleTypeDef *hdma)
{
    if (HAL_UART_STATE_READY == HAL_UART_GetState(&huart1)) {
        byte_fifo_user_report_transmit_cpl(&g_tByteFifoCB);
    }
}

void uart_tx_cpl_callback(UART_HandleTypeDef *hdma)
{
    if (HAL_DMA_STATE_READY == HAL_DMA_GetState(&hdma_usart1_tx)) {
        byte_fifo_user_report_transmit_cpl(&g_tByteFifoCB);
    }
}