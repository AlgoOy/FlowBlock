/*============================ INCLUDES ======================================*/

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "./byte_fifo.h"
#include "__common.h"

/*============================ MACROS ========================================*/

/*============================ MACROFIED FUNCTIONS ===========================*/

#define BYTE_FIFO_FLUSH_RESET_FSM() \
            do {this.u4FlushState = START;} while(0)
            
#define BYTE_FIFO_ADD_BYTE_RESET_FSM() \
            do {this.u4AddByteState = START;} while(0)
            
#define BYTE_FIFO_RECEIVE_RESET_FSM() \
            do {this.u4ReceiveState = START;} while(0)
            
#define BYTE_FIFO_READ_BYTE_RESET_FSM() \
            do {this.u4ReadByteState = START;} while(0)

#undef this
#define this (*ptThis)
    
/*============================ TYPES =========================================*/

enum {
    TRANSMIT_ON_GOING = 0,
    TRANSMIT_CPL = 1,    
};

enum {
    RECEIVE_ON_GOING = 0,
    RECEIVE_CPL = 1,    
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
    assert(NULL != ptCFG->fnForRxTx);

    memset(ptThis, 0, sizeof(byte_fifo_t));

    *ptThis = (byte_fifo_t) {
        .u4AddByteState = 0,
        .u4FlushState = 0,
        .u4ReceiveState = 0,
        .u4ReadByteState = 0,
        .bIsTransmitCpl = TRANSMIT_CPL,
        .bISReceiveCpl = RECEIVE_CPL,
        .ptMemBlkforRxTx = NULL,
        .ptMemBlkforQueue = NULL,
        .tQueue = {0},
        .tCFG = *ptCFG,
    };
    
    this.ptMemBlkforQueue = mem_blk_fifo_new(ptCFG->ptMemBlkFifo);
    if (NULL == this.ptMemBlkforQueue) {
        return NULL;
    }

    do {
        byte_queue_cfg_t tByteQueueCFG = {
            .tSize = this.ptMemBlkforQueue->tSizeInByte,
            .pchBuffer = this.ptMemBlkforQueue->chMemory,
        };
        byte_queue_t *ptQueue = byte_queue_init_empty(&this.tQueue, &tByteQueueCFG);
        if (NULL == ptQueue) {
            mem_blk_fifo_free(this.tCFG.ptMemBlkFifo, this.ptMemBlkforQueue);

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
    
    if (NULL != this.ptMemBlkforQueue) {
        mem_blk_fifo_free(this.tCFG.ptMemBlkFifo, this.ptMemBlkforQueue);
    }

    if (NULL != this.ptMemBlkforRxTx) {
        mem_blk_fifo_free(this.tCFG.ptMemBlkFifo, this.ptMemBlkforRxTx);
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
        case IS_TRANSMIT_CPL:       // TODO: 修改原子操作的位置
            if (!this.bIsTransmitCpl) {
                break;
            }
            this.u4FlushState = FETCH_FIFO_BLK;
            // break;
__IRQ_SAFE {
        case FETCH_FIFO_BLK:
            this.ptMemBlkforRxTx = mem_blk_fifo_fetch(this.tCFG.ptMemBlkFifo);
            if (NULL == this.ptMemBlkforRxTx) {
                __IRQ_SAFE_EXIT
            }
            this.u4FlushState = TRANSMIT_BLK;
            // break;
        case TRANSMIT_BLK:
            if (this.tCFG.fnForRxTx(this.ptMemBlkforRxTx->chMemory, this.ptMemBlkforRxTx->tSizeUsedInByte)) {
                this.bIsTransmitCpl = TRANSMIT_ON_GOING;
                BYTE_FIFO_FLUSH_RESET_FSM();
                emRetStatus = fsm_rt_cpl;
                __IRQ_SAFE_EXIT;
            }
            emRetStatus = fsm_rt_err;
        break;
}
    }
    
    return emRetStatus;
}
    
ARM_NONNULL(1)
void byte_fifo_user_report_transmit_cpl(byte_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    mem_blk_fifo_free(this.tCFG.ptMemBlkFifo, this.ptMemBlkforRxTx);
    this.ptMemBlkforRxTx = NULL;

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
    
    switch (this.u4AddByteState) {
        case START:
        {
            this.u4AddByteState = EN_QUEUE;
            // break;
        }
        case EN_QUEUE:
        {
            if (enqueue_byte(&this.tQueue, chData)) {
                BYTE_FIFO_ADD_BYTE_RESET_FSM();
                return fsm_rt_cpl;
            }
            mem_blk_t *ptMemBlk = (mem_blk_t *)((uintptr_t)this.tQueue.tCFG.pchBuffer - offsetof(mem_blk_t, chMemory));
            ptMemBlk->tSizeUsedInByte = this.tQueue.tLength;
            mem_blk_fifo_append(this.tCFG.ptMemBlkFifo, ptMemBlk);
            byte_fifo_flush(ptThis);
            this.u4AddByteState = NEW_MEM_BLK;
            // break;
        }
        case NEW_MEM_BLK:
        {
            this.ptMemBlkforQueue = mem_blk_fifo_new(this.tCFG.ptMemBlkFifo);
            if (NULL == this.ptMemBlkforQueue) {
                byte_fifo_flush(ptThis);
                break;
            }
            this.u4AddByteState = INIT_QUEUE;
            // break;
        }
        case INIT_QUEUE:
        {
            byte_queue_cfg_t tQueueCFG = (byte_queue_cfg_t) {
                .tSize = this.ptMemBlkforQueue->tSizeInByte,
                .pchBuffer = this.ptMemBlkforQueue->chMemory,
            };
            byte_queue_t *ptQueue = byte_queue_init_empty(&this.tQueue, &tQueueCFG);
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

//uint8_t *byte_fifo_receive_buffer()
//{
//    
//}

//void byte_fifo_receive_init(byte_fifo_t *ptThis)
//{
//  Assign MemBlk to this.ptMemBlk
//}

ARM_NONNULL(1)
fsm_rt_t byte_fifo_receive(byte_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    enum {
        START = 0,
        IS_RECEIVE_CPL,
        APPEND_MEM_BLK,
        NEW_MEM_BLK,
        RECEIVE,
    };
    
    fsm_rt_t emRetStatus = fsm_rt_on_going;
    
    switch(this.u4ReceiveState) {
        case START:
        {
            this.u4AddByteState = APPEND_MEM_BLK;
            // break;
        }
        case IS_RECEIVE_CPL:
        {
            if (!this.bISReceiveCpl) {
                break;
            }
            this.u4ReceiveState = APPEND_MEM_BLK;
            // break;
        }
        case APPEND_MEM_BLK:
        {
            if (NULL == this.ptMemBlkforRxTx) {
                emRetStatus = fsm_rt_err;
                break;
            }
            mem_blk_fifo_append(this.tCFG.ptMemBlkFifo, this.ptMemBlkforRxTx);
            this.ptMemBlkforRxTx = NULL;
            this.u4ReceiveState = NEW_MEM_BLK;
            // break;
        }
        case NEW_MEM_BLK:
        {
            this.ptMemBlkforRxTx = mem_blk_fifo_new(this.tCFG.ptMemBlkFifo);
            if (NULL == this.ptMemBlkforRxTx) {
                emRetStatus = (fsm_rt_t)ERR_MEM_BLOCK_NOT_ENOUGH;
                break;
            }
            this.u4ReceiveState = RECEIVE;
            // break;
        }
        case RECEIVE:
        {
            if (NULL == this.ptMemBlkforRxTx) {
                emRetStatus = fsm_rt_err;
                break;
            }
            if (this.tCFG.fnForRxTx(this.ptMemBlkforRxTx->chMemory, this.ptMemBlkforRxTx->tSizeInByte)) {
                BYTE_FIFO_RECEIVE_RESET_FSM();
                emRetStatus = fsm_rt_cpl;
                break;
            }
            emRetStatus = fsm_rt_err;
            break;
        }
        break;
    }
    
    return emRetStatus;
}

ARM_NONNULL(1)
fsm_rt_t byte_fifo_read_byte(byte_fifo_t *ptThis, uint8_t *pchData)
{    
    assert(NULL != ptThis);
    
    enum {
        START = 0,
        DE_QUEUE,
        FETCH_MEM_BLK,
        INIT_QUEUE,
    };
    
    fsm_rt_t emRetStatus = fsm_rt_on_going;
    
    switch(this.u4ReadByteState) {
        case START:
        {
            this.u4ReadByteState = DE_QUEUE;
            // break;
        }
        case DE_QUEUE:
        {
            if (dequeue_byte(&this.tQueue, pchData)) {
                BYTE_FIFO_READ_BYTE_RESET_FSM();
                break;
            }
            mem_blk_t *ptMemBlk = (mem_blk_t *)((uintptr_t)this.tQueue.tCFG.pchBuffer - offsetof(mem_blk_t, chMemory));
            mem_blk_fifo_free(this.tCFG.ptMemBlkFifo, ptMemBlk);
            this.u4ReadByteState = FETCH_MEM_BLK;
            //break;
        }
        case FETCH_MEM_BLK:
        {
            this.ptMemBlkforQueue = mem_blk_fifo_fetch(this.tCFG.ptMemBlkFifo);
            if (NULL == this.ptMemBlkforQueue) {
                break;
            }
            this.u4ReadByteState = INIT_QUEUE;
            // break;
        }
        case INIT_QUEUE:
        {
            byte_queue_cfg_t tQueueCFG = (byte_queue_cfg_t) {
                .tSize = this.ptMemBlkforQueue->tSizeUsedInByte,
                .pchBuffer = this.ptMemBlkforQueue->chMemory,
            };
            byte_queue_t *ptQueue = byte_queue_init_full(&this.tQueue, &tQueueCFG);
            if (NULL != ptQueue) {
                this.u4AddByteState = DE_QUEUE;
                break;
            }
            emRetStatus = fsm_rt_err;
            break;
        }
        break;
    }
    
    return emRetStatus;
}

ARM_NONNULL(1)
void byte_fifo_user_report_receive_cpl(byte_fifo_t *ptThis, uint16_t hwRemainingBytesSpace)
{
    assert(NULL != ptThis);
    
    uint16_t hwSizeUsedInByte = this.ptMemBlkforRxTx->tSizeInByte - hwRemainingBytesSpace;
    this.ptMemBlkforRxTx->tSizeUsedInByte = hwSizeUsedInByte;
    this.bISReceiveCpl = RECEIVE_CPL;

    byte_fifo_receive(ptThis);
}