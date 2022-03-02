/************************************* February 24, 2022 at 8:42:13 PM CST
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
#include <gpiod.h>
#include "control.h"
#include "codec.h"


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


/****************************************************************************
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
	if (txon)
		{
		gpioWrite(GPIO_TXRXRelay_line, 1);
		gpioWrite(GPIO_notRX_line, 1);
//		gpioWrite(GPIO_PWRAMP_ON_line, 1);
		gpioWrite(GPIO_notTX_line, 0);
		enableTXAudio(txon, mode);
		usleep(5000);
		gpioWrite(GPIO_PWRAMP_ON_line, 1);
		}
	else
		{
		enableTXAudio(txon, mode);
		gpioWrite(GPIO_notTX_line, 1);
		gpioWrite(GPIO_PWRAMP_ON_line, 0);
		usleep(500);
		gpioWrite(GPIO_TXRXRelay_line, 0);
		gpioWrite(GPIO_notRX_line,  0);
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
int result =  initGPIO();
	if (result)
		enableTX(false, NORMAL_AUDIO);
	return result;
}

void uninitControl(void)
{
	if (gChip)
		gpiod_chip_close(gChip);
}
