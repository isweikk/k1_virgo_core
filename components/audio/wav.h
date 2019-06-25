#include <Arduino.h>

// 16bit, monoral, 44100Hz,  linear PCM
void CreateWavHeader(char* header, int waveDataSize);  // size of header is 44
