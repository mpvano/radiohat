/******************************************* March 25, 2022 at 3:36:24 PM CDT
*****************************************************************************
*	SGTL5000 Sound card initialization and control support.
*	Initializes sound card and controls TX-RX mixer switching
*	
*	started by Mario Vano AE0GL, February 2022
*	Has almost no error handling - needs to be added
*
******************************************************************************
*****************************************************************************/
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <linux/i2c-dev.h>
#include "SGTL5000test.h"
#include "codec.h"


//	These are the initial levels for the gain controls (normalized)
//	note that these are private to this module
float gHpVol = 0.88;
float gLoVol = 0.8;			//	seems to be normal output setting
float gMicVol = 0.0;		//	has 4 steps, +0, +20, +30, +40
float gADCVol = 0.99;		//	default maximum gain on receive
float gDACVol = 0.94;		//	leave a little headroom for now
float gAGCLevel = 0.0;		//	default to reasonable gain
float gAGCAttack = 0.02;	//	distortion if attack is too fast
float gAGCHang = 0.97;		//	very long decay for now
bool gAGCEnabled = false;

int gSGTLfd;				//	i2c fd

/***************************************************************************
*	Low level IO primitives
****************************************************************************/

// Start I2C comms with the chip
int initSGTL_I2C(int i2c_bus_addr)
{
int reg_val = 0;

   if (		((gSGTLfd = open("/dev/i2c-1", O_RDWR)) >= 0)
   		&&	(ioctl(gSGTLfd, I2C_SLAVE, i2c_bus_addr & 0xff) >= 0) )
   		reg_val = 0;
	else reg_val = -1;
//	perror("in initSGTL");
	return reg_val;
}

unsigned int readSGTL(unsigned int reg)
{
uint8_t reg_val = 0;
uint8_t buf[33];

	buf[0] = (reg >> 8);
	buf[1] = (reg & 0xff);
	
	int readcount = 0;

	if ((write(gSGTLfd, buf, 2) == 2) && ((readcount = read(gSGTLfd, buf, 2)) > 0))
		reg_val = buf[0];

	//	A debugging aid
#ifdef SGTL5000_DUMPALL
	fprintf(stderr, "in  readSGTL    read 0x%04X from register 0x%04X",
		(buf[0] << 8) | buf[1],reg, reg);
	perror(" ");
#endif

	return (buf[0] << 8) | buf[1];
}


bool writeSGTL(unsigned int reg, unsigned int val)
{
uint8_t buf[4];
int result;

	buf[0] = (reg >> 8);
	buf[1] = (reg & 0xff);
	buf[2] = (val >> 8);
	buf[3] = (val  & 0xff);
	
#ifndef SGTL5000_DUMPALL
	result = write(gSGTLfd, buf, 4);
#else
	fprintf(stderr, "in writeSGTL writing 0x%04X to   register 0x%04X",val,reg);
	result = write(gSGTLfd, buf, 4);
	perror(" ");
	readSGTL(reg);	// Read it back and dump that too
#endif

	return (result < 0) ? false : true;
}


unsigned int modifySGTL(unsigned int reg, unsigned int val, unsigned int iMask)
{
unsigned int val1 = (readSGTL(reg)&(~iMask))|val;

	if(!writeSGTL(reg,val1)) return 0;
	return val1;
}

/****************************************************************************
*	Mixer getters and setters
****************************************************************************/
float getHpVol(void) { return gHpVol * 100.0; }

void setHpVol(float percent)
{
//	writeSGTL(CHIP_ANA_HP_CTRL,0x0F0F);	//	HP LR volume to +7.5db
//	HP LR volume to +7.5db (7F7F is min, 00 is max)
	if (percent > 99) percent = 99;
	if (percent < 0) percent = 0;
	int temp = 127 - ((percent * 127)/100);
	temp += (temp * 256);
	writeSGTL(CHIP_ANA_HP_CTRL,temp);
	gHpVol = percent / 100.0;
}

//	Just a convenient way to temporarily turn the HpVol to 0
//	without disturbing the current setting.
//	Called with muteOn true, it will restore that setting.
void muteHpVol(bool muteOn)
{
	if (muteOn)
		writeSGTL(CHIP_ANA_HP_CTRL,0x7f7f);
	else setHpVol(getHpVol());
}

float getLoVol(void) { return gLoVol * 100.0; }

void setLoVol(float percent)
{
//	writeSGTL(CHIP_LINE_OUT_VOL,0x0505);	// set normal lineout level
//	only values 0-25 allowed for this config
	if (percent > 99) percent = 99;
	if (percent < 0) percent = 0;
	int temp = (percent * 25)/100;
	temp += (temp * 256);
	writeSGTL(CHIP_LINE_OUT_VOL,temp);
	gLoVol = percent / 100.0;
}

float getMicVol(void) { return gMicVol * 100.0; }

void setMicVol(float percent)
{
//	writeSGTL(CHIP_MIC_CTRL, 0x0161);	// MIC: biasSourceR=2k, bias2.5v, gain=20dB
	if (percent > 99) percent = 99;
	if (percent < 0) percent = 0;
	int temp = (percent * 3.99)/100;
	writeSGTL(CHIP_MIC_CTRL, 0x0160 + temp);
	gMicVol = percent / 100.0;
}

float getADCVol(void) { return gADCVol * 100.0;}

void setADCVol(float percent)
{
	// writeSGTL(CHIP_ANA_ADC_CTRL,0x00FF);//	ADC LR to +22.5db from ZiF Mixer
	if (percent > 99) percent = 99;
	if (percent < 0) percent = 0;
	int temp = (percent * 16)/100;
	temp += (temp * 16);
	writeSGTL(CHIP_ANA_ADC_CTRL,temp);	//	LR Nibles 0-F for +0 to +16.5db gain
	gADCVol = percent / 100.0;
}

float getDACVol(void) { return gDACVol * 100.0;}

void setDACVol(float percent)
{
//	writeSGTL(CHIP_DAC_VOL,0x3C3C);		// DAC L and R to 0db
//	CAUTION values FC, FD, FE and FF MUTE DAC!!!
//	also: values below 3c3c are "reserved"
//	this means values can range from 60 (0db) to 251 (-90db)
	if (percent > 99) percent = 99;
	if (percent < 0) percent = 0;
	int temp = ((99.0-percent) * 1.84) + 60.0;	//	hokey, but works...
	if (temp > 0xf0) temp = 0xFF; 
	temp += (temp * 256);
	writeSGTL(CHIP_DAC_VOL,temp);
	gDACVol = percent / 100.0;
}

float getAGCHang(void) { return gAGCHang * 100.0;}

void setAGCHang(float percent)
{
//	writeSGTL(DAP_AVC_DECAY,0x50);		// default 4.0db/sec
	if (percent > 99) percent = 99;
	if (percent < 0) percent = 0;
	int temp = ((99.0-percent) * 100) + 1;
	if (temp > 0xf0) temp = 0xFF; 
	writeSGTL(DAP_AVC_DECAY,temp);
	gAGCHang = percent / 100.0;
}

float getAGCAttack(void) { return gAGCAttack * 100.0;}

void setAGCAttack(float percent)
{
//	writeSGTL(DAP_AVC_ATTACK,0x28);		// default 32.0db/sec
	if (percent > 99) percent = 99;
	if (percent < 0) percent = 0;
	int temp = percent * 200;		//	0-200 range
	writeSGTL(DAP_AVC_ATTACK,temp);
	gAGCAttack = percent / 100.0;
}


//	note that if patch is in adc path, it's active over entire baseband
//	not just the passband inside the SSB channel. This is not a good thing.
//	If we use this audio agc, we probably should stick to the DAC path instead
//	which does not have this problem.
void enableAGC(bool state)
{
const unsigned int cExpand12 = 0x2000;	// max AVC gain (below threshold) 12db
const unsigned int cExpand6 = 0x1000;	// max AVC gain (below threshold) 6db
unsigned int temp = (readSGTL(DAP_AVC_CTRL) & 0x3000) | cExpand12;


	if ((state == true) && (gAGCLevel > .0001))
		{
//		writeSGTL(CHIP_SSS_CTRL,0x00013);		// ADC->DSP->I2S
		writeSGTL(CHIP_SSS_CTRL,0x0070);		// I2S->DSP->DAC
		writeSGTL(DAP_AVC_CTRL,temp | 0x0001);	//	set enable bit
		gAGCEnabled = true;
		}
	else
		{
		writeSGTL(CHIP_SSS_CTRL,0x00010);		// restore normal patch
		writeSGTL(DAP_AVC_CTRL,temp & 0xFFFE);	//	unset enable bit
		gAGCEnabled = false;
		}
}

float getAGCLevel(void) { return gAGCLevel * 100.0;}

void setAGCLevel(float percent)
{
//	The value written sets the threshold above and below which the AGC acts
//	experimentally, values between 50 and 10000 seem useful
//	Note that this also sets the maximum output level from the dac.
	if (percent > 99) percent = 99;
	if (percent < 1) percent = 0;
	writeSGTL(DAP_AVC_THRESHOLD,percent * 100);	// range of 100-10000?
	gAGCLevel = percent / 100.0;
	enableAGC(gAGCLevel > .001);
}

//	sets all mixer registers to current values from globals
void resetMixers(void)
{
	setHpVol(getHpVol());
	setLoVol(getLoVol());
	setMicVol(getMicVol());
	setADCVol(getADCVol());
	setDACVol(getDACVol());
	setAGCLevel(getAGCLevel());
	setAGCAttack(getAGCAttack());
	setAGCHang(getAGCHang());
	enableAGC(gAGCEnabled);
}

/****************************************************************************
*	Low level IO primitives
****************************************************************************/

//	Don't do this until after 12.288 Mhz CLK2 from si5351 has been started
int initSGTLRegisters(void)
{
	unsigned int n = readSGTL(CHIP_ID);	//	gets chip version
	if ((n & 0xFF00) != 0xa000)			// chip not found, or not the right part!
		{
		//fprintf(stderr,"CHIP ID = 0x%04X\n",n);
		return 0;
		}
	else
		{
	// POWER CONFIGURATION
		writeSGTL(CHIP_ANA_POWER, 0x4260);	//	switch to external VDDD supply
		writeSGTL(CHIP_LINREG_CTRL,0x006C);	//	configure charge pump
		writeSGTL(CHIP_REF_CTRL,0x01F2);	//	set pwr references, bias and options
		writeSGTL(CHIP_LINE_OUT_CTRL,0x0F13);//	set LINEOUT reference and bias current
		writeSGTL(CHIP_REF_CTRL,0x004F);	// set to ramp up slowly

	// OPTIONS FOR OUTPUT LINES
		writeSGTL(CHIP_SHORT_CTRL,0x1106);	//	enable & set level for HP short detection
		writeSGTL(CHIP_ANA_CTRL,0x0104);	// Mute Mic, Mute DAC->LO, line in->ADC, DAC->HP
		writeSGTL(CHIP_DIG_POWER, 0x0073);
		writeSGTL(CHIP_PAD_STRENGTH, 0X02AF);	// double default i2s current

	// CLOCK CONFIGURATION
		writeSGTL(CHIP_ANA_POWER,0x6AFB);	//	don't power up capless HP (bit 2)
		writeSGTL(CHIP_DIG_POWER, 0x0073);	// power up I2S_IN/OUT, DAP, DAC and ADC
		writeSGTL(CHIP_LINE_OUT_VOL,0x0303);// set LINEOUT volume for now (max)
		writeSGTL(CHIP_CLK_CTRL, 0x0008);	// set SYS_FS: 48 kHz, 256*Fs, NO PLL
		writeSGTL(CHIP_I2S_CTRL,0x0080);	// set I2S & sample clock: MS, 32 bit dlength

	// AUDIO INPUT/OUTPUT ROUTING
		writeSGTL(CHIP_ADCDAC_CTRL,0x0000);	//	disable "volume ramp"
		writeSGTL(CHIP_SSS_CTRL,0x00010);	//	I2S -> DAC, ADC->I2S - Nearly ALWAYS
		writeSGTL(DAP_CONTROL,0x0001);		// enable DAP in bypass mode
	
	//	initialize VOLUME AND MUTEs from globals
		resetMixers();
		muteHpVol(false);

	#ifdef SGTL5000_DUMPALL
		for (unsigned int reg = 0; reg <= 0x03c; reg +=2)
			readSGTL(reg);	//	only even numbered registers
	#endif
	
	return true;
	}
}


/****************************************************************************
*	Public entry point for Transmit/Receive mode switching
****************************************************************************/
//	enum cAudioMode { NORMAL_AUDIO, DIGITAL_AUDIO, CW_AUDIO };

//	IMPORTANT: REMEMBER THAT AGC ONLY WORKS WITH INTERNAL AUDIO TO HEADSET
//	returns true if succeeds (for now)
//	Note also that DAC Volume level MUST BE SAVED AND RESTORED BY CALLER
//	The rest of RX state saving should probably also be moved there eventually
//	or assumptions made by the receiver and by other TX routines may fail
//	Ramps like this one should probably be used for all modes, but different slopes
int enableTXAudio(bool txon, cAudioMode mode)
{
//const float keyramp[] = { 0,10,20,30,65,80,85,88,90,92,93,94 };
//const float unkeyramp[] = { 94,93,92,90,88,85,80,65,30,20,10,0 };
const float keyramp[] 	= { 20,65,85,90,93,94 };
const float unkeyramp[] = { 94,93,90,85,65,20 };
//const float keyramp[] 	= { 20,85,90,94 };
//const float unkeyramp[] = { 94,90,85,20 };

	if (txon)
		{
		muteHpVol(true);
		enableAGC(false);
		writeSGTL(CHIP_LINE_OUT_VOL,0x0303);	//	normal TX max line Volume
		
		//	Experimental
#ifdef USE_ROLLOFF
		writeSGTL(DAP_AUDIO_EQ_TREBLE_BAND4,0);	//	maximum cut at 9900hz
		writeSGTL(CHIP_SSS_CTRL,0x0170);		// I2S->DSP->DAC
		writeSGTL(DAP_AUDIO_EQ,0x0003);
#endif

		if (mode == DIGITAL_AUDIO)
			{
			//	FOR TRANSMITTING I2S input to card ONLY
			//	for digital, the ADC is not used as the microphone will be muted
			writeSGTL(CHIP_ANA_CTRL,0x0017);	//	mute all but lineout from ADC/DAC
			writeSGTL(CHIP_ANA_ADC_CTRL,0x0100);//	ADC LR (0-22.5db) to -6db (min)
			}
		else if (mode == CW_AUDIO)
			{
			//	Mute DAC except lineout to tx and loopback to I2S
			writeSGTL(CHIP_ANA_CTRL,0x0013);
			writeSGTL(CHIP_ANA_ADC_CTRL,0x0100);//	ADC LR (0-22.5db) to -6db (min)
			writeSGTL(CHIP_SSS_CTRL,0x00010);	// do not enable echo of tone
//			writeSGTL(CHIP_SSS_CTRL,0x00011);	// enable echo of tone
			for (int i=0; i < (sizeof(keyramp)/sizeof(float)); i++)
				setDACVol(keyramp[i]);	//	gets timed by i2c
			}
		else
			{
			//	FOR TRANSMITTING: MIC_IN -> DAC,  ADC -> I2S_OUT
			writeSGTL(CHIP_ANA_CTRL,0x0010);	//	mute all but lineout from ADC/DAC
			writeSGTL(CHIP_ANA_ADC_CTRL,0x0077);// ADC (0-22.5db) to +10.5db for mic
			setDACVol(94);
			}
		}
	else	//	txon is false - setup for receiving
		{
		if (mode == CW_AUDIO)
			{
			for (int i=0; i < (sizeof(unkeyramp)/sizeof(float)); i++)
				 setDACVol(unkeyramp[i]);	//	gets timed by i2c
			writeSGTL(CHIP_SSS_CTRL,0x00010);	//	Disable tone echo
			}
		
		//	FOR RECEIVING:	I2S in to DAC and I2S out from ADC
#ifdef USE_ROLLOFF
		writeSGTL(DAP_AUDIO_EQ,0x0000);
#endif
		writeSGTL(CHIP_ANA_CTRL,0x0104);	// Mute Mic, Mute DAC->LO, line in->ADC, DAC->HP
		enableAGC(mode != DIGITAL_AUDIO);			
		muteHpVol(false);
		setLoVol(getLoVol());				// restore normal rx lineout level
		setADCVol(getADCVol());				//	and ADC levels for receive
		}
	return true;
}

//	public entry point for init
//	returns true if it works
//	Don't do this until after 12.288 Mhz CLK2 
//	from si5351 (or elsewhere) has been started
int initCodec(void)
{
int result = (initSGTL_I2C(SGTL5000_I2C_ADDR_CS_LOW) != -1);

	if (result)
		result = initSGTLRegisters();
	if (result)
		result = enableTXAudio(false, DIGITAL_AUDIO);
	return result;
}

