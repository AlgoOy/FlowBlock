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

extern byte_fifo_t byte_fifo;

ARM_NONNULL(1)
bool uart_transmit_by_dma(const byte *pchData, size_t tSizeInByte) {
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
    byte_fifo_dma_irq_handler(&byte_fifo);
}

void uart_tx_cpl_callback(UART_HandleTypeDef *hdma)
{
    byte_fifo_uart_irq_handler(&byte_fifo);
}