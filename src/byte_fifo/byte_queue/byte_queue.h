#ifndef __BYTE_QUEUE_H__
#define __BYTE_QUEUE_H__

/*============================ INCLUDES ======================================*/

#include <stddef.h>
#include <stdbool.h>

#include "utils.h"
#include "./app_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef struct byte_queue_cfg_t {
    size_t tSize;
    uint8_t *pchBuffer;
} byte_queue_cfg_t;

typedef struct byte_queue_t{
	size_t tHead;
	size_t tTail;
	size_t tLength;
	byte_queue_cfg_t tCFG;
} byte_queue_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

ARM_NONNULL(1, 2)
extern
byte_queue_t *byte_queue_init_empty(byte_queue_t *ptThis, byte_queue_cfg_t *ptCFG);

ARM_NONNULL(1, 2)
extern
byte_queue_t *byte_queue_init_full(byte_queue_t *ptThis, byte_queue_cfg_t *ptCFG);

ARM_NONNULL(1)
extern
void byte_queue_deinit(byte_queue_t *ptQueue);

ARM_NONNULL(1)
extern
bool is_byte_queue_empty(const byte_queue_t *ptQueue);

ARM_NONNULL(1)
extern
size_t byte_queue_length(const byte_queue_t *ptThis);

ARM_NONNULL(1)
extern
bool enqueue_byte(byte_queue_t *ptQueue, uint8_t chObj);

ARM_NONNULL(1, 2)
extern
bool dequeue_byte(byte_queue_t *ptQueue, uint8_t *pchAddr);

#ifdef __cplusplus
}
#endif

#endif