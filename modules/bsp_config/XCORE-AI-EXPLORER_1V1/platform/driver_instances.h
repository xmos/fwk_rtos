// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DRIVER_INSTANCES_H_
#define DRIVER_INSTANCES_H_

#include "rtos_intertile.h"
#include "rtos_mic_array.h"
#include "rtos_i2c_master.h"
#include "rtos_i2s.h"
#include "rtos_spi_master.h"
#include "rtos_qspi_flash.h"
#include "rtos_gpio.h"
#include "rtos_uart_tx.h"
#include "rtos_uart_rx.h"

#define FLASH_TILE_NO      0
#define I2C_TILE_NO        0
#define MICARRAY_TILE_NO   1
#define I2S_TILE_NO        1
#define UART_TILE_NO       1

/** TILE 0 Clock Blocks */
#define FLASH_CLKBLK  XS1_CLKBLK_1
#define MCLK_CLKBLK   XS1_CLKBLK_2
#define SPI_CLKBLK    XS1_CLKBLK_3
#define XUD_CLKBLK_1  XS1_CLKBLK_4 /* Reserved for lib_xud */
#define XUD_CLKBLK_2  XS1_CLKBLK_5 /* Reserved for lib_xud */

/** TILE 1 Clock Blocks */
#define PDM_CLKBLK_1  XS1_CLKBLK_1
#define PDM_CLKBLK_2  XS1_CLKBLK_2
#define I2S_CLKBLK    XS1_CLKBLK_3
// #define UNUSED_CLKBLK XS1_CLKBLK_4
// #define UNUSED_CLKBLK XS1_CLKBLK_5

extern rtos_intertile_t *intertile_ctx;
extern rtos_qspi_flash_t *qspi_flash_ctx;
extern rtos_spi_master_t *spi_master_ctx;
extern rtos_spi_master_device_t *wifi_device_ctx;
extern rtos_gpio_t *gpio_ctx_t0;
extern rtos_gpio_t *gpio_ctx_t1;
extern rtos_mic_array_t *mic_array_ctx;
extern rtos_i2c_master_t *i2c_master_ctx;
extern rtos_i2s_t *i2s_ctx;
extern rtos_uart_tx_t *uart_tx_ctx;
extern rtos_uart_rx_t *uart_rx_ctx;


#endif /* DRIVER_INSTANCES_H_ */
