/*
 * @Descripttion: general API of time.
 * @version: v1.0.0
 * @Author: Kevin
 * @Email: wkhome90@163.com
 * @Date: 2019-06-30 02:30:29
 * @LastEditors: Kevin
 * @LastEditTime: 2019-06-30 02:46:29
 */


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

uint64_t get_runtime_ms(void)
{
    uint64_t time_ms = xTaskGetTickCount() * portTICK_RATE_MS;
    return time_ms;
}

void delay_ms(uint32_t ms)
{
    vTaskDelay(ms / portTICK_RATE_MS);
}