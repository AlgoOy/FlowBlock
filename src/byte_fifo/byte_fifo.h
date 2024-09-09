#ifndef __BYTE_FIFO_H__
#define __BYTE_FIFO_H__

/*============================ INCLUDES ======================================*/

#include "utils.h"
#include "./app_cfg.h"

#include "./byte_queue/byte_queue.h"
#include "./mem_blk_fifo/mem_blk_fifo.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct byte_fifo_cfg_t {
    mem_blk_fifo_t *ptMemBlkFifo;
    bool (*fnForRxTx)(uint8_t *const pchData, const uint16_t hwSizeInByte);
} byte_fifo_cfg_t;

typedef struct byte_fifo_t {
    uint32_t u4AddByteState     :4;
    uint32_t u4FlushState       :4;
    uint32_t u4ReceiveState     :4;
    uint32_t u4ReadByteState    :4;    
    uint32_t bIsTransmitCpl     :1;
    uint32_t bISReceiveCpl      :1;
    uint32_t                    :6;
    uint32_t                    :8;
    mem_blk_t *ptMemBlkforRxTx;
    mem_blk_t *ptMemBlkforQueue;
    byte_queue_t tQueue;
    byte_fifo_cfg_t tCFG;
} byte_fifo_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

ARM_NONNULL(1, 2)
extern
byte_fifo_t *byte_fifo_init(byte_fifo_t *ptByteFifo, byte_fifo_cfg_t *ptCFG);

ARM_NONNULL(1)
extern
void byte_fifo_deinit(byte_fifo_t *ptByteFifo);

ARM_NONNULL(1)
extern
fsm_rt_t byte_fifo_flush(byte_fifo_t *ptByteFifo);

ARM_NONNULL(1)
extern
void byte_fifo_user_report_transmit_cpl(byte_fifo_t *ptByteFifo);

ARM_NONNULL(1)
extern
fsm_rt_t byte_fifo_add_byte(byte_fifo_t *ptByteFifo, uint8_t chData);

ARM_NONNULL(1)
extern
fsm_rt_t byte_fifo_receive(byte_fifo_t *ptByteFifo);

ARM_NONNULL(1)
extern
void byte_fifo_user_report_receive_cpl(byte_fifo_t *ptByteFifo, uint16_t hwRemainingBytesSpace);

ARM_NONNULL(1, 2)
extern
fsm_rt_t byte_fifo_read_byte(byte_fifo_t *ptByteFifo, uint8_t *pchData);

#ifdef __cplusplus
}
#endif

#endif