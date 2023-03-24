#ifndef __CONTROL_H__
#define __CONTROL_H__

//#define USE_ALL_GPIO

#include <stdint.h>
#include "codec.h"


extern "C"
{
int initControl(void);
void uninitControl(void);

void checkLPF(uint32_t frequency, bool nocache);
int isKeyInputActive(void);
bool isTXRequested(void);
void enableTX(bool txon, cAudioMode mode);
void checkKeydown(void);
}

#endif
