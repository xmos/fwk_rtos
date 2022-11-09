// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * Provides flash layout image helpers for DFU
 */

#ifndef RTOS_DFU_IMAGE_H_
#define RTOS_DFU_IMAGE_H_

/**
 * \addtogroup rtos_dfu_image_driver rtos_dfu_image_driver
 *
 * The public API for using the RTOS DFU image driver.
 * @{
 */
#include <stdlib.h>
#include <stdint.h>
#include <xclib.h>
#include <xcore/port.h>
#include <xcore/clock.h>
#include <quadflashlib.h>

#include "rtos_osal.h"

/**
 * Struct representing an RTOS DFU image driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
typedef struct {
    fl_BootImageInfo factory_image_ctx;
    fl_BootImageInfo upgrade_image_ctx;
    unsigned data_partition_base_addr;
} rtos_dfu_image_t;

/**
 * \addtogroup rtos_dfu_image_driver_core rtos_dfu_image_driver_core
 *
 * The core functions for using an RTOS DFU image driver instance after
 * it has been initialized.
 * @{
 */

/**
 * Get the starting address of the data partition
 *
 * \param ctx A pointer to the DFU image driver instance to use.
 * 
 * \returns   The byte address
 */
inline unsigned rtos_dfu_image_get_data_partition_addr(
        rtos_dfu_image_t *dfu_image_ctx)
{
    return dfu_image_ctx->data_partition_base_addr;
}

/**
 * Get the starting address of the factory image
 *
 * \param ctx A pointer to the DFU image driver instance to use.
 * 
 * \returns   The byte address
 */
inline unsigned rtos_dfu_image_get_factory_addr(
        rtos_dfu_image_t *dfu_image_ctx)
{
    return dfu_image_ctx->factory_image_ctx.startAddress;
}

/**
 * Get the size of the factory image
 *
 * \param ctx A pointer to the DFU image driver instance to use.
 * 
 * \returns   The size in bytes
 */
inline unsigned rtos_dfu_image_get_factory_size(
        rtos_dfu_image_t *dfu_image_ctx)
{
    return dfu_image_ctx->factory_image_ctx.size;
}

/**
 * Get the version of the factory image
 *
 * \param ctx A pointer to the DFU image driver instance to use.
 * 
 * \returns   The version
 */
inline unsigned rtos_dfu_image_get_factory_version(
        rtos_dfu_image_t *dfu_image_ctx)
{
    return dfu_image_ctx->factory_image_ctx.version;
}

/**
 * Get the starting address of the upgrade image
 *
 * \param ctx A pointer to the DFU image driver instance to use.
 * 
 * \returns   The byte address
 */
inline unsigned rtos_dfu_image_get_upgrade_addr(
        rtos_dfu_image_t *dfu_image_ctx)
{
    return dfu_image_ctx->upgrade_image_ctx.startAddress;
}

/**
 * Get the size of the upgrade image
 *
 * \param ctx A pointer to the DFU image driver instance to use.
 * 
 * \returns   The size in bytes
 */
inline unsigned rtos_dfu_image_get_upgrade_size(
        rtos_dfu_image_t *dfu_image_ctx)
{
    return dfu_image_ctx->upgrade_image_ctx.size;
}

/**
 * Get the version of the upgrade image
 *
 * \param ctx A pointer to the DFU image driver instance to use.
 * 
 * \returns   The version
 */
inline unsigned rtos_dfu_image_get_upgrade_version(
        rtos_dfu_image_t *dfu_image_ctx)
{
    return dfu_image_ctx->upgrade_image_ctx.version;
}

/**@}*/

/**
 * Initializes an RTOS DFU image driver instance. This must be called
 * before initializing the RTOS QSPI driver instance.
 *
 * This will search the flash for program images via libquadflash
 * and store then for application DFU use.
 *
 * \param dfu_image_ctx A pointer to the DFU image driver instance to initialize.
 */
void rtos_dfu_image_init(
        rtos_dfu_image_t *dfu_image_ctx,
        xclock_t clock_block,
        port_t cs_port,
        port_t sclk_port,
        port_t sio_port);

/**@}*/

#endif /* RTOS_DFU_IMAGE_H_ */
