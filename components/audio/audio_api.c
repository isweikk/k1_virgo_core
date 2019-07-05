#include "audio_api.h"

void audio_init(i2s_mode_t mode, i2s_bits_per_sample_t bps)
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | mode,
        .sample_rate = DEFAULT_SAMPLE_RATE,
        .bits_per_sample = bps,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, //2-channels
        .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 16,  // number of buffers, 128 max
        .dma_buf_len = 64,  // size of each buffer
        .use_apll = false
    };
    i2s_pin_config_t pin_config;
    pin_config.bck_io_num = PIN_I2S_BCLK;
    pin_config.ws_io_num = PIN_I2S_LRC;
    if (mode == I2S_MODE_RX) {
        pin_config.data_out_num = I2S_PIN_NO_CHANGE;
        pin_config.data_in_num = PIN_I2S_DIN;
    } else if (mode == I2S_MODE_TX) {
        pin_config.data_out_num = PIN_I2S_DOUT;
        pin_config.data_in_num = I2S_PIN_NO_CHANGE;
    }
    i2s_driver_install(USE_I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(USE_I2S_NUM, &pin_config);
    i2s_zero_dma_buffer(USE_I2S_NUM);
    i2s_start(USE_I2S_NUM);
}

int audio_read(char* data, int size)
{
    size_t tmp = 0;
    i2s_read(USE_I2S_NUM, (char *)data, size, &tmp, portMAX_DELAY);
    return tmp;
}

void audio_write(char* data, int size)
{
    size_t tmp = 0;
    i2s_write(USE_I2S_NUM, (const char *)data, size, &tmp, portMAX_DELAY);
}

// const int record_time = 60;  // second
// const char filename[] = "/sound.wav";

// const int headerSize = 44;
// const int waveDataSize = record_time * 88000;
// const int numCommunicationData = 8000;
// const int numPartWavData = numCommunicationData/4;
// byte header[headerSize];
// char communicationData[numCommunicationData];
// char partWavData[numPartWavData];
// File file;

// void setup() {
//   Serial.begin(115200);
//   if (!SD.begin()) Serial.println("SD begin failed");
//   CreateWavHeader(header, waveDataSize);
//   SD.remove(filename);
//   file = SD.open(filename, FILE_WRITE);
//   if (!file) return;
//   file.write(header, headerSize);
//   audio_init(I2S_MODE_RX, I2S_BITS_PER_SAMPLE_32BIT);
//   for (int j = 0; j < waveDataSize/numPartWavData; ++j) {
//     audio_read(communicationData, numCommunicationData);
//     for (int i = 0; i < numCommunicationData/8; ++i) {
//       partWavData[2*i] = communicationData[8*i + 2];
//       partWavData[2*i + 1] = communicationData[8*i + 3];
//     }
//     file.write((const byte*)partWavData, numPartWavData);
//   }
//   file.close();
//   Serial.println("finish");
// }

// void sound_i2s()
// {
// 	int counter=0;

//     i2s_config_t i2s_config = {
//         .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
//         .sample_rate = DEFAULT_SAMPLE_RATE,
//         .bits_per_sample = 16,                                                  //16-bit per channel
//         .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,                           //2-channels
//         .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_LSB,
//         .dma_buf_count = 6,
//         .dma_buf_len = 1024,                                                      //
//         .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1                                //Interrupt level 1
//     };
//     i2s_pin_config_t pin_config = {
//         .bck_io_num = 26,
//         .ws_io_num = 25,
//         .data_out_num = 27,
//         .data_in_num = -1                                                       //Not used
//     };

//     i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
//     i2s_set_pin(I2S_NUM, &pin_config);
    
//     while(counter < NUM_ELEMENTS)
//     {
//     	i2s_push_sample(I2S_NUM, (char *) &data[counter], portMAX_DELAY);
//     	counter += 2;
//     }

//     i2s_driver_uninstall(I2S_NUM);
    
// }