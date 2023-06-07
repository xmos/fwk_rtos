// Copyright 2020-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT RTOS_QSPI_FLASH

#include <string.h>
#include <stdbool.h>

#include <xcore/assert.h>
#include <xcore/interrupt.h>
#include <xcore/lock.h>

#include "rtos_qspi_flash.h"

#ifndef QSPI_FL_RETRY_DELAY_TICKS
#define QSPI_FL_RETRY_DELAY_TICKS 1000
#endif

#ifndef QSPI_FL_RETRY_ATTEMPT_CNT
#define QSPI_FL_RETRY_ATTEMPT_CNT 5
#endif

/* TODO, these will be removed once moved to the public API */
#define ERASE_CHIP 0xC7

extern void fl_int_read(
        unsigned char cmd, 
        unsigned int address, 
        unsigned char * destination, 
        unsigned int num_bytes);
extern void fl_int_write(
        unsigned char cmd,
        unsigned int pageAddress, 
        const unsigned char data[],
        unsigned int num_bytes);
extern void fl_int_sendSingleByteCommand(unsigned char cmd);
extern void fl_int_eraseSector(
        unsigned char cmd,
        unsigned int sectorAddress);
/* end TODO */

/* Library only supports 4096 sector size*/
#define QSPI_ERASE_TYPE_SIZE_LOG2   12

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define FLASH_OP_NONE  0
#define FLASH_OP_READ  1
#define FLASH_OP_WRITE 2
#define FLASH_OP_ERASE 3
#define FLASH_OP_READ_FAST_RAW 4
#define FLASH_OP_READ_FAST_NIBBLE_SWAP 5
#define FLASH_OP_LL_SETUP 6

extern unsigned __libc_hwlock;

typedef struct {
    int op;
    uint8_t *data;
    unsigned address;
    size_t len;
    unsigned priority;
} qspi_flash_op_req_t;

/*
 * Returns true if the spinlock is
 * acquired, false if not available.
 * NOT recursive.
 */
static bool spinlock_get(volatile int *lock)
{
    bool ret;

    lock_acquire(__libc_hwlock);
    {
        if (*lock == 0) {
            *lock = 1;
            ret = true;
        } else {
            ret = false;
        }
    }
    lock_release(__libc_hwlock);

    return ret;
}

/*
 * Releases the lock. It MUST be owned
 * by the caller.
 */
static void spinlock_release(volatile int *lock)
{
    *lock = 0;
}

static void rtos_qspi_flash_fl_connect_with_retry(
        rtos_qspi_flash_t *ctx)
{
    if (fl_connectToDevice(&ctx->qspi_ports, &ctx->qspi_spec, 1) == 0) {
        return;
    } else {
        int cnt = 0;
        while (fl_connectToDevice(&ctx->qspi_ports, &ctx->qspi_spec, 1) != 0) {
            delay_ticks(QSPI_FL_RETRY_DELAY_TICKS);
            if (cnt++ >= QSPI_FL_RETRY_ATTEMPT_CNT) {
                xassert(0); /* fl_connectToDevice failed too many times */
            }
        }
    }
}

int rtos_qspi_flash_read_ll(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    uint32_t irq_mask;
    bool lock_acquired;

    rtos_printf("Asked to ll read %d bytes at address 0x%08x\n", len, address);

    irq_mask = rtos_interrupt_mask_all();
    lock_acquired = spinlock_get(&ctx->spinlock);

    while (lock_acquired && len > 0) {

        size_t read_len = MIN(len, RTOS_QSPI_FLASH_READ_CHUNK_SIZE);

        /*
         * Cap the address at the size of the flash.
         * This ensures the correction below will work if
         * address is outside the flash's address space.
         */
        if (address >= ctx->flash_size) {
            address = ctx->flash_size;
        }

        if (address + read_len > ctx->flash_size) {
            int original_len = read_len;

            /* Don't read past the end of the flash */
            read_len = ctx->flash_size - address;

            /* Return all 0xFF bytes for addresses beyond the end of the flash */
            memset(&data[read_len], 0xFF, original_len - read_len);
        }

        rtos_printf("Read %d bytes from flash at address 0x%x\n", read_len, address);

        interrupt_mask_all();
        fl_int_read(ctx->qspi_spec.readCommand, address, data, read_len);
        interrupt_unmask_all();

        len -= read_len;
        data += read_len;
        address += read_len;
    }

    if (lock_acquired) {
        spinlock_release(&ctx->spinlock);
    }
    rtos_interrupt_mask_set(irq_mask);

    return lock_acquired ? 0 : -1;
}

int rtos_qspi_flash_fast_read_ll(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_fast_flash_read_ctx_t *qspi_flash_fast_read_ctx = &ctx->ctx;
    uint32_t irq_mask;
    bool lock_acquired;

    rtos_printf("Asked to ll fast read %d bytes at address 0x%08x\n", len, address);

    irq_mask = rtos_interrupt_mask_all();
    lock_acquired = spinlock_get(&ctx->spinlock);

    while (lock_acquired && len > 0) {

        size_t read_len = MIN(len, RTOS_QSPI_FLASH_READ_CHUNK_SIZE);

        /*
         * Cap the address at the size of the flash.
         * This ensures the correction below will work if
         * address is outside the flash's address space.
         */
        if (address >= ctx->flash_size) {
            address = ctx->flash_size;
        }

        if (address + read_len > ctx->flash_size) {
            int original_len = read_len;

            /* Don't read past the end of the flash */
            read_len = ctx->flash_size - address;

            /* Return all 0xFF bytes for addresses beyond the end of the flash */
            memset(&data[read_len], 0xFF, original_len - read_len);
        }

        rtos_printf("Read %d bytes from flash at address 0x%x\n", read_len, address);

        interrupt_mask_all();
        qspi_flash_fast_read(qspi_flash_fast_read_ctx, address, data, read_len);
        interrupt_unmask_all();

        len -= read_len;
        data += read_len;
        address += read_len;
    }

    if (lock_acquired) {
        spinlock_release(&ctx->spinlock);
    }
    rtos_interrupt_mask_set(irq_mask);

    return lock_acquired ? 0 : -1;
}

int rtos_qspi_flash_fast_read_mode_ll(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len,
        qspi_fast_flash_read_transfer_mode_t mode)
{
    qspi_fast_flash_read_ctx_t *qspi_flash_fast_read_ctx = &ctx->ctx;
    uint32_t irq_mask;
    bool lock_acquired;

    rtos_printf("Asked to ll fast read mode %d bytes at address 0x%08x\n", len, address);

    irq_mask = rtos_interrupt_mask_all();
    lock_acquired = spinlock_get(&ctx->spinlock);

    while (lock_acquired && len > 0) {

        size_t read_len = MIN(len, RTOS_QSPI_FLASH_READ_CHUNK_SIZE);

        /*
         * Cap the address at the size of the flash.
         * This ensures the correction below will work if
         * address is outside the flash's address space.
         */
        if (address >= ctx->flash_size) {
            address = ctx->flash_size;
        }

        if (address + read_len > ctx->flash_size) {
            int original_len = read_len;

            /* Don't read past the end of the flash */
            read_len = ctx->flash_size - address;

            /* Return all 0xFF bytes for addresses beyond the end of the flash */
            memset(&data[read_len], 0xFF, original_len - read_len);
        }

        rtos_printf("Read %d bytes from flash at address 0x%x in mode %d\n", read_len, address, mode);

        qspi_flash_fast_read_mode_set(&ctx->ctx, mode);

        interrupt_mask_all();
        qspi_flash_fast_read(qspi_flash_fast_read_ctx, address, data, read_len);
        interrupt_unmask_all();

        len -= read_len;
        data += read_len;
        address += read_len;
    }

    if (lock_acquired) {
        spinlock_release(&ctx->spinlock);
    }
    rtos_interrupt_mask_set(irq_mask);

    return lock_acquired ? 0 : -1;
}

void rtos_qspi_flash_fast_read_setup_ll(
        rtos_qspi_flash_t *ctx)
{
    ctx->ll_req_flag = 1;

    qspi_flash_op_req_t op = {
            .op = FLASH_OP_LL_SETUP,
            .data = 0,
            .address = 0,
            .len = 0
    };
    rtos_osal_queue_send(&ctx->op_queue, &op, RTOS_OSAL_WAIT_FOREVER);

    /* wait til task handshake */
    while(ctx->ll_req_flag != 0) {
        vTaskDelay(1);
    }

    qspi_flash_fast_read_setup_resources(&ctx->ctx);
    qspi_flash_fast_read_apply_calibration(&ctx->ctx);
}

void rtos_qspi_flash_fast_read_shutdown_ll(
        rtos_qspi_flash_t *ctx)
{
    ctx->last_op = FLASH_OP_NONE;
    qspi_flash_fast_read_shutdown(&ctx->ctx);
    vTaskResume(ctx->op_task.thread);
}

static void read_op(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    rtos_printf("Asked to read %d bytes at address 0x%08x\n", len, address);

    while (len > 0) {

        size_t read_len = MIN(len, RTOS_QSPI_FLASH_READ_CHUNK_SIZE);

        /*
         * Cap the address at the size of the flash.
         * This ensures the correction below will work if
         * address is outside the flash's address space.
         */
        if (address >= ctx->flash_size) {
            address = ctx->flash_size;
        }

        if (address + read_len > ctx->flash_size) {
            int original_len = read_len;

            /* Don't read past the end of the flash */
            read_len = ctx->flash_size - address;

            /* Return all 0xFF bytes for addresses beyond the end of the flash */
            memset(&data[read_len], 0xFF, original_len - read_len);
        }

        rtos_printf("Read %d bytes from flash at address 0x%x\n", read_len, address);

        interrupt_mask_all();
        fl_int_read(ctx->qspi_spec.readCommand, address, data, read_len);
        interrupt_unmask_all();

        len -= read_len;
        data += read_len;
        address += read_len;
    }
}

static void read_fast_op(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_fast_flash_read_ctx_t *qspi_flash_fast_read_ctx = &ctx->ctx;

    rtos_printf("Asked to fast read %d bytes at address 0x%08x\n", len, address);

    while (len > 0) {

        size_t read_len = MIN(len, RTOS_QSPI_FLASH_READ_CHUNK_SIZE);

        /*
         * Cap the address at the size of the flash.
         * This ensures the correction below will work if
         * address is outside the flash's address space.
         */
        if (address >= ctx->flash_size) {
            address = ctx->flash_size;
        }

        if (address + read_len > ctx->flash_size) {
            int original_len = read_len;

            /* Don't read past the end of the flash */
            read_len = ctx->flash_size - address;

            /* Return all 0xFF bytes for addresses beyond the end of the flash */
            memset(&data[read_len], 0xFF, original_len - read_len);
        }

        rtos_printf("Read %d bytes from flash at address 0x%x\n", read_len, address);

        interrupt_mask_all();
        qspi_flash_fast_read(qspi_flash_fast_read_ctx, address, data, read_len);
        interrupt_unmask_all();

        len -= read_len;
        data += read_len;
        address += read_len;
    }
}

static void while_busy(void)
{
    bool busy;

    do {
        interrupt_mask_all();
        busy = fl_getBusyStatus();
        interrupt_unmask_all();
    } while (busy);
}

static void write_op(
        rtos_qspi_flash_t *ctx,
        const uint8_t *data,
        unsigned address,
        size_t len)
{
    size_t bytes_left_to_write = len;
    unsigned address_to_write = address;
    const uint8_t *write_buf = data;

    rtos_printf("Asked to write %d bytes at address 0x%08x\n", bytes_left_to_write, address_to_write);

    while (bytes_left_to_write > 0) {
        /* compute the maximum number of bytes that can be written to the current page. */
        size_t max_bytes_to_write = fl_getPageSize() - (address_to_write & (fl_getPageSize() - 1));
        size_t bytes_to_write = bytes_left_to_write <= max_bytes_to_write ? bytes_left_to_write : max_bytes_to_write;

        if (address_to_write >= ctx->flash_size) {
            break; /* do not write past the end of the flash */
        }

        rtos_printf("Write %d bytes from flash at address 0x%x\n", bytes_to_write, address_to_write);
        interrupt_mask_all(); {
            fl_int_sendSingleByteCommand(ctx->qspi_spec.writeEnableCommand);
        } interrupt_unmask_all();
        interrupt_mask_all(); {
            fl_int_write(ctx->qspi_spec.programPageCommand, address_to_write, write_buf, bytes_to_write);
        } interrupt_unmask_all();
        while_busy();
        interrupt_mask_all(); {
            fl_int_sendSingleByteCommand(ctx->qspi_spec.writeDisableCommand);
        } interrupt_unmask_all();

        bytes_left_to_write -= bytes_to_write;
        write_buf += bytes_to_write;
        address_to_write += bytes_to_write;
    }
}

#define SECTORS_TO_BYTES(s, ss_log2) ((s) << (ss_log2))
#define BYTES_TO_SECTORS(b, ss_log2) (((b) + (1 << ss_log2) - 1) >> (ss_log2))

#define SECTOR_TO_BYTE_ADDRESS(s, ss_log2) SECTORS_TO_BYTES(s, ss_log2)
#define BYTE_TO_SECTOR_ADDRESS(b, ss_log2) ((b) >> (ss_log2))

static void erase_op(
        rtos_qspi_flash_t *ctx,
        unsigned address,
        size_t len)
{
    size_t bytes_left_to_erase = len;
    unsigned address_to_erase = address;

    rtos_printf("Asked to erase %d bytes at address 0x%08x\n", bytes_left_to_erase, address_to_erase);

    if (address_to_erase == 0 && bytes_left_to_erase >= ctx->flash_size) {
        /* Use chip erase when being asked to erase the entire address range */
        rtos_printf("Erasing entire chip\n");
        interrupt_mask_all(); {
            fl_int_sendSingleByteCommand(ctx->qspi_spec.writeEnableCommand);
        } interrupt_unmask_all();
        interrupt_mask_all(); {
            fl_int_sendSingleByteCommand(ERASE_CHIP);
        } interrupt_unmask_all();
        while_busy();
        interrupt_mask_all(); {
            fl_int_sendSingleByteCommand(ctx->qspi_spec.writeDisableCommand);
        } interrupt_unmask_all();
    } else {
        if (SECTOR_TO_BYTE_ADDRESS(BYTE_TO_SECTOR_ADDRESS(address_to_erase, QSPI_ERASE_TYPE_SIZE_LOG2), QSPI_ERASE_TYPE_SIZE_LOG2) != address_to_erase) {
            /*
             * If the provided starting erase address does not begin on the smallest
             * sector boundary, then update the starting address and number of bytes
             * to erase so that it does.
             */
            unsigned sector_address;
            sector_address = BYTE_TO_SECTOR_ADDRESS(address_to_erase, QSPI_ERASE_TYPE_SIZE_LOG2);
            bytes_left_to_erase += address_to_erase - SECTOR_TO_BYTE_ADDRESS(sector_address, QSPI_ERASE_TYPE_SIZE_LOG2);
            address_to_erase = SECTOR_TO_BYTE_ADDRESS(sector_address, QSPI_ERASE_TYPE_SIZE_LOG2);
            rtos_printf("adjusted starting erase address to %d\n", address_to_erase);
        }

        while (bytes_left_to_erase > 0) {
            int erase_length;
            int erase_length_log2 = QSPI_ERASE_TYPE_SIZE_LOG2;

            if (address_to_erase >= ctx->flash_size) {
                break; /* do not erase past the end of the flash */
            }

            erase_length = 1 << erase_length_log2;

            xassert(address_to_erase == SECTOR_TO_BYTE_ADDRESS(BYTE_TO_SECTOR_ADDRESS(address_to_erase, erase_length_log2), erase_length_log2));

            rtos_printf("Erasing %d bytes (%d) at byte address %d, sector %d\n", erase_length, bytes_left_to_erase, address_to_erase, BYTE_TO_SECTOR_ADDRESS(address_to_erase, erase_length_log2));

            interrupt_mask_all(); {
                fl_int_sendSingleByteCommand(ctx->qspi_spec.writeEnableCommand);
            } interrupt_unmask_all();
            interrupt_mask_all(); {
                fl_int_eraseSector(ctx->qspi_spec.sectorEraseCommand, address_to_erase);
            } interrupt_unmask_all();
            while_busy();
            interrupt_mask_all(); {
                fl_int_sendSingleByteCommand(ctx->qspi_spec.writeDisableCommand);
            } interrupt_unmask_all();

            address_to_erase += erase_length;
            bytes_left_to_erase -= erase_length < bytes_left_to_erase ? erase_length : bytes_left_to_erase;
        }
    }

    rtos_printf("Erasing complete\n");
}

static void qspi_flash_op_thread(rtos_qspi_flash_t *ctx)
{
    qspi_flash_op_req_t op;

    for (;;) {
        rtos_osal_queue_receive(&ctx->op_queue, &op, RTOS_OSAL_WAIT_FOREVER);

        if (op.op == FLASH_OP_LL_SETUP) {
            ctx->ll_req_flag = 0;
            vTaskSuspend(NULL);
        } else {
            /*
            * Inherit the priority of the task that requested this
            * operation.
            */
            rtos_osal_thread_priority_set(&ctx->op_task, op.priority);
            
            switch (ctx->last_op) {
            case FLASH_OP_NONE:
                if ((op.op == FLASH_OP_READ_FAST_RAW)
                || (op.op == FLASH_OP_READ_FAST_NIBBLE_SWAP)) {
                    qspi_flash_fast_read_setup_resources(&ctx->ctx);
                    qspi_flash_fast_read_apply_calibration(&ctx->ctx);
                } else {
                    rtos_qspi_flash_fl_connect_with_retry(ctx);
                }
                break;
            case FLASH_OP_READ:
            case FLASH_OP_WRITE:
            case FLASH_OP_ERASE:
                if ((op.op == FLASH_OP_READ_FAST_RAW)
                || (op.op == FLASH_OP_READ_FAST_NIBBLE_SWAP)) {
                    fl_disconnect();
                    qspi_flash_fast_read_setup_resources(&ctx->ctx);
                    qspi_flash_fast_read_apply_calibration(&ctx->ctx);
                }
                break;
            case FLASH_OP_READ_FAST_RAW:
            case FLASH_OP_READ_FAST_NIBBLE_SWAP:
                if ((op.op == FLASH_OP_READ)
                || (op.op == FLASH_OP_WRITE)
                || (op.op == FLASH_OP_ERASE)) {
                    qspi_flash_fast_read_shutdown(&ctx->ctx);
                    rtos_qspi_flash_fl_connect_with_retry(ctx);
                }
                break;
            }

            switch (op.op) {
            case FLASH_OP_READ:
                read_op(ctx, op.data, op.address, op.len);
                rtos_osal_semaphore_put(&ctx->data_ready);
                break;
            case FLASH_OP_WRITE:
                write_op(ctx, op.data, op.address, op.len);
                rtos_osal_free(op.data);
                break;
            case FLASH_OP_ERASE:
                erase_op(ctx, op.address, op.len);
                break;
            case FLASH_OP_READ_FAST_RAW:
                qspi_flash_fast_read_mode_set(&ctx->ctx, qspi_fast_flash_read_transfer_raw);
                read_fast_op(ctx, op.data, op.address, op.len);
                rtos_osal_semaphore_put(&ctx->data_ready);
                break;
            case FLASH_OP_READ_FAST_NIBBLE_SWAP:
                qspi_flash_fast_read_mode_set(&ctx->ctx, qspi_fast_flash_read_transfer_nibble_swap);
                read_fast_op(ctx, op.data, op.address, op.len);
                rtos_osal_semaphore_put(&ctx->data_ready);
                break;
            }

            ctx->last_op = op.op;

            /*
            * Reset back to the priority set by rtos_qspi_flash_start().
            */
            rtos_osal_thread_priority_set(&ctx->op_task, ctx->op_task_priority);
        }
    }
}

static void request(
        rtos_qspi_flash_t *ctx,
        qspi_flash_op_req_t *op)
{
    rtos_osal_mutex_get(&ctx->mutex, RTOS_OSAL_WAIT_FOREVER);

    rtos_osal_thread_priority_get(NULL, &op->priority);
    rtos_osal_queue_send(&ctx->op_queue, op, RTOS_OSAL_WAIT_FOREVER);

    rtos_osal_mutex_put(&ctx->mutex);
}

__attribute__((fptrgroup("rtos_qspi_flash_lock_fptr_grp")))
static void qspi_flash_local_lock(
        rtos_qspi_flash_t *ctx)
{
    bool mutex_owned = (xSemaphoreGetMutexHolder(ctx->mutex.mutex) == xTaskGetCurrentTaskHandle());

    while (rtos_osal_mutex_get(&ctx->mutex, RTOS_OSAL_WAIT_FOREVER) != RTOS_OSAL_SUCCESS);

    if (!mutex_owned) {
        while (!spinlock_get(&ctx->spinlock));
    } else {
        /*
         * The spinlock is already owned by this thread, so safe
         * to just increment it to keep track of the recursion.
         */
        ctx->spinlock++;
    }
}

__attribute__((fptrgroup("rtos_qspi_flash_unlock_fptr_grp")))
static void qspi_flash_local_unlock(
        rtos_qspi_flash_t *ctx)
{
    bool mutex_owned = (xSemaphoreGetMutexHolder(ctx->mutex.mutex) == xTaskGetCurrentTaskHandle());

    if (mutex_owned) {
        /*
         * Since the spinlock is already owned by this thread, it is safe
         * to just decrement it to unwind any recursion. Once it is
         * decremented down to 0 it will be released. Other threads will
         * not be able to acquire it until the mutex is also released, but
         * it will be possible a call to rtos_qspi_flash_read_ll() to
         * immediately acquire it.
         */
        ctx->spinlock--;
        rtos_osal_mutex_put(&ctx->mutex);
    }
}

__attribute__((fptrgroup("rtos_qspi_flash_read_fptr_grp")))
static void qspi_flash_local_read(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_op_req_t op = {
            .op = FLASH_OP_READ,
            .data = data,
            .address = address,
            .len = len
    };

    request(ctx, &op);

    rtos_osal_semaphore_get(&ctx->data_ready, RTOS_OSAL_WAIT_FOREVER);
}

__attribute__((fptrgroup("rtos_qspi_flash_read_mode_fptr_grp")))
static void qspi_flash_local_read_mode(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len,
        qspi_fast_flash_read_transfer_mode_t mode)
{
    qspi_flash_op_req_t op = {
            .op = FLASH_OP_READ,
            .data = data,
            .address = address,
            .len = len
    };

    request(ctx, &op);

    rtos_osal_semaphore_get(&ctx->data_ready, RTOS_OSAL_WAIT_FOREVER);
}

__attribute__((fptrgroup("rtos_qspi_flash_read_mode_fptr_grp")))
static void qspi_flash_local_read_mode_fast(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len,
        qspi_fast_flash_read_transfer_mode_t mode)
{
    qspi_flash_op_req_t op = {
            .op = FLASH_OP_READ_FAST_RAW,
            .data = (uint8_t*)data,
            .address = address,
            .len = len
    };
    if (mode == qspi_fast_flash_read_transfer_nibble_swap) {
        op.op = FLASH_OP_READ_FAST_NIBBLE_SWAP;
    }

    request(ctx, &op);

    rtos_osal_semaphore_get(&ctx->data_ready, RTOS_OSAL_WAIT_FOREVER);
}

__attribute__((fptrgroup("rtos_qspi_flash_read_fptr_grp")))
static void qspi_flash_local_read_fast_raw(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_op_req_t op = {
            .op = FLASH_OP_READ_FAST_RAW,
            .data = data,
            .address = address,
            .len = len
    };

    request(ctx, &op);

    rtos_osal_semaphore_get(&ctx->data_ready, RTOS_OSAL_WAIT_FOREVER);
}

__attribute__((fptrgroup("rtos_qspi_flash_read_fptr_grp")))
static void qspi_flash_local_read_fast_ns(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_op_req_t op = {
            .op = FLASH_OP_READ_FAST_NIBBLE_SWAP,
            .data = data,
            .address = address,
            .len = len
    };

    request(ctx, &op);

    rtos_osal_semaphore_get(&ctx->data_ready, RTOS_OSAL_WAIT_FOREVER);
}

__attribute__((fptrgroup("rtos_qspi_flash_write_fptr_grp")))
static void qspi_flash_local_write(
        rtos_qspi_flash_t *ctx,
        const uint8_t *data,
        unsigned address,
        size_t len)
{
    qspi_flash_op_req_t op = {
            .op = FLASH_OP_WRITE,
            .address = address,
            .len = len
    };

    op.data = rtos_osal_malloc(len);
    memcpy(op.data, data, len);

    request(ctx, &op);
}

__attribute__((fptrgroup("rtos_qspi_flash_erase_fptr_grp")))
static void qspi_flash_local_erase(
        rtos_qspi_flash_t *ctx,
        unsigned address,
        size_t len)
{
    qspi_flash_op_req_t op = {
            .op = FLASH_OP_ERASE,
            .address = address,
            .len = len
    };

    request(ctx, &op);
}

void rtos_qspi_flash_start(
        rtos_qspi_flash_t *ctx,
        unsigned priority)
{
    rtos_osal_mutex_create(&ctx->mutex, "qspi_lock", RTOS_OSAL_RECURSIVE);
    rtos_osal_queue_create(&ctx->op_queue, "qspi_req_queue", 2, sizeof(qspi_flash_op_req_t));
    rtos_osal_semaphore_create(&ctx->data_ready, "qspi_dr_sem", 1, 0);

    ctx->op_task_priority = priority;
    rtos_osal_thread_create(
            &ctx->op_task,
            "qspi_flash_op_thread",
            (rtos_osal_entry_function_t) qspi_flash_op_thread,
            ctx,
            RTOS_THREAD_STACK_SIZE(qspi_flash_op_thread),
            priority);

    if (ctx->rpc_config != NULL && ctx->rpc_config->rpc_host_start != NULL) {
        ctx->rpc_config->rpc_host_start(ctx->rpc_config);
    }
}

void rtos_qspi_flash_op_core_affinity_set(
        rtos_qspi_flash_t *ctx,
        uint32_t op_core_mask)
{
    rtos_osal_thread_core_exclusion_set(&ctx->op_task, ~op_core_mask);
}

void rtos_qspi_flash_init(
        rtos_qspi_flash_t *ctx,
        xclock_t clock_block,
        port_t cs_port,
        port_t sclk_port,
        port_t sio_port,
        fl_QuadDeviceSpec *spec)
{
    ctx->qspi_ports.qspiCS = cs_port;
    ctx->qspi_ports.qspiSCLK = sclk_port;
    ctx->qspi_ports.qspiSIO = sio_port;
    ctx->qspi_ports.qspiClkblk = clock_block;

    fl_QuadDeviceSpec default_spec = FL_QUADDEVICE_DEFAULT;

    if (spec == NULL) {
        memcpy(&ctx->qspi_spec, &default_spec, sizeof(fl_QuadDeviceSpec));
    } else {
        memcpy(&ctx->qspi_spec, spec, sizeof(fl_QuadDeviceSpec));
    }

    xassert(fl_connectToDevice(&ctx->qspi_ports, &ctx->qspi_spec, 1) == 0);
    /* Copy the spec back in case one was provided which has params populated by sfdp */
    xassert(fl_copySpec(&ctx->qspi_spec) == 0);
    
    ctx->flash_size = fl_getFlashSize();
    
    /* Driver currently only supports 4096 sector size */
    xassert(rtos_qspi_flash_sector_size_get(ctx) == (1 << QSPI_ERASE_TYPE_SIZE_LOG2));

    /* Enable quad flash */
    xassert(fl_quadEnable() == 0);

    ctx->calibration_valid = 0;
    ctx->last_op = FLASH_OP_NONE;
    ctx->rpc_config = NULL;
    ctx->read = qspi_flash_local_read;
    ctx->read_mode = qspi_flash_local_read_mode;
    ctx->write = qspi_flash_local_write;
    ctx->erase = qspi_flash_local_erase;
    ctx->lock = qspi_flash_local_lock;
    ctx->unlock = qspi_flash_local_unlock;
}

void rtos_qspi_flash_fast_read_init(
        rtos_qspi_flash_t *ctx,
        xclock_t clock_block,
        port_t cs_port,
        port_t sclk_port,
        port_t sio_port,
        fl_QuadDeviceSpec *spec,
        qspi_fast_flash_read_transfer_mode_t read_mode,
        uint8_t read_divide,
        uint32_t calibration_pattern_addr)
{
    ctx->qspi_ports.qspiCS = cs_port;
    ctx->qspi_ports.qspiSCLK = sclk_port;
    ctx->qspi_ports.qspiSIO = sio_port;
    ctx->qspi_ports.qspiClkblk = clock_block;

    fl_QuadDeviceSpec default_spec = FL_QUADDEVICE_DEFAULT;

    if (spec == NULL) {
        memcpy(&ctx->qspi_spec, &default_spec, sizeof(fl_QuadDeviceSpec));
    } else {
        memcpy(&ctx->qspi_spec, spec, sizeof(fl_QuadDeviceSpec));
    }

    xassert(fl_connectToDevice(&ctx->qspi_ports, &ctx->qspi_spec, 1) == 0);
    /* Copy the spec back in case one was provided which has params populated by sfdp */
    xassert(fl_copySpec(&ctx->qspi_spec) == 0);
    
    ctx->flash_size = fl_getFlashSize();
    
    /* Driver currently only supports 4096 sector size */
    xassert(rtos_qspi_flash_sector_size_get(ctx) == (1 << QSPI_ERASE_TYPE_SIZE_LOG2));

    /* Enable quad flash before we disconnect */
    xassert(fl_quadEnable() == 0);

    fl_disconnect();

    qspi_fast_flash_read_ctx_t *qspi_fast_flash_read_ctx = &ctx->ctx;

    qspi_flash_fast_read_init(
            qspi_fast_flash_read_ctx,
            clock_block,
            cs_port,
            sclk_port,
            sio_port,
            read_mode,
            read_divide);

    uint32_t scratch_buf[QFFR_DEFAULT_CAL_PATTERN_BUF_SIZE_WORDS];

    qspi_flash_fast_read_setup_resources(qspi_fast_flash_read_ctx);
    int32_t calibrate_res = qspi_flash_fast_read_calibrate(
            qspi_fast_flash_read_ctx,
            calibration_pattern_addr,
            qspi_flash_fast_read_pattern_expect_default,
            scratch_buf,
            QFFR_DEFAULT_CAL_PATTERN_BUF_SIZE_WORDS);

    qspi_flash_fast_read_shutdown(qspi_fast_flash_read_ctx);

    ctx->calibration_valid = (calibrate_res == 0);
    ctx->last_op = FLASH_OP_NONE;

    if (ctx->calibration_valid) {
        switch (read_mode) {
            default:
            case qspi_fast_flash_read_transfer_raw:
                ctx->read = qspi_flash_local_read_fast_raw;
                break;
            case qspi_fast_flash_read_transfer_nibble_swap:
                ctx->read = qspi_flash_local_read_fast_ns;
                break;
        }
        ctx->read_mode = qspi_flash_local_read_mode_fast;
    } else {
        ctx->read = qspi_flash_local_read;
        ctx->read_mode = qspi_flash_local_read_mode;
    }

    ctx->rpc_config = NULL;
    ctx->write = qspi_flash_local_write;
    ctx->erase = qspi_flash_local_erase;
    ctx->lock = qspi_flash_local_lock;
    ctx->unlock = qspi_flash_local_unlock;
}
