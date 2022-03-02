#include <stdio.h>
#include "libradiohat.h"

int main(int argc, char * argv[])
{
int result;
	result =  initVFO((long)CALIBRATION_FACTOR,
					(unsigned int)STARTFREQUENCY, 
					(unsigned int)SGTL5000_FREQ);
	if (result)
		result = initCodec();
	if (result)
		result = initControl();
	enableTX(true, NORMAL_AUDIO);
	if (result)
		result = initVSWR();
	
	printf(".... test main result = %d\n", result);
	enableTX(false, NORMAL_AUDIO);
	return result;
}
