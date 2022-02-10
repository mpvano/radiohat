#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Builtin Cw
# Generated: Thu Oct 14 07:17:24 2021
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


class BUILTIN_CW(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Builtin Cw")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Builtin Cw")
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

        self.settings = Qt.QSettings("GNU Radio", "BUILTIN_CW")
        self.restoreGeometry(self.settings.value("geometry", type=QtCore.QByteArray))


        ##################################################
        # Variables
        ##################################################
        self.txbalance = txbalance = 0 -.015
        self.samp_rate = samp_rate = 48000
        self.agcGain = agcGain = 1
        self.Volume = Volume = 0.1
        self.ToneLevel = ToneLevel = 0.25
        self.ToneFreq = ToneFreq = 700
        self.TXTone = TXTone = 1
        self.MuteMic = MuteMic = 1
        self.LPF = LPF = 400
        self.HPF = HPF = 1200

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
        self._txbalance_range = Range(-1, +1, .005, 0 -.015, 1)
        self._txbalance_win = RangeWidget(self._txbalance_range, self.set_txbalance, 'txbalance', "counter", float)
        self.RXTXTabs_grid_layout_2.addWidget(self._txbalance_win, 1, 0, 1, 1)
        for r in range(1, 2):
            self.RXTXTabs_grid_layout_2.setRowStretch(r, 1)
        for c in range(0, 1):
            self.RXTXTabs_grid_layout_2.setColumnStretch(c, 1)
        self._agcGain_range = Range(0.03, 5, .01, 1, 5)
        self._agcGain_win = RangeWidget(self._agcGain_range, self.set_agcGain, 'Volume', "dial", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._agcGain_win, 0, 1, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(1, 2):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self._Volume_range = Range(0, 0.5, .01, 0.1, 0)
        self._Volume_win = RangeWidget(self._Volume_range, self.set_Volume, 'Gain', "dial", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._Volume_win, 0, 0, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 1):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self._ToneLevel_range = Range(0, 1, .01, 0.25, 1)
        self._ToneLevel_win = RangeWidget(self._ToneLevel_range, self.set_ToneLevel, 'Tone Level', "counter", float)
        self.RXTXTabs_grid_layout_2.addWidget(self._ToneLevel_win, 0, 0, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_2.setRowStretch(r, 1)
        for c in range(0, 1):
            self.RXTXTabs_grid_layout_2.setColumnStretch(c, 1)
        self._ToneFreq_range = Range(10, 10000, 10, 700, 1)
        self._ToneFreq_win = RangeWidget(self._ToneFreq_range, self.set_ToneFreq, 'Tone Freq', "counter", float)
        self.RXTXTabs_grid_layout_2.addWidget(self._ToneFreq_win, 0, 1, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_2.setRowStretch(r, 1)
        for c in range(1, 2):
            self.RXTXTabs_grid_layout_2.setColumnStretch(c, 1)
        _TXTone_check_box = Qt.QCheckBox('Tone to TX')
        self._TXTone_choices = {True: 1, False: 0}
        self._TXTone_choices_inv = dict((v,k) for k,v in self._TXTone_choices.iteritems())
        self._TXTone_callback = lambda i: Qt.QMetaObject.invokeMethod(_TXTone_check_box, "setChecked", Qt.Q_ARG("bool", self._TXTone_choices_inv[i]))
        self._TXTone_callback(self.TXTone)
        _TXTone_check_box.stateChanged.connect(lambda i: self.set_TXTone(self._TXTone_choices[bool(i)]))
        self.RXTXTabs_grid_layout_2.addWidget(_TXTone_check_box, 0, 3, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_2.setRowStretch(r, 1)
        for c in range(3, 4):
            self.RXTXTabs_grid_layout_2.setColumnStretch(c, 1)
        self._LPF_range = Range(50, 600, 10, 400, 200)
        self._LPF_win = RangeWidget(self._LPF_range, self.set_LPF, 'High pass', "counter", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._LPF_win, 1, 0, 1, 1)
        for r in range(1, 2):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 1):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self._HPF_range = Range(700, 3000, 50, 1200, 200)
        self._HPF_win = RangeWidget(self._HPF_range, self.set_HPF, 'Low pass', "counter", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._HPF_win, 1, 1, 1, 1)
        for r in range(1, 2):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(1, 2):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self.qtgui_time_sink_x_0 = qtgui.time_sink_f(
        	1024, #size
        	samp_rate, #samp_rate
        	"", #name
        	1 #number of inputs
        )
        self.qtgui_time_sink_x_0.set_update_time(0.05)
        self.qtgui_time_sink_x_0.set_y_axis(-1, 1)

        self.qtgui_time_sink_x_0.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_0.enable_tags(-1, False)
        self.qtgui_time_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_0.enable_autoscale(False)
        self.qtgui_time_sink_x_0.enable_grid(True)
        self.qtgui_time_sink_x_0.enable_axis_labels(True)
        self.qtgui_time_sink_x_0.enable_control_panel(False)
        self.qtgui_time_sink_x_0.enable_stem_plot(False)

        if not False:
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

        for i in xrange(1):
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
        self.RXTXTabs_grid_layout_1.addWidget(self._qtgui_time_sink_x_0_win, 0, 0, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(0, 1):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)
        self.qtgui_freq_sink_x_0 = qtgui.freq_sink_c(
        	1024, #size
        	firdes.WIN_BLACKMAN_hARRIS, #wintype
        	0, #fc
        	samp_rate, #bw
        	"", #name
        	1 #number of inputs
        )
        self.qtgui_freq_sink_x_0.set_update_time(0.10)
        self.qtgui_freq_sink_x_0.set_y_axis(-110, -20)
        self.qtgui_freq_sink_x_0.set_y_label('level', 'dBm')
        self.qtgui_freq_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0.enable_grid(True)
        self.qtgui_freq_sink_x_0.set_fft_average(0.2)
        self.qtgui_freq_sink_x_0.enable_axis_labels(True)
        self.qtgui_freq_sink_x_0.enable_control_panel(False)

        if not False:
          self.qtgui_freq_sink_x_0.disable_legend()

        if "complex" == "float" or "complex" == "msg_float":
          self.qtgui_freq_sink_x_0.set_plot_pos_half(not True)

        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
                  1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
                  "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]
        for i in xrange(1):
            if len(labels[i]) == 0:
                self.qtgui_freq_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_0.set_line_alpha(i, alphas[i])

        self._qtgui_freq_sink_x_0_win = sip.wrapinstance(self.qtgui_freq_sink_x_0.pyqwidget(), Qt.QWidget)
        self.RXTXTabs_grid_layout_0.addWidget(self._qtgui_freq_sink_x_0_win, 2, 0, 1, 2)
        for r in range(2, 3):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 2):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self.hilbert_fc_1_0 = filter.hilbert_fc(301, firdes.WIN_HAMMING, 6.76)
        self.hilbert_fc_1 = filter.hilbert_fc(131, firdes.WIN_HAMMING, 6.76)
        self.hilbert_fc_0 = filter.hilbert_fc(131, firdes.WIN_HAMMING, 6.76)
        self.blocks_multiply_const_vxx_1 = blocks.multiply_const_vff((1 + txbalance/2, ))
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_vff((1 - txbalance/2, ))
        self.blocks_float_to_complex_0 = blocks.float_to_complex(1)
        self.blocks_complex_to_real_0 = blocks.complex_to_real(1)
        self.blocks_complex_to_imag_0 = blocks.complex_to_imag(1)
        self.blocks_complex_to_float_0 = blocks.complex_to_float(1)
        self.blocks_add_xx_0 = blocks.add_vff(1)
        self.band_pass_filter_2 = filter.fir_filter_ccf(1, firdes.band_pass(
        	1, samp_rate, 100, 3000, 50, firdes.WIN_HAMMING, 6.76))
        self.band_pass_filter_1 = filter.interp_fir_filter_fff(1, firdes.band_pass(
        	1, samp_rate, 100, 3000, 100, firdes.WIN_BLACKMAN, 6.76))
        self.band_pass_filter_0 = filter.fir_filter_fff(1, firdes.band_pass(
        	Volume, samp_rate, LPF, HPF, 30, firdes.WIN_HAMMING, 6.76))
        self.audio_source_0 = audio.source(samp_rate, 'hw:GenericStereoAu,1', True)
        self.audio_sink_1 = audio.sink(samp_rate, 'hw:Headphones,0', False)
        self.audio_sink_0 = audio.sink(samp_rate, 'hw:GenericStereoAu,0', True)
        self.analog_sig_source_x_0 = analog.sig_source_f(samp_rate, analog.GR_COS_WAVE, ToneFreq, ToneLevel * TXTone, 0)
        self.analog_agc2_xx_0 = analog.agc2_ff(1e-3, 1e-2, 1.7, 1)
        self.analog_agc2_xx_0.set_max_gain(agcGain)
        _MuteMic_check_box = Qt.QCheckBox('Mute Mic')
        self._MuteMic_choices = {True: 1, False: 0}
        self._MuteMic_choices_inv = dict((v,k) for k,v in self._MuteMic_choices.iteritems())
        self._MuteMic_callback = lambda i: Qt.QMetaObject.invokeMethod(_MuteMic_check_box, "setChecked", Qt.Q_ARG("bool", self._MuteMic_choices_inv[i]))
        self._MuteMic_callback(self.MuteMic)
        _MuteMic_check_box.stateChanged.connect(lambda i: self.set_MuteMic(self._MuteMic_choices[bool(i)]))
        self.RXTXTabs_grid_layout_1.addWidget(_MuteMic_check_box, 0, 2, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(2, 3):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)



        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_agc2_xx_0, 0), (self.audio_sink_1, 0))
        self.connect((self.analog_agc2_xx_0, 0), (self.audio_sink_1, 1))
        self.connect((self.analog_sig_source_x_0, 0), (self.band_pass_filter_1, 0))
        self.connect((self.audio_source_0, 0), (self.blocks_float_to_complex_0, 1))
        self.connect((self.audio_source_0, 1), (self.blocks_float_to_complex_0, 0))
        self.connect((self.audio_source_0, 1), (self.hilbert_fc_0, 0))
        self.connect((self.audio_source_0, 0), (self.hilbert_fc_1, 0))
        self.connect((self.band_pass_filter_0, 0), (self.analog_agc2_xx_0, 0))
        self.connect((self.band_pass_filter_1, 0), (self.hilbert_fc_1_0, 0))
        self.connect((self.band_pass_filter_2, 0), (self.blocks_complex_to_float_0, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.band_pass_filter_0, 0))
        self.connect((self.blocks_complex_to_float_0, 1), (self.blocks_multiply_const_vxx_0, 0))
        self.connect((self.blocks_complex_to_float_0, 0), (self.blocks_multiply_const_vxx_1, 0))
        self.connect((self.blocks_complex_to_float_0, 0), (self.qtgui_time_sink_x_0, 0))
        self.connect((self.blocks_complex_to_imag_0, 0), (self.blocks_add_xx_0, 1))
        self.connect((self.blocks_complex_to_real_0, 0), (self.blocks_add_xx_0, 0))
        self.connect((self.blocks_float_to_complex_0, 0), (self.qtgui_freq_sink_x_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.audio_sink_0, 0))
        self.connect((self.blocks_multiply_const_vxx_1, 0), (self.audio_sink_0, 1))
        self.connect((self.hilbert_fc_0, 0), (self.blocks_complex_to_imag_0, 0))
        self.connect((self.hilbert_fc_1, 0), (self.blocks_complex_to_real_0, 0))
        self.connect((self.hilbert_fc_1_0, 0), (self.band_pass_filter_2, 0))

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "BUILTIN_CW")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_txbalance(self):
        return self.txbalance

    def set_txbalance(self, txbalance):
        self.txbalance = txbalance
        self.blocks_multiply_const_vxx_1.set_k((1 + self.txbalance/2, ))
        self.blocks_multiply_const_vxx_0.set_k((1 - self.txbalance/2, ))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.qtgui_time_sink_x_0.set_samp_rate(self.samp_rate)
        self.qtgui_freq_sink_x_0.set_frequency_range(0, self.samp_rate)
        self.band_pass_filter_2.set_taps(firdes.band_pass(1, self.samp_rate, 100, 3000, 50, firdes.WIN_HAMMING, 6.76))
        self.band_pass_filter_1.set_taps(firdes.band_pass(1, self.samp_rate, 100, 3000, 100, firdes.WIN_BLACKMAN, 6.76))
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Volume, self.samp_rate, self.LPF, self.HPF, 30, firdes.WIN_HAMMING, 6.76))
        self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate)

    def get_agcGain(self):
        return self.agcGain

    def set_agcGain(self, agcGain):
        self.agcGain = agcGain
        self.analog_agc2_xx_0.set_max_gain(self.agcGain)

    def get_Volume(self):
        return self.Volume

    def set_Volume(self, Volume):
        self.Volume = Volume
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Volume, self.samp_rate, self.LPF, self.HPF, 30, firdes.WIN_HAMMING, 6.76))

    def get_ToneLevel(self):
        return self.ToneLevel

    def set_ToneLevel(self, ToneLevel):
        self.ToneLevel = ToneLevel
        self.analog_sig_source_x_0.set_amplitude(self.ToneLevel * self.TXTone)

    def get_ToneFreq(self):
        return self.ToneFreq

    def set_ToneFreq(self, ToneFreq):
        self.ToneFreq = ToneFreq
        self.analog_sig_source_x_0.set_frequency(self.ToneFreq)

    def get_TXTone(self):
        return self.TXTone

    def set_TXTone(self, TXTone):
        self.TXTone = TXTone
        self._TXTone_callback(self.TXTone)
        self.analog_sig_source_x_0.set_amplitude(self.ToneLevel * self.TXTone)

    def get_MuteMic(self):
        return self.MuteMic

    def set_MuteMic(self, MuteMic):
        self.MuteMic = MuteMic
        self._MuteMic_callback(self.MuteMic)

    def get_LPF(self):
        return self.LPF

    def set_LPF(self, LPF):
        self.LPF = LPF
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Volume, self.samp_rate, self.LPF, self.HPF, 30, firdes.WIN_HAMMING, 6.76))

    def get_HPF(self):
        return self.HPF

    def set_HPF(self, HPF):
        self.HPF = HPF
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Volume, self.samp_rate, self.LPF, self.HPF, 30, firdes.WIN_HAMMING, 6.76))


def main(top_block_cls=BUILTIN_CW, options=None):

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
