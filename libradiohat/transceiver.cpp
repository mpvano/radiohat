/************************************* February 23, 2022 at 11:11:13 AM CST
*****************************************************************************
*
*	A simple example program for a Zero IF Quadrature transceiver
*	controller (DSP done elsewhere) using Si5351 for CLK0 and CLK1.
*
*	Based on ncurses and keyboard for all user IO interaction.
*
*	It accepts keypad based frequency control with left/right arrows
*	to highlight a digit and up/down arrows to adjust by that
*	digit's weight.
*
*	NOTE that this program requires a compatibly mutant version of
*	the great Etherkit Si5351 library modified for the desired
*	target platform - the original library ONLY works on Arduino.
*
*	This is gradually accumulating more bits and pieces for
*	testing the RadioHat board with a prototype full transceiver box.
*
*		Initializes sound card and controls TX-RX mode switching
*		Controls RX and TX GPIO for hat, relay driver, LPF board
* 		and Pwr Amp Controls and displays VSWR A/D readings
*
*		Still needs internal DSP and Audio plumbing setup for digital
*
*		If there is a file named "CALFACTOR.txt in the same directory
*		as the binary program file and if it contains a valid ascii
*		value for the VFO calibration factor, that will be used instead
*		of the default value compiled into the program.
*
*	started by Mario Vano AE0GL, February 2021 (and never finished)
*
******************************************************************************
*****************************************************************************/
#include <ncurses.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "radiohat.h"


//	GLOBALS
bool gLSBMode = false;			//	output polarities switch phase sequence
bool gNeedsVFOUpdate = false;	//	 used in some polled mode apps



//	checks for a file called "CALFACTOR.txt" in same directory where the
//	program is located. If found and if it contains an ascii representation
//	of a signed long int it returns that value, otherwise it returns a default
//	from the vfo header file.
long getCalibrationFactorFromPath(char * progname)
{
long calfactor = CALIBRATION_FACTOR;
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

/*****************************************************************************
*	a simple ncurses frequency control keyboard UI
*
*	controls:
*		Frequency and LPF switching
*		USB/LSB VCO phasing and display
*		TX/RX switching
*		VSWR display
*
*		Volume
*		Digital mode
*		TX Gain
*		Digital TX gain
*
*	need to add:
*
*		Transverter OFFSET
*		OUT OF BAND TX PREVENTION
*		Modes: USB/LSB/NBFM/CW
*		Filter settings
*		S Meter
*		AGC mode
*		Keyer, Key type and Keyer Speed
*		Second VFO
*
*	And need to add support for external keying sources
*	and control via virtual RS232 and CAT.
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
long int rx_freq = getVFO();
int c;
int weight = 2;
int current_column = HZ_COLUMN - weight;
static long int weights[] = { 1L,10L,100L,1000L,10000L,
								100000L,1000000L,10000000L };
const int num_weights = sizeof(weights)/sizeof(*weights);

	//	draw initial display
    move(MENU_LINE,0);
    addstr("[>]Usb [<]Lsb [C]w [F]t8 [T]x [R]x [Q]uit");
    move(GAIN_LINE,0);
    printw("GAIN: [H]p%2.0f [L]o%2.0f [M]ic%2.0f [D]ac%2.0f [A]dc%2.0f a[G]c%2.0f",
			getHpVol(),getLoVol(),getMicVol(),getDACVol(),getADCVol(),getAGCLevel());
	move(TXRX_LINE,0);
	addstr("USB: Receive              ");
	move(HZ_LINE,0);
	printw("VFO %8i",rx_freq);
	move(HZ_LINE,current_column);
	refresh();

	while (true)
		{
		if (cwMode)
			checkKeydown();
		else if (digitalMode)
			{
			transmitter_on = isTXRequested();
			enableTX(transmitter_on, DIGITAL_AUDIO);
			}

    	if ((c = getch()) > 0)
			{
			switch (c)
				{
				case 'A':
				case 'a':
					setADCVol(getADCVol() + (isupper(c) ? 1 : -1));
					volume_changed = true;
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
				case 'd':
					setDACVol(getDACVol() + (isupper(c) ? 1 : -1));
					volume_changed = true;
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
				case 'g':
					if (!transmitter_on)
						{
						setAGCLevel(getAGCLevel() + (isupper(c) ? 1 : -1));
						volume_changed = true;
						}
					break;

				case 'H':
				case 'h':
					if (!transmitter_on)
						{
						setHpVol(getHpVol() + (isupper(c) ? 1 : -1));
						volume_changed = true;
						}
					break;

				case 'L':
				case 'l':
					if (!transmitter_on)
						{
						setLoVol(getLoVol() + (isupper(c) ? 1 : -1));
						volume_changed = true;
						}
					break;

				case 'M':
				case 'm':
					setMicVol(getMicVol() + (isupper(c) ? 1 : -1));
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
				case '>':
					gLSBMode = (c == '<');
					swapPhaseVFO(gLSBMode);
					break;

				case KEY_RIGHT:
					if (weight > 0)
						{
						weight--;
						current_column = HZ_COLUMN - weight;
						}
					break;

				case KEY_LEFT:
					if (weight < (num_weights-1))
						{
						weight++;
						current_column = HZ_COLUMN - weight;
						}
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

			//	limit frequency range entry then change frequency
			if (rx_freq < 3500000L) rx_freq = 3500000L;
			if (rx_freq > 32000000L) rx_freq = 32000000L;
			setVFO(rx_freq);
			checkLPF(rx_freq, true);

			//	update these display fields on all keystrokes
			move(TXRX_LINE,0);
			if (digitalMode)
				addstr(gLSBMode ? "LSBD: " : "USBD: ");
			else if (cwMode)
				addstr(gLSBMode ? "LSBC: " : "USBC: ");
			else
				addstr(gLSBMode ? "LSB:  " : "USB:  ");

			move(TXRX_LINE,5);
				addstr(transmitter_on ? "Transmit "
									  : "Receive              ");

			move(HZ_LINE,0);
				printw("VFO %8i", rx_freq
								+ (cwMode ? (gLSBMode ? -700 : 700) : 0) );

			move(HZ_LINE,current_column);
			refresh();
			}

		//	update these only if something audio related has changed
		if (volume_changed)
			{
    		move(GAIN_LINE,0);
    		printw("GAIN: [H]p%2.0f [L]o%2.0f [M]ic%2.0f [D]ac%2.0f [A]dc%2.0f a[G]c%2.0f",
					getHpVol(),getLoVol(),getMicVol(),getDACVol(),getADCVol(),getAGCLevel());
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

				move(TXRX_LINE,5);
				addstr("Transmit ");
				move(TXRX_LINE,15);
				printw("%1.2fw %1.1f:1 ",(pwr+lastpwr)/2,(swr+lastswr)/2 );

				lastpwr = pwr;
				lastswr = swr;

				move(HZ_LINE,current_column);
				refresh();
				}
			else
				{
				move(TXRX_LINE,5);
				addstr( "Receive              ");
				move(TXRX_LINE,15);
				move(HZ_LINE,current_column);
				refresh();
				}

			//	ugly debugging method (but effective)
			//move(4,0); printw("%s",gAGCEnabled ? "AGC On ": "AGC Off");
			//move(0,0); printw("%s",digitalRead(GPIO_PTTIn) ? "*": " ");
			//move(1,0); printw("%s",digitalRead(GPIO_SKeyIn) ? "*": " ");
			//move(2,0); printw("%s",digitalRead(GPIO_RTS) ? "*": " ");

			move(HZ_LINE,current_column); // always leave at freq display
			}
		}
	return true;
}


/****************************************************************************
*****************************************************************************
*	Main
*
*	Notes:
*		1. The VFO must be initialized before the codec - it provides MCLK.
*		2. The Codec must be initialized before the transceiver controls
*			since they call the codec to set up the startup audio mode
*		3. The VSWR module is not initialized in initonly mode.
*
*****************************************************************************
****************************************************************************/
int main(int argc, char * argv[])
{
bool initonly = (argc == 2) && (*(argv[1]) == '-') && (*(argv[1]+1) == 'i');

//	the string argument is expected to be the program name
const char * exitmsgs[] =
{
"%s: Devices initialized!\n",
"%s: Unable to initialize codec\n",
"%s: Unable to initialize GPIO control\n",
"%s: Unable to initialize VFO\n"
};

	int exitcode = 0;
	long cfactor = getCalibrationFactorFromPath(argv[0]);
	
	if (!initVFO(cfactor, STARTFREQUENCY, SGTL5000_FREQ)) exitcode = 3;
	else if (!initCodec()) exitcode = 1;
	else if (!initControl()) exitcode = 2;
	//	Note that there's no reason to set up the VSWR bridge if
	//	it's not going to be displayed by the loop.

	if (exitcode == 0)
		{
		checkLPF(STARTFREQUENCY, true);
		initVSWR();					//	optional device - ignore errors

		if (!initonly)
			{
			WINDOW * w = initscr();		// ncurses setup
			timeout(1);
			noecho();
			keypad(w,true);
			atexit([]{ endwin(); });
			vfo_interface();			//	loop processing input
			}
		}

	if (initonly || (exitcode != 0))
		fprintf(stderr, exitmsgs[exitcode], *argv);

	return exitcode;
}
