// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT RTOS_UART_TX
#define DEBUG_PRINT_ENABLE_RTOS_UART_TX 1

#include "rtos_uart_tx.h"

RTOS_UART_TX_CALL_ATTR
static void uart_tx_local_write(
        rtos_uart_tx_t *ctx,
        const uint8_t buff[],
        size_t n)
{
    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_WAIT_FOREVER);

    for(int i = 0; i < n; i++){
        uart_tx(&ctx->dev, buff[i]);
    }
    
    rtos_osal_mutex_put(&ctx->lock);
}


void rtos_uart_tx_start(
        rtos_uart_tx_t *uart_tx_ctx)
{
    rtos_osal_mutex_create(&uart_tx_ctx->lock, "uart_tx_lock", RTOS_OSAL_RECURSIVE);
}

void rtos_uart_tx_init(
        rtos_uart_tx_t *ctx,
        const port_t tx_port,
        const uint32_t baud_rate,
        const uint8_t num_data_bits,
        const uart_parity_t parity,
        const uint8_t stop_bits,
        hwtimer_t tmr){
    
    //uart init
    uart_tx_blocking_init(
            &ctx->dev,
            tx_port,
            baud_rate,
            num_data_bits,
            parity,
            stop_bits,
            tmr);

    ctx->write = uart_tx_local_write;
}
