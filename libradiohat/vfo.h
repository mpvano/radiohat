#ifndef LIBRADIOHAT_H
#define LIBRADIOHAT_H

#include <stdint.h>

//	The current production 25mhz crystals specify 8 pf crystal load
//	With this setting they  typically need around +20000 ppb correction here
//	much lower values (+-2000) can be achieved by using 10pf instead
#define CALIBRATION_FACTOR  21800	// for s/n 00110 at 8 pf

#define STARTFREQUENCY 7074000LL

//	use CLK2 on si5351 to drive the SGTL5000 master clock input
//	for 48Khz 32 bit duplex I2S, no PLL
#define SGTL5000_FREQ  12288000LL

#define cSI5351_I2C	0x60	//	default is for a 25Mhz crystal at 0x60h

volatile uint32_t gVFOA = STARTFREQUENCY;

extern "C"
{
void cachePLLDivisor(bool cacheIt);
long getVFO(void);
long setVFO(long freq);
void swapPhaseVFO(bool swap);
void updateTheVFO(void);
long getCalibrationFactor(char * path);
int initVFO(long correction, uint32_t startFreq, uint32_t codecfreq);

//extern int defaultVFO(void);		//	uses default values from this header
}
#endif
