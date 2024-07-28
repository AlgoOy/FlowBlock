/*============================ INCLUDES ======================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

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

ARM_NONNULL(1)
byte_fifo_t *__byte_fifo_init(byte_fifo_cfg_t *ptCFG, byte_fifo_t *ptThis)
{
    assert(NULL != ptCFG);
    
    struct {
        bool ctl;
        bool buffer;
    } tUserAllocated = {false, false};
    
    byte_fifo_cfg_t tCFG = {
        .ptMemBlk = NULL,
        .fnTransmit = NULL,
    };
    
    if (NULL != ptCFG) {
        tCFG = *ptCFG;
    }
    
    if (NULL == tCFG.fnTransmit) {
        return NULL;
    }
    
    if (NULL == tCFG.ptMemBlk) {
        tCFG.ptMemBlk = mem_blk_fifo_init(NULL);
        if (NULL == tCFG.ptMemBlk) {
            return NULL;
        }
    } else {
        tUserAllocated.buffer = true;
    }    
    
    mem_blk_t *ptBlk = mem_blk_fifo_new(tCFG.ptMemBlk);
    if (NULL == ptBlk) {
        if (!tUserAllocated.buffer) {
            mem_blk_fifo_deinit(tCFG.ptMemBlk);
        }
        return NULL;
    }
        
    if (NULL == ptThis) {
        ptThis = (byte_fifo_t *)malloc(sizeof(byte_fifo_t));
        if (NULL == ptThis) {
            if (!tUserAllocated.buffer) {
                mem_blk_fifo_deinit(tCFG.ptMemBlk);
            }
            return NULL;
        }
    } else {
        tUserAllocated.ctl = true;
    }
    memset(ptThis, 0, sizeof(byte_fifo_t));

    *ptThis = (byte_fifo_t) {
        .chAddByteState = 0,
        .chFlushState = 0,
        .ptMemBlk = tCFG.ptMemBlk,
        .ptBlk = NULL,
        .tQueue = {0},
        .bIsDMABusy = false,
        .bIsUARTBusy = false,
        .fnTransmit = tCFG.fnTransmit,
        .tUserAllocated = {
            .ctl = tUserAllocated.ctl,
            .buffer = tUserAllocated.buffer,
        },
    };
    
    do {
        byte_queue_cfg_t tByteQueueCFG = {
            .tSize = ptBlk->tSizeInByte,
            .pchBuffer = ptBlk->chMemory,
        };
        byte_queue_t *ptQueue = byte_queue_init(&tByteQueueCFG, &this.tQueue);
        if (NULL == ptQueue) {
            mem_blk_fifo_free(this.ptMemBlk, this.ptBlk);
            
            if (!this.tUserAllocated.buffer) {
                mem_blk_fifo_deinit(this.ptMemBlk);
            }
            
            if (!this.tUserAllocated.ctl) {
                memset(ptThis, 0, sizeof(byte_fifo_t));
                free(ptThis);
            } else {
                memset(ptThis, 0, sizeof(byte_fifo_t));
            }
            return NULL;
        }
    } while (0);
    
    return ptThis;
}

ARM_NONNULL(1)
void byte_fifo_deinit(byte_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    byte_queue_deinit(&this.tQueue);
    
    mem_blk_fifo_free(this.ptMemBlk, this.ptBlk);
    
    if (!this.tUserAllocated.buffer) {
        mem_blk_fifo_deinit(this.ptMemBlk);
    }
    
    if (!this.tUserAllocated.ctl) {
        memset(ptThis, 0, sizeof(byte_fifo_t));
        free(ptThis);
    } else {
        memset(ptThis, 0, sizeof(byte_fifo_t));
    }
}

#include "hardware.h"

ARM_NONNULL(1)
fsm_rt_t byte_fifo_flush(byte_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    enum {
        START = 0,
        IS_DMA_OK,
        IS_UART_OK,
        IS_BLK_NULL,
        FETCH_FIFO_BLK,
        TRANSMIT_BLK,
    };
    
//    uint32_t primask = __get_PRIMASK();
//    __disable_irq();
    
//    __set_PRIMASK(primask);
    
    switch (this.chFlushState) {
        case START:
            this.chFlushState = IS_DMA_OK;
        case IS_DMA_OK:
            if (this.bIsDMABusy) {
                this.chFlushState = START;
                break;
            }
            this.chFlushState = IS_UART_OK;
        case IS_UART_OK:
            if (this.bIsUARTBusy) {
                this.chFlushState = START;
                break;
            }
            this.chFlushState = IS_BLK_NULL;
        case IS_BLK_NULL:
            if (NULL != this.ptBlk) {
                this.chFlushState = TRANSMIT_BLK;
                break;
            }
            this.chFlushState = FETCH_FIFO_BLK;
        case FETCH_FIFO_BLK:
            this.ptBlk = mem_blk_fifo_fetch(this.ptMemBlk);
            if (NULL == this.ptBlk) {
                this.chFlushState = START;
                break;
            }
            this.chFlushState = TRANSMIT_BLK;
        case TRANSMIT_BLK:
            if (this.fnTransmit(this.ptBlk->chMemory, this.ptBlk->tSizeInByte)) {
                this.bIsDMABusy = true;
                this.bIsUARTBusy = true;
                this.chFlushState = START;
                return fsm_rt_cpl;
            }
            this.chFlushState = START;
            break;            
    }
    
    return fsm_rt_on_going;
}
    
ARM_NONNULL(1)
void byte_fifo_dma_irq_handler(byte_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    mem_blk_fifo_free(this.ptMemBlk, this.ptBlk);
    this.ptBlk = NULL;
    this.bIsDMABusy = false;
    
    byte_fifo_flush(ptThis);
}

ARM_NONNULL(1)
void byte_fifo_uart_irq_handler(byte_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    this.bIsUARTBusy = false;
    
    byte_fifo_flush(ptThis);
}

ARM_NONNULL(1)
fsm_rt_t byte_fifo_add_bytes(byte_fifo_t *ptThis, byte chData)
{
    assert(NULL != ptThis);
    
    enum {
        START = 0,
        EN_QUEUE,
        NEW_QUEUE,
        FLUSH_BLK,
        INIT_QUEUE,
    };
    
    switch (this.chAddByteState) {
        case START:
        {
            this.chAddByteState = EN_QUEUE;
        }
        case EN_QUEUE:
        {
            if (enqueue_byte(&this.tQueue, chData)) {
                this.chAddByteState = START;
                return fsm_rt_cpl;
            }
            mem_blk_t *ptBlk = (mem_blk_t *)((uintptr_t)this.tQueue.pchBuffer - offsetof(mem_blk_t, chMemory));
            mem_blk_fifo_append(this.ptMemBlk, ptBlk);
            byte_fifo_flush(ptThis);
            this.chAddByteState = NEW_QUEUE;
        }
        case NEW_QUEUE:
        {
            mem_blk_t *ptBlk = mem_blk_fifo_new(this.ptMemBlk);
            if (NULL != ptBlk) {
                byte_queue_cfg_t tByteQueueCFG = {
                    .tSize = ptBlk->tSizeInByte,
                    .pchBuffer = ptBlk->chMemory,
                };
                byte_queue_t *ptQueue = byte_queue_init(&tByteQueueCFG, &this.tQueue);
                if (NULL != ptQueue) {
                    this.chAddByteState = EN_QUEUE;
                    break;
                } else {
                    mem_blk_fifo_append(this.ptMemBlk, ptBlk);
                    break;
                }
            }
            this.chAddByteState = FLUSH_BLK;
        }
        case FLUSH_BLK:
        {
            if (fsm_rt_cpl != byte_fifo_flush(ptThis)) {
                break;
            }
            this.chAddByteState = NEW_QUEUE;
        }  
    }
    
    return fsm_rt_on_going;
}