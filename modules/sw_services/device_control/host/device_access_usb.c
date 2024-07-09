// Copyright 2016-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#if USE_USB
#include <stdio.h>
#if !defined(_MSC_VER) || (_MSC_VER >= 1800) // !MSVC or MSVC >=VS2013
#include <stdbool.h>
#else
typedef enum { false = 0, true = 1} bool;
#endif // MSC
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <libusb.h>
#include "device_control_host.h"
#include "control_host_support.h"
#include "util.h"

//#define DBG(x) x
#define DBG(x)
#define PRINT_ERROR(...)   fprintf(stderr, "Error  : " __VA_ARGS__)

static unsigned num_commands = 0;

static libusb_device_handle *devh = NULL;

static const int sync_timeout_ms = 500;

/* Control query transfers require smaller buffers */
#define VERSION_MAX_PAYLOAD_SIZE 64

void debug_libusb_error(int err_code)
{
#if defined _WIN32
  PRINT_ERROR("libusb_control_transfer returned %s\n", libusb_error_name(errno));
#elif defined __APPLE__
  PRINT_ERROR("libusb_control_transfer returned %s\n", libusb_error_name(err_code));
#elif defined __linux
  PRINT_ERROR("libusb_control_transfer returned %d\n", err_code);
#endif

}

control_ret_t control_query_version(control_version_t *version)
{
  uint16_t windex, wvalue, wlength;
  uint8_t request_data[VERSION_MAX_PAYLOAD_SIZE];

  control_usb_fill_header(&windex, &wvalue, &wlength,
    CONTROL_SPECIAL_RESID, CONTROL_GET_VERSION, sizeof(control_version_t));

  DBG(printf("%u: send version command: 0x%04x 0x%04x 0x%04x\n",
    num_commands, windex, wvalue, wlength));

  int ret = libusb_control_transfer(devh,
    LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
    0, wvalue, windex, request_data, wlength, sync_timeout_ms);

  num_commands++;

  if (ret != sizeof(control_version_t)) {
    debug_libusb_error(ret);
    return CONTROL_ERROR;
  }

  memcpy(version, request_data, sizeof(control_version_t));
  DBG(printf("version returned: 0x%X\n", *version));

  return CONTROL_SUCCESS;
}

/*
 * Ideally we would examine configuration descriptors and check for actual
 * wMaxPacketSize on given control endpoint.
 *
 * For now, just assume the greatest control transfer size, USB_TRANSACTION_MAX_BYTES. Have host
 * code only check payload size here. Device will not need any additional
 * checks. Device application code will set wMaxPacketSize in its
 * descriptors and take care of allocating a buffer for receiving control
 * requests of up to USB_TRANSACTION_MAX_BYTES bytes.
 *
 * Without checking, libusb would set wLength in header to any number and
 * only send 64 bytes of payload, truncating the rest.
 */
static bool payload_len_exceeds_control_packet_size(size_t payload_len)
{
  if (payload_len > USB_TRANSACTION_MAX_BYTES) {
    printf("control transfer of %zd bytes requested\n", payload_len);
    printf("maximum control packet size is %d\n", USB_TRANSACTION_MAX_BYTES);
    return true;
  }
  else {
    return false;
  }
}

control_ret_t
control_write_command(control_resid_t resid, control_cmd_t cmd,
                      const uint8_t payload[], size_t payload_len)
{
  uint16_t windex, wvalue, wlength;

  if (payload_len_exceeds_control_packet_size(payload_len))
    return CONTROL_DATA_LENGTH_ERROR;

  control_usb_fill_header(&windex, &wvalue, &wlength,
    resid, CONTROL_CMD_SET_WRITE(cmd), (unsigned int)payload_len);

  DBG(printf("%u: send write command: 0x%04x 0x%04x 0x%04x ",
    num_commands, windex, wvalue, wlength));
  DBG(print_bytes(payload, payload_len));

  int ret = libusb_control_transfer(devh,
    LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
    0, wvalue, windex, (unsigned char*)payload, wlength, sync_timeout_ms);

  num_commands++;

  if (ret != (int)payload_len) {
    debug_libusb_error(ret);
    return CONTROL_ERROR;
  }

  // Read back write command status
  uint8_t status;
  control_usb_fill_header(&windex, &wvalue, &wlength,
    resid, CONTROL_CMD_SET_WRITE(cmd), 1);

  ret = libusb_control_transfer(devh,
    LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
    0, wvalue, windex, &status, wlength, sync_timeout_ms);

  if (ret != (int)1) {
    debug_libusb_error(ret);
    return CONTROL_ERROR;
  }

  return status;
}

control_ret_t
control_read_command(control_resid_t resid, control_cmd_t cmd,
                     uint8_t payload[], size_t payload_len)
{
  uint16_t windex, wvalue, wlength;

  if (payload_len_exceeds_control_packet_size(payload_len))
    return CONTROL_DATA_LENGTH_ERROR;

  control_usb_fill_header(&windex, &wvalue, &wlength,
    resid, CONTROL_CMD_SET_READ(cmd), (unsigned int)payload_len);

  DBG(printf("%u: send read command: 0x%04x 0x%04x 0x%04x\n",
    num_commands, windex, wvalue, wlength));

  int ret = libusb_control_transfer(devh,
    LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
    0, wvalue, windex, payload, wlength, sync_timeout_ms);

  num_commands++;

  if (ret != (int)payload_len) {
    debug_libusb_error(ret);
    return CONTROL_ERROR;
  }

  DBG(printf("read data returned: "));
  DBG(print_bytes(payload, payload_len));

  return CONTROL_SUCCESS;
}

control_ret_t control_init_usb(int vendor_id, int product_id, int interface_num)
{
  int ret = libusb_init(NULL);
  if (ret < 0) {
    PRINT_ERROR("Failed to initialise libusb\n");
    return CONTROL_ERROR;
  }

  libusb_device **devs = NULL;
  int num_dev = libusb_get_device_list(NULL, &devs);

  libusb_device *dev = NULL;
  for (int i = 0; i < num_dev; i++) {
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(devs[i], &desc);
    if (desc.idVendor == vendor_id && desc.idProduct == product_id) {
      dev = devs[i];
      break;
    }
  }

  if (dev == NULL) {
    PRINT_ERROR("Could not find device\n");
    return CONTROL_ERROR;
  }

  if (libusb_open(dev, &devh) < 0) {
    PRINT_ERROR("Failed to open device. Ensure adequate permissions if using Linux\nor remove any pre-installed drivers with Device Manager on Windows.\n");
    return CONTROL_ERROR;
  }

  libusb_free_device_list(devs, 1);

  return CONTROL_SUCCESS;
}

control_ret_t control_cleanup_usb(void)
{
  libusb_close(devh);
  libusb_exit(NULL);

  return CONTROL_SUCCESS;
}

#endif // USE_USB
