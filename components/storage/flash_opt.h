#ifndef _FLASH_OPT_H_
#define _FLASH_OPT_H_

/*****************************************************************************************
 *
 * @target      : flash spiffs
 * @description :
 * @OS          : freertos
 * @version     : V1.0.0
 * @author      :
 * @date        : 2019-06-20
*****************************************************************************************/

#define USE_SPI_MODE

//the default path is /sdcard
int storage_flash_init(void);
int storage_flash_deinit(void);
//just for test
void test_spiffs_write(void);
void test_spiffs_read(void);

#endif  

