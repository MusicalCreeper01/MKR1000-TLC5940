#ifndef TLC_5940_H
#define TLC_5940_H

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


void TLC_init();
void TLC_refresh();
void TLC_SetLED(int channel, uint16_t value);
void fade (int channel); 
void fade2(int channel);
void TLC_UpdateLEDs();
bool TLC_cycle();
void TLC_SetChannel(int channel, uint16_t r, uint16_t g, uint16_t b);
void TLC_SetAllChannels(uint16_t r, uint16_t g, uint16_t b);
void TLC_Clear();



#ifdef __cplusplus
}
#endif

#endif // RTC_ZERO_H
