// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <string.h>

#include "rtos_printf.h"
#include "rtos_dfu_image.h"

void rtos_dfu_image_print_debug(
        rtos_dfu_image_t *dfu_image_ctx)
{
    rtos_printf("DFU Image Info\nFactory:\n\tAddr:0x%x\n\tSize:%d\n\tVersion:%d\nUpgrade:\n\tAddr:0x%x\n\tSize:%d\n\tVersion:%d\nData Partition\n\tAddr:0x%x\n",
            rtos_dfu_image_get_factory_addr(dfu_image_ctx),
            rtos_dfu_image_get_factory_size(dfu_image_ctx),
            rtos_dfu_image_get_factory_version(dfu_image_ctx),
            rtos_dfu_image_get_upgrade_addr(dfu_image_ctx),
            rtos_dfu_image_get_upgrade_size(dfu_image_ctx),
            rtos_dfu_image_get_upgrade_version(dfu_image_ctx),
            rtos_dfu_image_get_data_partition_addr(dfu_image_ctx));
}

void rtos_dfu_image_init(
        rtos_dfu_image_t *dfu_image_ctx,
        fl_QSPIPorts *qspi_ports,
        fl_QuadDeviceSpec *qspi_specs,
        unsigned int len)
{
    memset(dfu_image_ctx, 0x00, sizeof(rtos_dfu_image_t));

    xassert(fl_connectToDevice(qspi_ports, qspi_specs, len) == 0);

    fl_BootImageInfo tmp_img;
    dfu_image_ctx->data_partition_base_addr = fl_getDataPartitionBase();

    /* Setup the factory image and upgrade image contexts.
     * If no valid images are found, memset to 0 for app
     * to check before using */
    if (fl_getFactoryImage(&dfu_image_ctx->factory_image_ctx) == 0) {
        memcpy(&tmp_img, &dfu_image_ctx->factory_image_ctx, sizeof(fl_BootImageInfo));
        if (fl_getNextBootImage(&tmp_img) == 0) {
            memcpy(&dfu_image_ctx->upgrade_image_ctx, &tmp_img, sizeof(fl_BootImageInfo));
        } else {
            /* No upgrade image found so get next available spot 
             * A valid spot is a sector boundary below the data partition */
            unsigned last_addr = dfu_image_ctx->factory_image_ctx.startAddress 
                + dfu_image_ctx->factory_image_ctx.size;

            /* This is getSectorAtOrAfter logic to handle irregular sector sizes
             * from tools lib function not in public API */
            unsigned numSectors = fl_getNumSectors();
            unsigned sector;
            for (sector = 0; sector < numSectors; sector++) {
                if (fl_getSectorAddress(sector) > last_addr) {
                    last_addr = fl_getSectorAddress(sector);
                    break;
                }
            }

            if (last_addr < dfu_image_ctx->data_partition_base_addr) {
                dfu_image_ctx->upgrade_image_ctx.startAddress = last_addr;
            } else {
                dfu_image_ctx->upgrade_image_ctx.startAddress = 0;
            }
        }
    }
    fl_disconnect();
}
