
transceiver: transceiver.cpp libradiohat.so
	c++ transceiver.cpp -o transceiver -lncurses -L. -lradiohat -Wl,-rpath,/home/pi/dvl/libradiohat

libradiohat.so:	codec.cpp control.cpp si5351pi.cpp vfo.cpp vswr.cpp *.h
	c++ -c -fPIC control.cpp codec.cpp si5351pi.cpp vfo.cpp vswr.cpp
	c++ -shared control.o codec.o si5351pi.o vfo.o vswr.o -o libradiohat.so -L. -lgpiod
