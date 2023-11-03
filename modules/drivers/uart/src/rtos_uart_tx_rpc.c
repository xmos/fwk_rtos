// Copyright 2020-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "rtos_rpc.h"
#include "rtos_uart_tx.h"

enum {
    fcode_write
};

RTOS_UART_TX_CALL_ATTR
void rtos_uart_tx_remote_write(
        rtos_uart_tx_t *ctx,
        const uint8_t buf[],
        size_t n)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_uart_tx_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_IN_BUFFER(buf, n),
            RPC_PARAM_TYPE(n),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_write, rpc_param_desc,
            &host_ctx_ptr, buf, &n);

}


static int uart_tx_write_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_uart_tx_t *ctx;
    uint8_t *buf;
    size_t n;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx, &buf, &n);

    rtos_uart_tx_write(ctx, buf, n);

    msg_length = rpc_response_marshall(resp_msg, rpc_msg, ctx, buf, n);

    return msg_length;
}

static void uart_tx_rpc_thread(rtos_intertile_address_t *client_address)
{
    int msg_length;
    uint8_t *req_msg;
    uint8_t *resp_msg;
    rpc_msg_t rpc_msg;
    rtos_intertile_t *intertile_ctx = client_address->intertile_ctx;
    uint8_t intertile_port = client_address->port;

    for (;;) {
        /* receive RPC request message from client */
        msg_length = rtos_intertile_rx(intertile_ctx, intertile_port, (void **) &req_msg, RTOS_OSAL_WAIT_FOREVER);

        rpc_request_parse(&rpc_msg, req_msg);

        switch (rpc_msg.fcode) {
        case fcode_write:
            msg_length = uart_tx_write_rpc_host(&rpc_msg, &resp_msg);
            break;
        }

        rtos_osal_free(req_msg);

        /* send RPC response message to client */
        rtos_intertile_tx(intertile_ctx, intertile_port, resp_msg, msg_length);
        rtos_osal_free(resp_msg);
    }
}

__attribute__((fptrgroup("rtos_driver_rpc_host_start_fptr_grp")))
static void uart_tx_rpc_start(
        rtos_driver_rpc_t *rpc_config)
{
    xassert(rpc_config->host_task_priority >= 0);

    for (int i = 0; i < rpc_config->remote_client_count; i++) {

        rtos_intertile_address_t *client_address = &rpc_config->client_address[i];

        xassert(client_address->port >= 0);

        rtos_osal_thread_create(
                NULL,
                "uart_tx_rpc_thread",
                (rtos_osal_entry_function_t) uart_tx_rpc_thread,
                client_address,
                RTOS_THREAD_STACK_SIZE(uart_tx_rpc_thread),
                rpc_config->host_task_priority);
    }
}

void rtos_uart_tx_rpc_config(
        rtos_uart_tx_t *ctx,
        unsigned intertile_port,
        unsigned host_task_priority)
{
    rtos_driver_rpc_t *rpc_config = ctx->rpc_config;

    if (rpc_config->remote_client_count == 0) {
        /* This is a client */
        rpc_config->host_address.port = intertile_port;
    } else {
        for (int i = 0; i < rpc_config->remote_client_count; i++) {
            rpc_config->client_address[i].port = intertile_port;
        }
        rpc_config->host_task_priority = host_task_priority;
    }
}

void rtos_uart_tx_rpc_client_init(
        rtos_uart_tx_t *ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx)
{
    ctx->rpc_config = rpc_config;
    ctx->write = rtos_uart_tx_remote_write;
    rpc_config->rpc_host_start = NULL;
    rpc_config->remote_client_count = 0;
    rpc_config->host_task_priority = -1;

    /* This must be configured later with rtos_uart_tx_rpc_config() */
    rpc_config->host_address.port = -1;

    rpc_config->host_address.intertile_ctx = host_intertile_ctx;
    rpc_config->host_ctx_ptr = (void *) s_chan_in_word(host_intertile_ctx->c);
}

void rtos_uart_tx_rpc_host_init(
        rtos_uart_tx_t *ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count)
{
    ctx->rpc_config = rpc_config;
    rpc_config->rpc_host_start = uart_tx_rpc_start;
    rpc_config->remote_client_count = remote_client_count;

    /* This must be configured later with rtos_uart_tx_rpc_config() */
    rpc_config->host_task_priority = -1;

    for (int i = 0; i < remote_client_count; i++) {
        rpc_config->client_address[i].intertile_ctx = client_intertile_ctx[i];
        s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) ctx);

        /* This must be configured later with rtos_uart_tx_rpc_config() */
        rpc_config->client_address[i].port = -1; 
    }
}
