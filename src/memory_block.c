/*============================ INCLUDES ======================================*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "memory_block.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
#undef this
#define this (*ptThis)
    
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

mem_blk_fifo_t * __mem_blk_fifo_init(mem_blk_fifo_cfg_t *ptCFG, mem_blk_fifo_t *ptThis)
{
    struct {
        bool ctl;
        bool buffer;
    } tUserAllocated = {false, false};
    
    mem_blk_fifo_cfg_t tCFG = {
        .tNumOfMemBlk = 8,
        .tSizeInByte = 8,
        .pptMemBlk = NULL,
    };
    
    if (NULL != ptCFG) {
        tCFG = *ptCFG;
    }
    
    if (NULL != tCFG.pptMemBlk) {
        tUserAllocated.buffer = true;
    }
    
    if (NULL == ptThis) {
        ptThis = (mem_blk_fifo_t *)malloc(sizeof(mem_blk_fifo_t));
        if (NULL == ptThis) {
            return NULL;
        }
    } else {
        tUserAllocated.ctl = true;
    }
    memset(ptThis, 0, sizeof(mem_blk_fifo_t));
    
    *ptThis = (mem_blk_fifo_t) {
        .FIFO.ptHead = NULL,
        .FIFO.ptTail = NULL,
        .ptFreeList = NULL,
        .tUserAllocated = {
            .ctl = tUserAllocated.ctl,
            .buffer = tUserAllocated.buffer,
        },
    };
    
    for (size_t i = 0; i < tCFG.tNumOfMemBlk; ++i) {
        mem_blk_t *ptBlk = NULL;
        
        if (NULL == tCFG.pptMemBlk) {
            ptBlk = (mem_blk_t *)(malloc(sizeof(mem_blk_t) + sizeof(byte) * tCFG.tSizeInByte));
            if (NULL == ptBlk) {
                while (this.ptFreeList) {
                    mem_blk_t *ptTmpBlk = this.ptFreeList;
                    this.ptFreeList = this.ptFreeList->ptNext;
                    free(ptTmpBlk);
                }
                if (!tUserAllocated.ctl) {
                    free(ptThis);
                }
                return false;
            }
        } else {
            ptBlk = tCFG.pptMemBlk[i];
        }
        memset(ptBlk, 0, sizeof(mem_blk_t) + sizeof(byte) * tCFG.tSizeInByte);
        
        *ptBlk = (mem_blk_t) {
            .ptNext = NULL,
            .tSizeInByte = tCFG.tSizeInByte,
        };
        
        ptBlk->ptNext = this.ptFreeList;
        this.ptFreeList = ptBlk;
    }
    
    return ptThis;
}

ARM_NONNULL(1)
void mem_blk_fifo_deinit(mem_blk_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    while (this.ptFreeList) {
        mem_blk_t *ptTmpBlk = this.ptFreeList;
        this.ptFreeList = this.ptFreeList->ptNext;
        
        memset(ptTmpBlk, 0, sizeof(mem_blk_t) + sizeof(byte) * ptTmpBlk->tSizeInByte);
        
        if (!this.tUserAllocated.buffer) {
            free(ptTmpBlk);
        }
    }
    
    while (this.FIFO.ptHead) {
        mem_blk_t *ptTmpBlk = this.FIFO.ptHead;
        this.FIFO.ptHead = this.FIFO.ptHead->ptNext;
        
        memset(ptTmpBlk, 0, sizeof(mem_blk_t) + sizeof(byte) * ptTmpBlk->tSizeInByte);
        
        if (!this.tUserAllocated.buffer) {
            free(ptTmpBlk);
        }
    }
    
    if (!this.tUserAllocated.ctl) {
        memset(ptThis, 0, sizeof(mem_blk_fifo_t));
        free(ptThis);
    } else {
        memset(ptThis, 0, sizeof(mem_blk_fifo_t));
    }
}

ARM_NONNULL(1)
mem_blk_t* mem_blk_fifo_new(mem_blk_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    if (NULL == this.ptFreeList) {
        return NULL;
    }
    
//    uint32_t primask = __get_PRIMASK();
//    __disable_irq();
    mem_blk_t *ptBlk = this.ptFreeList;
    this.ptFreeList = ptBlk->ptNext;
    ptBlk->ptNext = NULL;
//    __set_PRIMASK(primask);
    return ptBlk;
}

ARM_NONNULL(1, 2)
void mem_blk_fifo_append(mem_blk_fifo_t *ptThis, mem_blk_t *ptBlk)
{
    assert(NULL != ptThis);
    assert(NULL != ptBlk);
    
    ptBlk->ptNext = NULL;
//    uint32_t primask = __get_PRIMASK();
//    __disable_irq();
    if (NULL == this.FIFO.ptTail) {
        this.FIFO.ptHead = ptBlk;
        this.FIFO.ptTail = ptBlk;
    } else {
        this.FIFO.ptTail->ptNext = ptBlk;
        this.FIFO.ptTail = ptBlk;
    }
//    __set_PRIMASK(primask);
}

ARM_NONNULL(1)
mem_blk_t* mem_blk_fifo_fetch(mem_blk_fifo_t *ptThis)
{
    assert(NULL != ptThis);
    
    if (NULL == this.FIFO.ptHead) {
        return NULL;
    }
//    uint32_t primask = __get_PRIMASK();
//    __disable_irq();
    mem_blk_t *ptBlk = this.FIFO.ptHead;
    this.FIFO.ptHead = ptBlk->ptNext;
    if (NULL == ptBlk->ptNext) {
        this.FIFO.ptTail = NULL;
    }
    ptBlk->ptNext = NULL;
//    __set_PRIMASK(primask);
    return ptBlk;
}

ARM_NONNULL(1, 2)
void mem_blk_fifo_free(mem_blk_fifo_t *ptThis, mem_blk_t *ptBlk)
{
    assert(NULL != ptThis);
    assert(NULL != ptBlk);
    
    ptBlk->ptNext = this.ptFreeList;
    this.ptFreeList = ptBlk;
}