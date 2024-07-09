#include "queue.h"

void InitByteQueue(byte_queue_t *queue, byte *buffer, uint16_t size)
{
    queue->pchBuffer = buffer;
    queue->hwSize = size;
    queue->hwHead = 0;
    queue->hwTail = 0;
    queue-> hwLength = 0;
}

bool IsByteQueueEmpty(const byte_queue_t *queue)
{
    return (queue->hwLength == 0);
}

bool EnQueueByte(byte_queue_t *queue, byte obj)
{
    if (queue->hwLength < queue->hwSize) {
        queue->pchBuffer[queue->hwTail] = obj;
        queue->hwTail = (queue->hwTail + 1) % queue->hwSize;
        queue->hwLength++;
        return true;
    } else {
        return false;
    }
}

bool DeQueueByte(byte_queue_t *queue, byte *addr)
{
    if (queue->hwLength > 0) {
        *addr = queue->pchBuffer[queue->hwHead];
        queue->hwHead = (queue->hwHead + 1) % queue->hwSize;
        queue->hwLength--;
        return true;
    } else {
        return false;
    }
}