#ifndef __MEM_BLK_FIFO_H__
#define __MEM_BLK_FIFO_H__

/*============================ INCLUDES ======================================*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils.h"
#include "./app_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/            
/*============================ TYPES =========================================*/

typedef struct mem_blk_t {
    struct mem_blk_t *ptNext;
    uint16_t tSizeInByte;       // DMA cache buffer max 64KB
    uint16_t tSizeUsedInByte; 
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
} mem_blk_fifo_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

ARM_NONNULL(1, 2)
extern
mem_blk_fifo_t *mem_blk_fifo_init(mem_blk_fifo_t *ptMemBlkFifo, mem_blk_fifo_cfg_t *ptCFG);

ARM_NONNULL(1)
extern
void mem_blk_fifo_deinit(mem_blk_fifo_t *ptMemBlkFifo);

ARM_NONNULL(1)
extern
mem_blk_t* mem_blk_fifo_new(mem_blk_fifo_t *ptMemBlkFifo);

ARM_NONNULL(1, 2)
extern
void mem_blk_fifo_append(mem_blk_fifo_t *ptMemBlkFifo, mem_blk_t *ptMemBlk);

ARM_NONNULL(1)
extern
mem_blk_t* mem_blk_fifo_fetch(mem_blk_fifo_t *ptMemBlkFifo);

ARM_NONNULL(1, 2)
extern
void mem_blk_fifo_free(mem_blk_fifo_t *ptMemBlkFifo, mem_blk_t *ptMemBlk);

#ifdef __cplusplus
}
#endif

#endif