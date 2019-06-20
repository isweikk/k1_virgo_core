#ifndef _SD_CARD_H_
#define _SD_CARD_H_

/*****************************************************************************************
 *
 * @target      : sd_card
 * @description :
 * @OS          : freertos
 * @version     : V1.0.0
 * @author      :
 * @date        : 2019-06-20
*****************************************************************************************/

// This example can use SDMMC and SPI peripherals to communicate with SD card.
// By default, SDMMC peripheral is used.
// To enable SPI mode, uncomment the following line:
// -standard SD-- - SPI MODE
// 14 <--> CLK,      --> CLK
// 15 <--> CMD,      --> MOSI
//  2 <--> D0,       --> MISO
//  4 <--> D1,       --> X
// 12 <--> D2,       --> X
// 13 <--> D3,       --> CS

#define USE_SPI_MODE

//the default path is /sdcard
int sc_card_init(void);
int sd_card_uninit(void);
		  		 
#endif  

