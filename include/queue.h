#ifndef __BLOCK_QUEUE_H__
#define __BLOCK_QUEUE_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t byte;

typedef struct byte_queue_t{
	byte        *pchBuffer;
	uint16_t    hwSize;
	uint16_t    hwHead;
	uint16_t    hwTail;
	uint16_t    hwLength;
} byte_queue_t;

extern
void byte_queue_init(byte_queue_t *ptQueue, byte *ptBuffer, uint16_t hwSize);

extern
bool is_byte_queue_empty(const byte_queue_t *ptQueue);

extern
bool enqueue_byte(byte_queue_t *ptQueue, byte chObj);

extern
bool dequeue_byte(byte_queue_t *ptQueue, byte *pchAddr);

#ifdef __cplusplus
}
#endif

#endif