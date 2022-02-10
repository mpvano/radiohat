#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Wsjtx1
# Generated: Wed Oct 13 10:56:38 2021
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

from PyQt5 import Qt
from PyQt5 import Qt, QtCore
from gnuradio import audio
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.qtgui import Range, RangeWidget
from optparse import OptionParser
import sys
from gnuradio import qtgui


class WSJTX1(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Wsjtx1")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Wsjtx1")
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

        self.settings = Qt.QSettings("GNU Radio", "WSJTX1")
        self.restoreGeometry(self.settings.value("geometry", type=QtCore.QByteArray))


        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 48000
        self.Volume = Volume = 0.5
        self.LPF = LPF = 50
        self.HPF = HPF = 3000

        ##################################################
        # Blocks
        ##################################################
        self.RXTXTabs = Qt.QTabWidget()
        self.RXTXTabs_widget_0 = Qt.QWidget()
        self.RXTXTabs_layout_0 = Qt.QBoxLayout(Qt.QBoxLayout.TopToBottom, self.RXTXTabs_widget_0)
        self.RXTXTabs_grid_layout_0 = Qt.QGridLayout()
        self.RXTXTabs_layout_0.addLayout(self.RXTXTabs_grid_layout_0)
        self.RXTXTabs.addTab(self.RXTXTabs_widget_0, 'Receiver')
        self.RXTXTabs_widget_1 = Qt.QWidget()
        self.RXTXTabs_layout_1 = Qt.QBoxLayout(Qt.QBoxLayout.TopToBottom, self.RXTXTabs_widget_1)
        self.RXTXTabs_grid_layout_1 = Qt.QGridLayout()
        self.RXTXTabs_layout_1.addLayout(self.RXTXTabs_grid_layout_1)
        self.RXTXTabs.addTab(self.RXTXTabs_widget_1, 'Transmitter')
        self.RXTXTabs_widget_2 = Qt.QWidget()
        self.RXTXTabs_layout_2 = Qt.QBoxLayout(Qt.QBoxLayout.TopToBottom, self.RXTXTabs_widget_2)
        self.RXTXTabs_grid_layout_2 = Qt.QGridLayout()
        self.RXTXTabs_layout_2.addLayout(self.RXTXTabs_grid_layout_2)
        self.RXTXTabs.addTab(self.RXTXTabs_widget_2, 'TX Tone')
        self.top_grid_layout.addWidget(self.RXTXTabs)
        self._Volume_range = Range(0, 1, .01, 0.5, 0)
        self._Volume_win = RangeWidget(self._Volume_range, self.set_Volume, 'Gain', "dial", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._Volume_win, 0, 0, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 1):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self._LPF_range = Range(50, 1000, 10, 50, 200)
        self._LPF_win = RangeWidget(self._LPF_range, self.set_LPF, 'High pass', "counter", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._LPF_win, 1, 0, 1, 1)
        for r in range(1, 2):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 1):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self._HPF_range = Range(300, 3000, 50, 3000, 200)
        self._HPF_win = RangeWidget(self._HPF_range, self.set_HPF, 'Low pass', "counter", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._HPF_win, 1, 1, 1, 1)
        for r in range(1, 2):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(1, 2):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self.hilbert_fc_1_0 = filter.hilbert_fc(301, firdes.WIN_HAMMING, 6.76)
        self.hilbert_fc_1 = filter.hilbert_fc(131, firdes.WIN_HAMMING, 6.76)
        self.hilbert_fc_0 = filter.hilbert_fc(131, firdes.WIN_HAMMING, 6.76)
        self.blocks_throttle_0_0 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate,True)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_float*1, samp_rate,True)
        self.blocks_complex_to_real_0 = blocks.complex_to_real(1)
        self.blocks_complex_to_imag_0 = blocks.complex_to_imag(1)
        self.blocks_complex_to_float_0 = blocks.complex_to_float(1)
        self.blocks_add_xx_0 = blocks.add_vff(1)
        self.band_pass_filter_2 = filter.fir_filter_ccc(1, firdes.complex_band_pass(
        	1, samp_rate, 150, 3000, 50, firdes.WIN_HAMMING, 6.76))
        self.band_pass_filter_1 = filter.interp_fir_filter_fff(1, firdes.band_pass(
        	1, samp_rate, 200, 3000, 100, firdes.WIN_BLACKMAN, 6.76))
        self.band_pass_filter_0 = filter.fir_filter_fff(1, firdes.band_pass(
        	Volume, samp_rate, LPF, HPF, 100, firdes.WIN_HAMMING, 6.76))
        self.audio_source_1 = audio.source(samp_rate, 'fromWSJTXtoGRC', False)
        self.audio_source_0 = audio.source(samp_rate, 'toGRCfromHat', False)
        self.audio_sink_1 = audio.sink(samp_rate, 'toWSJTXfromGRC', False)
        self.audio_sink_0 = audio.sink(samp_rate, 'toHatfromGRC', False)



        ##################################################
        # Connections
        ##################################################
        self.connect((self.audio_source_0, 1), (self.hilbert_fc_0, 0))
        self.connect((self.audio_source_0, 0), (self.hilbert_fc_1, 0))
        self.connect((self.audio_source_1, 0), (self.band_pass_filter_1, 0))
        self.connect((self.band_pass_filter_0, 0), (self.blocks_throttle_0, 0))
        self.connect((self.band_pass_filter_1, 0), (self.hilbert_fc_1_0, 0))
        self.connect((self.band_pass_filter_2, 0), (self.blocks_throttle_0_0, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.band_pass_filter_0, 0))
        self.connect((self.blocks_complex_to_float_0, 1), (self.audio_sink_0, 0))
        self.connect((self.blocks_complex_to_float_0, 0), (self.audio_sink_0, 1))
        self.connect((self.blocks_complex_to_imag_0, 0), (self.blocks_add_xx_0, 1))
        self.connect((self.blocks_complex_to_real_0, 0), (self.blocks_add_xx_0, 0))
        self.connect((self.blocks_throttle_0, 0), (self.audio_sink_1, 0))
        self.connect((self.blocks_throttle_0_0, 0), (self.blocks_complex_to_float_0, 0))
        self.connect((self.hilbert_fc_0, 0), (self.blocks_complex_to_imag_0, 0))
        self.connect((self.hilbert_fc_1, 0), (self.blocks_complex_to_real_0, 0))
        self.connect((self.hilbert_fc_1_0, 0), (self.band_pass_filter_2, 0))

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "WSJTX1")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0_0.set_sample_rate(self.samp_rate)
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)
        self.band_pass_filter_2.set_taps(firdes.complex_band_pass(1, self.samp_rate, 150, 3000, 50, firdes.WIN_HAMMING, 6.76))
        self.band_pass_filter_1.set_taps(firdes.band_pass(1, self.samp_rate, 200, 3000, 100, firdes.WIN_BLACKMAN, 6.76))
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Volume, self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_HAMMING, 6.76))

    def get_Volume(self):
        return self.Volume

    def set_Volume(self, Volume):
        self.Volume = Volume
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Volume, self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_HAMMING, 6.76))

    def get_LPF(self):
        return self.LPF

    def set_LPF(self, LPF):
        self.LPF = LPF
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Volume, self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_HAMMING, 6.76))

    def get_HPF(self):
        return self.HPF

    def set_HPF(self, HPF):
        self.HPF = HPF
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Volume, self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_HAMMING, 6.76))


def main(top_block_cls=WSJTX1, options=None):

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
