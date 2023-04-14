// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "tusb_config.h"

// Number of Alternate Interface (each for 1 flash partition)
#define ALT_COUNT   3

enum {
    ITF_NUM_DFU_MODE,
    ITF_NUM_TOTAL
};

#endif /* USB_DESCRIPTORS_H_ */
