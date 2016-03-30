
#include "Arduino.h"
#include "MKR1000_Interupts.h"

Tc* THE_TC = TC5;
IRQn_Type THE_TC_IRQn = TC5_IRQn;
int THE_TC_CHANNEL = 0;

static voidFuncPtr TC3_callBack = NULL;
static voidFuncPtr TC4_callBack = NULL;
static voidFuncPtr TC5_callBack = NULL;

//Different Handlers will have to be called depending on the Timer/Counter used
/*
void TCC0_Handler (void) __attribute__ ((weak, alias("Timer_Handler")));
void TCC1_Handler (void) __attribute__ ((weak, alias("Timer_Handler")));
void TCC2_Handler (void) __attribute__ ((weak, alias("Timer_Handler")));

void TC3_Handler (void) __attribute__ ((weak, alias("Timer_Handler")));
void TC4_Handler (void) __attribute__ ((weak, alias("Timer_Handler")));
void TC5_Handler(void) __attribute__((weak, alias("Timer_Handler")));
*/


static void GCLKSync(){
  if(GCLK->STATUS.bit.SYNCBUSY == 1){
    //Serial.println("----------GCLK----------");
    //Serial.println("waiting for gclk to sync...");
    while ( GCLK->STATUS.bit.SYNCBUSY == 1 ) ;
    //Serial.println("GCLK synced!");
    //Serial.println("------------------------");
  }
}


static void TCCSync(Tc* tc){
  //Serial.println("Checking TCC sync...");
  if(tc->COUNT16.STATUS.bit.SYNCBUSY  == 1){
    ////Serial.println("----------TCC----------");
    //Serial.println("waiting for TCC Enable to sync...");
    while ( tc->COUNT16.STATUS.bit.SYNCBUSY == 1 ) ;
    //Serial.println("TCC synced!");
    //Serial.println("-----------------------");
  }
}

void Interrupt::enableTCC(Tc* tc, int channel){
	

  THE_TC = tc;
  THE_TC_CHANNEL = channel;

  if (THE_TC == TC3) {
	  THE_TC_IRQn = TC3_IRQn;
	  //Serial.println("Enabling TC3");
  }
  else if (THE_TC == TC4) {
	  THE_TC_IRQn = TC4_IRQn;
	  //Serial.println("Enabling TC4");
  }
  else if (THE_TC == TC5) {
	  THE_TC_IRQn = TC5_IRQn;
	  //Serial.println("Enabling TC5");
  }
  else
	  return;

  //Serial.println("Configuring interrupt request");
  // Configure interrupt request
  NVIC_DisableIRQ(THE_TC_IRQn);
  NVIC_ClearPendingIRQ(THE_TC_IRQn);

  NVIC_SetPriority(THE_TC_IRQn, 0);

  //Serial.println("Enabling GCLK");

  //Serial.println(GCLK->CLKCTRL.reg, BIN);

  //GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3) | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4));

/*  if (THE_TC == TC3) {
	  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3) );
  }
  else if (THE_TC == TC4 || THE_TC == TC5) {
	  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5));
  }*/

  //GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5));
 /* GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3));
  GCLKSync();
  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5));*/

  // Enable clocks according to TCCx instance to use
  if (THE_TC == TC3) {
	  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3));
  }
  else if (THE_TC == TC4 || THE_TC == TC5) {
	  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5));
  }


  GCLKSync();

  //Serial.println(GCLK->CLKCTRL.reg, BIN);

  // Disable TCx
  THE_TC->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  TCCSync(THE_TC);

  // Reset TCx
  THE_TC->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  TCCSync(THE_TC);
  while (THE_TC->COUNT16.CTRLA.bit.SWRST);

  //Serial.println("Set register");
  uint16_t tmpReg = 0;
  tmpReg |= TC_CTRLA_MODE_COUNT16;  // Set Timer counter Mode to 16 bits
  tmpReg |= TC_CTRLA_WAVEGEN_MFRQ;  // Set TONE_TC mode as match frequency
  tmpReg |= TC_CTRLA_PRESCALER_DIV1024; // set the prescaler
  THE_TC->COUNT16.CTRLA.reg |= tmpReg;
  TCCSync(THE_TC);
  
  //Serial.println("Set value");
  THE_TC->COUNT16.CC[THE_TC_CHANNEL].reg = (uint16_t) 255;
  TCCSync(THE_TC);
  
  //Serial.println("Enabling interrupt");
  // Enable the TONE_TC interrupt request
  THE_TC->COUNT16.INTENSET.bit.MC0 = 1;
  TCCSync(THE_TC);
  
  //Serial.println("Enabling TC");
  // Enable TONE_TC
  THE_TC->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
  TCCSync(THE_TC);
  //Serial.println("TC Enabled");

  NVIC_EnableIRQ(THE_TC_IRQn);

  
}

void Interrupt::attachTCCInterrupt(voidFuncPtr callback){
  //TCC_callBack = callback;

  if (THE_TC == TC3) {
	  TC3_callBack = callback;
  }
  if (THE_TC == TC4) {
	  TC4_callBack = callback;
  }
  if (THE_TC == TC5) {
	  TC5_callBack = callback;
  }
}

void Interrupt::detachTCCInterrupt(){
  //TCC_callBack = NULL;
}

void TC3_Handler() {
	//Serial.println("tc3");
	if (TC3_callBack != NULL) {
		TC3_callBack();
	}
	TC3->COUNT16.INTFLAG.bit.MC0 = 1;
}

void TC4_Handler() {
	//Serial.println("tc4");
	if (TC4_callBack != NULL) {
		TC4_callBack();
	}
	TC4->COUNT16.INTFLAG.bit.MC0 = 1;
}
void TC5_Handler() {
	//Serial.println("tc5");
	if (TC5_callBack != NULL) {
		TC5_callBack();
	}
	TC5->COUNT16.INTFLAG.bit.MC0 = 1;
}

