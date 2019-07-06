/*
 * @Descripttion: font library
 * @version: v1.0.0
 * @Author: Kevin
 * @Email: kkcoding@qq.com
 * @Date: 2019-06-24 21:37:48
 * @LastEditors: Kevin
 * @LastEditTime: 2019-07-05 03:00:20
 */

#ifndef _FONTS_H_
#define _FONTS_H_

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

#include <stdint.h>


/**
 * @brief  Font structure used on my LCD libraries
 */
typedef struct {
	uint8_t width;    /*!< Font width in pixels */
	uint8_t height;   /*!< Font height in pixels */
	const uint8_t *data;  /*!< Pointer to data font data array */
} FontDef_t;

/** 
 * @brief  String length and height 
 */
typedef struct {
	uint16_t length;      /*!< String length in units of pixels */
	uint16_t height;      /*!< String height in units of pixels */
} FONTS_SIZE_t;

/**
 * @brief  6 x 8 pixels font size structure 
 */
extern const FontDef_t Font_6x8;

/**
 * @brief  8 x 16 pixels font size structure 
 */
extern const FontDef_t Font_8x16;

/**
 * @brief  12 x 24 pixels font size structure 
 */
extern const FontDef_t Font_12x24;

/**
 * @brief  16 x 32 pixels font size structure 
 */
extern const FontDef_t Font_16x32;

// /**
//  * @brief  7 x 10 pixels font size structure 
//  */
// extern const FontDef_t Font_7x10;

// /**
//  * @brief  11 x 18 pixels font size structure 
//  */
// extern const FontDef_t Font_11x18;

// /**
//  * @brief  16 x 26 pixels font size structure 
//  */
// extern const FontDef_t Font_16x26;


/**
 * @brief  Calculates string length and height in units of pixels depending on string and font used
 * @param  *str: String to be checked for length and height
 * @param  *SizeStruct: Pointer to empty @ref FONTS_SIZE_t structure where informations will be saved
 * @param  Font: Pointer to @ref FontDef_t font used for calculations
 * @retval Pointer to string used for length and height
 */
char* FONTS_GetStringSize(char* str, FONTS_SIZE_t* SizeStruct, FontDef_t Font);

/* C++ detection */
#ifdef __cplusplus
}
#endif

 
#endif
