// Copyright 2017-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#if USE_SPI && RPI

#include <stdio.h>
#include <unistd.h>
#include "device_control_host.h"
#include "control_host_support.h"
#include "bcm2835.h"
#include <time.h>

//#define DBG(x) x
#define DBG(x)
#define PRINT_ERROR(...)   fprintf(stderr, "Error  : " __VA_ARGS__)

control_ret_t
control_init_spi_pi(spi_mode_t spi_mode, bcm2835SPIClockDivider clock_divider)
{
  if(!bcm2835_init() ||
     !bcm2835_spi_begin()) {
    fprintf(stderr, "BCM2835 initialisation failed. Possibly not running as root\n");
    return CONTROL_ERROR;
  }

  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(spi_mode);
  bcm2835_spi_setClockDivider(clock_divider);
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
	
  return CONTROL_SUCCESS;
}
control_ret_t
control_write_command(control_resid_t resid, control_cmd_t cmd,
                      const uint8_t payload[], size_t payload_len)
{
  uint8_t data_sent_recieved[SPI_TRANSACTION_MAX_BYTES];

  do
  {
      int data_len = control_build_spi_data(data_sent_recieved, resid, cmd, payload, payload_len);
      bcm2835_spi_transfern((char *)data_sent_recieved, data_len);
  }while(data_sent_recieved[0] == CONTROL_COMMAND_IGNORED_IN_DEVICE);
  do
  {
      // get status
      memset(data_sent_recieved, 0, SPI_TRANSACTION_MAX_BYTES);
      unsigned transaction_length = payload_len < 8 ? 8 : payload_len;  
      bcm2835_spi_transfern((char *)data_sent_recieved, payload_len);
  }while(data_sent_recieved[0] == CONTROL_COMMAND_IGNORED_IN_DEVICE);
  //printf("data_sent_recieved[0] = 0x%x, 0x%x, 0x%x, 0x%x\n",data_sent_recieved[0], data_sent_recieved[1], data_sent_recieved[2], data_sent_recieved[3]);
  // data_sent_received[0] contains write command status so return it.
  return data_sent_recieved[0];
}

control_ret_t
control_read_command(control_resid_t resid, control_cmd_t cmd,
                     uint8_t payload[], size_t payload_len)
{
  uint8_t data_sent_recieved[SPI_TRANSACTION_MAX_BYTES] = {0};
  //printf("control_read_command(): resid 0x%x, cmd_id 0x%x, payload_len 0x%x\n",resid, cmd, payload_len);
  do
  {
      int data_len = control_build_spi_data(data_sent_recieved, resid, cmd, payload, payload_len);
      bcm2835_spi_transfern((char *)data_sent_recieved, data_len);

  }while(data_sent_recieved[0] == CONTROL_COMMAND_IGNORED_IN_DEVICE);
  
  do
  {
      memset(data_sent_recieved, 0, SPI_TRANSACTION_MAX_BYTES);
      unsigned transaction_length = payload_len < 8 ? 8 : payload_len;  

      bcm2835_spi_transfern((char *)data_sent_recieved, payload_len);
  }while(data_sent_recieved[0] == CONTROL_COMMAND_IGNORED_IN_DEVICE);

  //printf("data_sent_recieved[0] = 0x%x, 0x%x, 0x%x, 0x%x\n",data_sent_recieved[0], data_sent_recieved[1], data_sent_recieved[2], data_sent_recieved[3]);
  memcpy(payload, data_sent_recieved, payload_len);
  // TODO - For write commands, control_write_command() is returning status from the device. For read commands payload[0] has the
  // status from the device and control_read_command() always returns CONTROL_SUCCESS. Make status returning consistent across
  // for read and write command functions.
  return CONTROL_SUCCESS;
}

control_ret_t
control_cleanup_spi(void)
{
  bcm2835_spi_end();
  bcm2835_close();
  return CONTROL_SUCCESS;
}

#endif /* USE_SPI && RPI */
