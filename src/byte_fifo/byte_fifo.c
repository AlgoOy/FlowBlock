/*============================ INCLUDES ======================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "./byte_fifo.h"
#include "__common.h"

/*============================ MACROS ========================================*/

/*============================ MACROFIED FUNCTIONS ===========================*/

#define BTYE_FIFO_FLUSH_RESET_FSM() \
            do {this.u4FlushState = START;} while(0)
            
#define BTYE_FIFO_ADD_BYTE_RESET_FSM() \
            do {this.u4FlushState = START;} while(0)

#undef this
#define this (*ptThis)
    
/*============================ TYPES =========================================*/

enum {
    TRANSMIT_ON_GOING = 0,
    TRANSMIT_CPL = 1,
};

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

ARM_NONNULL(1, 2)
byte_fifo_t *byte_fifo_init(byte_fifo_t *ptThis, byte_fifo_cfg_t *ptCFG)
{
    assert(NULL != ptThis);
    assert(NULL != ptCFG);
    assert(NULL != ptCFG->ptMemBlkFifo);
    assert(NULL != ptCFG->fnTransmit);
    
    mem_blk_t *ptMemBlk = mem_blk_fifo_new(ptCFG->ptMemBlkFifo);
    if (NULL == ptMemBlk) {
        return NULL;
    }

    memset(ptThis, 0, sizeof(byte_fifo_t));

    *ptThis = (byte_fifo_t) {
        .u4AddByteState = 0,
        .u4FlushState = 0,
        .bIsTransmitCpl = TRANSMIT_CPL,
        .ptMemBlk = NULL,
        .tQueue = {0},
        .tCFG = *ptCFG,
    };

    do {
        byte_queue_cfg_t tByteQueueCFG = {
            .tSize = ptMemBlk->tSizeInByte,
            .pchBuffer = ptMemBlk->chMemory,
        };
        byte_queue_t *ptQueue = byte_queue_init(&this.tQueue, &tByteQueueCFG);
        if (NULL == ptQueue) {
            mem_blk_fifo_free(this.tCFG.ptMemBlkFifo, this.ptMemBlk);

            memset(ptThis, 0, sizeof(byte_fifo_t));
            
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
    
    if (NULL != this.ptMemBlk) {
        mem_blk_fifo_free(this.tCFG.ptMemBlkFifo, this.ptMemBlk);
    }

    memset(ptThis, 0, sizeof(byte_fifo_t));
}

ARM_NONNULL(1)
fsm_rt_t byte_fifo_flush(byte_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    enum {
        START = 0,
        IS_TRANSMIT_CPL,
        FETCH_FIFO_BLK,
        TRANSMIT_BLK,
    };
    
    fsm_rt_t emRetStatus = fsm_rt_on_going;
    
    switch (this.u4FlushState) {
        case START:
            this.u4FlushState = IS_TRANSMIT_CPL;
            // break;
        case IS_TRANSMIT_CPL:
            if (!this.bIsTransmitCpl) {
                BTYE_FIFO_FLUSH_RESET_FSM();
                break;
            }
            this.u4FlushState = FETCH_FIFO_BLK;
            // break;
__IRQ_SAFE {
        case FETCH_FIFO_BLK:
            this.ptMemBlk = mem_blk_fifo_fetch(this.tCFG.ptMemBlkFifo);
            if (NULL == this.ptMemBlk) {
                __IRQ_SAFE_EXIT
            }
            this.u4FlushState = TRANSMIT_BLK;
            // break;
        case TRANSMIT_BLK:
            if (this.tCFG.fnTransmit(this.ptMemBlk->chMemory, this.ptMemBlk->tSizeInByte)) {
                this.bIsTransmitCpl = TRANSMIT_ON_GOING;
                BTYE_FIFO_FLUSH_RESET_FSM();
                emRetStatus = fsm_rt_cpl;
                __IRQ_SAFE_EXIT;
            }
            emRetStatus = fsm_rt_err;
            // break;
}
            break;
    }
    
    return emRetStatus;
}
    
ARM_NONNULL(1)
void byte_fifo_user_report_transmit_cpl(byte_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    mem_blk_fifo_free(this.tCFG.ptMemBlkFifo, this.ptMemBlk);
    this.ptMemBlk = NULL;
    
    this.bIsTransmitCpl = TRANSMIT_CPL;

    byte_fifo_flush(ptThis);
}

ARM_NONNULL(1)
fsm_rt_t byte_fifo_add_byte(byte_fifo_t *ptThis, uint8_t chData)
{
    assert(NULL != ptThis);
    
    enum {
        START = 0,
        EN_QUEUE,
        NEW_MEM_BLK,
        INIT_QUEUE,
    };
    
    byte_queue_cfg_t tQueueCFG = {0};
    
    switch (this.u4AddByteState) {
        case START:
        {
            this.u4AddByteState = EN_QUEUE;
            // break;
        }
        case EN_QUEUE:
        {
            if (enqueue_byte(&this.tQueue, chData)) {
                BTYE_FIFO_ADD_BYTE_RESET_FSM();
                return fsm_rt_cpl;
            }
            mem_blk_t *ptMemBlk = (mem_blk_t *)((uintptr_t)this.tQueue.tCFG.pchBuffer - offsetof(mem_blk_t, chMemory));
            mem_blk_fifo_append(this.tCFG.ptMemBlkFifo, ptMemBlk);
            byte_fifo_flush(ptThis);
            this.u4AddByteState = NEW_MEM_BLK;
            // break;
        }
        case NEW_MEM_BLK:
        {
            mem_blk_t *ptMemBlk = mem_blk_fifo_new(this.tCFG.ptMemBlkFifo);
            if (NULL == ptMemBlk) {
                byte_fifo_flush(ptThis);
                break;
            }
            tQueueCFG = (byte_queue_cfg_t) {
                .tSize = ptMemBlk->tSizeInByte,
                .pchBuffer = ptMemBlk->chMemory,
            };
            this.u4AddByteState = INIT_QUEUE;
            // break;
        }
        case INIT_QUEUE:
        {
            byte_queue_t *ptQueue = byte_queue_init(&this.tQueue, &tQueueCFG);
            if (NULL != ptQueue) {
                this.u4AddByteState = EN_QUEUE;
                break;
            }
            return fsm_rt_err;
            // break;
        }
        break;
    }
    
    return fsm_rt_on_going;
}