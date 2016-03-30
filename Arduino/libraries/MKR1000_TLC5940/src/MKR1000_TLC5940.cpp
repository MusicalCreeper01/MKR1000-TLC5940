#define XLAT_PIN 2
#define GSCLK_PIN 3
#define BLANK_PIN 4

#define SIN_PIN 8
#define SCLK_PIN 9

#include "Arduino.h"
#include "analog_helpers.h"
#include <MKR1000_Interupts.h>
#include "MKR1000_TLC5940.h"

Interrupt inter;

//12 bits - 0 - 4059 per channel
uint16_t channelValues[192] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 15
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 14
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 13
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 12
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 11
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 10
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 09
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 08
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 07
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 06
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 05
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 04
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 03
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 02
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 01
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Channel 00
  };

String toBinary(int i, int bits) {
  String binary = String(i, BIN);
  String o = "";
  for (int x = 0; x < bits - binary.length(); ++x)
    o = "0" + o;
  return o + binary;
}

inline void digitalWriteDirect(int pin, boolean val){
  if(val) PORT->Group[g_APinDescription[pin].ulPort].OUTSET.reg = (1ul << g_APinDescription[pin].ulPin);
  else    PORT->Group[g_APinDescription[pin].ulPort].OUTCLR.reg = (1ul << g_APinDescription[pin].ulPin);

}

void TLC_init() {
  pinMode(GSCLK_PIN, OUTPUT);
  digitalWriteDirect(GSCLK_PIN, LOW);

  pinMode(SCLK_PIN, OUTPUT);
  digitalWriteDirect(SCLK_PIN, LOW);

  pinMode(XLAT_PIN, OUTPUT);
  digitalWriteDirect(XLAT_PIN, LOW);

  pinMode(BLANK_PIN, OUTPUT);
  digitalWriteDirect(BLANK_PIN, HIGH);

  pinMode(SIN_PIN, OUTPUT);
  digitalWriteDirect(SIN_PIN, LOW);

  pinMode(6, OUTPUT);

  digitalWriteDirect(XLAT_PIN, HIGH);
  digitalWriteDirect(XLAT_PIN, LOW);

  digitalWriteDirect(BLANK_PIN, LOW);

  analogWrite(GSCLK_PIN, 255);

  inter.enableTCC(TC5, 0);
  inter.attachTCCInterrupt(TLC_refresh);
}

bool gsFirstCycle = true;
bool newData = true;
bool needLatch = false;

void TLC_refresh(){
  //Serial.println("TLCLoop");
  digitalWriteDirect(BLANK_PIN, HIGH);
  if (needLatch) {
    needLatch = false;
    digitalWriteDirect(XLAT_PIN, HIGH);
    digitalWriteDirect(XLAT_PIN, LOW);
  }
  if (gsFirstCycle) {
    gsFirstCycle = false;
    digitalWriteDirect(SCLK_PIN, HIGH);
    digitalWriteDirect(SCLK_PIN, LOW);
  }

  digitalWriteDirect(BLANK_PIN, LOW);

  needLatch = TLC_cycle();
}

bool TLC_cycle (){
  if (newData) {
    newData = false;
    for (uint16_t Data_Counter=0; Data_Counter<192; ++Data_Counter) {
        
        if (channelValues[Data_Counter]){
          digitalWriteDirect(SIN_PIN, HIGH);
        }else{
          digitalWriteDirect(SIN_PIN, LOW);
        }
        
        digitalWriteDirect(SCLK_PIN, HIGH);
        digitalWriteDirect(SCLK_PIN, LOW);
    }
    return true;
  }
  return false;
}

void TLC_UpdateLEDs(){
    newData = true;
}

void TLC_SetLED(int channel, uint16_t value){
    channel = 15 - channel;
    value = 4095 - value;

    String binary = toBinary(value, 12);
    
    char chars[12] = {' '};
    binary.toCharArray(chars, 12);
    int x = 0;
    for(char c : chars){
      int i = c - '0';
      channelValues[(12*channel)+x] = i;
      ++x;
    }
    newData = true;
}

void TLC_SetChannel(int channel, uint16_t r, uint16_t g, uint16_t b) {

	channel = channel * 3;

	uint16_t channelr = channel;
	uint16_t channelg = channelr + 1;
	uint16_t channelb = channelg + 1;

	TLC_SetLED(channelr, r);
	TLC_SetLED(channelg, g);
	TLC_SetLED(channelb, b);

	newData = true;
}

void TLC_SetAllChannels(uint16_t r, uint16_t g, uint16_t b) {
	for (int i = 0; i < 5; ++i)
		TLC_SetChannel(i, r, g, b);
}

void TLC_SetAll(uint16_t r, uint16_t g, uint16_t b) {
	
}

void TLC_Clear() {

	for (int i = 0; i < 192; ++i) {
		channelValues[i] = 1;
	}

	newData = true;
}

void fade (int channel){
  for (int fadeValue = 0 ; fadeValue <= 4095; fadeValue += 5) {
    TLC_SetLED(channel, fadeValue);
    delay(1);
  }

  for (int fadeValue = 4095 ; fadeValue >= 0; fadeValue -= 5) {
    TLC_SetLED(channel, fadeValue);
    delay(1);
  }
}

void fade2(int channel) {
	for (int fadeValue = 0; fadeValue <= 4095; fadeValue += 5) {
		TLC_SetLED(channel, fadeValue);
		delayMicroseconds(1);
	}

	for (int fadeValue = 4095; fadeValue >= 0; fadeValue -= 5) {
		TLC_SetLED(channel, fadeValue);
		delayMicroseconds(1);
	}
}


