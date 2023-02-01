from time import sleep
from ctypes import *

vswrdac = CDLL('/home/pi/radiohat/libradiohat/libradiohat.so')
print(vswrdac.initVSWR())

readForwardOnly = vswrdac.readForwardOnly
readVSWROnly = vswrdac.readVSWROnly
readADCRaw = vswrdac.readADCRaw

readForwardOnly.restype = c_float
readVSWROnly.restype = c_float
readADCRaw.restype = c_float

while True:
    power = readForwardOnly()
    sleep(0.01)
    vswr = readVSWROnly()
    sleep(0.01)
    Battery = 10 * readADCRaw(1,0)
    
    print('%2.2f Watts %2.2f:1 SWR    %2.2f Volts' % ( power, vswr, Battery))
    sleep(1)
