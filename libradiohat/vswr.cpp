/*************************************** February 26, 2022 at 2:33:16 PM CST
*
*	ads1115 ADC io support for VSWR and other sensors
*
*	Hastily written to exterminate the wiringPI dependency
*	Hopefully it now only depends on standard kernel modules
*
*	by Mario P. Vano
****************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <linux/i2c-dev.h>

#include "vswr.h"

//	other jumper settings allow for 0x48-0x51 base address
#define	ADS_ADDR	0x48	

#define	ADS1115_POINTER_REG 		0

//	pointer register values
#define ADS1115_pConversion_p 	0x0000
#define ADS1115_Config_p 		0x0001
#define ADS1115_Lo_thresh_p		0x0002
#define ADS1115_Hi_thresh_p		0x0003

//	bitmasks for config register fields
#define	ADS1115_COMP_QUE0_bit	0x0001
#define	ADS1115_COMP_QUE1_bit	0x0002
#define	ADS1115_COMP_LAT_bit	0x0004
#define	ADS1115_COMP_POL_bit	0x0008
#define	ADS1115_COMP_MODE_bit	0x0010
#define	ADS1115_COMP_DR_bits	0x00E0
#define	ADS1115_MODE_bit		0x0100
#define	ADS1115_PGA_bits		0x0E00
#define	ADS1115_MUX_bits		0x7000
#define	ADS1115_OS_bit			0x8000

//	Useful for testing the os bit as seen in first byte
#define ADS1115_Donebit			0x80

////////////////
//	FIELD VALUES

//	+/- full scale gain ranges in mv
#define	ADS1115_GAIN_6144		0x0000
#define	ADS1115_GAIN_4096		0x1000
#define	ADS1115_GAIN_2048		0x2000
#define	ADS1115_GAIN_1024		0x3000
#define	ADS1115_GAIN_512		0x4000
#define	ADS1115_GAIN_256		0x5000

//	data rate values in Samples Per Second
#define	ADS1115_DR_8			0x0000
#define	ADS1115_DR_16			0x0020
#define	ADS1115_DR_32			0x0040
#define	ADS1115_DR_64			0x0060
#define	ADS1115_DR_128			0x0080
#define	ADS1115_DR_250			0x00A0
#define	ADS1115_DR_475			0x00C0
#define	ADS1115_DR_860			0x00E0

//	combinations of differential mode inputs
#define	ADS1115_MUX_Diff_0_1	0x0000
#define	ADS1115_MUX_Diff_0_3	0x1000
#define	ADS1115_MUX_Diff_1_3	0x2000
#define	ADS1115_MUX_Diff_2_3	0x3000

#define	ADS1115_MUX_Single0		0x4000
#define	ADS1115_MUX_Single1		0x5000
#define	ADS1115_MUX_Single2		0x6000
#define	ADS1115_MUX_Single3		0x7000

#define	ADS1115_MODE_Continuous	0x0000
#define	ADS1115_MODE_SingleShot	0x0100
#define ADS1115_START_Single	0x8000

//	prevent waiting for comparator to trigger the read operation!!!
#define ADS1115_MODE_NO_CMP		0x0003

//	how many microseconds to spin if you must wait around for a conversion
//	needs to be set for at lest one conversion time at current DR setting
#define ADS1115_DELAY			1000

// bit weight vs gain choices for later calculations
const float cLSBSIZE_TABLE[] =
	{ 0.0001875, 0.0001250, 0.0000625, 0.00003125, 0.000015625, 0.0000078125 };

//	translates 0 based single ended channel numbers into the needed values
const int gPINMAP[] =
	{ ADS1115_MUX_Single0, ADS1115_MUX_Single1, ADS1115_MUX_Single2, ADS1115_MUX_Single3 };
//	(For all the diffirential permutations, you're on your own!)

const int gGAINMAP[] =
	{ ADS1115_GAIN_6144, ADS1115_GAIN_4096, ADS1115_GAIN_2048, ADS1115_GAIN_1024,
		ADS1115_GAIN_512, ADS1115_GAIN_256, ADS1115_GAIN_256, ADS1115_GAIN_256	};

//	value chosen to allow or'ing in gain value and channel without masking them
const uint32_t ADS1115_BASIC_CONFIG = ADS1115_START_Single | ADS1115_MODE_SingleShot
										| ADS1115_DR_860 | ADS1115_MODE_NO_CMP;

// initial config register contents
uint32_t gADS1115_config = ADS1115_BASIC_CONFIG;

int gADS1115fd;					//	the unix file descriptor

/***************************************************************************
*	Low level IO primitives
****************************************************************************/

// Start I2C comms with the chip
int initADS1115(int i2c_bus_addr)
{
int reg_val = 0;

   if (		((gADS1115fd = open("/dev/i2c-1", O_RDWR)) >= 0)
   		&&	(ioctl(gADS1115fd, I2C_SLAVE, i2c_bus_addr & 0xff) >= 0) )
   		reg_val = 0;
	else reg_val = -1;
	return reg_val;
}

//	uses the default speed, mode and delay settings for now
//	pin and gain arguments are 0 based and use the global constant
//	tables defined above to look up the proper field values
int readADC(int pin, int gain)
{
int theValue = 0;
uint8_t readBuf[2] = { 0, 0 };
int theConfig = gADS1115_config | gPINMAP[pin] | gGAINMAP[gain];
uint8_t writeBuf[3] = { ADS1115_Config_p,
						(uint8_t)(theConfig >> 8),
						(uint8_t)(theConfig & 0xff) };
						
	bool result =  (write(gADS1115fd, writeBuf, 3) >= 0);
	if (result)
		{
		while ((readBuf[0] & ADS1115_Donebit) == 0)	//	wait for conversion 
		  	read(gADS1115fd, readBuf, 2);			// loop reading config reg
		}
	if (result)
		{
		writeBuf[0] = ADS1115_pConversion_p;
  		result = (write(gADS1115fd, writeBuf, 1) >= 0);
  		}
	if (result)
		{
		read(gADS1115fd, readBuf, 2);				// get conversion register
		theValue = readBuf[0] << 8 | readBuf[1];	// and assemble the full int
		}
	return theValue;
};



////////////////////////
//	PUBLIC ENTRY POINTS

int initVSWR(void)
{
	return (initADS1115(ADS_ADDR) == 0);
}


//	returns VSWR numerator and updates power in milliwatts in passed variable
//	This needs some smoothing, however
float readVSWR(float * fwdPower)
{
const float kPwrCalibration = 1.0;	//	fudge factor
const float cDirCoupling = 13.0;	//	fractional coupler loss factor
const float cDiodeDrop = 0.200;		//	germanium diode voltage offset
const int cTheGain = 0;				//  request lowest gain
const float cLSBSize = cLSBSIZE_TABLE[cTheGain] * cDirCoupling;

float rev = cDiodeDrop + (cLSBSize * readADC(0, cTheGain));
float fwd = cDiodeDrop + (cLSBSize * readADC(2, cTheGain));
static float rev_last;
static float fwd_last;

	fwd = (fwd + fwd_last)/2;
	rev = (rev + rev_last)/2;

	fwd = (fwd < 0.00001) ? 0.00001 : fwd;	//	crude divide by zero avoidance
	rev = (rev < 0.00001) ? 0.00001 : rev;

	float pwr = fwd * 0.7;				//	convert to rms
	pwr = ((pwr * pwr) / 50.0);			// estimate power in Watts at 50 ohms
	
	*fwdPower = pwr * kPwrCalibration;	//	update caller's power variable
	fwd_last = fwd;
	rev_last = rev;
	return (fwd + rev) / (fwd - rev);	//	return the numerator of the VSWR
}

