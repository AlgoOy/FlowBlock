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
bool uart_transmit_by_dma(uint8_t *const pchData, const uint16_t hwSizeInByte) 
{
    assert(NULL != pchData);
    
    if (HAL_OK == HAL_UART_Transmit_DMA(&huart1, pchData, hwSizeInByte)) {
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

bool uart_receive_by_dma(uint8_t *const pchData, const uint16_t hwSizeInByte)
{
    assert(NULL != pchData);

    if (HAL_OK == HAL_UART_Receive_DMA(&huart1, pchData, hwSizeInByte)) {
        return true;
    }

    return false;
}

void dma_rx_cpl_callback(DMA_HandleTypeDef *hdma)
{
    uint16_t hwRemainingBytesSpace = __HAL_DMA_GET_COUNTER(&hdma_usart1_tx);
    
    HAL_UART_DMAStop(&huart1);
    
    byte_fifo_user_report_receive_cpl(&g_tByteFifoCB, hwRemainingBytesSpace);
}

void uart_rx_cpl_callback(UART_HandleTypeDef *hdma)
{
    uint16_t hwRemainingBytesSpace = __HAL_DMA_GET_COUNTER(&hdma_usart1_tx);
    
    HAL_UART_DMAStop(&huart1);
    
    byte_fifo_user_report_receive_cpl(&g_tByteFifoCB, hwRemainingBytesSpace);
}