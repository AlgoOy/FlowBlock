#include "memory_block.h"
#include "cmsis_armclang.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>

void mem_blk_fifo_init(mem_blk_fifo_t *ptFifo, size_t tNumOfMemBlk, size_t tSizeInByte)
{
    if (NULL == ptFifo) {
        return;
    }
    
    ptFifo->ptFreeList = NULL;
    ptFifo->FIFO.ptHead = NULL;
    ptFifo->FIFO.ptTail = NULL;
    
    for (size_t i = 0; i < tNumOfMemBlk; ++i) {
        mem_blk_t *ptBlk = (mem_blk_t *)(malloc(sizeof(mem_blk_t) + sizeof(byte) * tSizeInByte));
        if (NULL == ptBlk) {
            while (ptFifo->ptFreeList) {
                mem_blk_t *temp = ptFifo->ptFreeList;
                ptFifo->ptFreeList = ptFifo->ptFreeList->ptNext;
                free(temp);
            }
            return;
        }
        memset(ptBlk, 0, sizeof(mem_blk_t) + sizeof(byte) * tSizeInByte);
        ptBlk->tSizeInByte = tSizeInByte;
        mem_blk_fifo_free(ptFifo, ptBlk);
    }
}

mem_blk_t* mem_blk_fifo_new(mem_blk_fifo_t *ptFifo)
{
    if (NULL == ptFifo || NULL == ptFifo->ptFreeList) {
        return NULL;
    }
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    mem_blk_t *blk = ptFifo->ptFreeList;
    ptFifo->ptFreeList = blk->ptNext;
    blk->ptNext = NULL;
    __set_PRIMASK(primask);
    return blk;
}

void mem_blk_fifo_append(mem_blk_fifo_t *ptFifo, mem_blk_t *ptBlk)
{
    if (NULL == ptFifo || NULL == ptBlk) {
        return;
    }
    ptBlk->ptNext = NULL;
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    if (NULL == ptFifo->FIFO.ptTail) {
        ptFifo->FIFO.ptHead = ptBlk;
        ptFifo->FIFO.ptTail = ptBlk;
    } else {
        ptFifo->FIFO.ptTail->ptNext = ptBlk;
        ptFifo->FIFO.ptTail = ptBlk;
    }
    __set_PRIMASK(primask);
}

mem_blk_t* mem_blk_fifo_fetch(mem_blk_fifo_t *ptFifo)
{
    if (NULL == ptFifo || NULL == ptFifo->FIFO.ptHead) {
        return NULL;
    }
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    mem_blk_t *blk = ptFifo->FIFO.ptHead;
    ptFifo->FIFO.ptHead = blk->ptNext;
    if (NULL == blk->ptNext) {
        ptFifo->FIFO.ptTail = NULL;
    }
    blk->ptNext = NULL;
    __set_PRIMASK(primask);
    return blk;
}

void mem_blk_fifo_free(mem_blk_fifo_t *ptFifo, mem_blk_t *ptBlk)
{
    if (NULL == ptFifo || NULL == ptBlk) {
        return;
    }
    ptBlk->ptNext = ptFifo->ptFreeList;
    ptFifo->ptFreeList = ptBlk;
}