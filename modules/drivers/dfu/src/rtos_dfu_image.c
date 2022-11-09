// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <string.h>

#include "rtos_dfu_image.h"

void rtos_dfu_image_init(
        rtos_dfu_image_t *dfu_image_ctx,
        xclock_t clock_block,
        port_t cs_port,
        port_t sclk_port,
        port_t sio_port)
{
    memset(dfu_image_ctx, 0x00, sizeof(rtos_dfu_image_t));

    fl_QSPIPorts qspi_ports;
    fl_QuadDeviceSpec qspi_spec = FL_QUADDEVICE_DEFAULT;

    qspi_ports.qspiCS = cs_port;
    qspi_ports.qspiSCLK = sclk_port;
    qspi_ports.qspiSIO = sio_port;
    qspi_ports.qspiClkblk = clock_block;

    xassert(fl_connectToDevice(&qspi_ports, &qspi_spec, 1) == 0);

    fl_BootImageInfo *tmp_img = &dfu_image_ctx->factory_image_ctx;
    dfu_image_ctx->data_partition_base_addr = fl_getDataPartitionBase();

    /* Setup the factory image and upgrade image contexts.
     * If no valid images are found, memset to 0 for app
     * to check before using */
    if (fl_getFactoryImage(&dfu_image_ctx->factory_image_ctx) == 0) {
        if (fl_getNextBootImage(tmp_img) == 0) {
            memcpy(&dfu_image_ctx->upgrade_image_ctx, tmp_img, sizeof(fl_BootImageInfo));
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
