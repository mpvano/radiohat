/************************************* February 10, 2022 at 1:30:14 PM CST
*****************************************************************************
*
*	A simple example program for a Zero IF Quadrature transceiver
*	using Si5351 for CLK0 and CLK1
*
*	Based on ncurses and keypad for all user IO interaction.
*	
*	It accepts a set of ascii increment/decrement keystrokes and also
*	a more intuitive keypad based method using left/right arrows
*	to highlight a digit and up/down arrows to adjust by that
*	digit's weight.
*	
*	NOTE that this program requires a compatibly mutant version of
*	the great Etherkit Si5351 library modified for the desired
*	target platform - the original library ONLY works on Arduino.
*	
*	This is gradually accumulating more bits and pieces for
*	controlling my radio hat for the pi...
*	
*		Initializes sound card and controls TX-RX mode switching
*		Controls RX and TX GPIO for hat, relay driver, LPF board and Pwr Amp
*		Controls and displays VSWR bridge
*
*	Still needs DSP modules and any Audio plumbing setup needed
*
*	If there is a file named "CALFACTOR.txt in the same directory
*		as the binary program file and if it contains a valid ascii
*		value for the VFO calibration factor, that will be used instead
*		of the default value compiled into the program.
*
*	one way to build this is:
*		c++ THISSOURCEFILE.cpp si5351pi.cpp -lncurses -lwiringpi
*	
*	started by Mario Vano AE0GL, February 2021 (and never finished)
*
******************************************************************************
*****************************************************************************/
#include <ncurses.h>
#include <math.h>
#include <unistd.h>


//	GLOBALS
bool gLSBMode = false;			//	output polarities switch phase sequence
bool gNeedsVFOUpdate = false;	//	 used in some polled mode apps



/****************************************************************************
*	si5351 definitions
****************************************************************************/
#include "si5351.h"
#undef SI5351_XTAL_FREQ
#define SI5351_XTAL_FREQ 25000000
#define cSI5351_I2C	0x60	//	default is for a 25Mhz crystal at 0x60h

//	This has to be done in the header file!
//#define SI5351_PLL_VCO_MIN 380000000


//	These define the operating limits of the VFO chip in this program
#define F_MIN	10000UL		// Chip Lower frequency limit  10 KHz
#define F_MAX	120000000UL // Chip Upper frequency limit 100 MHz

//	This is a useful default offset in most bands for my use
//	as it's the USB FT8 carrier frequency on 40,20,15 and 10 meters
//const int32_t INITIAL_FREQ = 7074000L;		// for 40m-10m FT8 USB
#ifdef WWV
const int32_t INITIAL_FREQ  = WWV * 1000000L;	//	useful during calibration
#else
const int32_t INITIAL_FREQ = 7074000L;			// for 40m-10m FT8 USB
#endif

//	The current production 25mhz crystals specify 8 pf crystal load
//	With this setting they  typically need around +20000 ppb correction here
//	much lower values (+-2000) can be achieved by using 10pf instead
//	if a file called CALFACTOR.txt is found in binary directory this value
//	will be read from it instead of using this defiition
#ifdef CALFACTOR
long gCalibrationFactor = CALFACTOR; // use this with -D when calibrating
#else
long gCalibrationFactor = 21800;	// for s/n 00110 at 8 pf
#endif

//	use CLK2 on si5351 to drive the SGTL5000 master clock input
const int32_t SGTL5000_FREQ = 12288000LL;	//	for 48Khz 32 bit duplex I2S, no PLL

Si5351 si5351(cSI5351_I2C);	


//	the VFO globals - these are all kept in DISPLAY units
//	the dual VFO mode is NOT active yet in this program
int gWhichVFO = 0;						// active vfo number 0-n				
long gTuningStepSize = 1000;
volatile uint32_t gVFOA = INITIAL_FREQ;
bool gRITA = false;
long gRITOffsetA = 0;
//	currently unused - for a future version with multiple VFOs
volatile uint32_t gVFOB = INITIAL_FREQ;
bool gRITB = false;
long gRITOffsetB = 0;

void checkLPF(uint32_t frequency, bool nocache);


/****************************************************************************
*
*	GPIO Assignments:
*
*		GPIOs I’m not using (I’ve used up the SPI pins for GPIO)
*			4
*		These are still free but try to avoid them for future use:
*			5,6		Pi built-in shutdown detection
*			4	is used by Pimoroni power switch shim
*		UART - 14: TXD, 15: RXD, 16: RTS, 17: CTS
*
*	These are using GPIO numbering, despite the use of WiringPi.
*
*	Wiring Pi is buggy and poorly maintained, so it probably needs
*	to be phased out. In particular, pull up/down controls
*	fail silently to take effect on Pi 4.
*
*	Note that the recommended way to initialize GPIO now seems to be
*	in config.txt or in the ID EEPROM, since these pins are hard wired
*	and dedicated to most peripheral hardware. Something like this should
*	do the trick (untested).
*
*		gpio=7-13=op,dl
*		gpio=22,23=op,dh
*		gpio=24-27=ip,pu
*		gpio=17=a3
*
****************************************************************************/
#include <wiringPi.h>			//	for now - need a better solution

//	SEE BELOW: 0-6 are RESERVED
//	Unique to my transceiver box
#define GPIO_RSTFilter	26		//	pulsed to reset latching relays
#define GPIO_B1Filter	7		//	10 meters - NOTE: Relay DISABLES filter 
#define GPIO_B2Filter	8		//	15 meters
#define GPIO_B3Filter	9		//	20 meters
#define GPIO_B4Filter	10		//	40 meters
#define GPIO_B5Filter	11		//	80 meters
#define GPIO_TXRXRelay	12		//	High switches TX->Filters else RX->Filters
#define GPIO_PWRAMP_ON	13		//	High enables Power amp Bias
//	14-21 are RESERVED
#define GPIO_notTX		22		//	a low enables the tx mixer HW
#define GPIO_notRX		23		//	a low enables the rx mixer HW

//	For current and future features of this software
#define GPIO_KeyerDIT	24		//	hw keying and ptt inputs
#define GPIO_KeyerDAH	25
//#define GPIO_SKeyIn		26
#define GPIO_PTTIn		27
#define GPIO_Keyer_KEYDOWN GPIO_KeyerDAH

//////////////////////////////////////////////////////
//	THE REST ARE RESERVED AND SHOULDN'T BE USED 
//	UART
#define GPIO_UART0_TX	14		//	RESERVED
#define GPIO_UART0_RX	15		//	RESERVED
#define GPIO_CTS		16		//	RESERVED
#define GPIO_RTS		17		//	WSJT-X and others can assert RTS for PTT

#define GPIO_PCM_CLK	18		//	USED BY AUDIO HARDWARE I2S
#define GPIO_PCM_FS		19		//	USED BY AUDIO HARDWARE I2S
#define GPIO_PCM_DIN	20		//	USED BY AUDIO HARDWARE I2S
#define GPIO_PCM_DOUT	21		//	USED BY AUDIO HARDWARE I2S

#define GPIO_ID_SD		0		//	I2C RESERVED BY OPERATING SYSTEM
#define GPIO_ID_SC		1		//	I2C RESERVED BY OPERATING SYSTEM
#define GPIO_SDA		2		//	RESERVED FOR GENERAL PURPOSE I2C
#define GPIO_SCL		3		//	RESERVED FOR GENERAL PURPOSE I2C
#define GPCLK0			4		//	RESERVED FOR ONEWIRE BUS
#define GPIO_SHUTDOWN1	5		//	pin 1 of gpio-shutdown overlay switch
#define GPIO_SHUTDOWN2	6		//	pin 2 of gpio-shutdown overlay switch
//////////////////////////////////////////////////////////


bool initGPIO()
{
	wiringPiSetupGpio();

	//	Init QRP Labs filter board connected via O.C. Level shifters
	//	at startup, LPF is set to 10 meters
	//	rx is enabled and tx and power amp are disabled
	pinMode(GPIO_B1Filter, OUTPUT);
	pinMode(GPIO_B2Filter, OUTPUT);
	pinMode(GPIO_B3Filter, OUTPUT);
	pinMode(GPIO_B4Filter, OUTPUT);
	pinMode(GPIO_B5Filter, OUTPUT);
	checkLPF(30000000, true);		//	sets LPF pins, cache and 10m high pass

	//	Two O.C. Level shifters on that board control Ant Switch and Power Amp
	pinMode(GPIO_TXRXRelay, OUTPUT);
	digitalWrite(GPIO_TXRXRelay, LOW);
	pinMode(GPIO_PWRAMP_ON, OUTPUT);
	digitalWrite(GPIO_PWRAMP_ON, LOW);

	//	These GPIO pins enable the Hat's TX and RX mixers
	pinMode(GPIO_notTX, OUTPUT);	
	digitalWrite(GPIO_notTX, HIGH);
	pinMode(GPIO_notRX, OUTPUT);	
	digitalWrite(GPIO_notRX, LOW);

	//	The WiringPi pullup controls DON'T WORK on PI4
	//	This is not a solution, just a temporary hack (see note above)...
	//	These pins are monitored by this program for keying
	pinMode(GPIO_KeyerDIT, INPUT);
	pullUpDnControl(GPIO_KeyerDIT, PUD_UP);
	pinMode(GPIO_KeyerDAH, INPUT);
	pullUpDnControl(GPIO_KeyerDAH, PUD_UP);
	//pinMode(GPIO_SKeyIn, INPUT);
	//pullUpDnControl(GPIO_SKeyIn, PUD_UP);
	pinMode(GPIO_PTTIn, INPUT);
	pullUpDnControl(GPIO_PTTIn, PUD_UP);
	system("raspi-gpio set 24 ip pu");
	system("raspi-gpio set 25 ip pu");
	system("raspi-gpio set 26 ip pu");
	system("raspi-gpio set 27 ip pu");
	system("raspi-gpio set 17 op a3");	//	wiring pi can't do this
	return true;
}


/****************************************************************************
*	ads1115 ADC io support for VSWR and other sensors
*	using the wiring Pi driver for that chip.
*
*	This is usually preinstalled on all raspberry Pi
*
*	The gain is changed by digitalWrite to user defined base pin number
*	Frame rate is set by digitalWrite to that pin number+1
****************************************************************************/
#include <ads1115.h>

#define  PINBASE  100		// choose  PINBASE  value arbitrarily
#define  ADS_ADDR 0x48		// set by jumpers on board

const float cLSBSIZE_TABLE[] =
	{ 0.0001875, 0.0001250, 0.0000625, 0.00003125, 0.000015625, 0.0000078125 };

int initADC(int address)
{
	ads1115Setup(PINBASE, address);
	analogWrite(PINBASE, ADS1115_GAIN_6);
	analogWrite(PINBASE+1,ADS1115_DR_16); 
	return 0;
}

int readADC(int pin, int gain)
{
	analogWrite(PINBASE, gain);
	int theValue = analogRead(PINBASE + pin);
	return theValue;
};

//	returns VSWR numerator and updates power in milliwatts in passed variable
//	This needs some smoothing, however
float readVSWR(float * fwdPower)
{
const float kPwrCalibration = 1.0;	//	fudge factor
const float cDirCoupling = 13.0;	//	fractional coupler loss factor
const float cDiodeDrop = 0.200;		//	germanium diode voltage offset
const int cTheGain = ADS1115_GAIN_6;
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


/****************************************************************************
*	SGTL5000 Sound card initialization support
****************************************************************************/
#include <linux/i2c-dev.h>
#include "SGTL5000test.h"

int SGTLfd;

// Start I2C comms with the chip
int initSGTL_I2C(int i2c_bus_addr)
{
int reg_val = 0;

   if (		((SGTLfd = open("/dev/i2c-1", O_RDWR)) >= 0)
   		&&	(ioctl(SGTLfd, I2C_SLAVE, i2c_bus_addr & 0xff) >= 0) )
   		reg_val = 0;
	else reg_val = -1;
	perror("in initSGTL");
	return reg_val;
}

unsigned int readSGTL(unsigned int reg)
{
uint8_t reg_val = 0;
uint8_t buf[33];

	buf[0] = (reg >> 8);
	buf[1] = (reg & 0xff);
	
	int readcount = 0;

	if ((write(SGTLfd, buf, 2) == 2) && ((readcount = read(SGTLfd, buf, 2)) > 0))
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
	result = write(SGTLfd, buf, 4);
#else
	fprintf(stderr, "in writeSGTL writing 0x%04X to   register 0x%04X",val,reg);
	result = write(SGTLfd, buf, 4);
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
	
//	These are the initial levels for the gain controls
float gHpVol = 0.88;
float gLoVol = 0.8;			//	seems to be normal output setting
float gMicVol = 0.0;		//	has 4 steps, +0, +20, +30, +40
float gADCVol = 0.99;		//	default maximum gain on receive
float gDACVol = 0.94;		//	leave a little headroom for now
bool gAGCEnabled = false;
float gAGCLevel = 0.0;		//	default to reasonable gain
float gAGCAttack = 0.02;	//	distortion if attack is too fast
float gAGCHang = 0.97;		//	very long decay for now

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



//	Don't do this until after 16.288Mhz CLK2 has been setup from si5351
int initSGTLRegisters(void)
{
	unsigned int n = readSGTL(CHIP_ID);	//	gets chip version
//	fprintf(stderr,"CHIP ID = 0x%04X\n",n);

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
	setMicVol(getMicVol());
	setADCVol(getADCVol());
	setDACVol(getDACVol());
	muteHpVol(false);
	setHpVol(getHpVol());
	
	setAGCAttack(getAGCAttack());
	setAGCHang(getAGCHang());
	setAGCLevel(getAGCLevel());
	enableAGC(true);



#ifdef SGTL5000_DUMPALL
	for (unsigned int reg = 0; reg <= 0x03c; reg +=2)
		readSGTL(reg);	//	only even numbered registers
#endif

	return true;
}

enum cAudioMode { NORMAL_AUDIO, DIGITAL_AUDIO, CW_AUDIO };

//	IMPORTANT: REMEMBER THAT AGC ONLY WORKS WITH INTERNAL AUDIO OUTPUT TO HEADSET
void enableTXAudio(bool txon, cAudioMode mode)
{
	if (txon)
		{
		muteHpVol(true);
		enableAGC(false);
		writeSGTL(CHIP_LINE_OUT_VOL,0x0303);	//	normal TX max line Volume
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
			writeSGTL(CHIP_SSS_CTRL,0x00011);	// enable echo of tone
			}
		else
			{
			//	FOR TRANSMITTING: MIC_IN -> DAC,  ADC -> I2S_OUT
			writeSGTL(CHIP_ANA_CTRL,0);	//	mute all but lineout from ADC/DAC
			writeSGTL(CHIP_ANA_ADC_CTRL,0x0077);// ADC (0-22.5db) to +10.5db for mic
			}
		}
	else
		{
		if (mode == CW_AUDIO)
			writeSGTL(CHIP_SSS_CTRL,0x00010);	//	Disable tone echo
		
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
}


/****************************************************************************
*	support functions for QRP labs filter board
*
*	Note that the QRP labs filter board has funny wiring for filter 1.
*	It is DISABLED by activating the coil! (I think - it's confusing!)
****************************************************************************/

//	called to see if need to update LPF setting
//	The cache argument is only used with latching relays to decide whether
//	we can skip the tedious and slow switching procedure they require.
void checkLPF(uint32_t frequency, bool nocache)
{
const uint32_t LPF1_LIMIT = 30000000L;
const uint32_t LPF2_LIMIT = 21500000L;
const uint32_t LPF3_LIMIT = 14500000L;
const uint32_t LPF4_LIMIT = 7500000L;
const uint32_t LPF5_LIMIT = 4000000L;

static int lastfilter = -1;
int thefilter;

	if (frequency < LPF5_LIMIT)			thefilter = 5;
	else if (frequency < LPF4_LIMIT)	thefilter = 4;
	else if (frequency < LPF3_LIMIT)	thefilter = 3;
	else if (frequency < LPF2_LIMIT)	thefilter = 2;
	else thefilter = 1;
	
	//	Should rewrite to use a constant table for clarity and efficiency
	if (nocache || (lastfilter != thefilter))
		{
		switch (thefilter)		//	enable the correct filter
			{
			default:
			case 1:
				digitalWrite(GPIO_B1Filter, LOW);	//	filter one enabled
				digitalWrite(GPIO_B2Filter, LOW);
				digitalWrite(GPIO_B3Filter, LOW);
				digitalWrite(GPIO_B4Filter, LOW);
				digitalWrite(GPIO_B5Filter, LOW);
				break;
			case 2:
				digitalWrite(GPIO_B1Filter, HIGH);	//	filter one disabled
				digitalWrite(GPIO_B2Filter, HIGH);
				digitalWrite(GPIO_B3Filter, LOW);
				digitalWrite(GPIO_B4Filter, LOW);
				digitalWrite(GPIO_B5Filter, LOW);
				break;
			case 3:
				digitalWrite(GPIO_B1Filter, HIGH);	//	filter one disabled
				digitalWrite(GPIO_B2Filter, LOW);
				digitalWrite(GPIO_B3Filter, HIGH);
				digitalWrite(GPIO_B4Filter, LOW);
				digitalWrite(GPIO_B5Filter, LOW);
				break;
			case 4:
				digitalWrite(GPIO_B1Filter, HIGH);	//	filter one disabled
				digitalWrite(GPIO_B2Filter, LOW);
				digitalWrite(GPIO_B3Filter, LOW);
				digitalWrite(GPIO_B4Filter, HIGH);
				digitalWrite(GPIO_B5Filter, LOW);
				break;
			case 5:
				digitalWrite(GPIO_B1Filter, HIGH);	//	filter one disabled
				digitalWrite(GPIO_B2Filter, LOW);
				digitalWrite(GPIO_B3Filter, LOW);
				digitalWrite(GPIO_B4Filter, LOW);
				digitalWrite(GPIO_B5Filter, HIGH);
				break;
			}
		lastfilter = thefilter;		//	update the cache
		}
}


/****************************************************************************
*	TX / RX Switching
****************************************************************************/

//	single point to poll for external PTT requests
bool isTXRequested(void)
{
bool request = !digitalRead(GPIO_PTTIn)
//			|| !digitalRead(GPIO_SKeyIn)
			|| !digitalRead(GPIO_RTS);
	return request;
}

//	hardware TX-RX switching handler, also calls audio switch handler
void enableTX(bool txon, cAudioMode mode)
{
	if (txon)
		{
		digitalWrite(GPIO_TXRXRelay, HIGH);
		digitalWrite(GPIO_notRX, HIGH);
//		digitalWrite(GPIO_PWRAMP_ON, HIGH);
		digitalWrite(GPIO_notTX, LOW);
		enableTXAudio(txon, mode);
		usleep(5000);
		digitalWrite(GPIO_PWRAMP_ON, HIGH);
		}
	else
		{
		enableTXAudio(txon, mode);
		digitalWrite(GPIO_notTX, HIGH);
		digitalWrite(GPIO_PWRAMP_ON, LOW);
		usleep(500);
		digitalWrite(GPIO_TXRXRelay, LOW);
		digitalWrite(GPIO_notRX,  LOW);
		}	
}

void checkKeydown(void)
{
static bool keyed;

	if (!digitalRead(GPIO_Keyer_KEYDOWN))
		{								//	key IS down
		if(!keyed)						//	key WAS NOT down
			{
			enableTX(true,CW_AUDIO);	//	TX, disable microphone & turn on sidetone
			keyed = true;				//	remember that we are keyed
			}
		}
	else
		{								//	Key IS NOT down
		if (keyed)						//	key WAS down
			{
			enableTX(false,CW_AUDIO);	//	not -digital, so AGC is re-enabled
			keyed = false;				//	remember that we are NOT keyed			
			}
		}
}

/****************************************************************************
*	VFO support functions
****************************************************************************/

//	returns true only if the current VFO has RIT turned on
bool RITActive()
{
	return ((gWhichVFO==0) ? gRITA : gRITB);
}

//	converts from a display frequency to a target device frequency
//	Handy for things like CW, but not currently in use
int32_t DisplayFreqToLO(int32_t theFreq)
{
//int32_t theOffset = (IF_NOMINAL + gIfModeP->ifOffset) * gBandP->ifOffsetDir;
//int32_t theResult = (theFreq - theOffset) / gBandP->vfoMultiple ;
//	return theResult;
	return theFreq;
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
	else if (freq < 9500000) return 88;
	else if (freq < 13600000) return 64;
	else if (freq < 17500000) return 44;
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

	si5351.set_freq_manual(freq * SI5351_FREQ_MULT,
							evenDivisor * freq * SI5351_FREQ_MULT,
							SI5351_CLK0	);
	si5351.set_freq_manual(freq * SI5351_FREQ_MULT,
							evenDivisor * freq * SI5351_FREQ_MULT,
							SI5351_CLK1	);
						
	si5351.set_phase(SI5351_CLK0, 0);
	si5351.set_phase(SI5351_CLK1, evenDivisor);

	if (evenDivisor != lastdivisor)
		si5351.pll_reset(SI5351_PLLA);
	lastdivisor = evenDivisor;
}


//	The single point routine to update the vfo from globals
//	It returns early if the frequency has not changed.
//	
//	uses global flags and vfo variables
//
//	the argument determines whether optional RIT is allowed or
//	is to be ignored when setting the frequency (e.g. on transmit)
void updateTheVFO(bool enableRIT)
{
uint32_t freqtemp = (gWhichVFO==0) ? gVFOA : gVFOB;
static uint32_t lastFreqSent = 0;
	if (enableRIT && RITActive())
		freqtemp += ((gWhichVFO==0) ? gRITOffsetA : gRITOffsetB);	
	freqtemp = DisplayFreqToLO(freqtemp);

	if (freqtemp != lastFreqSent)
		{
		lastFreqSent = freqtemp;
		SendFrequency(freqtemp);
		checkLPF(freqtemp, false);
		while (!isVFOLocked)
			;
		}
	gNeedsVFOUpdate = false;		
}

long getCalibrationFactor(char * progname)
{
long calfactor = gCalibrationFactor;
char * dirptr;
char configPath[128];
char buffer[128] =""; // Buffer to store data
FILE * theFile = NULL;

	strcpy(configPath,progname);
	dirptr = strrchr(configPath, '/');
	*dirptr = 0;
	strcat(configPath,"/CALFACTOR.txt");
	if ((theFile = fopen(configPath, "r")) != NULL)
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
void setupSynthesizers(long correction, uint32_t startFreq, bool calibrate)
{
	//	load can be 0, 6, 8  or 10 pf
	si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351_XTAL_FREQ, correction);
	if (calibrate)
		si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
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
	si5351.set_ms_source(SI5351_CLK2, SI5351_PLLB);
	si5351.set_freq((SGTL5000_FREQ * SI5351_FREQ_MULT), SI5351_CLK2);
	si5351.drive_strength(SI5351_CLK2,SI5351_DRIVE_2MA);
	si5351.output_enable(SI5351_CLK2, true);

	//	in quadrature versions this must be skipped to calibrate
	if (!calibrate)
		updateTheVFO(false);
}


/*****************************************************************************
*	a simple ncurses frequency control keyboard UI
*
*	controls:
*		Frequency and LPF switching
*		USB/LSB VCO phasing and display
*		TX/RX switching
*		VSWR display
*
*	need to add:
*		Volume
*		Digital mode
*		TX Gain
*		Digital TX gain
*		Transverter OFFSET
*		OUT OF BAND TX PREVENTION
*		Modes: USB/LSB/NBFM/CW
*		Filter settings
*		S Meter
*		AGC mode
*		Keyer, Key type and Keyer Speed
*		Second VFO
*
*	And need to add support for external keying sources and control via
*	virtual RS232 and CAT.
*
*****************************************************************************/
bool vfo_interface(void)
{
#define HZ_COLUMN 11
#define HZ_LINE 3
#define TXRX_LINE 2
#define MENU_LINE 0
#define GAIN_LINE 1
bool volume_changed = true;
bool transmitter_on = false;
bool digitalMode = false;
bool cwMode = false;
long int rx_freq = gVFOA;
int c;
int weight = 2;
int current_column = HZ_COLUMN - weight;
static long int weights[] = { 1L,10L,100L,1000L,10000L,
								100000L,1000000L,10000000L };
const int num_weights = sizeof(weights)/sizeof(*weights);

    move(MENU_LINE,0); addstr("[>]Usb [<]Lsb [C]w [F]t8 [T]x [R]x [Q]uit");
    move(GAIN_LINE,0); printw("GAIN: Hp%2.0f Lo%2.0f Mic%2.0f Dac%2.0f Adc%2.0f aGc%2.0f",
    							getHpVol(), getLoVol(), getMicVol(),
    							getDACVol(), getADCVol(), getAGCLevel()	);
	move(TXRX_LINE,0); addstr("USB: Receive              ");
	move(HZ_LINE,0); printw("VFO %8i",rx_freq);
	move(HZ_LINE,current_column);
	refresh();

	while (true)
		{
		
		if (cwMode)
			checkKeydown();	
		else if (digitalMode)
			{
			if (isTXRequested())
				{
				transmitter_on = true;
				enableTX(transmitter_on, DIGITAL_AUDIO);
				}
			if (!isTXRequested())
				{
				transmitter_on = false;
				enableTX(transmitter_on, DIGITAL_AUDIO);
				}
			}
		
    	if ((c = getch()) > 0)
			{
			switch (c)
				{					
				case 'A':
//					if (!transmitter_on)
						{
						setADCVol(getADCVol() + 1);
						volume_changed = true;
						}
					break;

				case 'a':
//					if (!transmitter_on)
						{
						setADCVol(getADCVol() - 1);
						volume_changed = true;
						}
					break;


				case 'c':
				case 'C':
					cwMode = ! cwMode;
					if (!cwMode)
						{
						transmitter_on =  false;
						enableTX(transmitter_on, NORMAL_AUDIO);
						}
					break;

				
				case 'D':
					if (!transmitter_on)
						{
						setDACVol(getDACVol() + 1);
						volume_changed = true;
						}
					break;

				case 'd':
					if (!transmitter_on)
						{
						setDACVol(getDACVol() - 1);
						volume_changed = true;
						}
					break;
				
				
				case 'f':
				case 'F':
					digitalMode = ! digitalMode;
					if (!digitalMode)
						{
						transmitter_on =  false;
						enableTX(transmitter_on, NORMAL_AUDIO);
						}
					enableAGC(digitalMode == false);
					break;

				case 'G':
					if (!transmitter_on)
						{
						setAGCLevel(getAGCLevel() + 1);
						volume_changed = true;
						}
					break;

				case 'g':
					if (!transmitter_on)
						{
						setAGCLevel(getAGCLevel() - 1);
						volume_changed = true;
						}
					break;

				
				case 'H':
					if (!transmitter_on)
						{
						setHpVol(getHpVol() + 1);
						volume_changed = true;
						}
					break;

				case 'h':
					if (!transmitter_on)
						{
						setHpVol(getHpVol() - 1);
						volume_changed = true;
						}
					break;
				
				case 'L':
					if (!transmitter_on)
						{
						setLoVol(getLoVol() + 1);
						volume_changed = true;
						}
					break;

				case 'l':
					if (!transmitter_on)
						{
						setLoVol(getLoVol() - 1);
						volume_changed = true;
						}
					break;
				
				case 'M':
					setMicVol(getMicVol() + 1);
					volume_changed = true;
					break;

				case 'm':
					setMicVol(getMicVol() - 1);
					volume_changed = true;
					break;
									
					
				case 'q':
				case 'Q':
					enableTX(false, NORMAL_AUDIO);
					return true;
					break;
				
				case 'R':
				case 'r':
					transmitter_on =  false;
					enableTX(transmitter_on, NORMAL_AUDIO);
					break;
								

				case 'T':
				case 't':
					transmitter_on =  true;
					enableTX(transmitter_on, NORMAL_AUDIO);
					break;
				
				case '<':
					gLSBMode = true;
					si5351.set_clock_invert(SI5351_CLK0,gLSBMode);
					break;
					
				case '>':
					gLSBMode = false;
					si5351.set_clock_invert(SI5351_CLK0,gLSBMode);
					break;
					
				case KEY_RIGHT:
					weight = max(--weight,0);
					current_column = HZ_COLUMN - weight;
					break;

				case KEY_LEFT:
					weight = min(++weight,(num_weights - 1));
					current_column = HZ_COLUMN - weight;
					break;

				case KEY_UP:
					rx_freq += weights[weight];
					break;

				case KEY_DOWN:
					rx_freq -= weights[weight];
					break;

				default:
					break;		// no change
				}
				
			c = -1;

			//	limit frequency range entry
			rx_freq = min(rx_freq,32000000L);
			rx_freq = max(rx_freq,3500000L);

			gVFOA = rx_freq;
			updateTheVFO(false);
			
			move(TXRX_LINE,0); 
			if (digitalMode) addstr(gLSBMode ? "LSBD: " : "USBD: ");
			else if (cwMode) addstr(gLSBMode ? "LSBC: " : "USBC: ");
			else addstr(gLSBMode ? "LSB:  " : "USB:  ");
			
			move(TXRX_LINE,5);
			addstr(transmitter_on ? "Transmit " : "Receive              ");
			
			move(HZ_LINE,0); printw("VFO %8i",
									rx_freq + (cwMode ? (gLSBMode ? -700 : 700) : 0) );
			
			move(HZ_LINE,current_column);
			refresh();
			}
		
		if (volume_changed)
			{
    		move(GAIN_LINE,0);
			printw("GAIN: Hp%2.0f Lo%2.0f Mic%2.0f Dac%2.0f Adc%2.0f aGc%2.0f",
					getHpVol(), getLoVol(), getMicVol(),
					getDACVol(), getADCVol(), getAGCLevel()	);
			volume_changed = false;
			move(HZ_LINE,current_column);
			refresh();
    		}

		if (!cwMode)	// need to minimize refresh calls to keep it responsive
			{
			//	if transmitting, need to check vswr here and display it
			if (transmitter_on)
				{
				static float pwr, lastpwr, lastswr;
				float swr = readVSWR(&pwr);
				if (pwr < .05) swr = 0;
				move(TXRX_LINE,5); addstr("Transmit ");
				move(TXRX_LINE,15);
				printw("%1.2fw %1.1f:1 ", (pwr+lastpwr)/2, (swr+lastswr)/2);
				lastpwr = pwr;
				lastswr = swr;
				move(HZ_LINE,current_column);
				refresh();
				}
			else
				{
				move(TXRX_LINE,5); addstr( "Receive              ");
				move(TXRX_LINE,15);
				move(HZ_LINE,current_column);
				refresh();
				}
			//	ugly debugging method (but effective)
			move(4,0); printw("%s",gAGCEnabled ? "AGC On ": "AGC Off");
			//move(0,0); printw("%s",digitalRead(GPIO_PTTIn) ? "*": " ");
			//move(1,0); printw("%s",digitalRead(GPIO_SKeyIn) ? "*": " ");
			//move(2,0); printw("%s",digitalRead(GPIO_RTS) ? "*": " ");
			move(HZ_LINE,current_column);
			}
		}
	return true;
}


/****************************************************************************
*****************************************************************************
*	Main
*****************************************************************************
****************************************************************************/
int main(int argc, char * argv[])
{
bool initonly = (argc == 2) && (*(argv[1]) == '-') && (*(argv[1]+1) == 'i');

	if (initonly)
		{
		initGPIO();
		long cfactor = getCalibrationFactor(argv[0]);
		setupSynthesizers(cfactor, gVFOA, false);
		fprintf(stderr,"%s: Devices initialized only!\n", *argv);
		}
		
		
	else if (initGPIO())
		{
		WINDOW *w = initscr();		// ncurses setup
		w = initscr();
		timeout(1);
		noecho();
		keypad(w,true);
		atexit([]{ endwin(); });
	
		long cfactor = getCalibrationFactor(argv[0]);
		setupSynthesizers(cfactor, gVFOA, false);
		
		if (initSGTL_I2C(SGTL5000_I2C_ADDR_CS_LOW) < 0)
			fprintf(stderr,"Unable to open I2C bus for SGTL5000\n");
		else
			{
			initSGTLRegisters();		//	do this twice for now
			initSGTLRegisters();
			}
		initADC(ADS_ADDR);
		
		//	could optionally set up redirectors for wsjtx and jackd here
		//	and undo them at exit
		//	system("pacmd load-module module-jack-source");
		//	system("pacmd load-module module-jack-sink");
		if (!initonly)
			vfo_interface();			//	loop processing input
		}
	else printf("initGPIO failed\n");
#ifdef USE_FSGPIO
	uninitGPIO();
#endif
	return true;
}
