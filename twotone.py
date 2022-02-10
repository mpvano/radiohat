#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Twotone
# Generated: Sun Sep 19 18:32:37 2021
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
from gnuradio import analog
from gnuradio import audio
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio import qtgui
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.qtgui import Range, RangeWidget
from optparse import OptionParser
import sip
import sys
from gnuradio import qtgui


class twotone(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Twotone")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Twotone")
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

        self.settings = Qt.QSettings("GNU Radio", "twotone")
        self.restoreGeometry(self.settings.value("geometry", type=QtCore.QByteArray))


        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 48000
        self.balance = balance = 0
        self.ToneLevel2 = ToneLevel2 = 0.23
        self.ToneLevel = ToneLevel = 0.23
        self.ToneFreq2 = ToneFreq2 = 1900
        self.ToneFreq = ToneFreq = 700

        ##################################################
        # Blocks
        ##################################################
        self._balance_range = Range(-1, +1, .01, 0, 200)
        self._balance_win = RangeWidget(self._balance_range, self.set_balance, 'balance', "counter_slider", float)
        self.top_grid_layout.addWidget(self._balance_win)
        self._ToneLevel2_range = Range(0, 1, .01, 0.23, 200)
        self._ToneLevel2_win = RangeWidget(self._ToneLevel2_range, self.set_ToneLevel2, "ToneLevel2", "counter_slider", float)
        self.top_grid_layout.addWidget(self._ToneLevel2_win)
        self._ToneLevel_range = Range(0, 1, .01, 0.23, 200)
        self._ToneLevel_win = RangeWidget(self._ToneLevel_range, self.set_ToneLevel, "ToneLevel", "counter_slider", float)
        self.top_grid_layout.addWidget(self._ToneLevel_win)
        self._ToneFreq2_range = Range(50, 10000, 100, 1900, 200)
        self._ToneFreq2_win = RangeWidget(self._ToneFreq2_range, self.set_ToneFreq2, "ToneFreq2", "counter_slider", float)
        self.top_grid_layout.addWidget(self._ToneFreq2_win)
        self._ToneFreq_range = Range(50, 10000, 100, 700, 200)
        self._ToneFreq_win = RangeWidget(self._ToneFreq_range, self.set_ToneFreq, "ToneFreq", "counter_slider", float)
        self.top_grid_layout.addWidget(self._ToneFreq_win)
        self.qtgui_time_sink_x_0 = qtgui.time_sink_f(
        	1024, #size
        	samp_rate, #samp_rate
        	"", #name
        	2 #number of inputs
        )
        self.qtgui_time_sink_x_0.set_update_time(0.10)
        self.qtgui_time_sink_x_0.set_y_axis(-1, 1)

        self.qtgui_time_sink_x_0.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_0.enable_tags(-1, True)
        self.qtgui_time_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_0.enable_autoscale(False)
        self.qtgui_time_sink_x_0.enable_grid(False)
        self.qtgui_time_sink_x_0.enable_axis_labels(True)
        self.qtgui_time_sink_x_0.enable_control_panel(False)
        self.qtgui_time_sink_x_0.enable_stem_plot(False)

        if not True:
          self.qtgui_time_sink_x_0.disable_legend()

        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
                  "magenta", "yellow", "dark red", "dark green", "blue"]
        styles = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
                   -1, -1, -1, -1, -1]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in xrange(2):
            if len(labels[i]) == 0:
                self.qtgui_time_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_time_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_0.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_0.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_0.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_0_win = sip.wrapinstance(self.qtgui_time_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_time_sink_x_0_win)
        self.low_pass_filter_0 = filter.fir_filter_ccf(1, firdes.low_pass(
        	1, samp_rate, 3500, 100, firdes.WIN_BLACKMAN, 6.76))
        self.blocks_multiply_const_vxx_1 = blocks.multiply_const_vff((1 + balance/2, ))
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_vff((1 - balance/2, ))
        self.blocks_complex_to_float_0 = blocks.complex_to_float(1)
        self.blocks_add_xx_0 = blocks.add_vcc(1)
        self.audio_sink_0 = audio.sink(samp_rate, 'hw:GenericStereoAu,0', True)
        self.analog_sig_source_x_0_0 = analog.sig_source_c(samp_rate, analog.GR_COS_WAVE, ToneFreq2, ToneLevel2, 0)
        self.analog_sig_source_x_0 = analog.sig_source_c(samp_rate, analog.GR_COS_WAVE, ToneFreq, ToneLevel, 0)



        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_sig_source_x_0, 0), (self.blocks_add_xx_0, 0))
        self.connect((self.analog_sig_source_x_0_0, 0), (self.blocks_add_xx_0, 1))
        self.connect((self.blocks_add_xx_0, 0), (self.low_pass_filter_0, 0))
        self.connect((self.blocks_complex_to_float_0, 1), (self.blocks_multiply_const_vxx_0, 0))
        self.connect((self.blocks_complex_to_float_0, 0), (self.blocks_multiply_const_vxx_1, 0))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.audio_sink_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.qtgui_time_sink_x_0, 0))
        self.connect((self.blocks_multiply_const_vxx_1, 0), (self.audio_sink_0, 1))
        self.connect((self.blocks_multiply_const_vxx_1, 0), (self.qtgui_time_sink_x_0, 1))
        self.connect((self.low_pass_filter_0, 0), (self.blocks_complex_to_float_0, 0))

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "twotone")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.qtgui_time_sink_x_0.set_samp_rate(self.samp_rate)
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate, 3500, 100, firdes.WIN_BLACKMAN, 6.76))
        self.analog_sig_source_x_0_0.set_sampling_freq(self.samp_rate)
        self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate)

    def get_balance(self):
        return self.balance

    def set_balance(self, balance):
        self.balance = balance
        self.blocks_multiply_const_vxx_1.set_k((1 + self.balance/2, ))
        self.blocks_multiply_const_vxx_0.set_k((1 - self.balance/2, ))

    def get_ToneLevel2(self):
        return self.ToneLevel2

    def set_ToneLevel2(self, ToneLevel2):
        self.ToneLevel2 = ToneLevel2
        self.analog_sig_source_x_0_0.set_amplitude(self.ToneLevel2)

    def get_ToneLevel(self):
        return self.ToneLevel

    def set_ToneLevel(self, ToneLevel):
        self.ToneLevel = ToneLevel
        self.analog_sig_source_x_0.set_amplitude(self.ToneLevel)

    def get_ToneFreq2(self):
        return self.ToneFreq2

    def set_ToneFreq2(self, ToneFreq2):
        self.ToneFreq2 = ToneFreq2
        self.analog_sig_source_x_0_0.set_frequency(self.ToneFreq2)

    def get_ToneFreq(self):
        return self.ToneFreq

    def set_ToneFreq(self, ToneFreq):
        self.ToneFreq = ToneFreq
        self.analog_sig_source_x_0.set_frequency(self.ToneFreq)


def main(top_block_cls=twotone, options=None):

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
