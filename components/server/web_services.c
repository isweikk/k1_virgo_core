/*
 * @Descripttion: 
 * @version: v1.0.0
 * @Author: Kevin
 * @Email: kkcoding@qq.com
 * @Date: 2019-06-24 11:49:29
 * @LastEditors: Kevin
 * @LastEditTime: 2019-06-27 00:58:44
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//platform support
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
//user
#include "flash_opt.h"
#include "camera.h"
#include "my_http_server.h"
#include "server_html.h"
#include "bitmap.h"
#include "general_dev.h"
//#include "qr_recoginize.h"
// #include "lcd.h"
// #include "font.h"
// #include "touch.h"

static void handle_grayscale_pgm(httpd_req_t *req);
static void handle_rgb_bmp(httpd_req_t *req);
static void handle_rgb_bmp_stream(httpd_req_t *req);
static void handle_jpg(httpd_req_t *req);
static void handle_jpg_stream(httpd_req_t *req);

static const char* TAG = "web_server";

static const char* STREAM_CONTENT_TYPE =
        "multipart/x-mixed-replace; boundary=123456789000000000000987654321";

static const char* STREAM_BOUNDARY = "--123456789000000000000987654321";

//camera config
#define CAMERA_PIXEL_FORMAT CAMERA_PF_JPEG
#define CAMERA_FRAME_SIZE CAMERA_FS_VGA

static camera_config_t *camera_config = NULL;
static int camera_status = false;   //status of camera. 0=not actived, 1= actived.


/**
 * @brief: 
 * @param {type} 
 * @return: 
 */
int services_camera_init(void)
{
    camera_config = (camera_config_t*)malloc(sizeof(camera_config_t));
    if (!camera_config) {
        ESP_LOGE(TAG, "camera start failed!");
        return ESP_FAIL;
    }
    camera_config->ledc_channel = LEDC_CHANNEL_0;
    camera_config->ledc_timer = LEDC_TIMER_0;
    camera_config->pin_d0 = CONFIG_D0;
    camera_config->pin_d1 = CONFIG_D1;
    camera_config->pin_d2 = CONFIG_D2;
    camera_config->pin_d3 = CONFIG_D3;
    camera_config->pin_d4 = CONFIG_D4;
    camera_config->pin_d5 = CONFIG_D5;
    camera_config->pin_d6 = CONFIG_D6;
    camera_config->pin_d7 = CONFIG_D7;
    camera_config->pin_xclk = CONFIG_XCLK;
    camera_config->pin_pclk = CONFIG_PCLK;
    camera_config->pin_vsync = CONFIG_VSYNC;
    camera_config->pin_href = CONFIG_HREF;
    camera_config->pin_sscb_sda = CONFIG_SDA;
    camera_config->pin_sscb_scl = CONFIG_SCL;
    camera_config->pin_reset = CONFIG_RESET;
    camera_config->xclk_freq_hz = CONFIG_XCLK_FREQ;

    camera_model_t camera_model;
    esp_err_t err = camera_probe(camera_config, &camera_model);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera probe failed with error 0x%x", err);
        free(camera_config);
        return ESP_FAIL;
    }

    if (camera_model == CAMERA_OV7725) {
        camera_config->pixel_format = CAMERA_PIXEL_FORMAT;
        camera_config->frame_size = CAMERA_FRAME_SIZE;
        ESP_LOGI(TAG, "Detected OV7725 camera, using %s bitmap format",
            CAMERA_PIXEL_FORMAT == CAMERA_PF_GRAYSCALE ? "grayscale" : "RGB565");
    } else if (camera_model == CAMERA_OV2640) {
        camera_config->pixel_format = CAMERA_PIXEL_FORMAT;
        camera_config->frame_size = CAMERA_FRAME_SIZE;
        if (camera_config->pixel_format == CAMERA_PF_JPEG) {
            camera_config->jpeg_quality = 15;
        }
        ESP_LOGI(TAG, "Detected OV2640 camera, using JPEG format");
    } else {
        ESP_LOGE(TAG, "Camera not supported");
        free(camera_config);
        return ESP_FAIL;
    }

    err = camera_init(camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        free(camera_config);
        return ESP_FAIL;
    }
    camera_status = true;
    return ESP_OK;
}

int services_camera_deinit(void)
{
    camera_status = false;
    free(camera_config);
    return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t video_get_handler(httpd_req_t *req)
{
    int response_err = 0;

    if (!camera_status) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "camera is out line");
        return ESP_FAIL;
    }
    switch (camera_config->pixel_format) {
    case CAMERA_PF_GRAYSCALE: {
        if (strcmp(req->uri, "/video/pgm")) {
            handle_grayscale_pgm(req);
        } else {
            response_err = 1;
        }
        break;
    }
    case CAMERA_PF_RGB565: {
        if (strcmp(req->uri, "/video/bmp")) {
            handle_rgb_bmp(req);
        } else if (strcmp(req->uri, "/video/bmp_stream")) {
            handle_rgb_bmp_stream(req);
        } else {
            response_err = 1;
        }
        break;
    }
    case CAMERA_PF_JPEG: {
        if (strcmp(req->uri, "/video/jpg")) {
            handle_jpg(req);
        } else if (strcmp(req->uri, "/video/jpg_stream")) {
            handle_jpg_stream(req);
        } else {
            response_err = 1;
        }
        break;
    }
    default:
        break;
    }

    if (response_err) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "does not support");
    } else {
        // End response
        httpd_resp_send_chunk(req, NULL, 0);
    }
    return ESP_OK;
}

//

static esp_err_t login_handler(httpd_req_t *req)
{
    char resp_str[1024];

    strcpy(resp_str, loginIndex);
    size_t response_size = strlen(resp_str);
    httpd_resp_set_type(req, "text/html;charset=UTF-8");
    httpd_resp_send_chunk(req, resp_str, response_size);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t handle_homepage(httpd_req_t *req)
{
    char resp_str[1024];

    strcpy(resp_str, loginIndex);
    size_t response_size = strlen(resp_str);
    httpd_resp_set_type(req, "text/html;charset=UTF-8");
    httpd_resp_send_chunk(req, resp_str, response_size);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t ctrl_post_handler(httpd_req_t *req)
{
    //get the command
    char cmd_stat = 0;
    char param_str[256], key_val[64];

    if (httpd_req_get_url_query_str(req, param_str, 256) != ESP_OK) {
        //no arguments, return the ctrl page.
        httpd_resp_send_chunk(req, NULL, 0);
        return ESP_FAIL;
    }
    if (httpd_query_key_value(param_str, "led0", key_val, 64) == ESP_OK) {
        if(get_light_state()) {
            if (!strcmp(key_val, "on")) {
                led_open();
            } else {
                led_close();
            }
        }
        cmd_stat = 1;
    }
    char resp_str[64] = "do command: ";
    if (cmd_stat) {
        strcat(resp_str, "success!");
    } else {
        strcat(resp_str, "failed, you may input a wrong word.");
    }
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

int web_services_register(httpd_handle_t server)
{
    if (services_camera_init() == ESP_OK) {
        /* URI handler for uploading files to server */
        httpd_uri_t video_play = {
            .uri       = "/video/*",   // Match all URIs of type /video/{video-type}
            .method    = HTTP_GET,
            .handler   = video_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &video_play);
    } else {
        ESP_LOGW(TAG, "register web camera failed");
    }

    /* URI handler for controlling device */
    httpd_uri_t ctrl = {
        .uri       = "/ctrl",
        .method    = HTTP_POST,
        .handler   = ctrl_post_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &ctrl);

    /* URI handler for login the server */
    httpd_uri_t login = {
        .uri       = "/home",   
        .method    = HTTP_POST,
        .handler   = login_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &login);

    return ESP_OK;
}

int services_http_deinit(void)
{
    
    return ESP_OK;
}

static esp_err_t write_frame(httpd_req_t *req)
{
    char *data = (char*)camera_get_fb();
    size_t size = camera_get_data_size(); 

    return httpd_resp_send_chunk(req, data, size);
}

static void handle_grayscale_pgm(httpd_req_t *req)
{
    esp_err_t err = camera_run();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera capture failed with error = %d", err);
        return;
    }
    char* pgm_header_str;
    asprintf(&pgm_header_str, "P5 %d %d %d\n",
            camera_get_fb_width(), camera_get_fb_height(), 255);
    if (pgm_header_str == NULL) {
        return;
    }

    httpd_resp_set_type(req, "image/x-portable-graymap");
    httpd_resp_set_hdr(req, "Content-disposition", "inline; filename=capture.pgm");

    httpd_resp_send_chunk(req, pgm_header_str, strlen(pgm_header_str));
    write_frame(req);
    httpd_resp_send_chunk(req, NULL, 0);
#if CONFIG_QR_RECOGNIZE
    camera_config_t *camera_config = ctx;
    xTaskCreate(qr_recoginze, "qr_recoginze", 111500, camera_config, 5, NULL);
#endif
}

static void handle_rgb_bmp(httpd_req_t *req)
{
    esp_err_t err = camera_run();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera capture failed with error = %d", err);
        return;
    }

    bitmap_header_t* header = bmp_create_header(camera_get_fb_width(), camera_get_fb_height());
    if (header == NULL) {
        return;
    }

    httpd_resp_set_type(req, "image/bmp");
    httpd_resp_set_hdr(req, "Content-disposition", "inline; filename=capture.bmp");

    httpd_resp_send_chunk(req, header, sizeof(*header));
    write_frame(req);
    httpd_resp_send_chunk(req, NULL, 0);
}

static void handle_jpg(httpd_req_t *req)
{
	if(get_light_state())
		led_open();
    esp_err_t err = camera_run();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera capture failed with error = %d", err);
        return;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-disposition", "inline; filename=capture.jpeg");
    write_frame(req);
    httpd_resp_send_chunk(req, NULL, 0);
    led_close();
}


static void handle_rgb_bmp_stream(httpd_req_t *req)
{
    bitmap_header_t* bmp_header = bmp_create_header(camera_get_fb_width(), camera_get_fb_height());
    if (bmp_header == NULL) {
        return;
    }
    httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    while (true) {
        esp_err_t err = camera_run();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Camera capture failed with error = %d", err);
            return;
        }

        httpd_resp_set_type(req, "image/bitmap");
        //httpd_resp_set_hdr(req, "Content-disposition", "inline; filename=capture.bmp");

        httpd_resp_send_chunk(req, bmp_header, sizeof(*bmp_header));
        write_frame(req);
        httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
    }
    httpd_resp_send_chunk(req, NULL, 0);
}

static void handle_jpg_stream(httpd_req_t *req)
{
    if(get_light_state())
    	led_open();
    while (true) {
        esp_err_t err = camera_run();
        if (err != ESP_OK) {
            ESP_LOGD(TAG, "Camera capture failed with error = %d", err);
            return;
        }
        httpd_resp_set_type(req, "image/jpeg");
        //httpd_resp_set_hdr(req, "Content-disposition", "inline; filename=capture.bmp");

        write_frame(req);
        httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
    }
    httpd_resp_send_chunk(req, NULL, 0);
    led_close();
}

