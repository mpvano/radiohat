'''
creating a .so and using it in ctypes

    example for libradiohat.cpp and si5351pi.cpp
    19-feb-2022, Mario P. Vano

IMPORTANT (but not obvious):
    * The shared library name in the link command drops the “lib” and the “.so”.
    * The linker must be told where to find the shared library with the -L. option
    * extern “C” { }  must bracket all definitions for access from ctypes


        c++ -c -fPIC libradiohat.cpp si5351pi.cpp
        c++ —shared libradiohat.o si5351pi.o  -o libradiohat.so

    then the following should work:

        gcc -o main main.cpp -L. -lradiohat
 
    and this:
'''

from ctypes import *
libradiohat = CDLL("./libradiohat.so")
print(libradiohat.getVFO())
print(libradiohat.initVFO(0,10000000,12288000))
print(libradiohat.setVFO(10000000))
print(libradiohat.getVFO())
