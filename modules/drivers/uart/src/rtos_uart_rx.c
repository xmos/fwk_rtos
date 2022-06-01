// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT RTOS_UART_RX
#define DEBUG_PRINT_ENABLE_RTOS_UART_RX 1

#include <xcore/triggerable.h>
#include <string.h>

#include "rtos_interrupt.h"
#include "rtos_uart_rx.h"
#include "task.h"

#include <print.h>
#include <xcore/hwtimer.h>


DEFINE_RTOS_INTERRUPT_CALLBACK(rtos_uart_rx_isr, arg)
{
    rtos_uart_rx_t *ctx = (rtos_uart_rx_t*)arg;

    /* Grab byte received from rx which triggered ISR */
    uint8_t byte = s_chan_in_byte(ctx->c.end_b);
    uint8_t cb_flags = s_chan_in_byte(ctx->c.end_b);

    /* Note only error flags are set so set complete flag too to eensure  */
    cb_flags |= UR_COMPLETE_CB_FLAG;
    

    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;


    uint32_t t0 = get_reference_time();
    vTaskNotifyGiveIndexedFromISR( ctx->isr_notification_task,
                                   1,
                                   &pxHigherPriorityTaskWoken );
    uart_buffer_error_t err = push_byte_into_buffer(&ctx->isr_to_app_fifo, byte);
    if(err != UART_BUFFER_OK){
        cb_flags |= UR_OVERRUN_ERR_CB_FLAG;
    }
    ctx->cb_flags = cb_flags; /* These should only get reset by the app thread */
    uint32_t t1 = get_reference_time();
    // printintln(t1-t0);
    // size_t xBytesSent = xStreamBufferSendFromISR(ctx->isr_byte_buffer, &byte, 1, &pxHigherPriorityTaskWoken);

    // if(xBytesSent != 1){
    //     cb_flags |= UR_OVERRUN_ERR_CB_FLAG;
    // }
    // rtos_osal_event_group_set_bits(&ctx->events, cb_flags);

    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}


// void test_b(void){
//     printstr("Task B about to unblock..\n");
//     xTaskToNotify = xTaskGetCurrentTaskHandle();

//     uint32_t val = ulTaskNotifyTakeIndexed( 1, pdTRUE, portMAX_DELAY );

//     rtos_printf("Task B got notification: %u\n", val);
//     while(1);
// }



/* There is no rx_complete callback setup and so cb_flags == 0  means rx_complete no issues */
HIL_UART_RX_CALLBACK_ATTR
static void uart_rx_error_callback(void * app_data){
    rtos_uart_rx_t *ctx = (rtos_uart_rx_t*) app_data;
    uart_callback_code_t cb_code = ctx->dev.cb_code;
    ctx->cb_flags |= (1 << cb_code); /* Or into flag bits. This is an optimisation based on UR_START_BIT_ERR_CB_CODE == 2 */
}

static void uart_rx_hil_thread(rtos_uart_rx_t *ctx)
{
    /* consume token (synch with RTOS driver) */
    (void) s_chan_in_byte(ctx->c.end_a);
    printstr("Grrr\n");
    /* We cannot afford for the RX task to be blocked or any ISRs in between frames */
    rtos_interrupt_mask_all();
    for (;;) {
        uint8_t byte = uart_rx(&ctx->dev);

        /*  Now store byte and send along with error flags. These will stay in synch. */
        s_chan_out_byte(ctx->c.end_a, byte);
        s_chan_out_byte(ctx->c.end_a, ctx->cb_flags);
        ctx->cb_flags = 0;
    }
}

static void uart_rx_app_thread(rtos_uart_rx_t *ctx)
{
    
    if (ctx->rx_start_cb != NULL) {
        ctx->rx_start_cb(ctx);
    }

    /* Setup the receiving notification task handle (this!) */
    ctx->isr_notification_task = xTaskGetCurrentTaskHandle();
    printstr("Grr\n");
    printhexln(ctx->isr_notification_task);
    ctx->cb_flags = 0;

    /* send token (synch with HIL logical core) */
    s_chan_out_byte(ctx->c.end_b, 0);

    for (;;) {
        /* Block until notification from ISR */
        uint32_t val = ulTaskNotifyTakeIndexed(1, pdTRUE, portMAX_DELAY);
        uint8_t bytes[RTOS_UART_RX_BUF_LEN];
        unsigned bytes_read = 0;
        uart_buffer_error_t ret = UART_BUFFER_EMPTY;
        do{
            ret = pop_byte_from_buffer(&ctx->isr_to_app_fifo, &bytes[bytes_read]);
            bytes_read += 1;
        } while(ret == UART_BUFFER_OK);
        bytes_read -= 1; /* important as we incremented this for the read fail too */



        // uint8_t bytes[RTOS_UART_RX_BUF_LEN];
        // size_t xBytesRead = xStreamBufferReceive(   ctx->isr_byte_buffer,
        //                                             bytes,
        //                                             RTOS_UART_RX_BUF_LEN,
        //                                             portMAX_DELAY);

        // /* This will not block ever because we set these immediately after stream buff push*/
        // rtos_osal_event_group_get_bits(
        //         &ctx->events,
        //         RX_ALL_FLAGS,
        //         RTOS_OSAL_OR_CLEAR,
        //         &error_flags,
        //         RTOS_OSAL_WAIT_FOREVER);
    
        size_t xBytesSent = xStreamBufferSend(ctx->app_byte_buffer, bytes, bytes_read, 0);

        if(xBytesSent != bytes_read){
            ctx->cb_flags |= UR_OVERRUN_ERR_CB_FLAG;
        }


        if(ctx->cb_flags){
            //TODO Handle ME
        }

        // if ((error_flags & RX_ERROR_FLAGS) && ctx->rx_error_cb) {
        //     (*ctx->rx_error_cb)(ctx, error_flags & RX_ERROR_FLAGS);
        // }

        if (ctx->rx_complete_cb) {
            (*ctx->rx_complete_cb)(ctx);
        }
    }
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

    // rtos_osal_thread_create(
    //                 NULL,
    //                 "test_b",
    //                 (rtos_osal_entry_function_t) test_b,
    //                 NULL,
    //                 RTOS_THREAD_STACK_SIZE(test_b),
    //                 configMAX_PRIORITIES / 2
    //                 );

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

    // rtos_osal_event_group_create(&uart_rx_ctx->events, "uart_rx_events");

    /* Ensure that the UART interrupt is enabled on the requested core */
    uint32_t core_exclude_map = 0;
    rtos_osal_thread_core_exclusion_get(NULL, &core_exclude_map);
    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << interrupt_core_id));

    triggerable_setup_interrupt_callback(uart_rx_ctx->c.end_b, uart_rx_ctx, RTOS_INTERRUPT_CALLBACK(rtos_uart_rx_isr));
    triggerable_enable_trigger(uart_rx_ctx->c.end_b);

    /* Restore the core exclusion map for the calling thread */
    rtos_osal_thread_core_exclusion_set(NULL, core_exclude_map);

    /* Setup buffer between ISR and receiving thread and set to trigger on single byte */
    // uart_rx_ctx->isr_byte_buffer = xStreamBufferCreate(RTOS_UART_RX_BUF_LEN, 1);
    init_buffer(&uart_rx_ctx->isr_to_app_fifo, uart_rx_ctx->isr_to_app_fifo_storage, RTOS_UART_RX_BUF_LEN);
    /* Setup buffer between uart_app_thread and app  */
    uart_rx_ctx->app_byte_buffer = xStreamBufferCreate(app_rx_buff_size, 1);

    /* This will be setup in the driver_app_thread */
    uart_rx_ctx->isr_notification_task = NULL;

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
