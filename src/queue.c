#include "queue.h"

void byte_queue_init(byte_queue_t *ptQueue, byte *ptBuffer, uint16_t hwSize)
{
    if (NULL == ptQueue || NULL == ptBuffer || 0 == hwSize) {
        return;
    }
    ptQueue->pchBuffer = ptBuffer;
    ptQueue->hwSize = hwSize;
    ptQueue->hwHead = 0;
    ptQueue->hwTail = 0;
    ptQueue-> hwLength = 0;
}

bool is_byte_queue_empty(const byte_queue_t *ptQueue)
{
    if (NULL == ptQueue) {
        // If the ptQueue is NULL, it means this is an empty queue.
        return true;
    }
    return (ptQueue->hwLength == 0);
}

bool enqueue_byte(byte_queue_t *ptQueue, byte chObj)
{
    if (NULL == ptQueue || NULL == ptQueue->pchBuffer) {
        return false;
    }
    if (ptQueue->hwLength < ptQueue->hwSize) {
        ptQueue->pchBuffer[ptQueue->hwTail] = chObj;
        ptQueue->hwTail = (ptQueue->hwTail + 1) % ptQueue->hwSize;
        ptQueue->hwLength++;
        return true;
    }
    return false;
}

bool dequeue_byte(byte_queue_t *ptQueue, byte *pchAddr)
{
    if (NULL == ptQueue || NULL == ptQueue->pchBuffer || NULL == pchAddr) {
        return false;
    }
    if (ptQueue->hwLength > 0) {
        *pchAddr = ptQueue->pchBuffer[ptQueue->hwHead];
        ptQueue->hwHead = (ptQueue->hwHead + 1) % ptQueue->hwSize;
        ptQueue->hwLength--;
        return true;
    }
    return false;
}