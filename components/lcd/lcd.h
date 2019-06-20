#ifndef __LCD_H
#define __LCD_H

/*****************************************************************************************
 *
 * @target      : lcd
 * @description :
 * @OS          : freertos
 * @version     : V1.0.0
 * @author      :
 * @date        : 2019-06-20
*****************************************************************************************/

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"


//LCD重要参数集
typedef struct {										    
	uint16_t width;			//LCD 宽度
	uint16_t height;		//LCD 高度
	uint16_t id;			//LCD ID
	uint8_t  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t wramcmd;		//开始写gram指令
	uint16_t setxcmd;		//设置x坐标指令
	uint16_t setycmd;		//设置y坐标指令	 
}_lcd_info; 

extern _lcd_info lcd_info;	//管理LCD重要参数
//IO连接  
#define PIN_NUM_MISO 2 		// SPI MISO
#define PIN_NUM_MOSI 15		// SPI MOSI
#define PIN_NUM_CLK  14		// SPI CLOCK pin
#define PIN_NUM_CS   12		// Display CS pin

#define PIN_NUM_DC   13		// Display command/data pin
#define PIN_NUM_RST  16  	// GPIO used for RESET control
#define PIN_NUM_BCKL  16    // GPIO used for backlight control

// #define PIN_NUM_TCS   0	  // Touch screen CS pin
// #define PIN_NUM_TIRQ   0	  // Touch screen IRQ pin

#define LCD_RST_SET() gpio_set_level(PIN_NUM_RST, 1)
#define LCD_RST_RESET() gpio_set_level(PIN_NUM_RST, 0)

#define LCD_DC_SET() gpio_set_level(PIN_NUM_DC, 1)
#define LCD_DC_RESET() gpio_set_level(PIN_NUM_DC, 0)

#define LCD_BL_SET() //gpio_set_level(PIN_NUM_BCKL, 1)
#define LCD_BL_RESET() //gpio_set_level(PIN_NUM_BCKL, 0)



//扫描方向定义
#define L2R_U2D  0 //从左到右,从上到下
#define L2R_D2U  1 //从左到右,从下到上
#define R2L_U2D  2 //从右到左,从上到下
#define R2L_D2U  3 //从右到左,从下到上

#define U2D_L2R  4 //从上到下,从左到右
#define U2D_R2L  5 //从上到下,从右到左
#define D2U_L2R  6 //从下到上,从左到右
#define D2U_R2L  7 //从下到上,从右到左	 

#define LCD_SCAN_DIR  L2R_U2D  //默认的扫描方向

//LCD
int lcd_init(void);
void lcd_deinit(void);
void lcd_display_on(void);
void lcd_display_off(void);
void lcd_set_back(uint16_t color);
void lcd_set_point(uint16_t color);
void lcd_clear(uint16_t color);
void lcd_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *data);
void lcd_fill_color(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_draw_circle(uint16_t x, uint16_t y, uint8_t r);
void lcd_show_char(uint16_t x, uint16_t y, char num, uint8_t size, uint8_t mode);
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p);

 
//void showhanzi(unsigned int x,unsigned int y,unsigned char index);
void showhanzi(uint16_t x, uint16_t y, char *p, uint8_t size)	;
void showimage(uint16_t x,uint16_t y); //显示40*40图片

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)
		  		 
#endif  

