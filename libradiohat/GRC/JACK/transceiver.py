#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Transceiver
# GNU Radio version: 3.8.2.0

from distutils.version import StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

import os
import sys
sys.path.append(os.environ.get('GRC_HIER_PATH', os.path.expanduser('~/.grc_gnuradio')))

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio.filter import firdes
import sip
from gnuradio import analog
from gnuradio import audio
from gnuradio import blocks
import pmt
from gnuradio import filter
from gnuradio import gr
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio.qtgui import Range, RangeWidget
from mic_preamp import mic_preamp  # grc-generated hier_block
from ssb_demod import ssb_demod  # grc-generated hier_block
from ssb_mod import ssb_mod  # grc-generated hier_block
import epy_block_0

from gnuradio import qtgui

class transceiver(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Transceiver")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Transceiver")
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

        self.settings = Qt.QSettings("GNU Radio", "transceiver")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Variables
        ##################################################
        self.txbalance = txbalance = 0
        self.samp_rate = samp_rate = 48000
        self.rxbalance = rxbalance = 0
        self.Volume = Volume = 5
        self.VFO = VFO = 7074000
        self.ToneLevel1 = ToneLevel1 = 0.25
        self.ToneLevel = ToneLevel = 0.25
        self.ToneFreq1 = ToneFreq1 = 1900
        self.ToneFreq = ToneFreq = 700
        self.TXTone1 = TXTone1 = 0
        self.TXTone = TXTone = 0
        self.PTT = PTT = 0
        self.MuteMic = MuteMic = 1
        self.LSB = LSB = 1
        self.LPF = LPF = 50
        self.HPF = HPF = 2750
        self.Gain = Gain = 2
        self.CW = CW = 0
        self.ALSA = ALSA = 0

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
        self.RXTXTabs.addTab(self.RXTXTabs_widget_2, 'Graph')
        self.top_grid_layout.addWidget(self.RXTXTabs, 0, 0, 3, 3)
        for r in range(0, 3):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 3):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._txbalance_range = Range(-1, 1, .001, 0, 200)
        self._txbalance_win = RangeWidget(self._txbalance_range, self.set_txbalance, 'balance', "counter", float)
        self.RXTXTabs_grid_layout_1.addWidget(self._txbalance_win, 0, 1, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(1, 2):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)
        self._rxbalance_range = Range(-1, 1, .001, 0, 1)
        self._rxbalance_win = RangeWidget(self._rxbalance_range, self.set_rxbalance, 'balance', "counter", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._rxbalance_win, 2, 2, 1, 1)
        for r in range(2, 3):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(2, 3):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self._Volume_range = Range(0.01, 30, .1, 5, 1)
        self._Volume_win = RangeWidget(self._Volume_range, self.set_Volume, 'Volume', "dial", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._Volume_win, 0, 1, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(1, 2):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self._VFO_range = Range(3500, 32000000, 100, 7074000, 3000)
        self._VFO_win = RangeWidget(self._VFO_range, self.set_VFO, 'VFO', "counter_slider", int)
        self.top_grid_layout.addWidget(self._VFO_win, 3, 2, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(2, 3):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._ToneLevel1_range = Range(0, 1, .01, 0.25, 200)
        self._ToneLevel1_win = RangeWidget(self._ToneLevel1_range, self.set_ToneLevel1, 'Level 1', "counter", float)
        self.RXTXTabs_grid_layout_1.addWidget(self._ToneLevel1_win, 2, 1, 1, 1)
        for r in range(2, 3):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(1, 2):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)
        self._ToneLevel_range = Range(0, 1, .01, 0.25, 200)
        self._ToneLevel_win = RangeWidget(self._ToneLevel_range, self.set_ToneLevel, 'Level 0', "counter", float)
        self.RXTXTabs_grid_layout_1.addWidget(self._ToneLevel_win, 1, 1, 1, 1)
        for r in range(1, 2):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(1, 2):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)
        self._ToneFreq1_range = Range(10, 10000, 10, 1900, 200)
        self._ToneFreq1_win = RangeWidget(self._ToneFreq1_range, self.set_ToneFreq1, 'Freq 1', "counter", float)
        self.RXTXTabs_grid_layout_1.addWidget(self._ToneFreq1_win, 2, 0, 1, 1)
        for r in range(2, 3):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(0, 1):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)
        self._ToneFreq_range = Range(10, 10000, 10, 700, 200)
        self._ToneFreq_win = RangeWidget(self._ToneFreq_range, self.set_ToneFreq, 'Freq 0', "counter", float)
        self.RXTXTabs_grid_layout_1.addWidget(self._ToneFreq_win, 1, 0, 1, 1)
        for r in range(1, 2):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(0, 1):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)
        _TXTone1_check_box = Qt.QCheckBox('Tone 1 to TX')
        self._TXTone1_choices = {True: 1, False: 0}
        self._TXTone1_choices_inv = dict((v,k) for k,v in self._TXTone1_choices.items())
        self._TXTone1_callback = lambda i: Qt.QMetaObject.invokeMethod(_TXTone1_check_box, "setChecked", Qt.Q_ARG("bool", self._TXTone1_choices_inv[i]))
        self._TXTone1_callback(self.TXTone1)
        _TXTone1_check_box.stateChanged.connect(lambda i: self.set_TXTone1(self._TXTone1_choices[bool(i)]))
        self.RXTXTabs_grid_layout_1.addWidget(_TXTone1_check_box, 2, 2, 1, 1)
        for r in range(2, 3):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(2, 3):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)
        _TXTone_check_box = Qt.QCheckBox('Tone 0 to TX')
        self._TXTone_choices = {True: 1, False: 0}
        self._TXTone_choices_inv = dict((v,k) for k,v in self._TXTone_choices.items())
        self._TXTone_callback = lambda i: Qt.QMetaObject.invokeMethod(_TXTone_check_box, "setChecked", Qt.Q_ARG("bool", self._TXTone_choices_inv[i]))
        self._TXTone_callback(self.TXTone)
        _TXTone_check_box.stateChanged.connect(lambda i: self.set_TXTone(self._TXTone_choices[bool(i)]))
        self.RXTXTabs_grid_layout_1.addWidget(_TXTone_check_box, 1, 2, 1, 1)
        for r in range(1, 2):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(2, 3):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)
        _PTT_check_box = Qt.QCheckBox('PTT')
        self._PTT_choices = {True: 1, False: 0}
        self._PTT_choices_inv = dict((v,k) for k,v in self._PTT_choices.items())
        self._PTT_callback = lambda i: Qt.QMetaObject.invokeMethod(_PTT_check_box, "setChecked", Qt.Q_ARG("bool", self._PTT_choices_inv[i]))
        self._PTT_callback(self.PTT)
        _PTT_check_box.stateChanged.connect(lambda i: self.set_PTT(self._PTT_choices[bool(i)]))
        self.top_grid_layout.addWidget(_PTT_check_box, 3, 0, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        _MuteMic_check_box = Qt.QCheckBox('Mute Mic')
        self._MuteMic_choices = {True: 1, False: 0}
        self._MuteMic_choices_inv = dict((v,k) for k,v in self._MuteMic_choices.items())
        self._MuteMic_callback = lambda i: Qt.QMetaObject.invokeMethod(_MuteMic_check_box, "setChecked", Qt.Q_ARG("bool", self._MuteMic_choices_inv[i]))
        self._MuteMic_callback(self.MuteMic)
        _MuteMic_check_box.stateChanged.connect(lambda i: self.set_MuteMic(self._MuteMic_choices[bool(i)]))
        self.RXTXTabs_grid_layout_1.addWidget(_MuteMic_check_box, 0, 2, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(2, 3):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)
        _LSB_check_box = Qt.QCheckBox('Lower Sideband')
        self._LSB_choices = {True: -1, False: 1}
        self._LSB_choices_inv = dict((v,k) for k,v in self._LSB_choices.items())
        self._LSB_callback = lambda i: Qt.QMetaObject.invokeMethod(_LSB_check_box, "setChecked", Qt.Q_ARG("bool", self._LSB_choices_inv[i]))
        self._LSB_callback(self.LSB)
        _LSB_check_box.stateChanged.connect(lambda i: self.set_LSB(self._LSB_choices[bool(i)]))
        self.top_grid_layout.addWidget(_LSB_check_box, 3, 1, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._LPF_range = Range(50, 1000, 10, 50, 200)
        self._LPF_win = RangeWidget(self._LPF_range, self.set_LPF, 'High pass', "counter", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._LPF_win, 1, 0, 1, 1)
        for r in range(1, 2):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 1):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self._HPF_range = Range(300, 3000, 50, 2750, 200)
        self._HPF_win = RangeWidget(self._HPF_range, self.set_HPF, 'Low pass', "counter", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._HPF_win, 1, 1, 1, 1)
        for r in range(1, 2):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(1, 2):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        self._Gain_range = Range(0.03, 5, .05, 2, 1)
        self._Gain_win = RangeWidget(self._Gain_range, self.set_Gain, 'Gain', "dial", float)
        self.RXTXTabs_grid_layout_0.addWidget(self._Gain_win, 0, 0, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 1):
            self.RXTXTabs_grid_layout_0.setColumnStretch(c, 1)
        _CW_check_box = Qt.QCheckBox('CW')
        self._CW_choices = {True: 1, False: 0}
        self._CW_choices_inv = dict((v,k) for k,v in self._CW_choices.items())
        self._CW_callback = lambda i: Qt.QMetaObject.invokeMethod(_CW_check_box, "setChecked", Qt.Q_ARG("bool", self._CW_choices_inv[i]))
        self._CW_callback(self.CW)
        _CW_check_box.stateChanged.connect(lambda i: self.set_CW(self._CW_choices[bool(i)]))
        self.RXTXTabs_grid_layout_1.addWidget(_CW_check_box, 1, 3, 1, 1)
        for r in range(1, 2):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(3, 4):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)
        _ALSA_check_box = Qt.QCheckBox('ALSA')
        self._ALSA_choices = {True: 1, False: 0}
        self._ALSA_choices_inv = dict((v,k) for k,v in self._ALSA_choices.items())
        self._ALSA_callback = lambda i: Qt.QMetaObject.invokeMethod(_ALSA_check_box, "setChecked", Qt.Q_ARG("bool", self._ALSA_choices_inv[i]))
        self._ALSA_callback(self.ALSA)
        _ALSA_check_box.stateChanged.connect(lambda i: self.set_ALSA(self._ALSA_choices[bool(i)]))
        self.RXTXTabs_grid_layout_1.addWidget(_ALSA_check_box, 0, 3, 1, 1)
        for r in range(0, 1):
            self.RXTXTabs_grid_layout_1.setRowStretch(r, 1)
        for c in range(3, 4):
            self.RXTXTabs_grid_layout_1.setColumnStretch(c, 1)
        self.ssb_mod_0 = ssb_mod(
            LSB=LSB,
            PTT=PTT + CW,
            samp_rate=samp_rate,
            txbalance=txbalance,
        )
        self.ssb_demod_0 = ssb_demod(
            LSB=LSB,
            samp_rate=samp_rate,
        )
        self.qtgui_time_sink_x_0 = qtgui.time_sink_f(
            1024, #size
            samp_rate, #samp_rate
            "", #name
            1 #number of inputs
        )
        self.qtgui_time_sink_x_0.set_update_time(0.05)
        self.qtgui_time_sink_x_0.set_y_axis(-1, 1)

        self.qtgui_time_sink_x_0.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_0.enable_tags(False)
        self.qtgui_time_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_0.enable_autoscale(False)
        self.qtgui_time_sink_x_0.enable_grid(True)
        self.qtgui_time_sink_x_0.enable_axis_labels(True)
        self.qtgui_time_sink_x_0.enable_control_panel(False)
        self.qtgui_time_sink_x_0.enable_stem_plot(False)

        self.qtgui_time_sink_x_0.disable_legend()

        labels = ['', '', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['blue', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(1):
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
            1
        )
        self.qtgui_freq_sink_x_0.set_update_time(.1)
        self.qtgui_freq_sink_x_0.set_y_axis(-110, -20)
        self.qtgui_freq_sink_x_0.set_y_label('level', 'dBm')
        self.qtgui_freq_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0.enable_grid(True)
        self.qtgui_freq_sink_x_0.set_fft_average(0.2)
        self.qtgui_freq_sink_x_0.enable_axis_labels(True)
        self.qtgui_freq_sink_x_0.enable_control_panel(False)

        self.qtgui_freq_sink_x_0.disable_legend()


        labels = ['', '', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
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
        self.mic_preamp_1 = mic_preamp(
            ALSA=ALSA,
            MuteMic=MuteMic,
            PTT=PTT,
        )
        self.epy_block_0 = epy_block_0.blk()
        self.blocks_multiply_const_vxx_2 = blocks.multiply_const_ff((1-CW))
        self.blocks_multiply_const_vxx_1 = blocks.multiply_const_ff((1+rxbalance/2))
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_ff((1- rxbalance/2))
        self.blocks_message_strobe_0_0 = blocks.message_strobe(pmt.from_long(PTT), 5)
        self.blocks_message_strobe_0 = blocks.message_strobe(pmt.from_long(VFO), 20)
        self.blocks_float_to_complex_0 = blocks.float_to_complex(1)
        self.blocks_add_xx_3 = blocks.add_vff(1)
        self.blocks_add_xx_2 = blocks.add_vff(1)
        self.blocks_add_xx_1 = blocks.add_vff(1)
        self.band_pass_filter_0 = filter.fir_filter_fff(
            1,
            firdes.band_pass(
                Gain * (1-PTT),
                samp_rate,
                LPF,
                HPF,
                100,
                firdes.WIN_BLACKMAN,
                6.76))
        self.audio_source_1 = audio.source(samp_rate, 'GRC_MIC_INPUT', True)
        self.audio_source_0 = audio.source(samp_rate, 'GRC_IQ_INPUT', False)
        self.audio_sink_1 = audio.sink(48000, 'GRC_CONSOLE_OUT', False)
        self.audio_sink_0 = audio.sink(samp_rate, 'GRC_IQ_OUT', False)
        self.analog_sig_source_x_0_0 = analog.sig_source_f(samp_rate, analog.GR_COS_WAVE, ToneFreq1, ToneLevel1 * TXTone1, 0, 0)
        self.analog_sig_source_x_0 = analog.sig_source_f(samp_rate, analog.GR_COS_WAVE, ToneFreq, (ToneLevel * TXTone)*(1-CW) + (ToneLevel * CW), 0, 0)
        self.analog_agc2_xx_0 = analog.agc2_ff(1e-3, 1e-2, 1.7, 1.0)
        self.analog_agc2_xx_0.set_max_gain(Volume)



        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_message_strobe_0, 'strobe'), (self.epy_block_0, 'freq'))
        self.msg_connect((self.blocks_message_strobe_0_0, 'strobe'), (self.epy_block_0, 'ptt'))
        self.connect((self.analog_agc2_xx_0, 0), (self.audio_sink_1, 0))
        self.connect((self.analog_agc2_xx_0, 0), (self.blocks_multiply_const_vxx_2, 0))
        self.connect((self.analog_sig_source_x_0, 0), (self.blocks_add_xx_1, 1))
        self.connect((self.analog_sig_source_x_0_0, 0), (self.blocks_add_xx_1, 2))
        self.connect((self.audio_source_0, 0), (self.blocks_multiply_const_vxx_0, 0))
        self.connect((self.audio_source_0, 1), (self.blocks_multiply_const_vxx_1, 0))
        self.connect((self.audio_source_0, 1), (self.mic_preamp_1, 0))
        self.connect((self.audio_source_1, 0), (self.mic_preamp_1, 1))
        self.connect((self.band_pass_filter_0, 0), (self.analog_agc2_xx_0, 0))
        self.connect((self.blocks_add_xx_1, 0), (self.ssb_mod_0, 0))
        self.connect((self.blocks_add_xx_2, 0), (self.audio_sink_0, 0))
        self.connect((self.blocks_add_xx_3, 0), (self.audio_sink_0, 1))
        self.connect((self.blocks_float_to_complex_0, 0), (self.qtgui_freq_sink_x_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.blocks_float_to_complex_0, 1))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.ssb_demod_0, 0))
        self.connect((self.blocks_multiply_const_vxx_1, 0), (self.blocks_float_to_complex_0, 0))
        self.connect((self.blocks_multiply_const_vxx_1, 0), (self.ssb_demod_0, 1))
        self.connect((self.blocks_multiply_const_vxx_2, 0), (self.blocks_add_xx_2, 0))
        self.connect((self.blocks_multiply_const_vxx_2, 0), (self.blocks_add_xx_3, 0))
        self.connect((self.mic_preamp_1, 0), (self.blocks_add_xx_1, 0))
        self.connect((self.ssb_demod_0, 0), (self.band_pass_filter_0, 0))
        self.connect((self.ssb_mod_0, 0), (self.blocks_add_xx_2, 1))
        self.connect((self.ssb_mod_0, 1), (self.blocks_add_xx_3, 1))
        self.connect((self.ssb_mod_0, 1), (self.qtgui_time_sink_x_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "transceiver")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_txbalance(self):
        return self.txbalance

    def set_txbalance(self, txbalance):
        self.txbalance = txbalance
        self.ssb_mod_0.set_txbalance(self.txbalance)

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate)
        self.analog_sig_source_x_0_0.set_sampling_freq(self.samp_rate)
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Gain * (1-self.PTT), self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_BLACKMAN, 6.76))
        self.qtgui_freq_sink_x_0.set_frequency_range(0, self.samp_rate)
        self.qtgui_time_sink_x_0.set_samp_rate(self.samp_rate)
        self.ssb_demod_0.set_samp_rate(self.samp_rate)
        self.ssb_mod_0.set_samp_rate(self.samp_rate)

    def get_rxbalance(self):
        return self.rxbalance

    def set_rxbalance(self, rxbalance):
        self.rxbalance = rxbalance
        self.blocks_multiply_const_vxx_0.set_k((1- self.rxbalance/2))
        self.blocks_multiply_const_vxx_1.set_k((1+self.rxbalance/2))

    def get_Volume(self):
        return self.Volume

    def set_Volume(self, Volume):
        self.Volume = Volume
        self.analog_agc2_xx_0.set_max_gain(self.Volume)

    def get_VFO(self):
        return self.VFO

    def set_VFO(self, VFO):
        self.VFO = VFO
        self.blocks_message_strobe_0.set_msg(pmt.from_long(self.VFO))

    def get_ToneLevel1(self):
        return self.ToneLevel1

    def set_ToneLevel1(self, ToneLevel1):
        self.ToneLevel1 = ToneLevel1
        self.analog_sig_source_x_0_0.set_amplitude(self.ToneLevel1 * self.TXTone1)

    def get_ToneLevel(self):
        return self.ToneLevel

    def set_ToneLevel(self, ToneLevel):
        self.ToneLevel = ToneLevel
        self.analog_sig_source_x_0.set_amplitude((self.ToneLevel * self.TXTone)*(1-self.CW) + (self.ToneLevel * self.CW))

    def get_ToneFreq1(self):
        return self.ToneFreq1

    def set_ToneFreq1(self, ToneFreq1):
        self.ToneFreq1 = ToneFreq1
        self.analog_sig_source_x_0_0.set_frequency(self.ToneFreq1)

    def get_ToneFreq(self):
        return self.ToneFreq

    def set_ToneFreq(self, ToneFreq):
        self.ToneFreq = ToneFreq
        self.analog_sig_source_x_0.set_frequency(self.ToneFreq)

    def get_TXTone1(self):
        return self.TXTone1

    def set_TXTone1(self, TXTone1):
        self.TXTone1 = TXTone1
        self._TXTone1_callback(self.TXTone1)
        self.analog_sig_source_x_0_0.set_amplitude(self.ToneLevel1 * self.TXTone1)

    def get_TXTone(self):
        return self.TXTone

    def set_TXTone(self, TXTone):
        self.TXTone = TXTone
        self._TXTone_callback(self.TXTone)
        self.analog_sig_source_x_0.set_amplitude((self.ToneLevel * self.TXTone)*(1-self.CW) + (self.ToneLevel * self.CW))

    def get_PTT(self):
        return self.PTT

    def set_PTT(self, PTT):
        self.PTT = PTT
        self._PTT_callback(self.PTT)
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Gain * (1-self.PTT), self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_BLACKMAN, 6.76))
        self.blocks_message_strobe_0_0.set_msg(pmt.from_long(self.PTT))
        self.mic_preamp_1.set_PTT(self.PTT)
        self.ssb_mod_0.set_PTT(self.PTT + self.CW)

    def get_MuteMic(self):
        return self.MuteMic

    def set_MuteMic(self, MuteMic):
        self.MuteMic = MuteMic
        self._MuteMic_callback(self.MuteMic)
        self.mic_preamp_1.set_MuteMic(self.MuteMic)

    def get_LSB(self):
        return self.LSB

    def set_LSB(self, LSB):
        self.LSB = LSB
        self._LSB_callback(self.LSB)
        self.ssb_demod_0.set_LSB(self.LSB)
        self.ssb_mod_0.set_LSB(self.LSB)

    def get_LPF(self):
        return self.LPF

    def set_LPF(self, LPF):
        self.LPF = LPF
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Gain * (1-self.PTT), self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_BLACKMAN, 6.76))

    def get_HPF(self):
        return self.HPF

    def set_HPF(self, HPF):
        self.HPF = HPF
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Gain * (1-self.PTT), self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_BLACKMAN, 6.76))

    def get_Gain(self):
        return self.Gain

    def set_Gain(self, Gain):
        self.Gain = Gain
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.Gain * (1-self.PTT), self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_BLACKMAN, 6.76))

    def get_CW(self):
        return self.CW

    def set_CW(self, CW):
        self.CW = CW
        self._CW_callback(self.CW)
        self.analog_sig_source_x_0.set_amplitude((self.ToneLevel * self.TXTone)*(1-self.CW) + (self.ToneLevel * self.CW))
        self.blocks_multiply_const_vxx_2.set_k((1-self.CW))
        self.ssb_mod_0.set_PTT(self.PTT + self.CW)

    def get_ALSA(self):
        return self.ALSA

    def set_ALSA(self, ALSA):
        self.ALSA = ALSA
        self._ALSA_callback(self.ALSA)
        self.mic_preamp_1.set_ALSA(self.ALSA)





def main(top_block_cls=transceiver, options=None):

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()

    tb.show()

    def sig_handler(sig=None, frame=None):
        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    def quitting():
        tb.stop()
        tb.wait()

    qapp.aboutToQuit.connect(quitting)
    qapp.exec_()

if __name__ == '__main__':
    main()
