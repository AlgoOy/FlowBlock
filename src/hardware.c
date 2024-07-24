#include "hardware.h"
#include "byte_fifo.h"
#include <stdio.h>

extern byte_fifo_t byte_fifo;

void dma_tx_cpl_callback(DMA_HandleTypeDef *hdma)
{
    byte_fifo_dma_irq_handler(&byte_fifo);
}