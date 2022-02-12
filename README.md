![3dView](FAB/radiohead3d.jpg?raw=true "Title")
# radiohat

This is an early version of the folder full of files that need to be installed on a raspberry pi to allow experimentation with the RadioHat version 1.0 hardware.

When downloading and installing it, ensure that it is named "radiohat" and that it is in your home directory, or some of the shell scripts it contains will not work properly. When downloaded as a .zip archive, it may result in a folder with the branch suffix "-main" that needs to be removed if you want to use the enclosed shell scripts.

One way to create the folder on a pi is to "cd" to the directory you want it to appear in and then clone the repository using git with the command: "git clone https://github.com/mpvano/radiohat.git". This will allow you to use all the normal git functionality.

Note also that some of the files in "docs" are very out of date or incomplete. The "Getting Started Guides" refer to a much earlier configuration with different directories on the pi and procedures to set things up. They're presented here only to give some idea what to do. I'll be replacing them with more modern versions as I get to them. RadioHat is still very much a work in progress.

### The RadioHat hardware

Radiohat is an open source peripheral board for the Raspberry Pi in the form of a Pi "hat". It is intended to be a module containing many of the devices that are commonly needed to implement 3-30Mhz radio transmitters and receivers. It may also be used with other micro-controllers that support I2S if they are sufficiently powerful to handle the needed DSP operations.

It contains the following devices:<ol>
	<li>A Freescale SGLT5000 24 bit full duplex 48Khz I2S Audio codec with headset interface</li>
	<li>A Quadrature Switched Demodulator preceded by a 3-32Mhz Band Pass Filter</li>
	<li>A Quadrature Switched Modulator followed by a 32Mhz Low Pass Filter</li>
	<li>An Si5351 I2C controlled clock oscillator configured to provide quadrature clocks for them</li>
	<li>An optional DS3231 Real Time Clock/Calendar device</li>
	<li>The EEPROM required by the Pi Foundation for "hat" devices</li>
	<li>On board analog regulators to provide cleaner power from an optional external source</li>
</ol>

These devices are wired together in the usual way needed for HF radio operation with DSP provided by the Raspberry Pi. For typical operation as a radio transceiver you still need to provide a power amplifier with input and output filters and a transmit/receive switching mechanism.

### Fabrication of boards
Although a small production run was completed to provide a few boards for use by software developers, there is currently no way to obtain boards other than fabricating them yourself.

The RadioHat is open sourced hardware licensed via an MIT license and the Gerber files, preliminary BOM and some draft documentation are all part of the project. The Kicad files for the layout will be part of a later release once they have been edited for clarity.

Note that this board is very compact and built with QFPn and 0603 components. It's not very practical to attempt assembly by hand. The boards need to be reflow soldered after the components are installed with the help of a front side solder paste screen.

### Software
The provided software is a collection of example and test programs that demonstrate operation of the board and allow testing and troubleshooting of it. They are only intended as a starting point for more serious software development.

This repository is newly setup and is still being tested. There may be some issues with missing files, etc. They are being addressed as quickly as possible.

### Dependencies
These are in the course of being documented. In general they are confined to the normal Raspberry Pi build tools, with the exception of ncurses and its development tools.

There's a log file of the updates to one clean Raspbian install that were needed in the "misc" folder that may be helpful in figuring out useful and missing dependencies.

### Mailing list

There is a moderated mailing list for discussing this project at https://groups.io/g/RadioHat. This project's goal is develop radio equipment that requires a valid radio amateur operator's license to operate. Please include your valid station call sign in requests to join the list, and the nature of your interest in the project.

Mario Vano
AE0GL
