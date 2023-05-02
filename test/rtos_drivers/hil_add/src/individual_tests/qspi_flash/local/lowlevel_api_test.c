// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* Library headers */
#include "rtos_osal.h"
#include "rtos_qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/qspi_flash/qspi_flash_test.h"

static const char* test_name = "lowlevel_api_test";

#define local_printf( FMT, ... )    qspi_flash_printf("%s|" FMT, test_name, ##__VA_ARGS__)

#define QSPI_FLASH_TILE         0

QSPI_FLASH_MAIN_TEST_ATTR
static int main_test(qspi_flash_test_ctx_t *ctx)
{
    local_printf("Start");

    #if ON_TILE(QSPI_FLASH_TILE)
    {
        uint8_t test_buf[4096];
        uint8_t test_buf2[4096];
        rtos_qspi_flash_fast_read_setup_ll(ctx->qspi_flash_ctx);

        rtos_qspi_flash_fast_read_mode_ll(ctx->qspi_flash_ctx, test_buf, 0x1000, 4096, qspi_fast_flash_read_transfer_raw);
        rtos_qspi_flash_fast_read_mode_ll(ctx->qspi_flash_ctx, test_buf2, 0x1000, 4096, qspi_fast_flash_read_transfer_nibble_swap);

        for (int i=0; i<4096; i++) {
            if (test_buf[i] != ((((test_buf2[i] & 0x0F) << 4) | ((test_buf2[i] & 0xF0) >> 4)))) {
                local_printf("Failed. test_buf[%d]: 0x%x test_buf2[%d]: 0x%x\n", i, test_buf[i], i, test_buf2[i]);
                return -1;
            }
        }

        rtos_qspi_flash_fast_read_shutdown_ll(ctx->qspi_flash_ctx);
    }
    #endif

    local_printf("Done");
    return 0;
}

void register_lowlevel_api_test(qspi_flash_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

    test_ctx->test_cnt++;
}

#undef local_printf
