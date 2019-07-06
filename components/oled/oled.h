/*
 * @Descripttion: the driver of oled with iic protocol
 * @version: v1.0.0
 * @Author: Kevin
 * @Email: kkcoding@qq.com
 * @Date: 2019-06-24 21:37:48
 * @LastEditors: Kevin
 * @LastEditTime: 2019-07-05 17:08:52
 */


#ifndef _OLED_H_
#define _OLED_H_

#include <stdio.h>
#include "esp_system.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include "freertos/task.h"
#include "driver/i2c.h"
#include "fonts.h"

//I2C
#define I2C_OLED_MASTER_SCL_IO          15               /*!< gpio number for I2C master clock */
#define I2C_OLED_MASTER_SDA_IO          33               /*!< gpio number for I2C master data  */
#define I2C_OLED_MASTER_NUM             I2C_NUM_1        /*!< I2C port number for master dev */
#define WRITE_BIT                       I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                        I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                    0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                   0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                         0x0              /*!< I2C ack value */
#define NACK_VAL                        0x1              /*!< I2C nack value */
    
//OLED
#define OLED_WRITE_ADDR    0x78							 /*通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78*/
#define SSD1306_WIDTH  128
#define SSD1306_HEIGHT 64
#define WRITE_CMD      			 0X00
#define WRITE_DATA     			 0X40

#define TURN_OFF_CMD             0xAE                     //--turn off oled panel
#define SET1_LOW_COL_ADDR_CMD    0x00                     //---set low column address
#define SET2_HI_COL_ADDR_CMD     0x10                     //---set high column address
#define SET3_START_LINE_CMD      0x40                     //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
#define SET4_CONTR_REG           0x81                     //--set contrast control register
#define SET5_OUT_CURR            0xff                     // Set SEG Output Current Brightness
#define SET6_SEG_MAPPING         0xA1                     //--Set SEG/Column Mapping     0xa0���ҷ��� 0xa1����
#define SET7_SCAN_DIR            0xC8                     //Set COM/Row Scan Direction   0xc0���·��� 0xc8����
#define SET8_NORMAL_DIS          0xA6                     //--set normal display
#define SET9_RATIO               0xA8                     //--set multiplex ratio(1 to 64)
#define SET10_DUTY               0x3f                     //--1/64 duty
#define SET11_DIS_OFFSET         0xD3                     //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
#define SET12_NO_OFFSET          0x00                     //-not offset
#define SET13_CLOCK_DIV          0xd5                     //--set display clock divide ratio/oscillator frequency
#define SET14_CLOCK_FC           0x80                     //--set divide ratio, Set Clock as 100 Frames/Sec
#define SET15_PRE_CHARGE         0xD9                     //--set pre-charge period
#define SET16_PER_CHARGE_15      0xF1                     //Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
#define SET17_COM_PIN_CONF       0xDA                     //--set com pins hardware configuration
#define SET18_STG1               0x12                     
#define SET19_vCOMH              0xDB                     //--set vcomh
#define SET20_vCOM_D_LEVVEL      0x40                     //Set VCOM Deselect Level
#define SET21_PAGE_ADDR_MODE     0x20                     //-Set Page Addressing Mode (0x00/0x01/0x02)
#define SET22_STG2               0x02                     //
#define SET23_CHARGE_PUMP        0x8D                     //--set Charge Pump enable/disable
#define SET24_DIS_               0x14                     //--set(0x10) disable
#define SET25_ENTIRE_DIS         0xA4                     // Disable Entire Display On (0xa4/0xa5)
#define SET26_INV_DIS            0xA6                     // Disable Inverse Display On (0xa6/a7) 
#define TURN_ON_CMD              0xAF                     //--turn on oled panel

//显示1，擦除0
typedef enum {
	SSD1306_COLOR_BLACK = 0x00, /*!< Black color, no pixel */
	SSD1306_COLOR_WHITE = 0x01  /*!< Pixel is set. Color depends on LCD */
} SSD1306_COLOR_t;

typedef struct {
	uint16_t cursor_x;
	uint16_t cursor_y;
	uint8_t Inverted;
	uint8_t Initialized;
} SSD1306_t;


void oled_init(void);
void oled_clear(void);
void oled_all_on(void);
void oled_set_pos(uint8_t x, uint8_t y);
int oled_write_cmd(uint8_t command);
int oled_write_data(uint8_t data);
void clean_oled_buff(void);
void oled_update_screen(void);
int oled_write_lang_data(uint8_t *data, uint16_t len);
void oled_draw_pixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color);
void oled_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t color);

void oled_set_cursor(uint16_t x, uint16_t y);
char oled_show_char(uint16_t x, uint16_t y, char ch, FontDef_t Font, SSD1306_COLOR_t color);
char oled_show_str(uint16_t x, uint16_t y, char* str, FontDef_t Font, SSD1306_COLOR_t color);
char oled_show_char_align8(uint16_t x, uint16_t y, char ch, FontDef_t font, SSD1306_COLOR_t color);
char oled_show_str_line(uint8_t line, uint8_t offset, char* str, FontDef_t font, SSD1306_COLOR_t color);
void oled_fill_chunk(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t color);
void oled_fill_chunk_update(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t color);
void oled_draw_bmp(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint8_t bmp[]);

#endif

