#ifndef __BLOCK_QUEUE_H__
#define __BLOCK_QUEUE_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define byte uint8_t

typedef struct {
	byte        *pchBuffer;
	uint16_t    hwSize;
	uint16_t    hwHead;
	uint16_t    hwTail;
	uint16_t    hwLength;
} byte_queue_t;

void InitByteQueue(byte_queue_t *queue, byte *buffer, uint16_t size);
bool IsByteQueueEmpty(const byte_queue_t *queue);
bool EnQueueByte(byte_queue_t *queue, byte obj);
bool DeQueueByte(byte_queue_t *queue, byte *addr);

#ifdef __cplusplus
}
#endif


#endif