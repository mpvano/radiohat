/*	December 24, 2022 at 6:17:47 AM CST
*****************************************************************************
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
#include <termios.h>

#include "radiohat.h"


//	GLOBALS from other modules (need to be eliminated!)
bool gLSBMode = false;			//	output polarities switch phase sequence
bool gNeedsVFOUpdate = false;	//	 used in some polled mode apps


//	controls stdio echo and line buffering behavior
void setRawConsole(void)
{
struct termios the_settings;

	tcgetattr(STDIN_FILENO,&the_settings);
		the_settings.c_lflag &= (~ICANON & ~ECHO) ;
	tcsetattr(STDIN_FILENO, TCSANOW, &the_settings);
}

void unsetRawConsole(void)
{
struct termios the_settings;

	tcgetattr(STDIN_FILENO,&the_settings);
	the_settings.c_lflag |= (ICANON | ECHO) ;
	tcsetattr(STDIN_FILENO, TCSANOW, &the_settings);
}


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


/*
******************************************************************************
*	A simple CAT transceiver control
*
*	(assumes raw console instead of ncurses)
*	start the program using socat like this (e.g.):
*
*		socat -d -d pty,raw,echo=0 \
*			exec:"/home/pi/radiohat/libradiohat/transceiver -c",pty,raw,setsid
*	
*	The -d options will print the name of the pty on stderr.
*	Note that pty are assigned dynamically in order from 1,
*	so the pty number may vary depending on other pty
*	started ahead of this socat invocation!
*
*	(for debugging, add -v to echo all data to stderr)
*
*	emulates a subset of the Kenwood TS-480 CAT commands
*	
*	Note that most commands send no response at all if args were supplied
*
*****************************************************************************/
bool cat_interface(void)
{
//	saved state of radio
//	kenwood radio modes
enum kKWMODE { KWLSB=1, KWUSB, KWCW, KWFM, KWAM, KWFSK, KWCW_R, KWFSK_R };
bool transmitter_on = false;
bool digitalMode = false;
bool cwMode = false;
long int VFOa = getVFO();
long int VFOb = getVFO();
int RXvfo = 0;		//	0 = VFOa, 1 = VFOb
int TXvfo = 0;
long int RIToffset = 0;
bool RITon = false;
bool XITon = false;
int DSPbw = 0;
int kw_mode = KWUSB;


//	state machine variables
enum CATSTATES { kIdle=0, kWaitForCommand1, kWaitForCommand2, kCollectParams };
char cmdbyte1, cmdbyte2;
const int kMaxCmdParmBytes = 32;
char cmdparms[kMaxCmdParmBytes];
int cmdParmCount = 0;
int cmdParmExpected = 0;
long int convertedParm = 0;
int catstate = kIdle;
int c;

#define CAT_ERROR_REPLY "?;"

	// could set up socat and open port here or let enclosing script do it

	while (true)
		{
		if (cwMode)			//	not presently useful because the loop blocks at getc
			checkKeydown();

		//	should add some kind of long timeout here to facilitate restarting
    	if ((c = getc(stdin)) > 0) switch (catstate)
			{
			case kIdle:
				catstate = kWaitForCommand1;
				cmdParmCount = cmdParmExpected = 0;
				cmdparms[0] = '\0';
				cmdbyte1 = cmdbyte2 = 0;
				convertedParm = 0;
				catstate = kWaitForCommand1;
				//	FALL THROUGH to process the received character

			//	waiting for first command letter
			case kWaitForCommand1:
				if (isalpha(c))
					{
					cmdbyte1 = toupper(c);
					catstate = kWaitForCommand2;
					}
				break;


			//	waiting for second command letter
			case kWaitForCommand2:
				if (isalpha(c))
					{
					cmdbyte2 = toupper(c);
					catstate = kCollectParams;	//	assume success!
					
					//	now check if legal command and set proper expected length
					if ((cmdbyte1 == 'A') && (cmdbyte2=='G'))
						cmdParmExpected = 4;	//	AF Gain
					else if  (cmdbyte1 == 'F')
						{
						switch (cmdbyte2)
							{
							case 'A':	cmdParmExpected = 11; break;	//	vfo A
							case 'B':	cmdParmExpected = 11; break;	//	vfo B
							case 'R':	cmdParmExpected = 1; break;		//	set RX VFO
							case 'T':	cmdParmExpected = 1; break;		//	set TX VFO
							case 'W':	cmdParmExpected = 4; break;		//	set DSP BW
							default:	catstate = kIdle; printf(CAT_ERROR_REPLY); break;
							}
						}
					else if  ((cmdbyte1 == 'I') && (cmdbyte2=='D'))
						cmdParmExpected = 0; 	//	send HW ID command (no args!)
					else if  ((cmdbyte1 == 'I') && (cmdbyte2=='F'))
						cmdParmExpected = 0; 	//	read info command (no args!)
					else if  ((cmdbyte1 == 'M') && (cmdbyte2=='D'))
						cmdParmExpected = 1; 	//	read mode command (no args!)
					else if  (cmdbyte1 == 'R') switch (cmdbyte2)
						{
						case 'D':	cmdParmExpected = 5; break;	//	RIT neg offset
						case 'T':	cmdParmExpected = 1; break;	//	RIT on or off
						case 'U':	cmdParmExpected = 5; break;	//	RIT pos offset
						case 'X':	cmdParmExpected = 0; break;//	RX func status
						default:	catstate = kIdle; printf(CAT_ERROR_REPLY); break;
						}
					else if  ((cmdbyte1 == 'T') && (cmdbyte2=='X'))
						cmdParmExpected = 1; 	//	TX command
					else { catstate = kIdle ; printf(CAT_ERROR_REPLY); } 
					}
				else { catstate = kIdle; printf(CAT_ERROR_REPLY); }
				break;

				
			//	collecting generic params until ';'
			//	if terminator received: validate, parse and execute
			case kCollectParams:	
				if (c != ';')			// we're not done yet
					{
					// must be digit or ';' and command must not exceed expectd length
					if ( (!isdigit(c)) || (cmdParmCount >=  cmdParmExpected) )
						 { catstate = kIdle; printf(CAT_ERROR_REPLY); } 
					else
						{
						cmdparms[cmdParmCount++] = c;
						cmdparms[cmdParmCount] = 0;
						}	//	its an expected digit, just save it and continue
					}
					
				else //	got terminator - parse, execute if we can only reply to queries
					{
					catstate = kIdle;				//	finished, one way or another
					convertedParm = atol(cmdparms);	//	convert from string
					
					// volume setting?
					if ((cmdbyte1 == 'A') && (cmdbyte2=='G'))	//	AF Gain
						{
						int v;
						if (cmdParmCount)
							{
							if (convertedParm <= 255)
								{
								v = (convertedParm * 100) / 255;
								setHpVol(v);
								}
							}
						else {
							v = (getHpVol() * 255)/99;
							printf("%c%c%04i;", cmdbyte1, cmdbyte2, v);
							}
						}

					// VFO command?
					//	for now, always just use VFOa
					else if  (cmdbyte1 == 'F') switch (cmdbyte2)
						{
						case 'A':	//	vfo A
						case 'B':	//	vfo B		//	for now only 1 vfo
							if (cmdParmCount)
								{
								VFOa = VFOb = convertedParm;
								if (VFOa < 3300000L) VFOa = 3300000L;
								if (VFOa > 32000000L) VFOa = 32000000L;
								setVFO(VFOa);
								checkLPF(VFOa, true);
								}
							else printf("%c%c%011li;", cmdbyte1, cmdbyte2, VFOa);
							break;
														
						case 'R':	//	switch RX VFO to VFO A or B
						case 'T':	//	set TX VFO to VFO A or B
							if (cmdParmCount)
								RXvfo = TXvfo = 0;		//	both use VFO A for now
							else printf("%c%c%1i;", cmdbyte1, cmdbyte2, RXvfo);
							break;
												
						// FIXME: The arguments need far more mode dependent checking!!!
						case 'W':	//	set DSP BW
							if (cmdParmCount)
								DSPbw = convertedParm;
							else printf("%c%c%03i;", cmdbyte1,cmdbyte2, DSPbw);
							break;
						
						default:
							catstate = kIdle;
							printf("CAT_ERROR_REPLY"); 
							break;
						}

					// HW ID command?
					//	read-only info command (no args!)
					else if  ((cmdbyte1 == 'I') && (cmdbyte2=='D'))
						printf("%c%c%s;",cmdbyte1, cmdbyte2, "020");
						
					// info command?
					//	read-only info command (no args!)
					//	WE  return	"IF00007074000     +00000000002000000 ;"
					else if  ((cmdbyte1 == 'I') && (cmdbyte2=='F'))
						printf("%c%c%011li     %c%04li%01i%01i000%01i%01i%s;", 
								cmdbyte1, cmdbyte2,
								VFOa, (RIToffset >= 0 ? '+' : '-'),
								abs(RIToffset), RITon, XITon,
								transmitter_on, kw_mode,
								"000000 ");	//	dummies for the rest

					// mode command
					else if  ((cmdbyte1 == 'M') && (cmdbyte2=='D'))
						{
						if (cmdParmCount)
							{
							switch(convertedParm)
								{
								case KWLSB:
									cwMode = false;
									digitalMode = false;
									swapPhaseVFO(true);
									gLSBMode = true;
									break;
									
								case KWUSB:
									cwMode = false;
									digitalMode = false;
									swapPhaseVFO(false);
									gLSBMode = false;
									break;

								case KWCW:
									cwMode = true;
									digitalMode = false;
									swapPhaseVFO(false);
									gLSBMode = false;
									break;

								case KWFM:
								case KWAM:
								case KWFSK:
									digitalMode = false; // true enables DTR keying
									swapPhaseVFO(false);
									gLSBMode = false;
									break;

								case KWCW_R:
									cwMode = false;
									digitalMode = false;
									swapPhaseVFO(true);
									gLSBMode = true;
									break;										

								case KWFSK_R:
									cwMode = false;
									digitalMode = false;
									swapPhaseVFO(true);
									gLSBMode = true;
									break;										
								
								default:
									break;
								}
							kw_mode = convertedParm;
							}
						else printf("%c%c%01i;", cmdbyte1, cmdbyte2, kw_mode);
						}

					// RIT command?
					else if  (cmdbyte1 == 'R') switch (cmdbyte2)
						{
						case 'D':	//	RIT neg offset
							if (cmdParmCount)
								RIToffset = -(convertedParm);
							else if (--RIToffset < -99999)
									RIToffset = -99999;
							break;
						case 'T':	//	RIT on or off
							if (cmdParmCount)
								RITon = (convertedParm == 1);
							else printf("%c%c%01i;", cmdbyte1, cmdbyte2, RITon==1);
							break;
						case 'U':	//	RIT pos offset
							if (cmdParmCount)
								RIToffset = convertedParm;
							else if (++RIToffset > 99999)
									RIToffset = 99999;
							break;
						case 'X':	//	RX func status  transmitter off
							transmitter_on = false;
							enableTX(transmitter_on, NORMAL_AUDIO);
							break;
						default:
							{ catstate = kIdle; printf("CAT_ERROR_REPLY"); }
							break;
						}

					// Transmit command?
					//	note peculiar parameter semantics:
					//		Always transmit regardless of Parm1 presence or value
					//		and always return TX0; regardless of param actually sent
					else if  ((cmdbyte1 == 'T') && (cmdbyte2=='X'))
						{
						transmitter_on = true;
						if (cmdParmCount==0) digitalMode = 0;
						else digitalMode = (cmdparms[0] == '1');
						enableTX(	transmitter_on,
									digitalMode ? DIGITAL_AUDIO : NORMAL_AUDIO);
						}

					// None of the above!
					else { catstate = kIdle; printf(CAT_ERROR_REPLY); }

					fflush(stdout);
					}
				break;
			}	//	of state machine switch statement

		}
	return true;
}



/*
******************************************************************************
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


/*
*****************************************************************************
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
bool catMode =  (argc == 2) && (*(argv[1]) == '-') && (*(argv[1]+1) == 'c');

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
			if (catMode)
				{
				setRawConsole();
				atexit([]{ unsetRawConsole(); });
				fprintf(stderr,"Kenwood TS-480 cat command mode - Ctrl-C to exit\n");
				cat_interface();
				unsetRawConsole();
				}
			else
				{
				WINDOW * w = initscr();		// ncurses setup
				timeout(1);
				noecho();
				keypad(w,true);
				atexit([]{ endwin(); });
				vfo_interface();			//	loop processing input
				endwin();
				}
			}
		}

	if (initonly || (exitcode != 0))
		fprintf(stderr, exitmsgs[exitcode], *argv);

	return exitcode;
}
