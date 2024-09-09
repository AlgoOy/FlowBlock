/*============================ INCLUDES ======================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "./mem_blk_fifo.h"
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
mem_blk_fifo_t *mem_blk_fifo_init(mem_blk_fifo_t *ptThis, mem_blk_fifo_cfg_t *ptCFG)
{
    assert(NULL != ptThis);
    assert(NULL != ptCFG);
    assert(NULL != ptCFG->pptMemBlk);
    
    memset(ptThis, 0, sizeof(mem_blk_fifo_t));
        
    *ptThis = (mem_blk_fifo_t) {
        .ptFreeList = NULL,
        .FIFO.ptHead = NULL,
        .FIFO.ptTail = NULL,
    };
    
    for (size_t i = 0; i < ptCFG->tNumOfMemBlk; ++i) {
        mem_blk_t *ptMemBlk = ptCFG->pptMemBlk[i];
        memset(ptMemBlk, 0, sizeof(mem_blk_t) + sizeof(uint8_t) * ptCFG->tSizeInByte);
        
        *ptMemBlk = (mem_blk_t) {
            .ptNext = this.ptFreeList,
            .tSizeInByte = ptCFG->tSizeInByte,
            .tSizeUsedInByte = 0,
        };
        
        this.ptFreeList = ptMemBlk;
    }
    
    return ptThis;
}

ARM_NONNULL(1)
void mem_blk_fifo_deinit(mem_blk_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    while (this.ptFreeList) {
        mem_blk_t *ptMemBlk = this.ptFreeList;
        this.ptFreeList = this.ptFreeList->ptNext;
        
        memset(ptMemBlk, 0, sizeof(mem_blk_t) + sizeof(uint8_t) * ptMemBlk->tSizeInByte);
    }
    
    while (this.FIFO.ptHead) {
        mem_blk_t *ptMemBlk = this.FIFO.ptHead;
        this.FIFO.ptHead = this.FIFO.ptHead->ptNext;
        
        memset(ptMemBlk, 0, sizeof(mem_blk_t) + sizeof(uint8_t) * ptMemBlk->tSizeInByte);
    }

    memset(ptThis, 0, sizeof(mem_blk_fifo_t));
}

ARM_NONNULL(1)
mem_blk_t* mem_blk_fifo_new(mem_blk_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    if (NULL == this.ptFreeList) {
        return NULL;
    }
    
    mem_blk_t *ptMemBlk = NULL;
    
    __IRQ_SAFE {
        ptMemBlk = this.ptFreeList;
        this.ptFreeList = ptMemBlk->ptNext;
        ptMemBlk->ptNext = NULL;
    }

    return ptMemBlk;
}

ARM_NONNULL(1, 2)
void mem_blk_fifo_append(mem_blk_fifo_t *ptThis, mem_blk_t *ptMemBlk)
{
    assert(NULL != ptThis);
    assert(NULL != ptMemBlk);
    
    ptMemBlk->ptNext = NULL;

    __IRQ_SAFE {
        if (NULL == this.FIFO.ptTail) {
            this.FIFO.ptHead = ptMemBlk;
            this.FIFO.ptTail = ptMemBlk;
        } else {
            this.FIFO.ptTail->ptNext = ptMemBlk;
            this.FIFO.ptTail = ptMemBlk;
        }
    }
}

ARM_NONNULL(1)
mem_blk_t* mem_blk_fifo_fetch(mem_blk_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    if (NULL == this.FIFO.ptHead) {
        return NULL;
    }
    
    mem_blk_t *ptMemBlk = NULL;

    __IRQ_SAFE {
        ptMemBlk = this.FIFO.ptHead;
        this.FIFO.ptHead = ptMemBlk->ptNext;
        if (NULL == ptMemBlk->ptNext) {
            this.FIFO.ptTail = NULL;
        }
        ptMemBlk->ptNext = NULL;
    }

    return ptMemBlk;
}

ARM_NONNULL(1, 2)
void mem_blk_fifo_free(mem_blk_fifo_t *ptThis, mem_blk_t *ptMemBlk)
{
    assert(NULL != ptThis);
    assert(NULL != ptMemBlk);
    
    __IRQ_SAFE {
        ptMemBlk->ptNext = this.ptFreeList;
        this.ptFreeList = ptMemBlk;
    }
}