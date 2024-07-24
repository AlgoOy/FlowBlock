#include <stdio.h>
#include <string.h>
#include <stm32l4xx_hal.h>
#include "byte_fifo.h"
#include "hardware.h"

volatile uint32_t share = 0;

void byte_fifo_init(byte_fifo_t *ptByteFifo, mem_blk_fifo_t *ptMemBlk)
{
    if (NULL == ptByteFifo || NULL == ptMemBlk) {
        return;
    }
    
    memset(ptByteFifo, 0, sizeof(byte_fifo_t));
    
    *ptByteFifo = (byte_fifo_t) {
        .chState = 0,
        .ptMemBlk = ptMemBlk,
        .ptBlk = NULL,
        .tQueue = {0},
        .bIsDMABusy = false,
    };
    
    mem_blk_t *ptBlk = mem_blk_fifo_new(ptMemBlk);
    if (NULL != ptBlk) {
        byte_queue_init(&ptByteFifo->tQueue, ptBlk->chMemory, ptBlk->tSizeInByte);
    }
}

void byte_fifo_flush(byte_fifo_t *ptByteFifo)
{
    if (NULL != ptByteFifo->ptBlk) {
        return;
    }
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    int ret1 = HAL_DMA_GetState(&hdma_usart1_tx);
    if (ret1 == HAL_DMA_STATE_BUSY) {
        __set_PRIMASK(primask);
        return;
    }
    ptByteFifo->ptBlk = mem_blk_fifo_fetch(ptByteFifo->ptMemBlk);
    if (NULL == ptByteFifo->ptBlk) {
        __set_PRIMASK(primask);
        return;
    }
    int ret = HAL_OK;
    do {
        ret = HAL_UART_Transmit_DMA(&huart1, ptByteFifo->ptBlk->chMemory, ptByteFifo->ptBlk->tSizeInByte);
        if (HAL_OK != ret) {
            HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_7);
        }
    } while(ret != HAL_OK);
    share ++;
    __set_PRIMASK(primask);
}

void byte_fifo_dma_irq_handler(byte_fifo_t *ptByteFifo)
{
    if (NULL == ptByteFifo->ptBlk) {
        return;
    }
    mem_blk_fifo_free(ptByteFifo->ptMemBlk, ptByteFifo->ptBlk);
    ptByteFifo->ptBlk = NULL;
    byte_fifo_flush(ptByteFifo);
}

fsm_rt_t byte_fifo_add_bytes(byte_fifo_t *ptByteFifo, byte chData)
{
    enum {
        START = 0,
        EN_QUEUE,
        NEW_QUEUE,
    };
    
    switch (ptByteFifo->chState) {
        case START:
        {
            ptByteFifo->chState = EN_QUEUE;
        }
        case EN_QUEUE:
        {
            if (enqueue_byte(&ptByteFifo->tQueue, chData)) {
                ptByteFifo->chState = START;
                return fsm_rt_cpl;
            }
            mem_blk_t *ptBlk = (mem_blk_t *)((uintptr_t)ptByteFifo->tQueue.pchBuffer - offsetof(mem_blk_t, chMemory));
            mem_blk_fifo_append(ptByteFifo->ptMemBlk, ptBlk);
            byte_fifo_flush(ptByteFifo);
            ptByteFifo->chState = NEW_QUEUE;
        }
        case NEW_QUEUE:
        {
            mem_blk_t *ptBlk = mem_blk_fifo_new(ptByteFifo->ptMemBlk);
            if (NULL != ptBlk) {
                byte_queue_init(&ptByteFifo->tQueue, ptBlk->chMemory, ptBlk->tSizeInByte);
                ptByteFifo->chState = EN_QUEUE;
            }
        }
    }
    
    return fsm_rt_on_going;
}