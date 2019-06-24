/*
 * @Descripttion: the driver of oled with iic protocol
 * @version: v1.0.0
 * @Author: Kevin
 * @Email: kkcoding@qq.com
 * @Date: 2019-06-24 21:37:48
 * @LastEditors: Kevin
 * @LastEditTime: 2019-06-25 02:03:38
 */

// ----------------------------------------------------------------
// OLED  ESP32
// GND   电源地
// VCC   接3.3v电源
// SCL   接PB8（SCL）
// SDA   接PB9（SDA）            
// ----------------------------------------------------------------
//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127

#include "oled.h"
#include "string.h"
#include "stdlib.h"
#include "fonts.h"

//OLED缓存128*64bit
static uint8_t oled_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
//OLED实时信息
static SSD1306_t oled;
//OLED是否正在显示，1显示，0等待
static bool is_show_str = 0;

/**
 * @brief: 
 * @param  NULL
 * @return: 
 */
void i2c_init(void)
{
    //注释参考sht30之i2c教程
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_OLED_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_OLED_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;
    i2c_param_config(I2C_OLED_MASTER_NUM, &conf);
    i2c_driver_install(I2C_OLED_MASTER_NUM, conf.mode,0, 0, 0);
}

/**
 * @brief: 向oled写命令
 * @param[in]   command
 * @return: 
 *      - ESP_OK 
 */
int oled_write_cmd(uint8_t command)
{
    //注释参考sht30之i2c教程
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ret = i2c_master_start(cmd);
    ret = i2c_master_write_byte(cmd, OLED_WRITE_ADDR |WRITE_BIT , ACK_CHECK_EN); 
    ret = i2c_master_write_byte(cmd, WRITE_CMD, ACK_CHECK_EN);
    ret = i2c_master_write_byte(cmd,command, ACK_CHECK_EN);
    ret = i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_OLED_MASTER_NUM, cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        return ret;
    }
    return ret;
}

/**
 * @brief: 向oled写数据
 * @param[in]   data
 * @return: 
 *      - ESP_OK 
 */
int oled_write_data(uint8_t data)
{
    //注释参考sht30之i2c教程
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ret = i2c_master_start(cmd);
    ret = i2c_master_write_byte(cmd, OLED_WRITE_ADDR | WRITE_BIT, ACK_CHECK_EN);
    ret = i2c_master_write_byte(cmd, WRITE_DATA, ACK_CHECK_EN);
    ret = i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    ret = i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_OLED_MASTER_NUM, cmd, 100 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        return ret;
    }
    return ret;
}

/**
 * @brief: 向oled写长数据
 * @param[in]   data   要写入的数据
 * @param[in]   len     数据长度
 * @return: 
 *      - ESP_OK 
 */
int oled_write_long_data(uint8_t *data,uint16_t len)
{
    //注释参考sht30之i2c教程
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ret = i2c_master_start(cmd);
    ret = i2c_master_write_byte(cmd, OLED_WRITE_ADDR | WRITE_BIT, ACK_CHECK_EN);
    ret = i2c_master_write_byte(cmd, WRITE_DATA, ACK_CHECK_EN);
    ret = i2c_master_write(cmd, data, len,ACK_CHECK_EN);
    ret = i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_OLED_MASTER_NUM, cmd, 10000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        return ret;
    }
    return ret;    
}

/**
 * @brief: init oled
 * @param {in}  NULL
 * @return: 
 */
void oled_init(void)
{
    //i2c初始化
    i2c_init();
    //oled配置
    oled_write_cmd(0xAE);  //display off
    oled_write_cmd(0X20);  //Set Memory Addressing Mode	
    oled_write_cmd(0X10);  //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    oled_write_cmd(0XB0);  //Set Page Start Address for Page Addressing Mode,0-7
    oled_write_cmd(0XC8);  //Set COM Output Scan Direction
    oled_write_cmd(0X00);  //---set low column address
    oled_write_cmd(0X10);  //---set high column address
    oled_write_cmd(0X40);  //--set start line address
    oled_write_cmd(0X81);  //--set contrast control register
    oled_write_cmd(0XFF);  //亮度调节 0x00~0xff
    oled_write_cmd(0XA1);  //--set segment re-map 0 to 127
    oled_write_cmd(0XA6);  //--set normal display
    oled_write_cmd(0XA8);  //--set multiplex ratio(1 to 64)
    oled_write_cmd(0X3F);  //
    oled_write_cmd(0XA4);  //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    oled_write_cmd(0XD3);  //-set display offset
    oled_write_cmd(0X00);  //-not offset
    oled_write_cmd(0XD5);  //--set display clock divide ratio/oscillator frequency
    oled_write_cmd(0XF0);  //--set divide ratio
    oled_write_cmd(0XD9);  //--set pre-charge period
    oled_write_cmd(0X22);  //
    oled_write_cmd(0XDA);  //--set com pins hardware configuration
    oled_write_cmd(0X12);  
    oled_write_cmd(0XDB);  //--set vcomh
    oled_write_cmd(0X20);  //0x20,0.77xVcc
    oled_write_cmd(0X8D);  //--set DC-DC enable
    oled_write_cmd(0X14);  //
    oled_write_cmd(0XAF);  //--turn on oled panel
    //清屏
    oled_clear();
}

/**
 * @brief: 唤醒屏幕
 * @param {type} 
 * @return: 
 */
void oled_on(void)
{
    oled_write_cmd(0x8D);   //设置电源
    oled_write_cmd(0x14);   //开启电源
    oled_write_cmd(0xAF);   //OLED唤醒
}

/**
 * @brief: 休眠屏幕，功耗<10uA
 * @param {type} 
 * @return: 
 */
void oled_off(void)
{
    oled_write_cmd(0x8D);   //设置电源
    oled_write_cmd(0x10);   //关闭电源
    oled_write_cmd(0xAE);   //OLED休眠
}

/**
 * @brief: 将显存内容刷新到oled显示区
 * @param {in}  NULL
 * @return: 
 */
void oled_update_screen(void)
{
    uint8_t line_index;
    for(line_index=0 ; line_index<8; line_index++)
    {
        oled_write_cmd(0xb0+line_index);
        oled_write_cmd(0x00);
        oled_write_cmd(0x10);
        
        oled_write_long_data(&oled_buffer[SSD1306_WIDTH * line_index], SSD1306_WIDTH);
    }
}

/**
 * @brief: fill all the screen 
 * @param {in} data: only choose '0x00' or '0xff'
 * @return: 
 */
void oled_fill(uint8_t data)
{
    //置ff缓存
    memset(oled_buffer, data, sizeof(oled_buffer));
    oled_update_screen();
}

/**
 * @brief: clear all screen with black
 * @param {in} NULL
 * @return: 
 */
void oled_clear(void)
{
    //清0缓存
    oled_fill(SSD1306_COLOR_BLACK);
}

/**
 * @brief: 移动坐标
 * @param[in]   x   显示区坐标 x
 * @param[in]   y   显示去坐标 y
 * @return: 
 */
void oled_goto_cursor(uint16_t x, uint16_t y) 
{
	oled.cursor_x = x;
	oled.cursor_y = y;
}

/**
 * @brief: 向显存写入
 * @param[in]   x   坐标
 * @param[in]   y   坐标
 * @param[in]   color   色值0/1
 * @return: 
 */
void oled_draw_pixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color) 
{
	if (x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT) {
		return;
	}
	if (color == SSD1306_COLOR_WHITE) {
		oled_buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} else {
		oled_buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}

/**
 * @brief: 在x，y位置显示字符
 * @param[in]   x    显示坐标x 
 * @param[in]   y    显示坐标y 
 * @param[in]   ch   要显示的字符
 * @param[in]   font 显示的字形
 * @param[in]   color 颜色  1显示 0不显示
 * @return: 
 */
char oled_show_char(uint16_t x, uint16_t y, char ch, FontDef_t* Font, SSD1306_COLOR_t color) 
{
	uint32_t i, b, j;
	if (SSD1306_WIDTH <= (oled.cursor_x + Font->FontWidth)
        || SSD1306_HEIGHT <= (oled.cursor_y + Font->FontHeight)) {
		return 0;
	}
	if (0 == is_show_str) {
        oled_goto_cursor(x,y);
    }

	for (i = 0; i < Font->FontHeight; i++) {
		b = Font->data[(ch - ' ') * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				oled_draw_pixel(oled.cursor_x + j, (oled.cursor_y + i), (SSD1306_COLOR_t) color);
			} else {
				oled_draw_pixel(oled.cursor_x + j, (oled.cursor_y + i), (SSD1306_COLOR_t)!color);
			}
		}
	}
	oled.cursor_x += Font->FontWidth;
	if(0 == is_show_str) {
       oled_update_screen(); 
    }
	return ch;
}

/**
 * @brief: 在x，y位置显示字符串 
 * @param[in]   x    显示坐标x 
 * @param[in]   y    显示坐标y 
 * @param[in]   str   要显示的字符串
 * @param[in]   font 显示的字形
 * @param[in]   color 颜色  1显示 0不显示
 * @return: 
 */
char oled_show_str(uint16_t x, uint16_t y, char* str, FontDef_t* Font, SSD1306_COLOR_t color) 
{
    is_show_str=1;
    oled_goto_cursor(x,y);
	while (*str) 
    {
		if (oled_show_char(x,y,*str, Font, color) != *str) 
        {
            is_show_str=0;
			return *str;
		}
		str++;
	}
    is_show_str=0;
    oled_update_screen();
	return *str;
}

/**
 * @brief: 
 * @param {type} 
 * @return: 
 */
void oled_draw_bmp(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char bmp[])
{ 	
    //TODO
} 