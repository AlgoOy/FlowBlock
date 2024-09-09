/*============================ INCLUDES ======================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "./byte_queue.h"
#include "./__common.h"

/*============================ MACROS ========================================*/

/*============================ MACROFIED FUNCTIONS ===========================*/

#define min(x, y) (((x) < (y)) ? (x) : (y))

#undef this
#define this (*ptThis)
    
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

ARM_NONNULL(1, 2)
byte_queue_t *byte_queue_init_empty(byte_queue_t *ptThis, byte_queue_cfg_t *ptCFG)
{
    assert(NULL != ptThis);
    assert(NULL != ptCFG);
    assert(NULL != ptCFG->pchBuffer);
    
    memset(ptThis, 0, sizeof(byte_queue_t));
    
    *ptThis = (byte_queue_t) {
        .tHead = 0,
        .tTail = 0,
        .tLength = 0,
        .tCFG = *ptCFG,
    };
    
    return ptThis;
}

ARM_NONNULL(1, 2)
byte_queue_t *byte_queue_init_full(byte_queue_t *ptThis, byte_queue_cfg_t *ptCFG)
{
    assert(NULL != ptThis);
    assert(NULL != ptCFG);
    assert(NULL != ptCFG->pchBuffer);
    
    memset(ptThis, 0, sizeof(byte_queue_t));
    
    *ptThis = (byte_queue_t) {
        .tHead = 0,
        .tTail = 0,
        .tLength = ptCFG->tSize,
        .tCFG = *ptCFG,
    };
    
    return ptThis;
}

ARM_NONNULL(1)
void byte_queue_deinit(byte_queue_t *ptThis)
{
    assert(NULL != ptThis);
    
    memset(ptThis, 0, sizeof(byte_queue_t));
}

ARM_NONNULL(1)
bool is_byte_queue_empty(const byte_queue_t *ptThis)
{
    assert(NULL != ptThis);
    
    return (this.tLength == 0);
}

ARM_NONNULL(1)
size_t byte_queue_length(const byte_queue_t *ptThis)
{
    assert(NULL != ptThis);
    
    return this.tLength;
}

ARM_NONNULL(1)
bool enqueue_byte(byte_queue_t *ptThis, uint8_t chObj)
{
    assert(NULL != ptThis);
    assert(NULL != this.tCFG.pchBuffer);
    
    bool bRet = false;
    
    // optimize %
    __IRQ_SAFE {
        if (this.tLength < this.tCFG.tSize) {
            this.tCFG.pchBuffer[this.tTail++] = chObj;  // TODO: 无保护指针
            if (this.tTail >= this.tCFG.tSize) {
                this.tTail = 0;
            }
            this.tLength++;
            bRet = true;
        }
    }
    
    return bRet;
}

ARM_NONNULL(1, 2)
bool dequeue_byte(byte_queue_t *ptThis, uint8_t *pchAddr)
{
    assert(NULL != ptThis);
    assert(NULL != this.tCFG.pchBuffer);
    assert(NULL != pchAddr);
    
    bool bRet = false;
    
    __IRQ_SAFE {
        if (this.tLength > 0) {
            *pchAddr = this.tCFG.pchBuffer[this.tHead++];
            if (this.tHead >= this.tCFG.tSize) {
                this.tHead = 0;
            }
            this.tLength--;
            bRet = true;
        }
    }
    return bRet;
}

// TODO:int32_t return number of bytes
size_t enqueue_bytes(byte_queue_t *ptThis, uint8_t *buffer, size_t tNum)
{
    assert(NULL != ptThis);
    assert(NULL != this.tCFG.pchBuffer);
    assert(NULL != buffer);
    
    size_t first_part_len = 0, second_part_len = 0;
    
    __IRQ_SAFE {
        tNum = min(tNum, this.tCFG.tSize - this.tLength);
        
        if (tNum == 0) {
            return 0;
        }
        
        if (this.tTail + tNum <= this.tCFG.tSize) {
            memcpy(this.tCFG.pchBuffer + this.tTail, buffer, tNum);
        } else {
            first_part_len = this.tCFG.tSize - this.tTail;
            second_part_len = tNum - first_part_len;
            memcpy(this.tCFG.pchBuffer + this.tTail, buffer, first_part_len);
            memcpy(this.tCFG.pchBuffer, buffer + first_part_len, second_part_len);
        }
        
        this.tTail = this.tTail + tNum >= this.tCFG.tSize ? this.tTail + tNum - this.tCFG.tSize : this.tTail + tNum;
        this.tLength += tNum;
    }
    
    return tNum;
}

// TODO:int32_t return number of bytes
size_t dequeue_bytes(byte_queue_t *ptThis, uint8_t *buffer, size_t tNum)
{
    assert(NULL != ptThis);
    assert(NULL != this.tCFG.pchBuffer);
    assert(NULL != buffer);
    
    size_t first_part_len = 0, second_part_len = 0;
    
    __IRQ_SAFE {
        tNum = min(tNum, this.tLength);
        
        if (tNum == 0) {
            return 0;
        }
        
        if (this.tHead + tNum <= this.tCFG.tSize) {
            memcpy(buffer, this.tCFG.pchBuffer + this.tHead, tNum);
        } else {
            first_part_len = this.tCFG.tSize - this.tHead;
            second_part_len = tNum - first_part_len;
            memcpy(buffer, this.tCFG.pchBuffer + this.tHead, first_part_len);
            memcpy(buffer + first_part_len, this.tCFG.pchBuffer, second_part_len);
        }
        
        this.tHead = this.tHead + tNum >= this.tCFG.tSize ? this.tHead + tNum - this.tCFG.tSize : this.tHead + tNum;
        this.tLength -= tNum;
    }
    
    return tNum;
}