/*************************************** February 23, 2022 at 12:04:55 PM CST
*****************************************************************************
*
*	VFO for Zero IF Quadrature transceiver
*	using Si5351 for CLK0 and CLK1
*
*	started by Mario Vano AE0GL, February 2022
*
******************************************************************************
*****************************************************************************/
#include <math.h>
#include <unistd.h>

/****************************************************************************
*	si5351 definitions
****************************************************************************/
#include "si5351.h"
#undef SI5351_XTAL_FREQ
#define SI5351_XTAL_FREQ 25000000

//	This has to be done in the header file!
//#define SI5351_PLL_VCO_MIN 380000000

#include "vfo.h"

//	This is a useful default offset in most bands for my use
//	as it's the USB FT8 carrier frequency on 40,20,15 and 10 meters
//const int32_t INITIAL_FREQ = 7074000L;		// for 40m-10m FT8 USB
#ifdef WWV
const int32_t INITIAL_FREQ  = WWV * 1000000L;	//	useful during calibration
#else
const int32_t INITIAL_FREQ = 7074000L;			// for 40m-10m FT8 USB
#endif

Si5351 si5351(cSI5351_I2C);

bool gCacheDivisor = true;

//volatile uint32_t gVFOA = STARTFREQUENCY;

/****************************************************************************
*	VFO support functions
****************************************************************************/

long getVFO(void)
{
	return gVFOA;
}

long setVFO(long freq)
{
	gVFOA = freq;
	updateTheVFO();
	return getVFO();
}

void swapPhaseVFO(bool swap)
{
	si5351.set_clock_invert(SI5351_CLK0,swap);
}

//	detect whether the VFO PLL has stabilized after a frequency change
bool isVFOLocked()
{
	si5351.update_status();
	return !(	si5351.dev_status.SYS_INIT
			||  si5351.dev_status.LOL_A
			|| si5351.dev_status.LOL_B
			|| si5351.dev_status.LOS	);
}

//
//	Si5351 Quadrature Clock Output routines for VFO that work as low as 3MHz
//	Thanks to Brian Harper M1CEM and Miguel Bartié PY2OHH
//
//	Edit si5351.h file. Change the SI5351_PLL_VCO_MIN to 380000000, i.e.,
//	#define SI5351_PLL_VCO_MIN 380000000


//	selects the best divider/phase constant for the range
int getEvenDivisor(uint32_t freq)
{
	if (freq < 6850000) return 126;
	else if (freq < 9500000) return 89;
	else if (freq < 13600000) return 64;
	else if (freq < 17500000) return 45;
	else if (freq < 25000000) return 34;
	else if (freq < 36000000) return 24;
	else if (freq < 45000000) return 18;
	else if (freq < 60000000) return 14;
	else if (freq < 80000000) return 10;
	else if (freq < 100000000) return 8;
	else if (freq < 146600000) return 6;
	else if (freq < 220000000) return 4;
	else return 2;
}

//	The low level quadrature VFO tuning.
//	Resets pll ONLY when divisor changes)
void SendFrequency(uint32_t freq)
{
static int lastdivisor;
int evenDivisor = getEvenDivisor(freq);

	//	precalculate these
	uint64_t frq = freq * SI5351_FREQ_MULT;
	uint64_t div = evenDivisor * freq * SI5351_FREQ_MULT;

	gVFOA = freq;
	si5351.set_freq_manual(frq, div, SI5351_CLK0);
	si5351.set_freq_manual(frq, div, SI5351_CLK1);

	si5351.set_phase(SI5351_CLK0, 0);
	si5351.set_phase(SI5351_CLK1, evenDivisor);

	if (gCacheDivisor)					//	reset only if needed to minimize cogging
		{
		if (evenDivisor != lastdivisor)
			si5351.pll_reset(SI5351_PLLA);
		}
	else si5351.pll_reset(SI5351_PLLA);	//	else always reset it
	lastdivisor = evenDivisor;
}


//	The single point routine to update the vfo from globals
//	It returns early if the frequency has not changed.
//
//	uses global flags and vfo variables
//
//	the argument determines whether optional RIT is allowed or
//	is to be ignored when setting the frequency (e.g. on transmit)
void updateTheVFO(void)
{
uint32_t freqtemp = gVFOA;
static uint32_t lastFreqSent = 0;

	if (freqtemp != lastFreqSent)
		{
		lastFreqSent = freqtemp;
		SendFrequency(freqtemp);
		while (!isVFOLocked)
			;
		}
}

long getCalibrationFactor(char * thePath)
{
long calfactor = CALIBRATION_FACTOR;
char buffer[128] =""; // Buffer to store data
FILE * theFile = NULL;

	if ((theFile = fopen(thePath, "r")) != NULL)
		{
		int count = fread(&buffer, sizeof(char), 64, theFile);
		fclose(theFile);
		if (count)
			{
			long temp = strtol(buffer, NULL, 10);
			if ((temp < 100000) && (temp > -100000))
				calfactor = temp;
			}
  		}
  	return calfactor;
}

////////////////////////////////////////////////////////////////////////
//	initialize the Si5351 chip with args and settings from VFO_si5351.h
int initVFO(long correction, uint32_t startFreq, uint32_t codecfreq)
{
	//	load can be 0, 6, 8  or 10 pf
	si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351_XTAL_FREQ, correction);
	si5351.set_int(SI5351_CLK0, 1);			// try to set integer mode
	si5351.set_int(SI5351_CLK1, 1);			// try to set integer mode

//	Drive Levels may be 2,4,6 or 8 (ma)
	si5351.set_freq((startFreq * SI5351_FREQ_MULT), SI5351_CLK0);
	si5351.drive_strength(SI5351_CLK0,SI5351_DRIVE_4MA);
	si5351.output_enable(SI5351_CLK0, true);

	si5351.set_freq((startFreq * SI5351_FREQ_MULT), SI5351_CLK1);
	si5351.drive_strength(SI5351_CLK1,SI5351_DRIVE_4MA);
	si5351.output_enable(SI5351_CLK1, true);

//	Set this up to drive the SGTL5000 sound chip master clock as needed
	//	need to use the second PLL so it will not be disturbed by VFO
	if (codecfreq)
		{
		si5351.set_ms_source(SI5351_CLK2, SI5351_PLLB);
		si5351.set_freq((codecfreq * SI5351_FREQ_MULT), SI5351_CLK2);
		si5351.drive_strength(SI5351_CLK2,SI5351_DRIVE_2MA);
		si5351.output_enable(SI5351_CLK2, true);
		}

	gVFOA = startFreq;
	updateTheVFO();

	si5351.update_status();
	return !(	si5351.dev_status.SYS_INIT
			||  si5351.dev_status.LOL_A
			|| si5351.dev_status.LOL_B
			|| si5351.dev_status.LOS	);
}


void cachePLLDivisor(bool cacheIt)
{
	gCacheDivisor = cacheIt;
}

