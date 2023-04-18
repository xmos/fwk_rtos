// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include "usb_support.h"
#include "tusb.h"

#ifndef USB_TASK_STACK_SIZE
#define USB_TASK_STACK_SIZE 1000
#endif

static void usb_task(void* args)
{
    static bool connected;
    xassert(tusb_inited() && "Tiny USB must first be initialized with usb_manager_init()");

    taskENTER_CRITICAL();
    if (!connected) {
        connected = true;
        taskEXIT_CRITICAL();
        tud_connect();
    } else {
        taskEXIT_CRITICAL();
    }

    while (1) {
        tud_task();
    }
}

void usb_manager_start(unsigned priority)
{
    xTaskCreate((TaskFunction_t) usb_task,
                "usb_task",
				USB_TASK_STACK_SIZE,
                NULL,
				priority,
                NULL);
}

#if RUN_EP0_VIA_PROXY
#include "rtos_usb.h"
extern rtos_usb_t usb_ctx;
void usb_manager_init(chanend_t c_ep0_proxy, chanend_t c_ep_hid_proxy, chanend_t c_ep0_proxy_xfer_complete)
{
    tusb_init();
    usb_ctx.c_ep_proxy[0] = c_ep0_proxy;
    usb_ctx.c_ep_proxy[2] = c_ep_hid_proxy; // TODO Hardcoded, assuming endpoint 2 is HID
    usb_ctx.c_ep0_proxy_xfer_complete = c_ep0_proxy_xfer_complete;
}
#else /* RUN_EP0_VIA_PROXY */
void usb_manager_init(void)
{
    tusb_init();
}
#endif /* RUN_EP0_VIA_PROXY */
