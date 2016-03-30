/*
  Copyright (c) 2014 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Arduino.h"
#include "wiring_private.h"
#include "analog_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

static int _readResolution = 10;
static int _ADCResolution = 10;
static int _writeResolution = 8;

// Wait for synchronization of registers between the clock domains
static __inline__ void syncADC() __attribute__((always_inline, unused));
static void syncADC() {
  while (ADC->STATUS.bit.SYNCBUSY == 1)
    ;
}

// Wait for synchronization of registers between the clock domains
static __inline__ void syncDAC() __attribute__((always_inline, unused));
static void syncDAC() {
  while (DAC->STATUS.bit.SYNCBUSY == 1)
    ;
}

// Wait for synchronization of registers between the clock domains
static __inline__ void syncTC_8(Tc* TCx) __attribute__((always_inline, unused));
static void syncTC_8(Tc* TCx) {
  while (TCx->COUNT8.STATUS.bit.SYNCBUSY);
}

// Wait for synchronization of registers between the clock domains
static __inline__ void syncTCC(Tcc* TCCx) __attribute__((always_inline, unused));
static void syncTCC(Tcc* TCCx) {
  while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK);
}

static inline uint32_t mapResolution( uint32_t value, uint32_t from, uint32_t to )
{
  if ( from == to )
  {
    return value ;
  }

  if ( from > to )
  {
    return value >> (from-to) ;
  }
  else
  {
    return value << (to-from) ;
  }
}

// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
void analogWritePrescale( uint32_t ulPin, uint32_t ulValue, uint32_t prescale)
{
  uint32_t attr = g_APinDescription[ulPin].ulPinAttribute ;

  if ( (attr & PIN_ATTR_ANALOG) == PIN_ATTR_ANALOG )
  {
    if ( ulPin != PIN_A0 )  // Only 1 DAC on A0 (PA02)
    {
      return;
    }

    ulValue = mapResolution(ulValue, _writeResolution, 10);

    syncDAC();
    DAC->DATA.reg = ulValue & 0x3FF;  // DAC on 10 bits.
    syncDAC();
    DAC->CTRLA.bit.ENABLE = 0x01;     // Enable DAC
    syncDAC();
    return ;
  }

  if ( (attr & PIN_ATTR_PWM) == PIN_ATTR_PWM )
  {
    if (attr & PIN_ATTR_TIMER) {
      #if !(ARDUINO_SAMD_VARIANT_COMPLIANCE >= 10603)
      // Compatibility for cores based on SAMD core <=1.6.2
      if (g_APinDescription[ulPin].ulPinType == PIO_TIMER_ALT) {
        pinPeripheral(ulPin, PIO_TIMER_ALT);
      } else
      #endif
      {
        pinPeripheral(ulPin, PIO_TIMER);
      }
    } else {
      // We suppose that attr has PIN_ATTR_TIMER_ALT bit set...
      pinPeripheral(ulPin, PIO_TIMER_ALT);
    }

    Tc*  TCx  = 0 ;
    Tcc* TCCx = 0 ;
    uint8_t Channelx = GetTCChannelNumber( g_APinDescription[ulPin].ulPWMChannel ) ;
    if ( GetTCNumber( g_APinDescription[ulPin].ulPWMChannel ) >= TCC_INST_NUM )
    {
      TCx = (Tc*) GetTC( g_APinDescription[ulPin].ulPWMChannel ) ;
      //Serial.println("TC: " + g_APinDescription[ulPin].ulPWMChannel);
    }
    else
    {
      TCCx = (Tcc*) GetTC( g_APinDescription[ulPin].ulPWMChannel ) ;
      //Serial.println("TCC: " + g_APinDescription[ulPin].ulPWMChannel);
    }

    // Enable clocks according to TCCx instance to use
    switch ( GetTCNumber( g_APinDescription[ulPin].ulPWMChannel ) )
    {
      case 0: // TCC0
      case 1: // TCC1
        // Enable GCLK for TCC0 and TCC1 (timer counter input clock)
        GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID( GCM_TCC0_TCC1 )) ;
      break ;

      case 2: // TCC2
      case 3: // TC3
        // Enable GCLK for TCC2 and TC3 (timer counter input clock)
        GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID( GCM_TCC2_TC3 )) ;
      break ;

      case 4: // TC4
      case 5: // TC5
        // Enable GCLK for TC4 and TC5 (timer counter input clock)
        GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID( GCM_TC4_TC5 ));
      break ;

      case 6: // TC6 (not available on Zero)
      case 7: // TC7 (not available on Zero)
        // Enable GCLK for TC6 and TC7 (timer counter input clock)
        GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID( GCM_TC6_TC7 ));
      break ;
    }

    while ( GCLK->STATUS.bit.SYNCBUSY == 1 ) ;

    ulValue = mapResolution(ulValue, _writeResolution, 8);

    // Set PORT
    if ( TCx )
    {
      Serial.println("TC");
      // -- Configure TC
    
      // Disable TCx
      TCx->COUNT8.CTRLA.reg &= ~TC_CTRLA_ENABLE;
      syncTC_8(TCx);
      // Set Timer counter Mode to 8 bits
      TCx->COUNT8.CTRLA.reg |= TC_CTRLA_MODE_COUNT8;
      // Set TCx as normal PWM
      TCx->COUNT8.CTRLA.reg |= TC_CTRLA_WAVEGEN_NPWM;

      switch (prescale){
        case DIV1:
          TCx->COUNT8.CTRLA.reg |= ( (0 << 8) | (0 << 9) | (0 << 10));
        break;
        case DIV2:
          TCx->COUNT8.CTRLA.reg |= ( (0 << 8) | (0 << 9) | (1 << 10));
        break;
        case DIV4:
          TCx->COUNT8.CTRLA.reg |= ( (0 << 8) | (1 << 9) | (0 << 10));
        break;
        case DIV8:
          TCx->COUNT8.CTRLA.reg |= ( (0 << 8) | (1 << 9) | (1 << 10));
        break;
        case DIV16:
          TCx->COUNT8.CTRLA.reg |= ( (1 << 8) | (0 << 9) | (0 << 10));
        break;
        case DIV64:
          TCx->COUNT8.CTRLA.reg |= ( (1 << 8) | (0 << 9) | (1 << 10));
        break;
        case DIV256:
          TCx->COUNT8.CTRLA.reg |= ( (1 << 8) | (1 << 9) | (0 << 10));
        break;
        case DIV1024:
          TCx->COUNT8.CTRLA.reg |= ( (1 << 8) | (1 << 9) | (1 << 10));
        break;
        default:
          TCCx->CTRLA.reg |= ( (0 << 8) | (0 << 9) | (0 << 10));
        break;
      }
      
      // Set TCx in waveform mode Normal PWM
      TCx->COUNT8.CC[Channelx].reg = (uint8_t) ulValue;
      syncTC_8(TCx);
      // Set PER to maximum counter value (resolution : 0xFF)
      TCx->COUNT8.PER.reg = 0xFF;
      syncTC_8(TCx);
      // Enable TCx
      TCx->COUNT8.CTRLA.reg |= TC_CTRLA_ENABLE;
      syncTC_8(TCx);
    }
    else
    {
      Serial.println("TCC");
      Serial.println("Before changes: " + String(TCCx->CTRLA.reg, BIN));
      
      // -- Configure TCC
      // Disable TCCx
      TCCx->CTRLA.reg &= ~TCC_CTRLA_ENABLE;
      Serial.println("Disabled: " + String(TCCx->CTRLA.reg, BIN));
      syncTCC(TCCx);
      // Set TCx as normal PWM
      TCCx->WAVE.reg |= TCC_WAVE_WAVEGEN_NPWM;
      syncTCC(TCCx);
      // Set TCx in waveform mode Normal PWM
      TCCx->CC[Channelx].reg = (uint32_t)ulValue;
      syncTCC(TCCx);
      // Set PER to maximum counter value (resolution : 0xFF)
      TCCx->PER.reg = 0xFF;
      syncTCC(TCCx);
      //Set Prescaler
      //TCCx->CTRLA.reg |= ( (1 << 8) | (1 << 9) | (1 << 10));
      switch (prescale){
        case DIV1:
          TCCx->CTRLA.reg |= ( (0 << 8) | (0 << 9) | (0 << 10));
        break;
        case DIV2:
          TCCx->CTRLA.reg |= ( (0 << 8) | (0 << 9) | (1 << 10));
        break;
        case DIV4:
          TCCx->CTRLA.reg |= ( (0 << 8) | (1 << 9) | (0 << 10));
        break;
        case DIV8:
          TCCx->CTRLA.reg |= ( (0 << 8) | (1 << 9) | (1 << 10));
        break;
        case DIV16:
          TCCx->CTRLA.reg |= ( (1 << 8) | (0 << 9) | (0 << 10));
        break;
        case DIV64:
          TCCx->CTRLA.reg |= ( (1 << 8) | (0 << 9) | (1 << 10));
        break;
        case DIV256:
          TCCx->CTRLA.reg |= ( (1 << 8) | (1 << 9) | (0 << 10));
        break;
        case DIV1024:
          TCCx->CTRLA.reg |= ( (1 << 8) | (1 << 9) | (1 << 10));
        break;
        default:
          TCCx->CTRLA.reg |= ( (0 << 8) | (0 << 9) | (0 << 10));
        break;
      }

      Serial.println("After prescale: " + String(TCCx->CTRLA.reg, BIN));

    //RCT has ctc MATCHCTL interupt
      //TCCx->CTRL |= (1 << 7);

     // Serial.println("After CTC: " + String(TCCx->CTRL.reg, BIN));
      
      // Enable TCCx
      TCCx->CTRLA.reg |= TCC_CTRLA_ENABLE ;
      syncTCC(TCCx);

      Serial.println("After enable: " + String(TCCx->CTRLA.reg, BIN));
    }

    return ;
  }

  // -- Defaults to digital write
  pinMode( ulPin, OUTPUT ) ;
  ulValue = mapResolution(ulValue, _writeResolution, 8);
  if ( ulValue < 128 )
  {
    digitalWrite( ulPin, LOW ) ;
  }
  else
  {
    digitalWrite( ulPin, HIGH ) ;
  }
}

#ifdef __cplusplus
}
#endif

