// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_UART_TX_H_
#define RTOS_UART_TX_H_

/**
 * This attribute must be specified on the RTOS UART tx call function
 * provided by the application to allow compiler stack calculation.
 */
#define RTOS_UART_TX_CALL_ATTR  __attribute__((fptrgroup("rtos_uart_tx_fptr_grp")))

/**
 * \addtogroup rtos_uart_tx_driver rtos_uart_tx_driver
 *
 * The public API for using the RTOS UART tx driver.
 * @{
 */

#include "uart.h"

#include "rtos_osal.h"
#include "rtos_driver_rpc.h"

/**
 * Typedef to the RTOS UART tx driver instance struct.
 */
typedef struct rtos_uart_tx_struct rtos_uart_tx_t;

/**
 * Struct representing an RTOS UART tx driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_uart_tx_struct {
    rtos_driver_rpc_t *rpc_config;
    RTOS_UART_TX_CALL_ATTR void (*write)(rtos_uart_tx_t *, const uint8_t buf[], size_t);
    uart_tx_t dev;
    rtos_osal_mutex_t lock;
};



/**
 * Writes data to an initialized and started UART instance.
 * Unlike the UART rx, an xcore logical core is not reserved. The UART transmission
 * is a function call and the the function blocks until the stop bit of the last
 * byte to be transmittted has completed. Interrupts are masked during this time
 * to avoid stretching of the waveform. Consequently, the tx consumes cycles from
 * the caller thread.
 *
 * \param ctx             A pointer to the UART Tx driver instance to use.
 * \param buf             The buffer containing data to write.
 * \param n               The number of bytes to write.
 */
inline void rtos_uart_tx_write(
        rtos_uart_tx_t *ctx,
        const uint8_t buf[],
        size_t n)
{
    ctx->write(ctx, buf, n);
}

/**
 * Initialises an RTOS UART tx driver instance.
 * This must only be called by the tile that owns the driver instance. It may be
 * called either before or after starting the RTOS, but must be called before calling
 * rtos_uart_tx_start() or any of the core UART tx driver functions with this instance.
 *
 * \param ctx           A pointer to the UART tx driver instance to initialise.
 * \param tx_port       The port containing the transmit pin
 * \param baud_rate     The baud rate of the UART in bits per second.
 * \param num_data_bits The number of data bits per frame sent.
 * \param parity        The type of parity used. See uart_parity_t above.
 * \param stop_bits     The number of stop bits asserted at the of the frame. 
 * \param tmr           The resource id of the timer to be used by the UART tx. 
 */
void rtos_uart_tx_init(
        rtos_uart_tx_t *ctx,
        const port_t tx_port,
        const uint32_t baud_rate,
        const uint8_t num_data_bits,
        const uart_parity_t parity,
        const uint8_t stop_bits,
        hwtimer_t tmr);

/**
 * Starts an RTOS UART tx driver instance. This must only be called by the tile that
 * owns the driver instance. It may be called either before or after starting
 * the RTOS, but must be called before any of the core UART tx driver functions are
 * called with this instance.
 *
 * rtos_uart_tx_init() must be called on this UART tx driver instance prior to calling this.
 *
 * \param ctx       A pointer to the UART tx driver instance to start.
 */
void rtos_uart_tx_start(
        rtos_uart_tx_t *ctx);


/**@}*/

#endif /* RTOS_UART_TX_H_ */
