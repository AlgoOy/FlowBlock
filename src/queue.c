/*============================ INCLUDES ======================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/*============================ MACROS ========================================*/

/*============================ MACROFIED FUNCTIONS ===========================*/
#undef this
#define this (*ptThis)
    
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

byte_queue_t * __byte_queue_init(byte_queue_cfg_t *ptCFG, byte_queue_t *ptThis)
{
    struct {
        bool ctl;
        bool buffer;
    } tUserAllocated = {false, false};
    
    byte_queue_cfg_t tCFG = {
        .tSize = 8,
        .pchBuffer = NULL,
    };
    
    if (NULL != ptCFG) {
        tCFG = *ptCFG;
    }
    
    if (NULL == tCFG.pchBuffer) {
        tCFG.pchBuffer = (byte *)malloc(sizeof(byte) * tCFG.tSize);
        if (NULL == tCFG.pchBuffer) {
            return NULL;
        }
    } else {
        tUserAllocated.buffer = true;
    }
    memset(tCFG.pchBuffer, 0, sizeof(byte) * tCFG.tSize);
    
    if (NULL == ptThis) {
        ptThis = (byte_queue_t *)malloc(sizeof(byte_queue_t));
        if (NULL == ptThis) {
            if (!tUserAllocated.buffer) {
                free(tCFG.pchBuffer);
            }
            return NULL;
        }
    } else {
        tUserAllocated.ctl = true;
    }
    memset(ptThis, 0, sizeof(byte_queue_t));
    
    *ptThis = (byte_queue_t) {
        .tHead = 0,
        .tTail = 0,
        .tLength = 0,
        .tSize = tCFG.tSize,
        .pchBuffer = tCFG.pchBuffer,
        .tUserAllocated = {
            .ctl = tUserAllocated.ctl,
            .buffer = tUserAllocated.buffer,
        },
    };
    
    return ptThis;
}

ARM_NONNULL(1)
void byte_queue_deinit(byte_queue_t *ptThis)
{
    assert(NULL != ptThis);
    
    memset(this.pchBuffer, 0, sizeof(byte) * this.tSize);
    
    if (!this.tUserAllocated.buffer) {
        free(this.pchBuffer);
    }
    
    if (!this.tUserAllocated.ctl) {
        memset(ptThis, 0, sizeof(byte_queue_t));
        free(ptThis);
    } else {
        memset(ptThis, 0, sizeof(byte_queue_t));
    }
}

ARM_NONNULL(1)
bool is_byte_queue_empty(const byte_queue_t *ptThis)
{
    assert(NULL != ptThis);
    
    return (ptThis->tLength == 0);
}

ARM_NONNULL(1)
bool enqueue_byte(byte_queue_t *ptThis, byte chObj)
{
    assert(NULL != ptThis);
    assert(NULL != this.pchBuffer);
    
    if (this.tLength < this.tSize) {
        this.pchBuffer[this.tTail] = chObj;
        this.tTail = (this.tTail + 1) % this.tSize;
        this.tLength++;
        return true;
    }
    return false;
}

ARM_NONNULL(1, 2)
bool dequeue_byte(byte_queue_t *ptThis, byte *pchAddr)
{
    assert(NULL != ptThis);
    assert(NULL != this.pchBuffer);
    assert(NULL != pchAddr);
    
    if (this.tLength > 0) {
        *pchAddr = this.pchBuffer[this.tHead];
        this.tHead = (this.tHead + 1) % this.tSize;
        this.tLength--;
        return true;
    }
    return false;
}