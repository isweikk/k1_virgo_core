
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//platform support
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_http_client.h"
//user
#include "utl_time.h"
#include "sd_card.h"
#include "flash_opt.h"
#include "server_api.h"
#include "ota.h"
#include "general_dev.h"
#include "lcd.h"
#include "font.h"
#include "oled.h"
#include "fonts.h"
// #include "touch.h"
#include "audio_api.h"
#include "eye_img.h"

static void display_task(void *prm);
static esp_err_t event_handler(void *ctx, system_event_t *event);
static void wifi_init_sta(void);
static void wifi_init_softap(void);
static void wifi_task(void);

static const char* TAG = "main";

EventGroupHandle_t s_wifi_event_group;
static const int CONNECTED_BIT = BIT0;
static ip4_addr_t s_ip_addr;

void monitor_task(void *prm)
{
    uint32_t free_heap = 0;
    while(1) {
        if (free_heap != xPortGetFreeHeapSize()) {
            free_heap = xPortGetFreeHeapSize();
            ESP_LOGI(TAG, "Free heap: %u", free_heap);
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void test_audio(void)
{
    char buf[1024];
    int len = 0, tmp=0;
    ESP_LOGI(TAG, "Opening file");
    FILE* f = fopen("/spiffs/music-44100-2ch.wav", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fread(buf, 1, 64, f);
    ESP_LOGD(TAG, "HEAD:%02x,%02x,%02x,%02x", buf[0], buf[1], buf[2], buf[3]);
    ESP_LOGD(TAG, "end:%02x,%02x,%02x,%02x", buf[36], buf[37], buf[38], buf[39]);
    fseek(f, 0x2c, SEEK_SET);
    i2s_start(USE_I2S_NUM);
    while((len = fread(buf, 1, 1024, f)) > 0) {
        if (tmp++ < 200) {
            continue;
        }
        //ESP_LOGE(TAG, "len=%d:,%02x,%02x", len, buf[0], buf[1]);
        audio_write(buf, len);

    }
    i2s_stop(USE_I2S_NUM);
    fclose(f);

    // extern const unsigned char upload_html_start[] asm("_binary_music-44100-2ch_wav_start");
    // extern const unsigned char upload_html_end[]   asm("_binary_music-44100-2ch_wav_end");
    // const size_t upload_html_size = (upload_html_end - upload_html_start);
    // ESP_LOGE(TAG, "len=%d", upload_html_size);
    // ESP_LOGD(TAG, "HEAD:%02x,%02x,%02x,%02x", upload_html_start[0], upload_html_start[1], upload_html_start[2], upload_html_start[3]);
    // ESP_LOGD(TAG, "end:%02x,%02x,%02x,%02x", upload_html_start[36], upload_html_start[37], upload_html_start[38], upload_html_start[39]);
    // ESP_LOGD(TAG, "start:%02x,%02x,%02x,%02x", upload_html_start[40], upload_html_start[41], upload_html_start[42], upload_html_start[43]);
    // audio_write(&upload_html_start[40], upload_html_size-40);
    ESP_LOGI(TAG, "test audio over");
}

void app_main()
{
    esp_log_level_set("wifi", ESP_LOG_WARN);
    esp_log_level_set("gpio", ESP_LOG_WARN);
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ESP_ERROR_CHECK( nvs_flash_init() );
    }
    //sc_card_init();
    if (storage_flash_init() < 0) {
        ESP_ERROR_CHECK(storage_flash_init());
    }
    //monitor
    xTaskCreate(monitor_task, "monitor_task", 2048, NULL, 10, NULL);
    //led
    led_init();
    //oled
    oled_init();
    err = xTaskCreate(display_task, "display_task", 2048, NULL, 10, NULL);
    if (err != pdPASS) {
        ESP_LOGE(TAG, "display_task create failed");
    }
    //audio
    audio_init(I2S_MODE_TX, I2S_BITS_PER_SAMPLE_16BIT);
    //wifi
    wifi_task();
    //web services
    if (start_server_core() == NULL) {
        ESP_LOGE(TAG, "Server start failed!");
    }

    //xTaskCreate(&ota_upgrade_task, "ota_upgrade_task", 8192, NULL, 5, NULL);

    //test
    test_audio();
    
    //TODO, USART task

    ESP_LOGI(TAG, "K1 is ready");

}

static void display_task(void *prm)
{
    int current_emotion = 0;

    eye_set_emotion(EmEmotionWakeUp);
    while(1) {
        delay_ms(125);  //8 fps
        current_emotion = eye_get_emotion();
        switch(current_emotion) {
            case EmEmotionSleep:
                eye_show_sleep();
                break;
            case EmEmotionWakeUp:
                if (eye_show_wakeup() == 0) {
                    eye_set_emotion(EmEmotionNictation);
                    //eye_set_emotion(EmEmotionSleep);
                }
                break;
            case EmEmotionNictation:
                eye_show_nictation();
                break;
            default:
                break;
        }
        //ESP_LOGI(TAG, "display continue\n");
    } 
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
            s_ip_addr = event->event_info.got_ip.ip_info.ip;
            ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&s_ip_addr));
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
                    MAC2STR(event->event_info.sta_connected.mac),
                    event->event_info.sta_connected.aid);
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
                    MAC2STR(event->event_info.sta_disconnected.mac),
                    event->event_info.sta_disconnected.aid);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void wifi_init_softap(void)
{
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = CONFIG_WIFI_AP_SSID,
            .ssid_len = strlen(CONFIG_WIFI_AP_SSID),
            .password = CONFIG_WIFI_AP_PASSWORD,
            .max_connection = CONFIG_WIFI_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(CONFIG_WIFI_AP_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s",
             CONFIG_WIFI_AP_SSID, CONFIG_WIFI_AP_PASSWORD);
}

static void wifi_init_sta(void)
{
    tcpip_adapter_init();
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg) );
//    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_STA_SSID,
            .password = CONFIG_WIFI_STA_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_LOGI(TAG, "Connecting to \"%s\"", wifi_config.sta.ssid);
    xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected");
}

static void wifi_task(void)
{
    
#ifndef CONFIG_WIFI_MODE_STA
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
#else
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
#endif /*CONFIG_WIFI_MODE_STA*/
}

/**
 *  ota download
 */
/*
static void http_perform_as_stream_reader()
{
    char *buffer = malloc(MAX_HTTP_RECV_BUFFER + 1);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot malloc http receive buffer");
        return;
    }
    esp_http_client_config_t config = {
        .url = "http://httpbin.org/get",
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err;
    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        free(buffer);
        return;
    }
    int content_length =  esp_http_client_fetch_headers(client);
    int total_read_len = 0, read_len;
    if (total_read_len < content_length && content_length <= MAX_HTTP_RECV_BUFFER) {
        read_len = esp_http_client_read(client, buffer, content_length);
        if (read_len <= 0) {
            ESP_LOGE(TAG, "Error read data");
        }
        buffer[read_len] = 0;
        ESP_LOGD(TAG, "read_len = %d", read_len);
    }
    ESP_LOGI(TAG, "HTTP Stream reader Status = %d, content_length = %d",
                    esp_http_client_get_status_code(client),
                    esp_http_client_get_content_length(client));
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    free(buffer);
}
*/
