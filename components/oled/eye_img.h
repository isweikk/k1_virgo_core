/*
 * @Descripttion: the eyes image, draw them by algorithm
 * @version: v1.0.0
 * @Author: Kevin
 * @Email: wkhome90@163.com
 * @Date: 2019-07-05 11:49:20
 * @LastEditors: Kevin
 * @LastEditTime: 2019-07-06 00:38:04
 */

#ifndef _EYE_IMG_H_
#define _EYE_IMG_H_

#include <stdio.h>

enum EyeEmotion{
	EmEmotionSleep = 0,
	EmEmotionWakeUp,
	EmEmotionNictation,
};

void eye_reset(void);
int eye_get_emotion(void);
void eye_set_emotion(int emotion);
int eye_show_sleep(void);
int eye_show_wakeup(void);
int eye_show_nictation(void);

#endif

