// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT RTOS_UART_RX
#define DEBUG_PRINT_ENABLE_RTOS_UART_RX 1

#include <xcore/triggerable.h>
#include <string.h>

#include "rtos_interrupt.h"
#include "rtos_uart_rx.h"
#include "task.h"


DEFINE_RTOS_INTERRUPT_CALLBACK(rtos_uart_rx_isr, arg)
{
    rtos_uart_rx_t *ctx = (rtos_uart_rx_t*)arg;

    /* Grab byte received from rx which triggered ISR */
    uint8_t byte = s_chan_in_byte(ctx->c.end_b);
    
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

    /* We already know the task handle of the receiver so cast to correct type */
    TaskHandle_t* notified_task = (TaskHandle_t*)&ctx->app_thread;
    vTaskNotifyGiveFromISR( *notified_task, &pxHigherPriorityTaskWoken);
    uart_buffer_error_t err = push_byte_into_buffer(&ctx->isr_to_app_fifo, byte);
    if(err != UART_BUFFER_OK){
        ctx->cb_flags |= UR_OVERRUN_ERR_CB_FLAG;
    }

    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}




/* There is no rx_complete callback setup and so cb_flags == 0  means rx_complete no issues */
HIL_UART_RX_CALLBACK_ATTR
static void uart_rx_error_callback(uart_callback_code_t callback_code, void * app_data){
    rtos_uart_rx_t *ctx = (rtos_uart_rx_t*) app_data;
    ctx->cb_flags |= (1 << callback_code); /* Or into flag bits. This is an optimisation based on UR_START_BIT_ERR_CB_CODE == 2 */
}

static void uart_rx_hil_thread(rtos_uart_rx_t *ctx)
{
    /* consume token (synch with RTOS driver) */
    (void) s_chan_in_byte(ctx->c.end_a);

    /* We cannot afford for the RX task to be blocked or any ISRs in between frames */
    rtos_interrupt_mask_all();
    for (;;) {
        uint8_t byte = uart_rx(&ctx->dev);

        /*  Now store byte and send to trigger ISR */
        s_chan_out_byte(ctx->c.end_a, byte);
    }
}

static void uart_rx_app_thread(rtos_uart_rx_t *ctx)
{
    ctx->cb_flags = 0;

    /* send token (synch with HIL logical core) */
    s_chan_out_byte(ctx->c.end_b, 0);

    /* signal that UART Rx is go */
    if (ctx->rx_start_cb != NULL) {
        ctx->rx_start_cb(ctx);
    }

    for (;;) {
        /* Block until notification from ISR */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        uint8_t bytes[RTOS_UART_RX_BUF_LEN];
        unsigned bytes_read = 0;
        uart_buffer_error_t ret = UART_BUFFER_EMPTY;
        do{
            ret = pop_byte_from_buffer(&ctx->isr_to_app_fifo, &bytes[bytes_read]);
            bytes_read += 1;
        } while(ret == UART_BUFFER_OK);
        bytes_read -= 1; /* important as we incremented this for the last read fail too */
    
        if(bytes_read){
            size_t xBytesSent = xStreamBufferSend(ctx->app_byte_buffer, bytes, bytes_read, 0);

            if(xBytesSent != bytes_read){
                ctx->cb_flags |= UR_OVERRUN_ERR_CB_FLAG;
            }

            if ((ctx->cb_flags & RX_ERROR_FLAGS) && ctx->rx_error_cb) {
                (*ctx->rx_error_cb)(ctx, ctx->cb_flags & RX_ERROR_FLAGS);
                ctx->cb_flags = 0;
            }

            if (ctx->rx_complete_cb) {
                (*ctx->rx_complete_cb)(ctx);
            }
        }
    }
}


size_t rtos_uart_rx_read(rtos_uart_rx_t *ctx, uint8_t *buf, size_t n, rtos_osal_tick_t timeout){
    rtos_osal_tick_t t_entry = rtos_osal_tick_get();
    size_t num_read_total = 0;
    rtos_osal_tick_t time_elapsed = 0;
    while(num_read_total < n && time_elapsed < timeout){
        size_t num_rx = xStreamBufferReceive(ctx->app_byte_buffer,
                                            &buf[num_read_total],
                                            n - num_read_total,
                                            timeout - time_elapsed);
        num_read_total += num_rx;
        time_elapsed = rtos_osal_tick_get() - t_entry;
    }
    return num_read_total;
}

void rtos_uart_rx_reset_buffer(rtos_uart_rx_t *ctx){
    xStreamBufferReset(ctx->app_byte_buffer);
}


void rtos_uart_rx_init(
        rtos_uart_rx_t *uart_rx_ctx,
        uint32_t io_core_mask,
        port_t rx_port,
        uint32_t baud_rate,
        uint8_t data_bits,
        uart_parity_t parity,
        uint8_t stop_bits,
        hwtimer_t tmr)
{
    memset(uart_rx_ctx, 0, sizeof(rtos_uart_rx_t));

    rtos_printf("rtos_uart_rx_init\n");

    uart_rx_init(
        &uart_rx_ctx->dev,
        rx_port,
        baud_rate,
        data_bits,
        parity,
        stop_bits,
        tmr,
        NULL, /* Unbuffered (blocking) version with no ISR */
        0,
        NULL, /* Rx complete callback not needed */
        uart_rx_error_callback,
        uart_rx_ctx
        );
 

    uart_rx_ctx->c = s_chan_alloc();

    rtos_osal_thread_create(
            &uart_rx_ctx->hil_thread,
            "uart_rx_hil_thread",
            (rtos_osal_entry_function_t) uart_rx_hil_thread,
            uart_rx_ctx,
            RTOS_THREAD_STACK_SIZE(uart_rx_hil_thread),
            RTOS_OSAL_HIGHEST_PRIORITY);

    /* Ensure the UART thread is never preempted - effectively give whole logical core */
    rtos_osal_thread_preemption_disable(&uart_rx_ctx->hil_thread);
    /* And ensure it only runs on one of the specified cores */
    rtos_osal_thread_core_exclusion_set(&uart_rx_ctx->hil_thread, ~io_core_mask);
}


void rtos_uart_rx_start(
        rtos_uart_rx_t *uart_rx_ctx,
        void *app_data,
        RTOS_UART_RX_CALLBACK_ATTR rtos_uart_rx_started_cb_t rx_start,
        RTOS_UART_RX_CALLBACK_ATTR rtos_uart_rx_complete_cb_t rx_complete_cb,
        RTOS_UART_RX_CALLBACK_ATTR rtos_uart_rx_error_t rx_error,
        unsigned interrupt_core_id,
        unsigned priority,
        size_t app_rx_buff_size){
    
    /* Init callbacks & args */
    uart_rx_ctx->app_data = app_data;
    uart_rx_ctx->rx_start_cb = rx_start;
    uart_rx_ctx->rx_complete_cb = rx_complete_cb;
    uart_rx_ctx->rx_error_cb = rx_error;

    uart_rx_ctx->cb_flags = 0; /* Clear all cb code bits */

    /* Ensure that the UART interrupt is enabled on the requested core */
    uint32_t core_exclude_map = 0;
    rtos_osal_thread_core_exclusion_get(NULL, &core_exclude_map);
    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << interrupt_core_id));

    triggerable_setup_interrupt_callback(uart_rx_ctx->c.end_b, uart_rx_ctx, RTOS_INTERRUPT_CALLBACK(rtos_uart_rx_isr));
    triggerable_enable_trigger(uart_rx_ctx->c.end_b);

    /* Restore the core exclusion map for the calling thread */
    rtos_osal_thread_core_exclusion_set(NULL, core_exclude_map);

    /* Setup buffer between ISR and receiving app driver thread */
    init_buffer(&uart_rx_ctx->isr_to_app_fifo, uart_rx_ctx->isr_to_app_fifo_storage, RTOS_UART_RX_BUF_LEN);

    /* Setup buffer between uart_app_thread and user app */
    uart_rx_ctx->app_byte_buffer = xStreamBufferCreate(app_rx_buff_size, 1);

    rtos_osal_thread_create(
            &uart_rx_ctx->app_thread,
            "uart_rx_app_thread",
            (rtos_osal_entry_function_t) uart_rx_app_thread,
            uart_rx_ctx,
            RTOS_THREAD_STACK_SIZE(uart_rx_app_thread),
            priority);

    if(rx_complete_cb != NULL){
        (*rx_complete_cb)(uart_rx_ctx);
    }
}
