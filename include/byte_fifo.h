#ifndef __BLOCK_FIFO_H__
#define __BLOCK_FIFO_H__

#include "memory_block.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef volatile struct byte_fifo_t {
    uint8_t chState;
    mem_blk_fifo_t *ptMemBlk;
    byte_queue_t tQueue;
    mem_blk_t *ptBlk;
    bool bIsDMABusy;
} byte_fifo_t;

typedef enum fsm_rt_t {
    fsm_rt_err      = -1,
    fsm_rt_on_going = 0,
    fsm_rt_cpl      = 1,
} fsm_rt_t;

extern
void byte_fifo_init(byte_fifo_t *ptByteFifo, mem_blk_fifo_t *ptMemBlk);

extern
fsm_rt_t byte_fifo_add_bytes(byte_fifo_t *ptByteFifo, byte chData);

extern
void byte_fifo_flush(byte_fifo_t *ptByteFifo);

extern
void byte_fifo_dma_irq_handler(byte_fifo_t *ptByteFifo);

#ifdef __cplusplus
}
#endif

#endif