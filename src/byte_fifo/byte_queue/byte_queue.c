/*============================ INCLUDES ======================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "./byte_queue.h"
#include "./__common.h"

/*============================ MACROS ========================================*/

/*============================ MACROFIED FUNCTIONS ===========================*/
#undef this
#define this (*ptThis)
    
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

ARM_NONNULL(1, 2)
byte_queue_t *byte_queue_init(byte_queue_t *ptThis, byte_queue_cfg_t *ptCFG)
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

ARM_NONNULL(1)
void byte_queue_deinit(byte_queue_t *ptThis)
{
    assert(NULL != ptThis);
    
    // destory content 
    memset(this.tCFG.pchBuffer, 0, sizeof(uint8_t) * this.tCFG.tSize);
    
    memset(ptThis, 0, sizeof(byte_queue_t));
}

ARM_NONNULL(1)
bool is_byte_queue_empty(const byte_queue_t *ptThis)
{
    assert(NULL != ptThis);
    
    return (ptThis->tLength == 0);
}

ARM_NONNULL(1)
bool enqueue_byte(byte_queue_t *ptThis, uint8_t chObj)
{
    assert(NULL != ptThis);
    assert(NULL != this.tCFG.pchBuffer);
    
    bool bRet = false;
    
    __IRQ_SAFE {
        if (this.tLength < this.tCFG.tSize) {
            this.tCFG.pchBuffer[this.tTail] = chObj;
            this.tTail = (this.tTail + 1) % this.tCFG.tSize;
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
            *pchAddr = this.tCFG.pchBuffer[this.tHead];
            this.tHead = (this.tHead + 1) % this.tCFG.tSize;
            this.tLength--;
            bRet = true;
        }
    }
    return bRet;
}