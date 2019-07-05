/*
 * @Descripttion: the eyes image, draw them by algorithm
 * @version: v1.0.0
 * @Author: Kevin
 * @Email: wkhome90@163.com
 * @Date: 2019-07-05 11:49:20
 * @LastEditors: Kevin
 * @LastEditTime: 2019-07-05 17:44:13
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
static uint8_t eye_reset = true;
static int eye_emotion = 0;

/**
 * @brief: 
 * @param {type} 
 * @return: 
 */
void eye_reset(void)
{
    eye_reset = true;
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
    static uint16_t x0, y0, x1,y1;
    static uint8_t step;

    if (eye_reset) {
        x0 = 8;
        y0 = 32;
        x1 = 56;
        y1 = 32;
        step = 0;
        oled_clear();
        eye_reset = false;
    }

    if (step == 0) {    //clear
        oled_clear();
        step = 1;
    } else if (step == 1) {
        oled_show_char_align8(32, 48, 'Z', Font_6x8);
        step = 2;
    } else if (step == 2) {
        oled_show_char_align8(48, 32, 'Z', Font_8x16);
        step = 3;
    } else if (step == 3) {
        oled_show_char_align8(64, 8, 'Z', Font_12x24);
        step = 4;
    } else if (step == 4) {
        step = 0;   //a period over
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

    if (eye_reset) {
        x0 = 0;
        y0 = 32;
        step = 0;
        delay = 0;
        oled_clear();
        eye_reset = false;
    }

    if (step == 0) {
        oled_fill_chunk(x0, y0 - 8, x0 + 16, y0 + 8, SSD1306_COLOR_WHITE);
        step = 1;
    } else if (step == 1) {
        if (++delay >= 8) {
            delay = 0;
            oled_fill_chunk(x0, y0 - 8, x0 + 16, y0 + 8, SSD1306_COLOR_BLACK);
            x0 += 32;
            oled_fill_chunk(x0, y0 - 8, x0 + 16, y0 + 8, SSD1306_COLOR_WHITE);
        }
        if (x0 >= 64) {
            step = 2;
        }
    } else if (step == 2) {
        ++delay;
        if (delay >= 12) {
            delay = 0;
            step = 3;
        } else if (delay >= 4) {
            oled_fill_chunk(48, 16, 80, 48, SSD1306_COLOR_WHITE);
        }
    } else if (step == 3) {
        step = 0;   //a period over
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
    static uint8_t step;

    if (eye_reset) {
        x0 = 8;
        y0 = 32;
        x1 = 56;
        y1 = 32;
        step = 0;
        oled_clear();
        eye_reset = false;
    }

    if (step == 0) {    //open eyes
        y0 -= 2;
        y1 += 2;
        //left eye
        oled_fill_chunk(x0, y0, x1, y0+2, SSD1306_COLOR_WHITE);
        oled_fill_chunk(x0, y1-2, x1, y1, SSD1306_COLOR_WHITE);
        //right eye
        oled_fill_chunk(x0 + 64, y0, x1 + 64, y0+2, SSD1306_COLOR_WHITE);
        oled_fill_chunk(x0 + 64, y1-2, x1 + 64, y1, SSD1306_COLOR_WHITE);
        if (y0 == 16) {
            step = 1;
        }
    } else if (step == 1) {
        step = 2;   //wait a period
    } else if (step == 2) {
        y0 += 2;
        y1 -= 2;
        oled_fill_chunk(x0, y0-2, x1, y0, SSD1306_COLOR_BLACK);
        oled_fill_chunk(x0, y1, x1, y1+2, SSD1306_COLOR_BLACK);
        oled_fill_chunk(x0 + 64, y0-2, x1 + 64, y0, SSD1306_COLOR_BLACK);
        oled_fill_chunk(x0 + 64, y1, x1 + 64, y1+2, SSD1306_COLOR_BLACK);
        if (y0 == y1) {
            step = 3;
        }
    } else if (step == 3) {
        step = 0;   //a period over
    }
    oled_update_screen();
    if (step == 0) {
        return 0;
    }
    return -1;
}
