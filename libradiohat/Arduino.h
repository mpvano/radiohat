#ifndef __Arduino_h
#define __Arduino_h	

#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;  

#define F(x) x

//#define delay( x ) usleep( (long)x * 1000 )

#define println( x ) cout << x << endl

#define print( x ) cout << (x)

#define printh( x ) cout << (int)(x)
#define printlnh( x ) cout << (int)(x) << endl
#endif

