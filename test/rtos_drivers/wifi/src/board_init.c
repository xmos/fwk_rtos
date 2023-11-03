// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <stdarg.h>
#include <xcore/hwtimer.h>

#include "app_conf.h"
#include "board_init.h"

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile_ctx,
        rtos_spi_master_t *spi_master_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_gpio_t *gpio_ctx,
        rtos_spi_master_device_t *wifi_device_ctx
    )
{
    rtos_intertile_init(intertile_ctx, tile1);

    rtos_gpio_init(
            gpio_ctx);

    rtos_spi_master_init(
            spi_master_ctx,
            XS1_CLKBLK_1,
            WIFI_CS_N,
            WIFI_CLK,
            WIFI_MOSI,
            WIFI_MISO);

    rtos_spi_master_device_init(
            wifi_device_ctx,
            spi_master_ctx,
            1, /* WiFi CS pin is on bit 1 of the CS port */
            SPI_MODE_0,
            spi_master_source_clock_ref,
            0, /* 50 MHz */
            spi_master_sample_delay_2, /* what should this be? 2? 3? 4? */
            0, /* should this be > 0 if the above is 3-4 ? */
            1,
            0,
            0);

    rtos_qspi_flash_init(
            qspi_flash_ctx,
            XS1_CLKBLK_4,
            PORT_SQI_CS,
            PORT_SQI_SCLK,
            PORT_SQI_SIO,
            NULL);
}

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile1_ctx)
{
    rtos_intertile_init(intertile1_ctx, tile0);
}
