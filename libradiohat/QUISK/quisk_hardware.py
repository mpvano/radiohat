# Define the name of the hardware and the items on the hardware screen (see quisk_conf_defaults.py):
################ Receivers RadioHat, Pi HF transceiver hat
## hardware_file_name		Hardware file path, rfile
# This is the file that contains the control logic for each radio.
#hardware_file_name = 'radiohatpkg/quisk_hardware.py'

from __future__ import print_function
from __future__ import absolute_import
from __future__ import division

import _quisk as QS

from ctypes import *

from quisk_hardware_model import Hardware as BaseHardware


class Hardware(BaseHardware):
    mode = ""
    
    def __init__(self, app, conf):
        BaseHardware.__init__(self, app, conf)

    def pre_open(self):  # Quisk calls this once before open() is called
        pass

    def open(self):  # Quisk calls this once to open the Hardware
        self.libradiohat = CDLL("/home/pi/dvl/libradiohat/libradiohat.so")
        if self.libradiohat == 0:
            t = "Can't find LibRadiohat!!"
        else:
            self.libradiohat.initVFO(21800,7074000,12288000)
            self.libradiohat.setVFO(707400)
            self.libradiohat.initCodec()
            self.libradiohat.initControl()
            self.libradiohat.setADCVol(c_float(0.5))
            t = "Found RadioHat extension."
        return t

    def post_open(
        self,
    ):  # Quisk calls this once after open() and after sound is started
        pass

    def close(self):  # Quisk calls this once to close the Hardware
        pass

    def ChangeFrequency(self, tune, vfo, source="", band="", event=None):
        # Change and return the tuning and VFO frequency in Hertz.  The VFO frequency is the
        # frequency in the center of the display; that is, the RF frequency corresponding to an
        # audio frequency of zero Hertz.  The tuning frequency is the RF frequency indicated by
        # the tuning line on the display, and is equivalent to the transmit frequency.  The quisk
        # receive frequency is the tuning frequency plus the RIT (receive incremental tuning).
        # If your hardware will not change to the requested frequencies, return different
        # frequencies.
        # The source is a string indicating the source of the change:
        #   BtnBand       A band button
        #   BtnUpDown     The band Up/Down buttons
        #   FreqEntry     The user entered a frequency in the box
        #   MouseBtn1     Left mouse button press
        #   MouseBtn3     Right mouse button press
        #   MouseMotion   The user is dragging with the left button
        #   MouseWheel    The mouse wheel up/down
        #   NewDecim      The decimation changed
        # For "BtnBand", the string band is in the band argument.
        # For the mouse events, the handler event is in the event argument.
        self.libradiohat.setVFO(tune)
        return tune, tune

    def ReturnFrequency(self):
        # Return the current tuning and VFO frequency.  If neither have changed,
        # you can return (None, None).  This is called at about 10 Hz by the main.
        # return (tune, vfo)	# return changed frequencies
        t = self.libradiohat.getVFO()
        return t, t  # frequencies have not changed

    def ReturnVfoFloat(self):
        # Return the accurate VFO frequency as a floating point number.
        # You can return None to indicate that the integer VFO frequency is valid.
        return None

    def ChangeMode(self, mode):  # Change the tx/rx mode
        # mode is a string: "USB", "AM", etc.
        self.mode = mode
        pass

    def ChangeBand(self, band):
        # band is a string: "60", "40", "WWV", etc.
        try:
            self.transverter_offset = self.conf.bandTransverterOffset[band]
        except:
            self.transverter_offset = 0
        pass

    def OnButtonPTT(self, event):
        pass

    def OnBtnFDX(self, is_fdx):  # Status of FDX button, 0 or 1
        pass

    def HeartBeat(self):  # Called at about 10 Hz by the main
        pass


    def OnChangeRxTx(self, is_tx):	# Called by Quisk when changing between Rx and Tx. "is_tx" is 0 or 1
  	    self.libradiohat.enableTX(is_tx, 0);
  	    pass




    def PollCwKey(self):  # Called frequently by Quisk to check the CW key status
        if self.mode not in ('CWU', 'CWL'):
            return
        if (self.libradiohat.isKeyInputActive() != 0):
            QS.set_cwkey(1)
        else:			# key is up
            QS.set_cwkey(0)
        pass

