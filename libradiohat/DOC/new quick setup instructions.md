## A quick start list for Bullseye (13-March-2022) ##
	
### 1. INSTALL packages we'll need later ###
	sudo apt-get install libncurses5-dev libgpiod-dev libx11-dev gnuradio quisk audacity

### 2. Clone the repository ###
	cd
	git clone https://github.com/mpvano/radiohat.git

###	3. BUILD library and applications ###
	cd ~/radiohat/libradiohat
	make clean
	make

###	4. BUILD and install the device overlay ###
	cd ~/radiohat/libradiohat/radiohatcodec
	#Compile it:
	dtc -@ -H epapr -O dtb -o radiohatcodec.dtbo -Wno-unit_address_vs_reg radiohatcodec.dts
	#Copy radiohatcodec.dtbo to /boot/overlays
	sudo cp radiohatcodec.dtbo /boot/overlays

###	5. APPEND this stuff to /boot/config.txt ###
	# *RadioHat* 1.0
	#for the optional rtc
	#dtoverlay=i2c-rtc,ds3231,addr=0x68
	dtparam=i2c=on
	dtparam=i2s=on
	dtoverlay=RadioHatCodec
	# initialize the *RadioHat* GPIO
	gpio=22,23=op,dh
	gpio=17=a3

###	6. Power Down, Install card (PI power is OK for now), POWER UP ###

###	7. Fixup GRC files ###
	#Delete caches
	#open all module files and top level files
	#Build flow graph for each

###	8. Do a minimal Quisk install ###
	#	start quisk
	#	make a new radio called radiohat (or whatever) of type Softrock Fixed
	#	point the configuration file to
	#		/home/pi/radiohat/libradiohat/QUISK/quisk-hardware.py
	#	configure the console outand in as pulse default
	#	configure the radio IQ in and out as RadioHatCodec
	#	swap I and Q on the RadioHatCodec inputs and outputs
	#	be aware of problems with Pulse Audio in Quisk

###	9. MISC Fixups ###
	#	link /usr/lxterminal /usr/xterm
