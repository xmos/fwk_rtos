// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_UART_RX_H_
#define RTOS_UART_RX_H_

/**
 * \addtogroup rtos_uart_rx_driver rtos_uart_rx_driver
 *
 * The public API for using the RTOS UART rx driver.
 * @{
 */

#include <xcore/channel_streaming.h>
#include "uart.h"

#include "rtos_osal.h"
#include "stream_buffer.h"

/**
 * The callback code bit positions available for RTOS UART Rx.
 */
#define UR_COMPLETE_CB_CODE       0
#define UR_STARTED_CB_CODE        1
#define UR_START_BIT_ERR_CB_CODE  2
#define UR_PARITY_ERR_CB_CODE     3
#define UR_FRAMING_ERR_CB_CODE    4
#define UR_OVERRUN_ERR_CB_CODE    5

/**
 * The callback code flag masks available for RTOS UART Rx.
 */
#define UR_COMPLETE_CB_FLAG        (1 << UR_COMPLETE_CB_CODE)
#define UR_STARTED_CB_FLAG         (1 << UR_STARTED_CB_CODE)
#define UR_START_BIT_ERR_CB_FLAG   (1 << UR_START_BIT_ERR_CB_CODE)
#define UR_PARITY_ERR_CB_FLAG      (1 << UR_PARITY_ERR_CB_CODE)
#define UR_FRAMING_ERR_CB_FLAG     (1 << UR_FRAMING_ERR_CB_CODE)
#define UR_OVERRUN_ERR_CB_FLAG     (1 << UR_OVERRUN_ERR_CB_CODE)

#if (UR_START_BIT_ERR_CB_CODE != UART_START_BIT_ERROR_VAL)
#error Please align the HIL uart_callback_code_t with CB codes in rtos_uart_rx.c
#endif /* We use this alignment as an optimisation */

#define RX_ERROR_FLAGS (UR_START_BIT_ERR_CB_FLAG | UR_PARITY_ERR_CB_FLAG | UR_FRAMING_ERR_CB_FLAG)
#define RX_ALL_FLAGS (UR_COMPLETE_CB_FLAG | UR_STARTED_CB_FLAG | RX_ERROR_FLAGS)


/**
 * The size of the byte buffer between the ISR and the appthread. It needs to be able to
 * hold sufficient bytes received until the app_thread is able to service it.
 * This is not the same as app_byte_buffer_size which can be of any size, specified by 
 * the user at device start.
 * At 1Mbps we get a byte every 10us so 64B allows 640us for the app thread to respond.
 * Note buffer is size n+1 as required by lib_uart.
 */
#ifndef RTOS_UART_RX_BUF_LEN
#define RTOS_UART_RX_BUF_LEN (64 + 1)
#endif

/**
 * This attribute must be specified on all RTOS UART rx callback functions
 * provided by the application to allow compiler stack calculation.
 */
#define RTOS_UART_RX_CALLBACK_ATTR __attribute__((fptrgroup("rtos_uart_rx_callback_fptr_grp")))

/**
 * This attribute must be specified on all RTOS UART functions
 * provided by the application to allow compiler stack calculation.
 */
#define RTOS_UART_RX_CALL_ATTR __attribute__((fptrgroup("rtos_uart_rx_call_fptr_grp")))

/**
 * Typedef to the RTOS UART rx driver instance struct.
 */
typedef struct rtos_uart_rx_struct rtos_uart_rx_t;

/**
 * Function pointer type for application provided RTOS UART rx start callback functions.
 *
 * This callback function is optionally (may be NULL) called by an UART rx driver's thread when it is first
 * started. This gives the application a chance to perform startup initialization from within the
 * driver's thread.
 *
 * \param ctx           A pointer to the associated UART rx driver instance.
 */

typedef void (*rtos_uart_rx_started_cb_t)(rtos_uart_rx_t *ctx);

/**
 * Function pointer type for application provided RTOS UART rx receive callback function.
 *
 * This callback functions are called when an UART rx driver instance has received data to a specified
 * depth. Please use the xStreamBufferReceive(rtos_uart_rx_ctx->isr_byte_buffer, ... to read the bytes.
 *
 * \param ctx           A pointer to the associated UART rx driver instance.
 */
typedef void (*rtos_uart_rx_complete_cb_t)(rtos_uart_rx_t *ctx);


/**
 * Function pointer type for application provided RTOS UART rx error callback functions.
 *
 * This callback function is optionally (may be NULL_ called when an UART rx driver instance experiences an error 
 * in reception. These error types are defined in uart.h of the underlying HIL driver but can be of the following
 * types for the RTOS rx:  UART_START_BIT_ERROR, UART_PARITY_ERROR, UART_FRAMING_ERROR, UART_OVERRUN_ERROR.
 * 
 *
 * \param ctx           A pointer to the associated UART rx driver instance.
 * \param err_flags     An 8b word containing error flags set during reception of last frame. 
 *                      See rtos_uart_rx.h for the bit field definitions.
 */
typedef void (*rtos_uart_rx_error_t)(rtos_uart_rx_t *ctx, uint8_t err_flags);

/**
 * Struct representing an RTOS UART rx driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_uart_rx_struct {
    uart_rx_t dev;

    RTOS_UART_RX_CALL_ATTR void (*read)(rtos_uart_rx_t *, uint8_t buf[], size_t *num_bytes);

    void *app_data;

    RTOS_UART_RX_CALLBACK_ATTR rtos_uart_rx_started_cb_t rx_start_cb;
    RTOS_UART_RX_CALLBACK_ATTR rtos_uart_rx_complete_cb_t rx_complete_cb;
    RTOS_UART_RX_CALLBACK_ATTR rtos_uart_rx_error_t rx_error_cb;

    streaming_channel_t c;

    uart_buffer_t isr_to_app_fifo;
    uint8_t isr_to_app_fifo_storage[RTOS_UART_RX_BUF_LEN];
    uint8_t cb_flags;
    StreamBufferHandle_t app_byte_buffer;

    rtos_osal_thread_t hil_thread;
    rtos_osal_thread_t app_thread;
};


/**
 * Reads data from a UART Rx instance. It will read up to n bytes or timeout,
 * whichever comes first.
 *
 * \param uart_rx_ctx     A pointer to the UART Rx driver instance to use.
 * \param buf             The buffer to be written with the read UART bytes.
 * \param n               The number of bytes to write.
 * \param timeout         How long in ticks before the read operation should timeout.
 * 
 * \returns               The number of bytes read.
 */
size_t rtos_uart_rx_read(rtos_uart_rx_t *uart_rx_ctx, uint8_t *buf, size_t n, rtos_osal_tick_t timeout);


/**
 * Resets the receive buffer. Clears the contents and sets number of items rto zero.
 *
 * \param uart_rx_ctx      A pointer to the UART Rx driver instance to use.
  */
void rtos_uart_rx_reset_buffer(rtos_uart_rx_t *uart_rx_ctx);


/**
 * Initializes an RTOS UART rx driver instance.
 * This must only be called by the tile that owns the driver instance. It should be
 * called before starting the RTOS, and must be called before calling rtos_uart_rx_start().
 * Note that UART rx requires a whole logical core for the underlying HIL UART Rx instance.
 *  
 *
 * \param uart_rx_ctx A pointer to the UART rx driver instance to initialize.
 * \param io_core_mask  A bitmask representing the cores on which the low UART Rx thread
 *                      created by the driver is allowed to run. Bit 0 is core 0, bit 1 is core 1,
 *                      etc.
 * \param rx_port       The port containing the receive pin
 * \param baud_rate     The baud rate of the UART in bits per second.
 * \param data_bits     The number of data bits per frame sent.
 * \param parity        The type of parity used. See uart_parity_t above.
 * \param stop_bits     The number of stop bits asserted at the of the frame. 
 * \param tmr           The resource id of the timer to be used by the UART Rx. */
 
void rtos_uart_rx_init(
        rtos_uart_rx_t *uart_rx_ctx,
        uint32_t io_core_mask,
        port_t rx_port,
        uint32_t baud_rate,
        uint8_t data_bits,
        uart_parity_t parity,
        uint8_t stop_bits,
        hwtimer_t tmr
        );

/**
 * Starts an RTOS UART rx driver instance. This must only be called by the tile that
 * owns the driver instance. It must be called after starting the RTOS and from an RTOS thread.
 *
 * rtos_uart_rx_init() must be called on this UART rx driver instance prior to calling this.
 *
 * \param uart_rx_ctx       A pointer to the UART rx driver instance to start.
 * \param app_data          A pointer to application specific data to pass to
 *                          the callback functions available in rtos_uart_rx_struct.
 * \param start             The callback function that is called when the driver's
 *                          thread starts. This is optional and may be NULL.
 * \param rx_complete       The callback function to indicate data received by the UART.
 * \param error             The callback function called when a reception error has occured.
 * \param interrupt_core_id The ID of the core on which to enable the UART rx interrupt.
 * \param priority          The priority of the task that gets created by the driver to
 *                          call the callback functions.
 * \param app_rx_buff_size  The size in bytes of the RTOS xstreambuffer used to buffer 
 *                          received words for the application.
 */
void rtos_uart_rx_start(
        rtos_uart_rx_t *uart_rx_ctx,
        void *app_data,
        rtos_uart_rx_started_cb_t start,
        rtos_uart_rx_complete_cb_t rx_complete,
        rtos_uart_rx_error_t error,
        unsigned interrupt_core_id,
        unsigned priority,
        size_t app_rx_buff_size);


/**@}*/

#endif /* RTOS_UART_RX */
