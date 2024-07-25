#ifndef __BLOCK_QUEUE_H__
#define __BLOCK_QUEUE_H__

/*============================ INCLUDES ======================================*/

#include <stddef.h>
#include <stdbool.h>

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/

#define byte_queue_init(__BYTE_QUEUE_CFG_PTR, ...)  \
            __byte_queue_init((__BYTE_QUEUE_CFG_PTR), (NULL, ##__VA_ARGS__))

/*============================ TYPES =========================================*/

typedef struct byte_queue_cfg_t {
    size_t tSize;
    byte *pchBuffer;
} byte_queue_cfg_t;

typedef struct byte_queue_t{
	size_t tHead;
	size_t tTail;
	size_t tLength;
	size_t tSize;
	byte *pchBuffer;
    struct {
        bool ctl;
        bool buffer;
    } tUserAllocated;
} byte_queue_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern
byte_queue_t * __byte_queue_init(byte_queue_cfg_t *ptCFG, byte_queue_t *ptQueue);

ARM_NONNULL(1)
extern
void byte_queue_deinit(byte_queue_t *ptQueue);

ARM_NONNULL(1)
extern
bool is_byte_queue_empty(const byte_queue_t *ptQueue);

ARM_NONNULL(1)
extern
bool enqueue_byte(byte_queue_t *ptQueue, byte chObj);

ARM_NONNULL(1, 2)
extern
bool dequeue_byte(byte_queue_t *ptQueue, byte *pchAddr);

#ifdef __cplusplus
}
#endif

#endif