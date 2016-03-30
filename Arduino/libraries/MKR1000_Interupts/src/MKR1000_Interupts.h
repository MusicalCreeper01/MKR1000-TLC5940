#ifndef MKR1000_INTERUPTS_H
#define MKR1000_INTERUPTS_H

#pragma once

#ifdef __cplusplus

#include "Arduino.h"



class Interrupt {
	//voidFuncPtr TCC_callBack = NULL;
public: 
	void enableTCC(Tc* tc, int ch);
	void attachTCCInterrupt(voidFuncPtr callback);
	void detachTCCInterrupt();
private:
	void Timer_Handler(void);
};

static void TCCSync(Tc* tc);

#endif

#endif