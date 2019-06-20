#include "lcd.h"

#include "esp_log.h"

#include "font.h"

static char tag[] = "lcd";
_lcd_info lcd_info;

//背景色，画笔色
uint16_t lcd_back_color = BLACK;
uint16_t lcd_point_color = WHITE;

spi_device_handle_t spi_lcd;
spi_device_handle_t spi_sd;
//uint16_t char_map[320*240];//显示缓存

#define SPI_BUF_LEN 512


void spi_pre_transfer_callback(spi_transaction_t *t);
/**
 * these are base function for transfering on spi.
 * hardward config: spi & gpio.
 */

int spi_init(void)
{
    esp_err_t ret;
    
    gpio_pad_select_gpio(PIN_NUM_DC);
    gpio_pad_select_gpio(PIN_NUM_RST);
    gpio_pad_select_gpio(PIN_NUM_BCKL);
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    gpio_set_level(PIN_NUM_DC, 0);
    gpio_set_level(PIN_NUM_RST, 0);
    gpio_set_level(PIN_NUM_BCKL, 0);

    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 240 * 240 * 2+8,
    };
    /*SPI四种模式区别
    spi四种模式SPI的相位(CPHA)和极性(CPOL)分别可以为0或1，对应的4种组合构成了SPI的4种模式(mode)
    Mode 0 CPOL=0, CPHA=0 
    Mode 1 CPOL=0, CPHA=1
    Mode 2 CPOL=1, CPHA=0 
    Mode 3 CPOL=1, CPHA=1
    时钟极性CPOL: 即SPI空闲时，时钟信号SCLK的电平（1:空闲时高电平; 0:空闲时低电平）
    时钟相位CPHA: 即SPI在SCLK第几个边沿开始采样（0:第一个边沿开始; 1:第二个边沿开始）*/
    spi_device_interface_config_t devcfg={
        .clock_speed_hz = 26000000,				// Initial clock out at 8 MHz
        .mode = 3,								// SPI mode 0
        .spics_io_num = PIN_NUM_CS,				// set SPI CS pin
        .queue_size = SPI_BUF_LEN,              //We want to be able to queue 7 transactions at a time
        .pre_cb = spi_pre_transfer_callback,    //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi_lcd);
    ESP_ERROR_CHECK(ret);
    
    // spi_device_interface_config_t sd_devcfg={
    //     .clock_speed_hz = 20000000,				// Initial clock out at 8 MHz
    //     .mode = 0,								// SPI mode 0
    //     .spics_io_num = 13,				// set SPI CS pin
    //     .queue_size = SPI_BUF_LEN,              //We want to be able to queue 7 transactions at a time
    // };
    // ret = spi_bus_add_device(HSPI_HOST, &sd_devcfg, &spi_sd);
    // ESP_ERROR_CHECK(ret);
    return ESP_OK;
}

int spi_deinit(void)
{
    return ESP_OK;
}

void spi_pre_transfer_callback(spi_transaction_t *t) 
{
    int dc = (int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

//Send a command to the LCD. Uses spi_device_transmit, which waits until the transfer is complete.
void spi_send_cmd(const uint8_t cmd) 
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));			//Zero out the transaction
    t.length = 8;                     	//Command is 8 bits
    t.tx_buffer = &cmd;              	//The data is the cmd itself
    t.user = (void*)0;               	//D/C needs to be set to 0
    ret = spi_device_transmit(spi_lcd, &t); //Transmit!
    assert(ret == ESP_OK);          	//Should have had no issues.
}

//Send data to the LCD. Uses spi_device_transmit, which waits until the transfer is complete.
void spi_send_data(const uint8_t *data, int len) 
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len == 0) {             		//no need to send anything
        return;
    }
    memset(&t, 0, sizeof(t));			//Zero out the transaction
    t.length = len*8;               	//Len is in bytes, transaction length is in bits.
    t.tx_buffer = data;             	//Data
    t.user = (void*)1;              	//D/C needs to be set to 1
    ret = spi_device_transmit(spi_lcd, &t);	//Transmit!
    assert(ret == ESP_OK);            	//Should have had no issues.
}

/**
 * lcd send api
 */
//send one byte data
void lcd_write_data8(const uint8_t da)
{
    spi_send_data(&da, 1);
} 

//send two bytes data
void lcd_write_data16(uint16_t da)
{
    uint8_t dat[2];
    dat[0] = da >>8;
    dat[1] = da;
    spi_send_data(dat, 2);
}	

void lcd_write_reg(uint8_t da)	 
{	
    spi_send_cmd(da);
}

void lcd_fast_write_data8(uint8_t* data ,uint32_t len)
{
    uint32_t i;

    for(i = 0; (i + SPI_BUF_LEN) < len; i += SPI_BUF_LEN) {
        spi_send_data(data + i, SPI_BUF_LEN);
    }

    if(len - i > 0)	{
        spi_send_data(data + i, len - i);
    }
}

void lcd_fast_write_data16(uint16_t* data ,uint32_t len)
{
    uint32_t i;
    uint8_t *data8 = (uint8_t *)data;
    uint16_t tmp = 0;
    
    for (i = 0; i < len; i++) {
        tmp = data[i];
        data8[2*i] = tmp >> 8;//高位
        data8[2*i + 1] = tmp; //低位
    }
    lcd_fast_write_data8(data8, len*2);
}  

void lcd_fast_write_color(uint16_t color ,uint32_t len)
{
    uint32_t i;
    uint8_t *data8 = (uint8_t *)malloc(2 * len);

    if (!data8) {
        ESP_LOGE("LCD", "ERR, len=%d", len);
        return;
    }
    for (i = 0; i < len; i++) {
        data8[2*i] = color >> 8;//高位
        data8[2*i + 1] = color; //低位
    }	
    lcd_fast_write_data8(data8, len*2);

    free(data8);	 
}

/**
 * the APIs below are irrelevant with hardware
 */

void lcd_write_reg_data8(uint8_t reg, uint8_t data)
{
    lcd_write_reg(reg);
    lcd_write_data8(data);
}

void lcd_write_reg_data16(uint8_t reg, uint16_t data)
{
    lcd_write_reg(reg);
    lcd_write_data16(data);
}

//读LCD数据
uint16_t lcd_read_data16(void)
{	
    return 0;
}
//读寄存器数据
uint16_t lcd_read_reg_data16(uint8_t reg)
{
    lcd_write_reg(reg);
    return lcd_read_data16();
}

//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
    lcd_write_reg(lcd_info.wramcmd);  
}

//LCD写GRAM
void lcd_write_ram(uint16_t RGB_Code)
{
    lcd_write_data16(RGB_Code);  
}


//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
uint16_t lcd_bgr2rgb(uint16_t c)
{
    uint16_t r,g,b,rgb;   
    b = (c>>0)&0x1f;
    g = (c>>5)&0x3f;
    r = (c>>11)&0x1f;	 
    rgb = (b<<11) + (g<<5) + (r<<0);
    return rgb;
} 

void lcd_set_addr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{ 
    lcd_write_reg(lcd_info.setxcmd);
    lcd_write_data16(x1);
    lcd_write_data16(x2);
    
    lcd_write_reg(lcd_info.setycmd);
    lcd_write_data16(y1);
    lcd_write_data16(y2);

    lcd_write_reg(lcd_info.wramcmd);					 						 
}
/**
 *  设置光标位置
 */
void lcd_set_cursor(uint16_t x_pos, uint16_t y_pos)
{
    lcd_write_reg(lcd_info.setxcmd); 
    lcd_write_data16(x_pos);	 
    lcd_write_reg(lcd_info.setycmd); 
    lcd_write_data16(y_pos);
} 

//设置LCD的自动扫描方向
//注意:其他函数可能会受到此函数设置的影响(尤其是9341/6804这两个奇葩),
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
//dir:0~7,代表8个方向(具体定义见lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341/5310/5510等IC已经实际测试

void lcd_set_scan_dir(uint8_t dir)
{
    uint16_t regval=0;
    uint16_t temp;

    switch(dir) {
        case L2R_U2D://从左到右,从上到下
            regval |= (0<<7)|(0<<6)|(0<<5)|(1<<4); 
            break;
        case L2R_D2U://从左到右,从下到上
            regval |= (1<<7)|(0<<6)|(0<<5)|(1<<4); 
            break;
        case R2L_U2D://从右到左,从上到下
            regval |= (0<<7)|(1<<6)|(0<<5)|(1<<4); 
            break;
        case R2L_D2U://从右到左,从下到上
            regval |= (1<<7)|(1<<6)|(0<<5)|(1<<4); 
            break;
        case U2D_L2R://从上到下,从左到右
            regval |= (0<<7)|(0<<6)|(1<<5)|(0<<4); 
            break;
        case U2D_R2L://从上到下,从右到左
            regval |= (0<<7)|(1<<6)|(1<<5)|(0<<4); 
            break;
        case D2U_L2R://从下到上,从左到右
            regval |= (1<<7)|(0<<6)|(1<<5)|(0<<4); 
            break;
        case D2U_R2L://从下到上,从右到左
            regval |= (1<<7)|(1<<6)|(1<<5)|(0<<4); 
            break;
        default ://从左到右,从上到下
            regval |= (0<<7)|(0<<6)|(0<<5)|(1<<4);
            break;
    }
    lcd_write_reg_data8(0x36, regval|0x08);
    //
    if((regval&0X20) || dir == L2R_D2U) {
        if (lcd_info.width < lcd_info.height) {
            temp = lcd_info.width;
            lcd_info.width = lcd_info.height;
            lcd_info.height = temp;
        }
    } else {
        if(lcd_info.width > lcd_info.height) {
            temp = lcd_info.width;
            lcd_info.width = lcd_info.height;
            lcd_info.height = temp;
        }
    }  
}   

void lcd_draw_point(uint16_t x,uint16_t y)
{
    lcd_set_addr(x,y,x,y);
    lcd_write_data16(lcd_point_color); 
}

//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
    lcd_write_reg(lcd_info.setxcmd); 
    lcd_write_data16(x);	 
    lcd_write_reg(lcd_info.setycmd); 
    lcd_write_data16(y);
    
    lcd_write_reg_data16(lcd_info.wramcmd,color);		
}

//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
//sx,sy:窗口起始坐标(左上角)
//width,height:窗口宽度和高度,必须大于0!!
//窗体大小:width*height.
//68042,横屏时不支持窗口设置!! 
void lcd_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{    
    width = sx+width-1;
    height = sy+height-1;
    lcd_write_reg(lcd_info.setxcmd); 
    lcd_write_data16(sx);	 
    lcd_write_data16(width);   
    lcd_write_reg(lcd_info.setycmd); 
    lcd_write_data16(sy); 
    lcd_write_data16(height);
} 
//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
     uint16_t r=0,g=0,b=0;
    uint16_t LCD_RAM;
    if(x>=lcd_info.width||y>=lcd_info.height)return 0;	//超过了范围,直接返回		   
    lcd_set_cursor(x,y);	    
    lcd_write_reg(0X2E);//9341/6804/3510 发送读GRAM指令

    LCD_RAM = lcd_read_data16();//第一次为假读
    LCD_RAM = lcd_read_data16();
    ESP_LOGI(tag, "point date4:0x%x\n",LCD_RAM);   

    if(LCD_RAM)r=0;							//dummy Read	     
    r=LCD_RAM;  		  						//实际坐标颜色
           
    b = LCD_RAM; 
    g = r&0XFF;		//对于9341/5310/5510,第一次读取的是RG的值,R在前,G在后,各占8位
    g <<= 8;
    
    return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));
}

void lcd_delay(int ms)
{
    vTaskDelay(ms / portTICK_RATE_MS);
}

int lcd_init(void)
{  
    if (spi_init() < 0) {
        ESP_LOGE(tag, "init spi failed");
        return -1;
    }
    LCD_BL_RESET();

    LCD_RST_RESET();
    lcd_delay(120);
    LCD_RST_SET();
    lcd_delay(120);

    lcd_info.dir = 0;	//竖屏
    lcd_info.width = 240;
    lcd_info.height = 240;
    lcd_info.id = 0X7789;
    lcd_info.wramcmd = 0X2C;
    lcd_info.setxcmd = 0X2A;
    lcd_info.setycmd = 0X2B;

    /* Sleep Out */
    lcd_write_reg(0x11);
    /* wait for power stability */
    lcd_delay(120);

    /* Memory Data Access Control */
    lcd_write_reg(0x36);
    lcd_write_data8(0x00);

    /* RGB 5-6-5-bit  */
    lcd_write_reg(0x3A);
    lcd_write_data8(0x65);

    /* Porch Setting */
    lcd_write_reg(0xB2);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x00);
    lcd_write_data8(0x33);
    lcd_write_data8(0x33);

    /*  Gate Control */
    lcd_write_reg(0xB7);
    lcd_write_data8(0x72);

    /* VCOM Setting */
    lcd_write_reg(0xBB);
    lcd_write_data8(0x3D);   //Vcom=1.625V

    /* LCM Control */
    lcd_write_reg(0xC0);
    lcd_write_data8(0x2C);

    /* VDV and VRH Command Enable */
    lcd_write_reg(0xC2);
    lcd_write_data8(0x01);

    /* VRH Set */
    lcd_write_reg(0xC3);
    lcd_write_data8(0x19);

    /* VDV Set */
    lcd_write_reg(0xC4);
    lcd_write_data8(0x20);

    /* Frame Rate Control in Normal Mode */
    lcd_write_reg(0xC6);
    lcd_write_data8(0x0F);	//60MHZ

    /* Power Control 1 */
    lcd_write_reg(0xD0);
    lcd_write_data8(0xA4);
    lcd_write_data8(0xA1);

    /* Positive Voltage Gamma Control */
    lcd_write_reg(0xE0);
    lcd_write_data8(0xD0);
    lcd_write_data8(0x04);
    lcd_write_data8(0x0D);
    lcd_write_data8(0x11);
    lcd_write_data8(0x13);
    lcd_write_data8(0x2B);
    lcd_write_data8(0x3F);
    lcd_write_data8(0x54);
    lcd_write_data8(0x4C);
    lcd_write_data8(0x18);
    lcd_write_data8(0x0D);
    lcd_write_data8(0x0B);
    lcd_write_data8(0x1F);
    lcd_write_data8(0x23);

    /* Negative Voltage Gamma Control */
    lcd_write_reg(0xE1);
    lcd_write_data8(0xD0);
    lcd_write_data8(0x04);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x11);
    lcd_write_data8(0x13);
    lcd_write_data8(0x2C);
    lcd_write_data8(0x3F);
    lcd_write_data8(0x44);
    lcd_write_data8(0x51);
    lcd_write_data8(0x2F);
    lcd_write_data8(0x1F);
    lcd_write_data8(0x1F);
    lcd_write_data8(0x20);
    lcd_write_data8(0x23);

    /* Display Inversion On */
    lcd_write_reg(0x21);

    lcd_write_reg(0x29);
    lcd_write_reg(0x2c);
    lcd_set_scan_dir(L2R_U2D);	//默认扫描方向
    lcd_clear(lcd_back_color);
    LCD_BL_SET();
    ESP_LOGI(tag, "LCD init ok!\n");

    return 0;
}

void lcd_deinit(void)
{
    spi_deinit();
}

//LCD开启显示
void lcd_display_on(void)
{
    lcd_write_reg(0x29);
    LCD_BL_SET();
}
//LCD关闭显示
void lcd_display_off(void)
{
    lcd_write_reg(0x28);
    LCD_BL_RESET();
}

void lcd_set_back(uint16_t color)
{
    lcd_back_color = color;
}

void lcd_set_point(uint16_t color)
{
    lcd_point_color = color;
}

//清屏函数
//Color:要清屏的填充色
void lcd_clear(uint16_t color)
{
    lcd_set_addr(0, 0, lcd_info.width - 1, lcd_info.height - 1);
    lcd_fast_write_color(color, lcd_info.width*lcd_info.height); 	
}

//在指定区域内填充data
//(x1,y1),(x2,y2):填充矩形对角坐标,区域大小为:(x2-x1+1) * (y2-y1+1)  
//data:要填充的data
void lcd_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *data)
{  
    lcd_set_addr(x1, y1, x2, y2);
    lcd_fast_write_data16(data, (x2-x1+1) * (y2-y1+1));	
}

//在指定区域内填充指定颜色
//区域大小: (x2-x1+1) * (y2-y1+1)
void lcd_fill_color(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{   
    lcd_set_addr(x1, y1, x2, y2);      //设置光标位置 
    lcd_fast_write_color(color, (x2-x1+1) * (y2-y1+1));
} 

//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint16_t t; 
    int xerr=0,yerr=0,delta_x,delta_y,distance; 
    int incx,incy,uRow,uCol;

    delta_x = x2-x1; //计算坐标增量 
    delta_y = y2-y1; 
    uRow=x1; 
    uCol=y1;
    if (delta_x > 0) {
        incx = 1; //设置单步方向 
    } else if (delta_x == 0) {
        incx = 0;//垂直线 
    } else {
        incx = -1;
        delta_x = -delta_x;
    } 
    if (delta_y > 0) {
        incy = 1;
    } else if (delta_y == 0) {
        incy = 0;//水平线 
    } else {
        incy = -1;
        delta_y = -delta_y;
    } 
    if (delta_x > delta_y) {
        distance=delta_x; //选取基本增量坐标轴 
    } else {
        distance=delta_y;
    }
    for (t=0; t<=distance+1; t++) {//画线输出 
        lcd_draw_point(uRow,uCol);//画点 
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance; 
            uRow += incx; 
        }
        if (yerr > distance) { 
            yerr -= distance; 
            uCol += incy; 
        }
    }
}
//画矩形
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_fill_color(x1, y1, x2, y1, lcd_point_color);
    lcd_fill_color(x1, y1, x1, y2, lcd_point_color);
    lcd_fill_color(x1, y2, x2, y2, lcd_point_color);
    lcd_fill_color(x2, y1, x2, y2, lcd_point_color);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void lcd_draw_circle(uint16_t x, uint16_t y, uint8_t r)
{
    int a = 0;
    int b = r;
    int di = 3-(r<<1);  //判断下个点位置的标志
    
    while (a<=b) {
        lcd_draw_point(x+a, y-b);             //5
        lcd_draw_point(x+b, y-a);             //0           
        lcd_draw_point(x+b, y+a);             //4               
        lcd_draw_point(x+a, y+b);             //6 
        lcd_draw_point(x-a, y+b);             //1       
        lcd_draw_point(x-b, y+a);             
        lcd_draw_point(x-a, y-b);             //2             
        lcd_draw_point(x-b, y-a);             //7     	         
        a++;
        //使用Bresenham算法画圆     
        if (di<0) {
            di += 4*a+6;
        } else {
            di += 10+4*(a-b);
            b--;
        }
    }
}
//在指定位置显示一个字符
//num:要显示的字符:" "--->"~"
//mode:叠加方式(1)还是非叠加方式(0)
void lcd_show_char(uint16_t x, uint16_t y, char num, uint8_t size, uint8_t mode)
{  							  
    uint8_t temp = 0, t1, t, xwidth;
    uint16_t y0 = y;
    uint16_t colortemp = lcd_point_color;   
    uint16_t char_map[16*16];

    xwidth = size/2;
    num = num - ' ';//得到偏移后的值
    if(!mode) //非叠加方式
    {
        for(t=0; t<size; t++)
        {   
            if(size == 12) {
                temp = asc2_1206[(uint8_t)num][t];
            } else {
                temp = asc2_1608[(uint8_t)num][t];
            }
            for (t1 = 0; t1 < 8; t1++) {
                if (temp&0x01) {
                    lcd_point_color = colortemp;
                } else {
                    lcd_point_color = lcd_back_color;
                }
                char_map[t*xwidth + t1] = lcd_point_color;	//先存起来再显示
                temp >>= 1;
                y++;
                if(y >= lcd_info.height) {
                    lcd_point_color=colortemp;
                    return;
                }
                if ((y-y0) == size) {
                    y = y0;
                    x++;
                    if (x >= lcd_info.width) {
                        lcd_point_color = colortemp;
                        return;
                    }
                    break;
                }
            }
        }
    } else {//叠加方式
        for (t=0; t<size; t++) {
            if (size == 12) {
                temp = asc2_1206[(uint8_t)num][t];
            } else {
                temp=asc2_1608[(uint8_t)num][t];
            }
            for (t1=0; t1<8; t1++) {			    
                if (temp&0x01) {//从左到右 逐列扫描
                    //lcd_draw_point(x,y); 
                    char_map[t*xwidth + t1] = lcd_point_color;	//先存起来再显示
                } else {
                    char_map[t*xwidth + t1] = lcd_back_color;
                }
                temp >>= 1;
                y++;
                if (y >= lcd_info.height) {
                    lcd_point_color = colortemp; return;
                }
                if ((y-y0) == size) {
                    y=y0;
                    x++;
                    if (x >= lcd_info.width) {
                        lcd_point_color = colortemp;
                        return;
                    }
                    break;
                }
            }
        }
    }
    lcd_fill(x - size/2, y0, x-1, y+size-1, char_map);
    lcd_point_color = colortemp;	    	   	 	  
}

//显示字符串
//x,y:起点坐标
//width,height:区域大小  
//size:字体大小
//*p:字符串起始地址		  
void lcd_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t size, char *p)
{         
    uint16_t x0=x;
    width += x;
    height += y;
    while ((*p<='~') && (*p>=' ')) {//判断是不是非法字符!
        if(x>=width) {
            x = x0;
            y += size;
        }
        if(y >= height) {
            break;//退出
        }
        lcd_show_char(x, y, *p, size, 0);
        x += size/2;
        p++;
    }  
}

//m^n函数,m to the power of n
uint32_t math_power(uint8_t m,uint8_t n)
{
    uint32_t result = 1;	 
    while (n--) {
        result *= m;
    }
    return result;
}

//在指定位置显示一个汉字(32*32大小)
//dcolor为内容颜色，gbcolor为背静颜色
void showhanzi(uint16_t x, uint16_t y, char *p, uint8_t size)	
{  
    unsigned char i,j;
    unsigned char *temp = (uint8_t *)p;  
    uint16_t char_map[32*32];
    uint16_t cnt = 32 * math_power(size/16,2); 	
    for (j=0;j<cnt;j++) {
        for (i=0;i<8;i++) { 		     
            if ((*temp&(1<<i)) != 0)	
                char_map[j*8 + i] = lcd_point_color;
            else
                char_map[j*8 + i] = lcd_back_color; 
        }
        temp++;
     }
     lcd_fill(x,y,x + size - 1,y + size -1,char_map);
}

void showimage(uint16_t x, uint16_t y) //显示40*40图片
{  
    uint16_t i,j,k;
    uint16_t da;
    uint16_t char_map[40*40];
    k=0;
    for (i=0;i<40;i++) {	
        for (j=0;j<40;j++) {
            da = qqimage[k*2+1];
            da <<= 8;
            da |= qqimage[k*2]; 
            char_map[i*40 + j] = da;					
            k++;  			
        }
    }
    lcd_fill(x, y, x+39, y+39, char_map);
}


