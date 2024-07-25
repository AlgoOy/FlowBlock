#ifndef __BLOCK_MEMORY_BLOCK_H__
#define __BLOCK_MEMORY_BLOCK_H__

/*============================ INCLUDES ======================================*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/

#define mem_blk_fifo_init(__MEM_BLK_FIFO_CFG_PTR, ...)  \
            __mem_blk_fifo_init((__MEM_BLK_FIFO_CFG_PTR), (NULL, ##__VA_ARGS__))
            
/*============================ TYPES =========================================*/

typedef struct mem_blk_t {
    struct mem_blk_t *ptNext;
    size_t tSizeInByte;
    uint8_t chMemory[];
} mem_blk_t;

typedef struct mem_blk_fifo_cfg_t {
    size_t tNumOfMemBlk;
    size_t tSizeInByte;
    mem_blk_t **pptMemBlk;
} mem_blk_fifo_cfg_t;

typedef struct mem_blk_fifo_t {
    mem_blk_t *ptFreeList;
    struct {
        mem_blk_t *ptHead;
        mem_blk_t *ptTail;
    } FIFO;
    struct {
        bool ctl;
        bool buffer;
    } tUserAllocated;
} mem_blk_fifo_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern
mem_blk_fifo_t * __mem_blk_fifo_init(mem_blk_fifo_cfg_t *ptCFG, mem_blk_fifo_t *ptMemBlk);

ARM_NONNULL(1)
extern
void mem_blk_fifo_deinit(mem_blk_fifo_t *ptFifo);

ARM_NONNULL(1)
extern
mem_blk_t* mem_blk_fifo_new(mem_blk_fifo_t *ptFifo);

ARM_NONNULL(1, 2)
extern
void mem_blk_fifo_append(mem_blk_fifo_t *ptFifo, mem_blk_t *ptMemBlk);

ARM_NONNULL(1)
extern
mem_blk_t* mem_blk_fifo_fetch(mem_blk_fifo_t *ptFifo);

ARM_NONNULL(1, 2)
extern
void mem_blk_fifo_free(mem_blk_fifo_t *ptFifo, mem_blk_t *ptMemBlk);

#ifdef __cplusplus
}
#endif

#endif