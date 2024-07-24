#ifndef __BLOCK_MEMORY_BLOCK_H__
#define __BLOCK_MEMORY_BLOCK_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mem_blk_t {
    struct mem_blk_t *ptNext;
    size_t tSizeInByte;
    uint8_t chMemory[];
} mem_blk_t;

typedef struct mem_blk_fifo_t {
    mem_blk_t *ptFreeList;
    struct {
        mem_blk_t *ptHead;
        mem_blk_t *ptTail;
    } FIFO;
} mem_blk_fifo_t;

extern
void mem_blk_fifo_init(mem_blk_fifo_t *ptFifo, size_t tNumOfMemBlk, size_t tSizeInByte);

extern
mem_blk_t* mem_blk_fifo_new(mem_blk_fifo_t *fifo);

extern
void mem_blk_fifo_append(mem_blk_fifo_t *fifo, mem_blk_t *blk);

extern
mem_blk_t* mem_blk_fifo_fetch(mem_blk_fifo_t *fifo);

extern
void mem_blk_fifo_free(mem_blk_fifo_t *fifo, mem_blk_t *blk);

#ifdef __cplusplus
}
#endif

#endif