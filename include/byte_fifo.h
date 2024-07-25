#ifndef __BLOCK_FIFO_H__
#define __BLOCK_FIFO_H__

/*============================ INCLUDES ======================================*/

#include "queue.h"
#include "memory_block.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/

#define byte_fifo_init(__BYTE_FIFO_CFG_PTR, ...)  \
            __byte_fifo_init((__BYTE_FIFO_CFG_PTR), (NULL, ##__VA_ARGS__))

/*============================ TYPES =========================================*/

typedef struct byte_fifo_cfg_t {
    mem_blk_fifo_t *ptMemBlk;
    bool (*fnTransmit)(byte *pchData, size_t tSizeInByte);
} byte_fifo_cfg_t;

typedef struct byte_fifo_t {
    uint8_t chState;
    mem_blk_fifo_t *ptMemBlk;
    mem_blk_t *ptBlk;
    byte_queue_t tQueue;
    bool bIsDMABusy;
    bool bIsUARTBusy;
    bool (*fnTransmit)(byte *pchData, size_t tSizeInByte);
    struct {
        bool ctl;
        bool buffer;
    } tUserAllocated;
} byte_fifo_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

ARM_NONNULL(1)
extern
byte_fifo_t *__byte_fifo_init(byte_fifo_cfg_t *ptCFG, byte_fifo_t *ptByteFifo);

ARM_NONNULL(1)
extern
void byte_fifo_deinit(byte_fifo_t *ptByteFifo);

ARM_NONNULL(1)
extern
fsm_rt_t byte_fifo_add_bytes(byte_fifo_t *ptByteFifo, byte chData);

ARM_NONNULL(1)
extern
void byte_fifo_flush(byte_fifo_t *ptByteFifo);

ARM_NONNULL(1)
extern
void byte_fifo_dma_irq_handler(byte_fifo_t *ptByteFifo);

#ifdef __cplusplus
}
#endif

#endif