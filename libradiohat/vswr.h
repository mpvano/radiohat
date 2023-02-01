#ifndef __VSWR_H__
#define __VSWR_H__

//	simple ads1115 ADC io support for VSWR on two inputs
//	refer to source file for more details

extern "C"
{
int initVSWR(void);		//	true if succeeds

//	returns VSWR numerator and also updates power in milliwatts to passed variable
//	This reading needs some smoothing, however
float readVSWR(float * fwdPower);

float readForwardOnly(void);
float readVSWROnly(void);

//	gain choices are 0-7 and channels 0-3 (single ended only)
//	There's some indication you need to discard the first value read!
float readADCRaw(int channel, int gain);

}

#endif