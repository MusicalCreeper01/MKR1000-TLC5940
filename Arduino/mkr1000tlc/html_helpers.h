
#ifndef HTML_HELPERS_H
#define HTML_HELPERS_H

#pragma once

#ifdef __cplusplus

#include "Arduino.h"

void WifiInit(int port);
bool Listen();

String* LastData();
int LastDataSize();

String* parseHTML(String s);

String* StringSplit(char* str, char* split);

int countChars( char* s, char c );

#endif

#endif



