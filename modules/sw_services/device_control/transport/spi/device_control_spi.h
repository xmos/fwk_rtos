#ifndef SPI_DEVICE_CONTROL_H_
#define SPI_DEVICE_CONTROL_H_

#include "rtos_spi_slave.h"
#include "device_control.h"

void device_control_spi_start_cb(rtos_spi_slave_t *ctx,
                                 device_control_t *device_control_ctx);

void device_control_spi_xfer_done_cb(rtos_spi_slave_t *ctx,
                                     device_control_t *device_control_ctx);

#endif                    
