// Copyright 2017-2025 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#if USE_SPI

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include "device_control_host.h"
#include "control_host_support.h"

// SPI device file descriptor
static int spi_fd = -1;

// Number of nsec to delay between spi transactions
static long intertransaction_delay = 0;

// Bits per word
static const uint8_t SPI_BITS_PER_WORD = 8;

// Should be plenty enough for the max length of `/dev/spidev%d.%d` with int max or min
static const size_t SPIDEV_PATH_MAX = 55;

// Sleep for intertransaction_delay nanoseconds. Yield to kernel so expect minimum delay
// to be hundreds of microseconds at least.
static void apply_intertransaction_delay(void)
{
    if (intertransaction_delay > 0) {
        struct timespec req = {
            .tv_sec = 0,
            .tv_nsec = intertransaction_delay,
        };
        struct timespec rem;

        while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
            req = rem;
        }
    }
}

// SPI transfer function using spidev ioctl
static int spi_transfer(uint8_t *data, size_t len)
{
    // Make sure we have a handle on the spidev
    if (spi_fd < 0)
        return -1;

    struct spi_ioc_transfer xfer = {
        .tx_buf = (unsigned long)data,
        .rx_buf = (unsigned long)data,
        .len = len,
    };

    // Send transfer
    int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &xfer);

    return (ret == len) ? 0 : -1;
}

// Initialise the spidev with the given SPI mode, frequency, bus, cs, and intertransaction delay
control_ret_t control_init_spidev(uint8_t spi_mode, uint32_t speed_hz,
                                  int spidev_bus, int spidev_cs, long delay_ns)
{
    char device[SPIDEV_PATH_MAX];
    snprintf(device, sizeof(device), "/dev/spidev%d.%d", spidev_bus, spidev_cs);

    // Open SPI device
    spi_fd = open(device, O_RDWR);
    if (spi_fd < 0) {
        PRINT_ERROR("SPI initialisation failed, may need root permissions.");
        return CONTROL_ERROR;
    }

    // Set SPI mode
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode) < 0) {
        PRINT_ERROR("Failed to set SPI mode: %s\n", strerror(errno));
        close(spi_fd);
        spi_fd = -1;
        return CONTROL_ERROR;
    }

    // Set bits per word
    uint8_t bits = SPI_BITS_PER_WORD;
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        PRINT_ERROR("Failed to set SPI bits per word: %s\n", strerror(errno));
        close(spi_fd);
        spi_fd = -1;
        return CONTROL_ERROR;
    }

    // Set SPI frequency
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz) < 0) {
        PRINT_ERROR("Failed to set SPI frequency: %s\n", strerror(errno));
        close(spi_fd);
        spi_fd = -1;
        return CONTROL_ERROR;
    }

    intertransaction_delay = delay_ns;

    return CONTROL_SUCCESS;
}

control_ret_t control_write_command(control_resid_t resid, control_cmd_t cmd,
                                    const uint8_t payload[], size_t payload_len)
{
    // Avoid buffer overflow
    if (payload_len > SPI_DATA_MAX_BYTES) {
        return CONTROL_ERROR;
    }

    // Make sure we have a handle on the spidev
    if (spi_fd < 0) {
        return CONTROL_ERROR;
    }

    uint8_t data_sent_received[SPI_TRANSACTION_MAX_BYTES] = { 0 };

    do {
        int data_len;

// When LOW_LEVEL_TESTING is defined, resid and cmd fields are ignored and payload is sent directly to the device.
// This allows the user to write a stream of bytes directly into the device allowing for low level testing like testing the
// error handling mechanism.
#if LOW_LEVEL_TESTING
        if (resid == 0 && cmd == 0) {
            memcpy(data_sent_received, payload, payload_len);
            data_len = payload_len;
        } else {
            data_len = control_build_spi_data(data_sent_received, resid, cmd,
                                              payload, payload_len);
        }
#else
        data_len = control_build_spi_data(data_sent_received, resid, cmd,
                                          payload, payload_len);
#endif

        if (spi_transfer(data_sent_received, (size_t)data_len) < 0) {
            return CONTROL_ERROR;
        }

        apply_intertransaction_delay();
    } while (data_sent_received[0] == CONTROL_COMMAND_IGNORED_IN_DEVICE);

    do {
        // Get status
        memset(data_sent_received, 0, SPI_TRANSACTION_MAX_BYTES);
        size_t transaction_length = (payload_len < 8) ? 8 : payload_len;

        if (spi_transfer(data_sent_received, transaction_length) < 0) {
            return CONTROL_ERROR;
        }

        apply_intertransaction_delay();
    } while (data_sent_received[0] == CONTROL_COMMAND_IGNORED_IN_DEVICE);

    return data_sent_received[0];
}

control_ret_t control_read_command(control_resid_t resid, control_cmd_t cmd,
                                   uint8_t payload[], size_t payload_len)
{
    // Avoid buffer overflow
    if (payload_len > SPI_DATA_MAX_BYTES) {
        return CONTROL_ERROR;
    }

    // Make sure we have a handle on the spidev
    if (spi_fd < 0) {
        return CONTROL_ERROR;
    }

    uint8_t data_sent_received[SPI_TRANSACTION_MAX_BYTES] = { 0 };

    do {
        int data_len;

#if LOW_LEVEL_TESTING
        if (resid == 0 && cmd == 0) {
            memcpy(data_sent_received, payload, payload_len);
            data_len = payload_len;
        } else {
            data_len = control_build_spi_data(data_sent_received, resid, cmd,
                                              payload, payload_len);
        }
#else
        data_len = control_build_spi_data(data_sent_received, resid, cmd,
                                          payload, payload_len);
#endif

        if (spi_transfer(data_sent_received, (size_t)data_len) < 0) {
            return CONTROL_ERROR;
        }

        apply_intertransaction_delay();
    } while (data_sent_received[0] == CONTROL_COMMAND_IGNORED_IN_DEVICE);

    do {
        // Get status
        memset(data_sent_received, 0, SPI_TRANSACTION_MAX_BYTES);
        size_t transaction_length = (payload_len < 8) ? 8 : payload_len;

        if (spi_transfer(data_sent_received, transaction_length) < 0) {
            return CONTROL_ERROR;
        }

        apply_intertransaction_delay();
    } while (data_sent_received[0] == CONTROL_COMMAND_IGNORED_IN_DEVICE);

    memcpy(payload, data_sent_received, payload_len);
    // TODO - For write commands, control_write_command() is returning status from the device. For read commands payload[0] has the
    // status from the device and control_read_command() always returns CONTROL_SUCCESS. Make status returning consistent across
    // for read and write command functions.

    return CONTROL_SUCCESS;
}

// Close the spidev FD if still open.
control_ret_t control_cleanup_spi(void)
{
    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
    }
    return CONTROL_SUCCESS;
}

#endif /* USE_SPI */
