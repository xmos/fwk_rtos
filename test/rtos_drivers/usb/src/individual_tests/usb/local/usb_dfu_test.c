/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <xcore/assert.h>

#include "FreeRTOS.h"
#include "tusb.h"
#include "individual_tests/usb/usb_test.h"
#include "individual_tests/usb/local/usb_dfu_test.h"
#include "usb_descriptors.h"

static const char *test_name = "dfu_test";

#define LOCAL_PRINTF(FMT, ...) usb_printf("%s|" FMT, test_name, ##__VA_ARGS__)

/*
 * This test application allows for host-side validation of the USB driver
 * by the use of DFU operations using "dfu-util". This application should
 * continue to run until it receives the DFU detach command. In case of USB
 * driver failure, the host test logic should fallback after a timeout period
 * to a failure case that terminates the application.
 *
 * Test firmware files must not exceed a length of MAX_PARTITION_SIZE.
 *
 */

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTOTYPES
//--------------------------------------------------------------------+
#define NUM_PARITIONS ALT_COUNT
#define MAX_PARTITION_SIZE 1024

char test_images[NUM_PARITIONS][MAX_PARTITION_SIZE] = {
    "Partition 0 Test Data: The quick brown fox jumps over the lazy dog.",
    "Partition 1 Test Data: The quick brown fox jumps over the lazy dog.",
    "Partition 2 Test Data: The quick brown fox jumps over the lazy dog.",
};

static bool dfu_detach = false;

//--------------------------------------------------------------------+
// DFU callbacks
//--------------------------------------------------------------------+

uint32_t tud_dfu_get_timeout_cb(uint8_t alt, uint8_t state)
{
    LOCAL_PRINTF("tud_dfu_get_timeout_cb: alt=%u, state=%u", alt, state);

    if (state == DFU_DNBUSY) {
        return (alt == 0) ? 1 : 100;
    } else if (state == DFU_MANIFEST) {
        return 0;
    }

    return 0;
}

void tud_dfu_download_cb(uint8_t alt, uint16_t block_num, uint8_t const *data,
                         uint16_t length)
{
    LOCAL_PRINTF("tud_dfu_download_cb: alt=%u, block_num=%u, length=%u", alt,
                 block_num, length);

    xassert(alt < NUM_PARITIONS);
    xassert(length < MAX_PARTITION_SIZE);

    uint32_t start_offset = block_num * CFG_TUD_DFU_XFER_BUFSIZE;
    for (uint16_t i = 0; i < length; i++) {
        test_images[alt][start_offset + i] = data[i];
    }

    tud_dfu_finish_flashing(DFU_STATUS_OK);
}

void tud_dfu_manifest_cb(uint8_t alt)
{
    LOCAL_PRINTF("tud_dfu_manifest_cb: alt=%u", alt);
    tud_dfu_finish_flashing(DFU_STATUS_OK);
}

uint16_t tud_dfu_upload_cb(uint8_t alt, uint16_t block_num, uint8_t *data,
                           uint16_t length)
{
    LOCAL_PRINTF("tud_dfu_upload_cb: alt=%u, block_num=%u, length=%u - request",
                 alt, block_num, length);

    uint32_t start_offset = block_num * CFG_TUD_DFU_XFER_BUFSIZE;
    uint32_t end_of_partition_offset = MAX_PARTITION_SIZE - start_offset;
    xassert(start_offset < MAX_PARTITION_SIZE);

    uint16_t xfer_len = (uint16_t)strnlen(&test_images[alt][start_offset],
                                          end_of_partition_offset);
    if (xfer_len > length) {
        xfer_len = length;
    }

    memcpy(data, &test_images[alt][start_offset], xfer_len);

    LOCAL_PRINTF("tud_dfu_upload_cb: alt=%u, block_num=%u, length=%u - response",
                 alt, block_num, xfer_len);
    return xfer_len;
}

// Invoked when the Host has terminated a download or upload transfer
void tud_dfu_abort_cb(uint8_t alt)
{
    LOCAL_PRINTF("tud_dfu_abort_cb: alt=%u", alt);
}

// Invoked when a DFU_DETACH request is received
void tud_dfu_detach_cb(void)
{
    LOCAL_PRINTF("tud_dfu_detach_cb");
    dfu_detach = true;
}

//--------------------------------------------------------------------+
// Main test functions
//--------------------------------------------------------------------+

USB_MAIN_TEST_ATTR
static int dfu_test(usb_test_ctx_t *ctx)
{
    LOCAL_PRINTF("Start");

#if ON_TILE(0)
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        // The test concludes when the DFU detach command is received.
        // The host is permitted to issue other DFU commands as needed to verify
        // TinyUSB functionality.
        if (dfu_detach) {
            break;
        }
    }
#endif

    LOCAL_PRINTF("Done");
    return 0;
}

void register_dfu_test(usb_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    LOCAL_PRINTF("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char *)test_name;
    test_ctx->main_test[this_test_num] = dfu_test;

    test_ctx->test_cnt++;
}