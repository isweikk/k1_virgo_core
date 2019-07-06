/*
 * @Descripttion: the eyes image, draw them by algorithm
 * @version: v1.0.0
 * @Author: Kevin
 * @Email: kkcoding@qq.com
 * @Date: 2019-07-05 11:49:20
 * @LastEditors: Kevin
 * @LastEditTime: 2019-07-06 03:19:26
 */

#include "eye_img.h"
#include "oled.h"
#include "string.h"
#include "stdlib.h"

/* you must konw the the size of the eys is 48*48, 8 fps.
 */
// ---------------------------
// |            |            |
// |  ******    |    ******  |
// |  ******    |    ******  |
// |  ******    |    ******  |
// |            |            |
// ---------------------------
//when you want to stop a eye animation, you must clear eyes befor a new one.
static uint8_t eye_reset_flag = true;
static int eye_emotion = 0;

/**
 * @brief: 
 * @param {type} 
 * @return: 
 */
void eye_reset(void)
{
    eye_reset_flag = true;
}

int eye_get_emotion(void)
{
    return eye_emotion;
}

void eye_set_emotion(int emotion)
{
    eye_reset();
    eye_emotion = emotion;
}
/**
 * @brief: 
 * @param {type} 
 * @return: 
 */
int eye_show_sleep(void)
{ 	
    static uint8_t step, delay = 0;

    if (eye_reset_flag) {
        step = 0;
        oled_clear();
        eye_reset_flag = false;
    }

    if (step == 0) {    //clear
        oled_clear();
        step = 1;
    } else if (step == 1) {
        if (++delay%4 == 0) {
            oled_show_char_align8(48, 48, 'Z', Font_6x8, SSD1306_COLOR_WHITE);
            step = 2;
        }
    } else if (step == 2) {
        if (++delay%4 == 0) {
            oled_show_char_align8(64, 32, 'Z', Font_8x16, SSD1306_COLOR_WHITE);
            step = 3;
        }
    } else if (step == 3) {
        if (++delay%4 == 0) {
            oled_show_char_align8(80, 8, 'Z', Font_12x24, SSD1306_COLOR_WHITE);
            step = 4;
        }
    } else if (step == 4) {
        step = 0;   //a period over
        eye_reset_flag = true;
        delay = 0;
    }
    oled_update_screen();
    if (step == 0) {
        return 0;
    }
    return -1;
}

/**
 * @brief: wake up from sleeping
 * @param {type} 
 * @return: 
 */
int eye_show_wakeup(void)
{ 	
    static uint16_t x0, y0, x1,y1;
    static uint8_t step, delay;
    static char logo[] = "Virgo";
    static uint8_t count = 0;

    if (eye_reset_flag) {
        x0 = 0;
        y0 = 32;
        step = 0;
        delay = 0;
        oled_clear();
        eye_reset_flag = false;
    }

    if (step == 0) {
        x0 += 8;
        oled_fill_chunk(x0, y0 - 8, x0 + 16, y0 + 8, SSD1306_COLOR_WHITE);
        x0 += 24;
        step = 1;
    } else if (step == 1) {
        ++delay;
        if (delay%2 == 0) {
            delay = 0;
            oled_show_char_align8(x0, y0 - Font_16x32.height/2, logo[count], Font_16x32, SSD1306_COLOR_WHITE);
            count++;
            x0 += Font_16x32.width;
            if (count == strlen(logo)) {
                count = 0;
                step = 2;
            }
        }
        //TODO, 方块增加旋转特效
    } else if (step == 2) {
        if (++delay > 4) {
            step = 0;   //a period over
            eye_reset_flag = true;
            delay = 0;
        }
    }
    oled_update_screen();
    if (step == 0) {
        return 0;
    }
    return -1;
}

/**
 * @brief: blink eyes
 * @param {type} 
 * @return: 
 */
int eye_show_nictation(void)
{ 	
    static uint16_t x0, y0, x1,y1;
    static uint8_t step = 0, speed = 8, keep = 0;

    if (eye_reset_flag) {
        x0 = 8;
        y0 = 16;
        x1 = 56;
        y1 = 48;
        step = 0;
        keep = 0;
        oled_clear();
        eye_reset_flag = false;
    }

    if (step == 0) {
        oled_fill_chunk(x0, y0, x1, y1, SSD1306_COLOR_WHITE);
        oled_fill_chunk(x0+64, y0, x1+64, y1, SSD1306_COLOR_WHITE);
        step = 1;
    } else if (step == 1) {    //close eyes
        y0 += speed;
        y1 -= speed;
        oled_fill_chunk(x0, y0-speed, x1, y0, SSD1306_COLOR_BLACK);
        oled_fill_chunk(x0, y1, x1, y1+speed, SSD1306_COLOR_BLACK);
        oled_fill_chunk(x0 + 64, y0-speed, x1 + 64, y0, SSD1306_COLOR_BLACK);
        oled_fill_chunk(x0 + 64, y1, x1 + 64, y1+speed, SSD1306_COLOR_BLACK);
        if (y0 == y1) {
            step = 2;
        }
    } else if (step == 2) {
        if (++keep > 1) {
        y0 -= speed;
        y1 += speed;
        //left eye
        oled_fill_chunk(x0, y0, x1, y0+speed, SSD1306_COLOR_WHITE);
        oled_fill_chunk(x0, y1-speed, x1, y1, SSD1306_COLOR_WHITE);
        //right eye
        oled_fill_chunk(x0 + 64, y0, x1 + 64, y0+speed, SSD1306_COLOR_WHITE);
        oled_fill_chunk(x0 + 64, y1-speed, x1 + 64, y1, SSD1306_COLOR_WHITE);
        }
        if (y0 <= 16) {
            step = 3;
            keep = 0;
        }
    } else if (step == 3) {
        if (++keep > 16) {
            step = 0;   //a period over
            //eye_reset_flag = true;
            keep = 0;
        }
    }
    oled_update_screen();
    if (step == 0) {
        return 0;
    }
    return -1;
}
