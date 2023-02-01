#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Usb
# GNU Radio version: 3.8.2.0

from gnuradio import audio
from gnuradio import blocks
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation


class usb(gr.top_block):

    def __init__(self, GAIN=0.4, HPF=3000, LPF=100, RXSINK='', Sideband=1, TXSOURCE='plughw:CARD=Loopback,DEV=1', parameter_0=0):
        gr.top_block.__init__(self, "Usb")

        ##################################################
        # Parameters
        ##################################################
        self.GAIN = GAIN
        self.HPF = HPF
        self.LPF = LPF
        self.RXSINK = RXSINK
        self.Sideband = Sideband
        self.TXSOURCE = TXSOURCE
        self.parameter_0 = parameter_0

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 48000

        ##################################################
        # Blocks
        ##################################################
        self.low_pass_filter_0_0 = filter.fir_filter_fff(
            1,
            firdes.low_pass(
                1,
                samp_rate,
                3000,
                100,
                firdes.WIN_KAISER,
                6.76))
        self.low_pass_filter_0 = filter.fir_filter_fff(
            1,
            firdes.low_pass(
                1,
                samp_rate,
                3000,
                100,
                firdes.WIN_KAISER,
                6.76))
        self.hilbert_fc_1_0 = filter.hilbert_fc(201, firdes.WIN_BLACKMAN_hARRIS, 6.76)
        self.hilbert_fc_1 = filter.hilbert_fc(201, firdes.WIN_BLACKMAN_hARRIS, 6.76)
        self.hilbert_fc_0 = filter.hilbert_fc(201, firdes.WIN_BLACKMAN_hARRIS, 6.76)
        self.blocks_multiply_const_vxx_0_0_1 = blocks.multiply_const_ff(Sideband)
        self.blocks_multiply_const_vxx_0_0_0_0 = blocks.multiply_const_ff(1)
        self.blocks_multiply_const_vxx_0_0_0 = blocks.multiply_const_ff(1)
        self.blocks_multiply_const_vxx_0_0 = blocks.multiply_const_ff(Sideband)
        self.blocks_complex_to_real_1 = blocks.complex_to_real(1)
        self.blocks_complex_to_real_0 = blocks.complex_to_real(1)
        self.blocks_complex_to_imag_1 = blocks.complex_to_imag(1)
        self.blocks_complex_to_imag_0 = blocks.complex_to_imag(1)
        self.blocks_add_xx_0 = blocks.add_vff(1)
        self.band_pass_filter_1 = filter.interp_fir_filter_fff(
            1,
            firdes.band_pass(
                1,
                samp_rate,
                100,
                3000,
                100,
                firdes.WIN_BLACKMAN,
                6.76))
        self.band_pass_filter_0 = filter.fir_filter_fff(
            1,
            firdes.band_pass(
                GAIN,
                samp_rate,
                LPF,
                HPF,
                100,
                firdes.WIN_BLACKMAN,
                6.76))
        self.audio_source_1 = audio.source(samp_rate, TXSOURCE, True)
        self.audio_source_0 = audio.source(samp_rate, 'hw:RadioHatCodec,1', False)
        self.audio_sink_1 = audio.sink(48000, RXSINK, False)
        self.audio_sink_0 = audio.sink(samp_rate, 'hw:RadioHatCodec,0', False)



        ##################################################
        # Connections
        ##################################################
        self.connect((self.audio_source_0, 1), (self.hilbert_fc_0, 0))
        self.connect((self.audio_source_0, 0), (self.hilbert_fc_1, 0))
        self.connect((self.audio_source_1, 0), (self.band_pass_filter_1, 0))
        self.connect((self.band_pass_filter_0, 0), (self.audio_sink_1, 0))
        self.connect((self.band_pass_filter_1, 0), (self.hilbert_fc_1_0, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.band_pass_filter_0, 0))
        self.connect((self.blocks_complex_to_imag_0, 0), (self.blocks_multiply_const_vxx_0_0_0, 0))
        self.connect((self.blocks_complex_to_imag_1, 0), (self.blocks_multiply_const_vxx_0_0_1, 0))
        self.connect((self.blocks_complex_to_real_0, 0), (self.blocks_multiply_const_vxx_0_0, 0))
        self.connect((self.blocks_complex_to_real_1, 0), (self.blocks_multiply_const_vxx_0_0_0_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0_0, 0), (self.blocks_add_xx_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0_0_0, 0), (self.blocks_add_xx_0, 1))
        self.connect((self.blocks_multiply_const_vxx_0_0_0_0, 0), (self.low_pass_filter_0_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0_0_1, 0), (self.low_pass_filter_0, 0))
        self.connect((self.hilbert_fc_0, 0), (self.blocks_complex_to_imag_0, 0))
        self.connect((self.hilbert_fc_1, 0), (self.blocks_complex_to_real_0, 0))
        self.connect((self.hilbert_fc_1_0, 0), (self.blocks_complex_to_imag_1, 0))
        self.connect((self.hilbert_fc_1_0, 0), (self.blocks_complex_to_real_1, 0))
        self.connect((self.low_pass_filter_0, 0), (self.audio_sink_0, 0))
        self.connect((self.low_pass_filter_0_0, 0), (self.audio_sink_0, 1))


    def get_GAIN(self):
        return self.GAIN

    def set_GAIN(self, GAIN):
        self.GAIN = GAIN
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.GAIN, self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_BLACKMAN, 6.76))

    def get_HPF(self):
        return self.HPF

    def set_HPF(self, HPF):
        self.HPF = HPF
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.GAIN, self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_BLACKMAN, 6.76))

    def get_LPF(self):
        return self.LPF

    def set_LPF(self, LPF):
        self.LPF = LPF
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.GAIN, self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_BLACKMAN, 6.76))

    def get_RXSINK(self):
        return self.RXSINK

    def set_RXSINK(self, RXSINK):
        self.RXSINK = RXSINK

    def get_Sideband(self):
        return self.Sideband

    def set_Sideband(self, Sideband):
        self.Sideband = Sideband
        self.blocks_multiply_const_vxx_0_0.set_k(self.Sideband)
        self.blocks_multiply_const_vxx_0_0_1.set_k(self.Sideband)

    def get_TXSOURCE(self):
        return self.TXSOURCE

    def set_TXSOURCE(self, TXSOURCE):
        self.TXSOURCE = TXSOURCE

    def get_parameter_0(self):
        return self.parameter_0

    def set_parameter_0(self, parameter_0):
        self.parameter_0 = parameter_0

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.band_pass_filter_0.set_taps(firdes.band_pass(self.GAIN, self.samp_rate, self.LPF, self.HPF, 100, firdes.WIN_BLACKMAN, 6.76))
        self.band_pass_filter_1.set_taps(firdes.band_pass(1, self.samp_rate, 100, 3000, 100, firdes.WIN_BLACKMAN, 6.76))
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate, 3000, 100, firdes.WIN_KAISER, 6.76))
        self.low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.samp_rate, 3000, 100, firdes.WIN_KAISER, 6.76))




def argument_parser():
    parser = ArgumentParser()
    parser.add_argument(
        "-G", "--GAIN", dest="GAIN", type=eng_float, default="400.0m",
        help="Set GAIN [default=%(default)r]")
    parser.add_argument(
        "--HPF", dest="HPF", type=intx, default=3000,
        help="Set HPF [default=%(default)r]")
    parser.add_argument(
        "--LPF", dest="LPF", type=intx, default=100,
        help="Set LPF [default=%(default)r]")
    parser.add_argument(
        "--RXSINK", dest="RXSINK", type=str, default='',
        help="Set RXSINK [default=%(default)r]")
    parser.add_argument(
        "-S", "--Sideband", dest="Sideband", type=intx, default=1,
        help="Set Sideband [default=%(default)r]")
    parser.add_argument(
        "--TXSOURCE", dest="TXSOURCE", type=str, default='plughw:CARD=Loopback,DEV=1',
        help="Set TXSOURCE [default=%(default)r]")
    return parser


def main(top_block_cls=usb, options=None):
    if options is None:
        options = argument_parser().parse_args()
    if gr.enable_realtime_scheduling() != gr.RT_OK:
        print("Error: failed to enable real-time scheduling.")
    tb = top_block_cls(GAIN=options.GAIN, HPF=options.HPF, LPF=options.LPF, RXSINK=options.RXSINK, Sideband=options.Sideband, TXSOURCE=options.TXSOURCE)

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        sys.exit(0)

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    tb.start()

    tb.wait()


if __name__ == '__main__':
    main()
