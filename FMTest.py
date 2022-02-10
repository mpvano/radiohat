#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Fmtest
# Generated: Thu Jul  8 16:22:48 2021
##################################################

from distutils.version import StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

from PyQt5 import Qt, QtCore
from gnuradio import analog
from gnuradio import audio
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import sys
from gnuradio import qtgui


class FMTest(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Fmtest")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Fmtest")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "FMTest")
        self.restoreGeometry(self.settings.value("geometry", type=QtCore.QByteArray))


        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 48000

        ##################################################
        # Blocks
        ##################################################
        self.blocks_multiply_const_xx_0 = blocks.multiply_const_ff(0.1)
        self.blocks_float_to_complex_0 = blocks.float_to_complex(1)
        self.audio_source_0 = audio.source(samp_rate, 'hw:GenericStereoAu,1', True)
        self.audio_sink_0 = audio.sink(samp_rate, 'hw:Audio,0', True)
        self.analog_nbfm_rx_0 = analog.nbfm_rx(
        	audio_rate=samp_rate,
        	quad_rate=samp_rate,
        	tau=75e-6,
        	max_dev=5e3,
          )
        self.analog_ctcss_squelch29_620m = analog.ctcss_squelch_ff(samp_rate, 146.2, 0.01, 0, 0, False)



        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_ctcss_squelch29_620m, 0), (self.blocks_multiply_const_xx_0, 0))
        self.connect((self.analog_nbfm_rx_0, 0), (self.analog_ctcss_squelch29_620m, 0))
        self.connect((self.audio_source_0, 0), (self.blocks_float_to_complex_0, 0))
        self.connect((self.audio_source_0, 1), (self.blocks_float_to_complex_0, 1))
        self.connect((self.blocks_float_to_complex_0, 0), (self.analog_nbfm_rx_0, 0))
        self.connect((self.blocks_multiply_const_xx_0, 0), (self.audio_sink_0, 0))

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "FMTest")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate


def main(top_block_cls=FMTest, options=None):

    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()
    tb.start()
    tb.show()

    def quitting():
        tb.stop()
        tb.wait()
    qapp.aboutToQuit.connect(quitting)
    qapp.exec_()


if __name__ == '__main__':
    main()
