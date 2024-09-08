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
            do {this.u4AddByteState = START;} while(0)
            
#define BTYE_FIFO_RECEIVE_RESET_FSM() \
            do {this.u4ReceiveState = START;} while(0)
            
#define BTYE_FIFO_READ_BYTE_RESET_FSM() \
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
    assert(NULL != ptCFG->fnTransmit);
    
    mem_blk_t *ptMemBlk = mem_blk_fifo_new(ptCFG->ptMemBlkFifo);
    if (NULL == ptMemBlk) {
        return NULL;
    }

    memset(ptThis, 0, sizeof(byte_fifo_t));

    *ptThis = (byte_fifo_t) {
        .u4AddByteState = 0,
        .u4FlushState = 0,
        .u4ReceiveState = 0,
        .u4ReadByteState = 0,
        .bIsTransmitCpl = TRANSMIT_CPL,
        .bISReceiveCpl = RECEIVE_CPL,
        .ptMemBlk = NULL,
        .rx = {
            .ptMemBlkBuffer = NULL,
            .tPrePos = 0,
        },
        .tQueue = {0},
        .tCFG = *ptCFG,
    };

    do {
        byte_queue_cfg_t tByteQueueCFG = {
            .tSize = ptMemBlk->tSizeInByte,
            .pchBuffer = ptMemBlk->chMemory,
        };
        byte_queue_t *ptQueue = byte_queue_init_empty(&this.tQueue, &tByteQueueCFG);
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
            mem_blk_fifo_free(this.tCFG.ptMemBlkFifo, this.ptMemBlk);
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
fsm_rt_t byte_fifo_receive(byte_fifo_t *ptThis, size_t tCurPos)
{
    assert(NULL != ptThis);
    
    enum {
        START = 0,
        IS_EXCEEDED,
        EXCEEDED,
        NOT_EXCEEDED,
        NEW_MEM_BLOCK,
    };
    
    fsm_rt_t emRetStatus = fsm_rt_on_going;
    
    switch(this.u4ReceiveState) {
        case START:
        {
            this.u4AddByteState = IS_EXCEEDED;
            // break;
        }
        case IS_EXCEEDED:
        {
            if (this.rx.tPrePos > tCurPos) {
                this.u4ReceiveState = EXCEEDED;
                break;
            }
            this.u4ReceiveState = NOT_EXCEEDED;
            // break;
        }
        case NOT_EXCEEDED:
        {
            size_t tDataLength = tCurPos - this.rx.tPrePos;
            size_t tAvailableSpace = this.ptMemBlk->tSizeInByte - this.ptMemBlk->tSizeUsedInByte;
            
            if (tAvailableSpace > 0) {
                size_t tCopyLength = (tDataLength <= tAvailableSpace) ? tDataLength : tAvailableSpace;
                
                uint8_t* pchDst = this.ptMemBlk->chMemory + this.ptMemBlk->tSizeUsedInByte;
                uint8_t* pchSrc = this.rx.ptMemBlkBuffer->chMemory + this.rx.tPrePos;
                memcpy(pchDst, pchSrc, tCopyLength);
                
                this.ptMemBlk->tSizeUsedInByte += tCopyLength;
                this.rx.tPrePos += tCopyLength;
                
                if (tDataLength <= tAvailableSpace) {
                    BTYE_FIFO_RECEIVE_RESET_FSM();
                    emRetStatus = fsm_rt_cpl;
                    break;
                }
            }
            mem_blk_fifo_append(this.tCFG.ptMemBlkFifo, this.ptMemBlk);
            this.u4ReceiveState = NEW_MEM_BLOCK;
            // break;
        }
        case NEW_MEM_BLOCK:
        {
            this.ptMemBlk = mem_blk_fifo_new(this.tCFG.ptMemBlkFifo);
            if (NULL == this.ptMemBlk) {
                emRetStatus = (fsm_rt_t)ERR_MEM_BLOCK_NOT_ENOUGH;
                break;
            }
            this.u4ReceiveState = IS_EXCEEDED;
            break;
        }
        case EXCEEDED:
        {
            size_t tDataLength = (this.rx.ptMemBlkBuffer->tSizeInByte - this.rx.tPrePos) + tCurPos;
            size_t tAvailableSpace = this.ptMemBlk->tSizeInByte - this.ptMemBlk->tSizeUsedInByte;
            
            if (tAvailableSpace > 0) {
                size_t tCopyLength = (tDataLength <= tAvailableSpace) ? tDataLength : tAvailableSpace;
                
                size_t tFirstPartLength = this.rx.ptMemBlkBuffer->tSizeInByte - this.rx.tPrePos;
                if (tCopyLength <= tFirstPartLength) {
                    // situation: tAvailableSpace <= tFirstPartLength
                    memcpy(this.ptMemBlk->chMemory + this.ptMemBlk->tSizeUsedInByte, this.rx.ptMemBlkBuffer->chMemory + this.rx.tPrePos, tCopyLength);
                    this.ptMemBlk->tSizeUsedInByte += tCopyLength;
                    this.rx.tPrePos += tCopyLength;
                } else {
                    // situation: tAvailableSpace or tDataLength > tFirstPartLength
                    memcpy(this.ptMemBlk->chMemory + this.ptMemBlk->tSizeUsedInByte, this.rx.ptMemBlkBuffer->chMemory + this.rx.tPrePos, tFirstPartLength);
                    this.ptMemBlk->tSizeUsedInByte += tFirstPartLength;
                    this.rx.tPrePos = 0;
                    
                    memcpy(this.ptMemBlk->chMemory + this.ptMemBlk->tSizeUsedInByte, this.rx.ptMemBlkBuffer->chMemory, tCopyLength - tFirstPartLength);
                    this.ptMemBlk->tSizeUsedInByte += tCopyLength - tFirstPartLength;
                    this.rx.tPrePos += tCopyLength - tFirstPartLength;
                }
                
                if (tDataLength <= tAvailableSpace) {
                    BTYE_FIFO_RECEIVE_RESET_FSM();
                    emRetStatus = fsm_rt_cpl;
                    break;
                }
            }
            mem_blk_fifo_append(this.tCFG.ptMemBlkFifo, this.ptMemBlk);
            this.u4ReceiveState = NEW_MEM_BLOCK;
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
    
    byte_queue_cfg_t tQueueCFG = {0};
    
    switch(this.u4ReadByteState) {
        case START:
        {
            this.u4ReadByteState = DE_QUEUE;
            // break;
        }
        case DE_QUEUE:
        {
            if (dequeue_byte(&this.tQueue, pchData)) {
                BTYE_FIFO_READ_BYTE_RESET_FSM();
                break;
            }
            mem_blk_t *ptMemBlk = (mem_blk_t *)((uintptr_t)this.tQueue.tCFG.pchBuffer - offsetof(mem_blk_t, chMemory));
            mem_blk_fifo_append(this.tCFG.ptMemBlkFifo, ptMemBlk);
            this.u4ReadByteState = FETCH_MEM_BLK;
            //break;
        }
        case FETCH_MEM_BLK:
        {
            mem_blk_t *ptMemBlk = mem_blk_fifo_fetch(this.tCFG.ptMemBlkFifo);
            if (NULL == ptMemBlk) {
                break;
            }
            tQueueCFG = (byte_queue_cfg_t) {
                .tSize = ptMemBlk->tSizeInByte,
                .pchBuffer = ptMemBlk->chMemory,
            };
            this.u4ReadByteState = INIT_QUEUE;
            // break;
        }
        case INIT_QUEUE:
        {
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