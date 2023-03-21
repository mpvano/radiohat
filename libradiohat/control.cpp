/*
**************************************** February 23, 2023 at 10:59:26 AM CST
*****************************************************************************
*
*	A simple example program for a Zero IF Quadrature transceiver
*	one way to build this is:
*
*	Libgpio implemented despite the fact that it's not fully implemented,
*	badly documented and clumsy to use in every way.
*
*	It is now, however the only sanctioned way to control pins!
*	The linux gods have spoken!
*	
*	started by Mario Vano AE0GL, February 2022
*
******************************************************************************
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <gpiod.h>

#include "control.h"
#include "codec.h"



/*
***************************************************************************
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

//////////////////////////////////////////////////////////////////////////////
//	THE REST ARE RESERVED AND SHOULDN'T BE USED BY THIS MODULE
#define GPIO_ID_SD		0		//	I2C RESERVED BY OPERATING SYSTEM
#define GPIO_ID_SC		1		//	I2C RESERVED BY OPERATING SYSTEM
#define GPIO_SDA		2		//	RESERVED FOR GENERAL PURPOSE I2C
#define GPIO_SCL		3		//	RESERVED FOR GENERAL PURPOSE I2C
#define GPCLK0			4		//	RESERVED FOR ONEWIRE BUS
#define GPIO_SHUTDOWN1	5		//	pin 1 of gpio-shutdown overlay switch
#define GPIO_SHUTDOWN2	6		//	pin 2 of gpio-shutdown overlay switch

#define GPIO_UART0_TX	14		//	RESERVED
#define GPIO_UART0_RX	15		//	RESERVED
#define GPIO_CTS		16		//	RESERVED
#define GPIO_RTS		17		//	WSJT-X and others can assert RTS for PTT

#define GPIO_PCM_CLK	18		//	USED BY AUDIO HARDWARE I2S
#define GPIO_PCM_FS		19		//	USED BY AUDIO HARDWARE I2S
#define GPIO_PCM_DIN	20		//	USED BY AUDIO HARDWARE I2S
#define GPIO_PCM_DOUT	21		//	USED BY AUDIO HARDWARE I2S
//////////////////////////////////////////////////////////////////////////////

//	needs globals to store handles for now
#define	CONSUMER	"RadioHat"
struct gpiod_line * GPIO_RSTFilter_line; 
struct gpiod_line * GPIO_B1Filter_line; 
struct gpiod_line * GPIO_B2Filter_line; 
struct gpiod_line * GPIO_B3Filter_line; 
struct gpiod_line * GPIO_B4Filter_line; 
struct gpiod_line * GPIO_B5Filter_line; 
struct gpiod_line * GPIO_TXRXRelay_line; 
struct gpiod_line * GPIO_PWRAMP_ON_line;
struct gpiod_line * GPIO_notTX_line;
struct gpiod_line * GPIO_notRX_line;
struct gpiod_line * GPIO_KeyerDIT_line; 
struct gpiod_line * GPIO_KeyerDAH_line;
struct gpiod_line * GPIO_RTS_line;

// struct gpiod_line GPIO_SKeyIn_line; 
struct gpiod_line * GPIO_PTTIn_line;


char const * chipname = "gpiochip0";
struct gpiod_chip *gChip;

//	helper for opening output lines
struct gpiod_line * openLineForOutput(int gpioNum)
{
struct gpiod_line * theLine;

	if (	(theLine = gpiod_chip_get_line(gChip, gpioNum))
		&&	((gpiod_line_request_output(theLine, CONSUMER, 0) >= 0))
		) return theLine;
	else
		{
		if (theLine) gpiod_line_release(theLine);
		return 0;
		}
}

//	helper for opening input lines
struct gpiod_line * openLineForInput(int gpioNum)
{
struct gpiod_line * theLine;

	if (	(theLine = gpiod_chip_get_line(gChip, gpioNum))
		&&	((gpiod_line_request_input(theLine, CONSUMER) >= 0))
		) return theLine;
	else
		{
		if (theLine)
			gpiod_line_release(theLine);
		return 0;
		}
}

//	helper for writing output lines
bool gpioWrite(struct gpiod_line * line, int val)
{
	return gpiod_line_set_value(line, val);
}

int gpioRead(struct gpiod_line * line)
{
	return gpiod_line_get_value(line);
}


//	opens all the lines and saves global handles to them
bool initGPIO()
{
bool result = false;
struct gpiod_chip * line;

	gChip = gpiod_chip_open_by_name(chipname);
	if (!gChip)
		perror("Open chip failed\n");
	else
		{
		//	setup the output lines
		result = (	(GPIO_RSTFilter_line 	= openLineForOutput(GPIO_RSTFilter))
				 &&	(GPIO_B2Filter_line 	= openLineForOutput(GPIO_B2Filter))
				 &&	(GPIO_B3Filter_line 	= openLineForOutput(GPIO_B3Filter))
				 &&	(GPIO_B4Filter_line 	= openLineForOutput(GPIO_B4Filter))
				 &&	(GPIO_B5Filter_line 	= openLineForOutput(GPIO_B5Filter))
				 &&	(GPIO_TXRXRelay_line 	= openLineForOutput(GPIO_TXRXRelay))
				 &&	(GPIO_PWRAMP_ON_line 	= openLineForOutput(GPIO_PWRAMP_ON))
				 &&	(GPIO_notTX_line 		= openLineForOutput(GPIO_notTX))
				 &&	(GPIO_notRX_line 		= openLineForOutput(GPIO_notRX))
				 &&	(GPIO_B1Filter_line 	= openLineForOutput(GPIO_B1Filter))
				 );	
		if (!result)
			perror("Requesting line(s) as output failed\n");
		else
		    {
		    //	setup the input lines
		    //	These pins are monitored by this program for keying
		    result = (	(GPIO_KeyerDIT_line = openLineForInput(GPIO_KeyerDIT))
				 	&&	(GPIO_KeyerDAH_line = openLineForInput(GPIO_KeyerDAH))
				 	&&	(GPIO_PTTIn_line 	= openLineForInput(GPIO_PTTIn))
				 	&&	(GPIO_RTS_line 	= openLineForInput(GPIO_RTS))
					);
			if (!result)
				perror("Requesting line(s) as input failed\n");
			}				
		}
	
	if (result)
		{
		//	Init QRP Labs filter board connected via O.C. Level shifters
		//	at startup, LPF is set to 10 meters
		//	Two O.C. Level shifters on that board control Ant Switch and Power Amp
		//	These GPIO pins enable the Hat's TX and RX mixers
		//	rx is enabled and tx and power amp are disabled
		result = (	(gpiod_line_set_value(GPIO_TXRXRelay_line, 0) >= 0)
				&&	(gpiod_line_set_value(GPIO_PWRAMP_ON_line, 0) >= 0)
				&&	(gpiod_line_set_value(GPIO_notTX_line,1) >= 0)
				&	(gpiod_line_set_value(GPIO_notRX_line, 0) >= 0)
				);
		if (!result)
			perror("Setting output lines failed\n");
		}

	//	As of this date libgpiod STILL CANNOT CONTROL PULL UPS!!!!
	//	This is unbelievably STUPID but it's currently the only way to do this
	//	They've been arguing for two years about how to prevent people from
	//	doing this in libgpiod and still are foot dragging about putting it
	//	in Raspbian!
	if (result)
		{
		//	set the needed pullup resistors and set the RTS line mode
		system("raspi-gpio set 24-27 ip pu");
		system("raspi-gpio set 17 op a3");
		}

	if (!result && gChip)
		gpiod_chip_close(gChip);

	return result;
}

/*
*****************************************************************************
*	mcp 23017 I2C gpio output support
*	currently uses hardcoded bit assignments to simplify hardware tests
*****************************************************************************/
const int mc23017_0 = 0x20;
const int IODIRA = 0x00;
const int IODIRB = 0x01;
const int IOCON = 0x05;
const int GPIOA = 0x12;
const int GPIOB = 0x13;
const int OLATA = 0x14;
const int OLATB = 0x15;

int gMC23017fd = 0;

int openI2CGPIO(int busaddr)
{
	// Start I2C comms
   int reg_val = false;
   if (		((gMC23017fd = open("/dev/i2c-1", O_RDWR)) >= 0)
   		&&	(ioctl(gMC23017fd, I2C_SLAVE, busaddr & 0xff) >= 0) )
   		reg_val = true;
	return reg_val;
}

int writeI2CGPIO(uint8_t addr, uint8_t data)
{
uint8_t buf[2];
int result = 0;

	if (gMC23017fd)
		{
		buf[0]=addr; buf[1]=data;
		result = write(gMC23017fd, buf, 2);
		result = (result < 0) ? 0 : 1;
		}
	return result;
}

//	the Linux I2C system driver takes the first byte in the past buffer
//	and uses it to set the register value sent to the I2C device!
int readI2CGPIO(uint8_t addr)
{
	uint8_t reg_val = -1;
	uint8_t buf[2];
	buf[0] = addr;

	if (gMC23017fd)
		{
		if ((write(gMC23017fd, buf, 1) == 1) && ((read(gMC23017fd, buf, 1)) > 0))
			reg_val = buf[0];
		}
	if (reg_val < 0)
		perror("fail in readi2cgpio\n");
	return reg_val;
}


//	the second 8 GPIOS are usually non-latching relays
int setRelay(int theRelay, int theGPIO, int theOLAT, bool value)
{
int portvalue = readI2CGPIO(theOLAT);

	if (portvalue < 0)
		return portvalue;
	else {
		portvalue &= 0X00FF;
		if (value) portvalue |= (1 << theRelay);
		else portvalue &= (~( 1 << theRelay) & 0x00FF);
		return (writeI2CGPIO(theGPIO, portvalue) != 0);
		}
}


//	relay 0 is a reset line to all the latching relays
//	relays 1-7 are mutually exclusive latching relays and must be pulsed
//	this always tries to clear the relay bit, even if the set operation fails
//	All the latching relays are usually on port A.
int pulseRelay(int theRelay)
{
int result = false;
int result2 = false;

	result = setRelay(theRelay, GPIOA, OLATA, true);
	usleep(5000);
	result2 = setRelay(theRelay, GPIOA, OLATA, false);

	return result && result2;
}

//	writes all relay bits at once without disturbing other bits
int writeLPFRelays(uint8_t byteval)
{
int portvalue = readI2CGPIO(OLATB);

	if (portvalue < 0)
		return portvalue;
	else {
		portvalue &= 0xFF70;		//	zero the relay bits
		portvalue |= (byteval & 0x001f);
		return (writeI2CGPIO(GPIOB,portvalue) != 0);
		}
}



void checkRelays(uint32_t frequency, bool nocache)
{
const uint32_t LPF1_LIMIT = 30000000L;
const uint32_t LPF2_LIMIT = 25000000L;
const uint32_t LPF3_LIMIT = 14500000L;
const uint32_t LPF4_LIMIT = 7500000L;
const uint32_t LPF5_LIMIT = 4000000L;

const uint32_t PRE1_LIMIT = 18500000L;
const uint32_t PRE2_LIMIT = 10500000L;
const uint32_t PRE3_LIMIT = 5600000L;

int theLPfilter;
int thePREfilter;
static int lastLPfilter = -1;
static int lastPREfilter = -1;

		if (frequency < LPF5_LIMIT)			theLPfilter = 0x10;
		else if (frequency < LPF4_LIMIT)	theLPfilter = 0x08;
		else if (frequency < LPF3_LIMIT)	theLPfilter = 0x04;
		else if (frequency < LPF2_LIMIT)	theLPfilter = 0x02;
		else theLPfilter = 0x01;
		if (nocache || (lastLPfilter != theLPfilter))
			{
			writeLPFRelays(theLPfilter & 0x001f);
			lastLPfilter = theLPfilter;
			}
	
		if (frequency < PRE3_LIMIT)			thePREfilter = 1;
		else if (frequency < PRE2_LIMIT)	thePREfilter = 2;
		else if (frequency < PRE1_LIMIT)	thePREfilter = 3;
		else thePREfilter = 0;
		if (nocache || (lastPREfilter != thePREfilter))
			{
			pulseRelay(0);
			pulseRelay(thePREfilter);
			lastPREfilter = thePREfilter;
			}
}

void setRelayTXANT(bool txOn)
{	setRelay(7, GPIOB, OLATB, txOn); }

void setRelayPwramp(bool pwrampOn)
{	setRelay(6,  GPIOB, OLATB, pwrampOn);  }

//	these have no connectors or buffers, but they're included to allow
//	experimentation with controlling the Radiohat board from I2C as well.
//	Note that the mixer enables are active LOW!
void setRelayTXMixer(bool txMixerOn)
{	setRelay(7,  GPIOA, OLATA, ! txMixerOn);  }

void setRelayRXMixer(bool rxMixerOn)
{	setRelay(6,  GPIOA, OLATA, ! rxMixerOn); 
}



int initRelays(void)
{
int result = false;
int result2 = false;

	if (	(openI2CGPIO(mc23017_0) != 0)
		&&	(writeI2CGPIO(IOCON,0) != 0)	//	Global config register
		&&	(writeI2CGPIO(IODIRA,0) != 0)	//	dir registers all to output
		&&	(writeI2CGPIO(IODIRB,0) != 0)
		&&	(writeI2CGPIO(GPIOA,0) != 0)
		&&	(writeI2CGPIO(GPIOB,0) != 0)
		&&	pulseRelay(0)					//	send reset pulse to all relays
		) result = true;

	if (result)
		checkRelays(28000000L, true);
	else
		perror("Open mc23017 failed\n");
	return result;
}

int uninitRelays(void)
{
	if (gMC23017fd)
		close(gMC23017fd);
	return true;
}




/*
*****************************************************************************
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

	checkRelays(frequency, nocache);
	
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
				gpioWrite(GPIO_B1Filter_line, 0);	//	filter one enabled
				gpioWrite(GPIO_B2Filter_line, 0);
				gpioWrite(GPIO_B3Filter_line, 0);
				gpioWrite(GPIO_B4Filter_line, 0);
				gpioWrite(GPIO_B5Filter_line, 0);
				break;
			case 2:
				gpioWrite(GPIO_B1Filter_line, 1);	//	filter one disabled
				gpioWrite(GPIO_B2Filter_line, 1);
				gpioWrite(GPIO_B3Filter_line, 0);
				gpioWrite(GPIO_B4Filter_line, 0);
				gpioWrite(GPIO_B5Filter_line, 0);
				break;
			case 3:
				gpioWrite(GPIO_B1Filter_line, 1);	//	filter one disabled
				gpioWrite(GPIO_B2Filter_line, 0);
				gpioWrite(GPIO_B3Filter_line, 1);
				gpioWrite(GPIO_B4Filter_line, 0);
				gpioWrite(GPIO_B5Filter_line, 0);
				break;
			case 4:
				gpioWrite(GPIO_B1Filter_line, 1);	//	filter one disabled
				gpioWrite(GPIO_B2Filter_line, 0);
				gpioWrite(GPIO_B3Filter_line, 0);
				gpioWrite(GPIO_B4Filter_line, 1);
				gpioWrite(GPIO_B5Filter_line, 0);
				break;
			case 5:
				gpioWrite(GPIO_B1Filter_line, 1);	//	filter one disabled
				gpioWrite(GPIO_B2Filter_line, 0);
				gpioWrite(GPIO_B3Filter_line, 0);
				gpioWrite(GPIO_B4Filter_line, 0);
				gpioWrite(GPIO_B5Filter_line, 1);
				break;
			}
		lastfilter = thefilter;		//	update the cache
		}
}


/*
*****************************************************************************
*	TX / RX Switching
****************************************************************************/

//	temporary straight key hack for quisk until CW mode is cleaned up
int isKeyInputActive(void)
{
	return !gpioRead(GPIO_KeyerDAH_line);
}


//	single point to poll for external PTT requests
bool isTXRequested(void)
{
bool request = !gpioRead(GPIO_PTTIn_line)
//			|| !gpioRead(GPIO_SKeyIn_line)
			|| !gpioRead(GPIO_RTS_line);
	return request;
}

//	hardware TX-RX switching handler, also calls audio switch handler
void enableTX(bool txon, cAudioMode mode)
{
float savedDACVol;				//	must restore this because modulator may need to
								//	change it and it is needed by built-in audio
								//	output when receiving

	if (txon)
		{
		savedDACVol = getDACVol();
		setDACVol(0);						//	to prevent a click
		gpioWrite(GPIO_TXRXRelay_line, 1);
		gpioWrite(GPIO_notRX_line, 1);
//		gpioWrite(GPIO_PWRAMP_ON_line, 1);
		gpioWrite(GPIO_notTX_line, 0);
		enableTXAudio(txon, mode);
//		usleep(5000);
		gpioWrite(GPIO_PWRAMP_ON_line, 1);
		setRelayTXANT(true);
		setRelayPwramp(true);
		setRelayTXMixer(true);
		setRelayRXMixer(false);

		}
	else
		{
		enableTXAudio(txon, mode);
		setRelayTXMixer(false);
		setRelayPwramp(false);
		setRelayTXANT(false);
		setRelayRXMixer(true);
		gpioWrite(GPIO_notTX_line, 1);
		gpioWrite(GPIO_PWRAMP_ON_line, 0);
//		usleep(500);
		gpioWrite(GPIO_TXRXRelay_line, 0);
		gpioWrite(GPIO_notRX_line,  0);
		setDACVol(94);
//		setDACVol(savedDACVol);
		}	
}

void checkKeydown(void)
{
static bool keyed;

	if (!gpioRead(GPIO_KeyerDAH_line))
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

	//	at startup, LPF is set to 10 meters
	//	rx is enabled and tx and power amp are disabled
int initControl(void)
{
int result;

	result = (	(initGPIO() == true)
			&&	(initRelays() == true)	);
	if (result)
		enableTX(false, NORMAL_AUDIO);
	return result;
}

void uninitControl(void)
{
	uninitRelays();
	if (gChip)
		gpiod_chip_close(gChip);
}
