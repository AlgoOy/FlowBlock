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

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == huart1.Instance)
    {
        while(fsm_rt_cpl != byte_fifo_receive(&g_tByteFifoCB, __HAL_DMA_GET_COUNTER(huart1.hdmarx)));
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == huart1.Instance)
    {
        while(fsm_rt_cpl != byte_fifo_receive(&g_tByteFifoCB, __HAL_DMA_GET_COUNTER(huart1.hdmarx)));
    }
}

void UART_IDLE_IRQHandler(UART_HandleTypeDef *huart)
{

}

bool uart_rx_fsm_init(uart_rx_ctl_t *ptThis, uart_rx_ctl_cfg_t *ptCFG)
{
    assert(NULL != ptThis);
    assert(NULL != ptCFG);
    assert(NULL != ptCFG->ptHandleUart);
    
    *ptThis = (uart_rx_ctl_t) {
        .chState = 0,
        .tPrePos = 0,
        .tCFG = *ptCFG,
    };
    return true;
}

fsm_rt_t uart_rx_data_migration(uart_rx_ctl_t *ptThis)
{
    enum {
        START = 0,
        MOVE_DATA,
        NOTIFY_RX_CPL,
        
    };
    
    fsm_rt_t emRetStatus = fsm_rt_on_going;
    
    switch(this.chState) {
        case START:
        {
            this.chState = MOVE_DATA;
            // break;
        }
        case MOVE_DATA:
        {
            while(fsm_rt_cpl != byte_fifo_receive(&g_tByteFifoCB, __HAL_DMA_GET_COUNTER(this.tCFG.ptHandleUart->hdmarx)));
        }
    }
    
    return emRetStatus;
}