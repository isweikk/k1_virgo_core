
#ifndef _AUDIO_API_H_
#define _AUDIO_API_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_system.h"

#define USE_I2S_NUM  I2S_NUM_1

#define PIN_I2S_BCLK 14  //sck26
#define PIN_I2S_LRC 2  //ws22
#define PIN_I2S_DIN 13  //sd34
#define PIN_I2S_DOUT 12  //25


#define DEFAULT_SAMPLE_RATE (16000)
// This I2S specification : 
//  -   LRC high is channel 2 (right).
//  -   LRC signal transitions once each word.
//  -   DATA is valid on the CLOCK rising edge.
//  -   Data bits are MSB first.
//  -   DATA bits are left-aligned with respect to LRC edge.
//  -   DATA bits are right-shifted by one with respect to LRC edges.
//        It's valid for ADMP441 (microphone) and MAX98357A (speaker). 
//        It's not valid for SPH0645LM4H(microphone) and WM8960(microphon/speaker).
//
//  -   44100Hz
//  -   stereo

/// @parameter MODE : I2S_MODE_RX or I2S_MODE_TX
/// @parameter BPS : I2S_BITS_PER_SAMPLE_16BIT or I2S_BITS_PER_SAMPLE_32BIT
void audio_init(i2s_mode_t MODE, i2s_bits_per_sample_t BPS);

/// audio_read() for I2S_MODE_RX
/// @parameter data: pointer to buffer
/// @parameter numData: buffer size
/// @return Number of bytes read
int audio_read(char* data, int numData);

/// audio_write() for I2S_MODE_TX
/// @param data: pointer to buffer
/// @param numData: buffer size
void audio_write(char* data, int numData);

#endif /*_AUDIO_API_H_ */