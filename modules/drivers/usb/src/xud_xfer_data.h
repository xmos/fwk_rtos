// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XUD_XFER_DATA_H_
#define XUD_XFER_DATA_H_

#include <xcore/chanend.h>
#include <xcore/assert.h>
#include <xud.h>

#define XUD_DEV_XS3 1

static inline XUD_Result_t xud_setup_data_get_start(XUD_ep ep, uint8_t *buffer)
{
    XUD_ep_info *ep_struct = (XUD_ep_info *) ep;

    /* Firstly check if we have missed a USB reset - endpoint should not receive after a reset */
    if (ep_struct->resetting) {
        return XUD_RES_RST;
    }

    ep_struct->buffer = (unsigned int)buffer;
    *(unsigned int *)ep_struct->array_ptr_setup = ep;

    return XUD_RES_OKAY;
}

static inline XUD_Result_t xud_data_get_check(chanend_t c, unsigned *length, int *is_setup)
{
    int32_t word_len;
    int8_t tail_bitlen;
    int32_t byte_len;

    /* Test if there is a RESET */
    if (chanend_test_control_token_next_byte(c)) {
        *length = 0;
        *is_setup = 0;
        return XUD_RES_RST;
    }

    /* First word on channel is packet word length */
    word_len = chanend_in_word(c);

    /* Test if this is a SETUP packet */
    if (chanend_test_control_token_next_byte(c)) {
        *is_setup = 1;
        tail_bitlen = chanend_in_control_token(c);
    } else {
        *is_setup = 0;
        tail_bitlen = chanend_in_byte(c);

        /*
         * Data packets have a 16 bit CRC. Subtract this from
         * the length.
         */
        tail_bitlen -= 16;
    }

    byte_len = (4 * word_len) + (tail_bitlen / 8);

    if (byte_len >= 0) {
        *length = byte_len;
        return XUD_RES_OKAY;
    } else {
        *length = 0;
        return XUD_RES_ERR;
    }
}

XUD_Result_t xud_data_get_finish(XUD_ep ep);
XUD_Result_t xud_setup_data_get_finish(XUD_ep ep);

#endif /* XUD_XFER_DATA_H_ */
