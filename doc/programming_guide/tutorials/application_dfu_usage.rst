
####################
RTOS Application DFU
####################

This document is intended to help you use the RTOS DFU driver and RTOS QSPI flash driver in an application.

*******************
DFU Driver Overview
*******************

This driver provides the application with the boot partition and data partition layout of the flash used by the second stage bootloader.  The driver provides a subset of the functionality of `libquadflash <https://www.xmos.ai/documentation/XM-014363-PC-LATEST/html/tools-guide/tools-ref/libraries/libquadflash-api/libquadflash-api.html>`_ enabling the application to use any transport method and the RTOS qspi flash driver to read the factory image, read/write a single upgrade image, and read/write the data partition.

*************************
Reading the Factory Image
*************************

To read back the factory image:

.. code-block:: C

   unsigned addr = rtos_dfu_image_get_factory_addr(dfu_image_ctx);
   unsigned size = rtos_dfu_image_get_factory_size(dfu_image_ctx);

   unsigned char *buf = pvPortMalloc(sizeof(unsigned char) * size);

   rtos_qspi_flash_read(
         qspi_flash_ctx,
         (uint8_t *)buf,
         addr,
         size);

   // buf now contains the factory image contents


It is advised to perform this operation in blocks rather than full image size to reduce memory usage. Once the buffer is populated from flash, it can be sent over the desired transport method, such as USB, |I2C|, etc.

*************************
Reading the Upgrade Image
*************************

To read back the upgrade image:

.. code-block:: C

   unsigned addr = rtos_dfu_image_get_upgrade_addr(dfu_image_ctx);
   unsigned size = rtos_dfu_image_get_upgrade_size(dfu_image_ctx);

   unsigned char *buf = pvPortMalloc(sizeof(unsigned char) * size);

   rtos_qspi_flash_read(
         qspi_flash_ctx,
         (uint8_t *)buf,
         addr,
         size);

   // buf now contains the upgrade image contents


It is advised to perform this operation in blocks rather than full image size to reduce memory usage. Once the buffer is populated from flash, it can be sent over the desired transport method, such as USB, |I2C|, etc.

*************************
Writing the Upgrade Image
*************************

To overwrite the current upgrade image:

.. code-block:: C

   // Assuming buf contains the image data
   // and size contains the size in bytes

   unsigned addr = rtos_dfu_image_get_upgrade_addr(dfu_image_ctx);
   unsigned data_partition_base_addr = rtos_dfu_image_get_data_partition_addr(dfu_image_ctx);
   unsigned bytes_avail = data_partition_base_addr - addr;    
                 
   size_t sector_size = rtos_qspi_flash_sector_size_get(qspi_flash_ctx);

   if(size < bytes_avail) {
      unsigned char *tmp_buf = pvPortMalloc(sizeof(unsigned char) * sector_size);
      unsigned cur_offset = 0;
      do {
         unsigned length = (size - (cur_offset - addr)) >= sector_size ? sector_size : (size - (cur_offset - addr));
         rtos_qspi_flash_lock(qspi_flash_ctx);
         {
            rtos_qspi_flash_read(
                     qspi_flash_ctx,
                     tmp_buf,
                     addr + cur_offset,
                     sector_size);
            memcpy(tmp_buf, data + cur_offset, length);
            rtos_qspi_flash_erase(
                     qspi_flash_ctx,
                     addr + cur_offset,
                     sector_size);
            rtos_qspi_flash_write(
                     qspi_flash_ctx,
                     (uint8_t *) tmp_buf,
                     addr + cur_offset,
                     sector_size);
         }
         rtos_qspi_flash_unlock(qspi_flash_ctx);
         cur_offset += length;
      } while(cur_offset < (size - 1));

      vPortFree(tmp_buf);
   } else {
      rtos_printf("Insufficient space for upgrade image\n");
   }

It is advised to perform this operation in blocks rather than full image size to reduce memory usage. The buffer can be populated over the desired transport method, such as USB, |I2C|, etc.

********************************
Reading the Data Partition Image
********************************

To read back the data partition image:

.. code-block:: C

   unsigned addr = rtos_dfu_image_get_data_partition_addr(dfu_image_ctx);
   unsigned size = rtos_qspi_flash_size_get(qspi_flash_ctx);

   unsigned char *buf = pvPortMalloc(sizeof(unsigned char) * size);

   rtos_qspi_flash_read(
         qspi_flash_ctx,
         (uint8_t *)buf,
         addr,
         size);

   // buf now contains the data partition image contents


It is advised to perform this operation in blocks rather than full image size to reduce memory usage. The data partition will likely be too large to read into SRAM in a read single operation. Once the buffer is populated from flash, it can be sent over the desired transport method, such as USB, |I2C|, etc. 

********************************
Writing the Data Partition Image
********************************

To overwrite the current data partition image:

.. code-block:: C

   // Assuming buf contains the image data
   // and size contains the size in bytes

   unsigned addr = rtos_dfu_image_get_data_partition_addr(dfu_image_ctx);
   unsigned end_addr = rtos_qspi_flash_size_get(qspi_flash_ctx);
   unsigned bytes_avail = end_addr - addr;    
                 
   size_t sector_size = rtos_qspi_flash_sector_size_get(qspi_flash_ctx);

   if(size < bytes_avail) {
      unsigned char *tmp_buf = pvPortMalloc(sizeof(unsigned char) * sector_size);
      unsigned cur_offset = 0;
      do {
         unsigned length = (size - (cur_offset - addr)) >= sector_size ? sector_size : (size - (cur_offset - addr));
         rtos_qspi_flash_lock(qspi_flash_ctx);
         {
            rtos_qspi_flash_read(
                     qspi_flash_ctx,
                     tmp_buf,
                     addr + cur_offset,
                     sector_size);
            memcpy(tmp_buf, data + cur_offset, length);
            rtos_qspi_flash_erase(
                     qspi_flash_ctx,
                     addr + cur_offset,
                     sector_size);
            rtos_qspi_flash_write(
                     qspi_flash_ctx,
                     (uint8_t *) tmp_buf,
                     addr + cur_offset,
                     sector_size);
         }
         rtos_qspi_flash_unlock(qspi_flash_ctx);
         cur_offset += length;
      } while(cur_offset < (size - 1));

      vPortFree(tmp_buf);
   } else {
      rtos_printf("Insufficient space for data partition image\n");
   }

It is advised to perform this operation in blocks rather than full image size to reduce memory usage. The buffer can be populated over the desired transport method, such as USB, |I2C|, etc.
