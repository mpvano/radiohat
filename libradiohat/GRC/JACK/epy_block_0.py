"""
Embedded Python Blocks:

Each time this file is saved, GRC will instantiate the first class it finds
to get ports and parameters of your block. The arguments to __init__  will
be the parameters. All of them are required to have default values!
"""

import numpy as np
from gnuradio import gr
import pmt

from ctypes import *

class blk(gr.basic_block):  # other base classes are basic_block, decim_block, interp_block
    """Embedded Python Block example - a simple multiply const"""

    def __init__(self):
        gr.basic_block.__init__(
            self,
            name='libradiohat',   # will show up in GRC
            in_sig = None,
            out_sig = None
        )
        self.libradiohat = CDLL("/home/pi/radiohat/libradiohat/libradiohat.so")
        self.libradiohat.initVFO(21800,7074000,12288000)
#        self.libradiohat.cachePLLDivisor(False);
        self.libradiohat.setVFO(707400)
        self.libradiohat.initCodec()
        self.libradiohat.setADCVol(c_double(0.5))
        self.libradiohat.initControl()
        self.message_port_register_in(pmt.intern('freq'))
        self.set_msg_handler(pmt.intern('freq'),self.handle_msg)
        self.message_port_register_in(pmt.intern('ptt'))
        self.set_msg_handler(pmt.intern('ptt'),self.handle_msg1)
        self.ptt = 0
        self.VFO = 7074000000
        return
 
    def handle_msg(self,msg):
        self.vfomsg = int(pmt.to_double(msg))
        if (self.VFO != self.vfomsg):
            self.libradiohat.setVFO(self.vfomsg)
            self.libradiohat.checkLPF(self.vfomsg,c_bool(1))
            self.VFO = self.vfomsg
        return
 
    def handle_msg1(self,msg):
        self.pttmsg = pmt.to_long(msg)
        if (self.pttmsg != self.ptt):
            if self.pttmsg == 0:
                self.libradiohat.enableTX(c_bool(0),c_int(0))
            else:
                self.libradiohat.enableTX(c_bool(1),c_int(0))
            self.ptt = self.pttmsg
        else:
            if (self.ptt == 0):
                self.libradiohat.checkKeydown()
        return

#

#    def work(self, input_items, output_items):
#        return 0



