#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Ssbio
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


class ssbio(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "Ssbio")

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 48000
        self.LSB = LSB = 0

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
        self.blocks_multiply_const_vxx_0_0_1 = blocks.multiply_const_ff(LSB)
        self.blocks_multiply_const_vxx_0_0_0_0 = blocks.multiply_const_ff(1)
        self.blocks_multiply_const_vxx_0_0_0 = blocks.multiply_const_ff(1)
        self.blocks_multiply_const_vxx_0_0 = blocks.multiply_const_ff(LSB)
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
                .4,
                samp_rate,
                100,
                3000,
                100,
                firdes.WIN_BLACKMAN,
                6.76))
        self.audio_source_1 = audio.source(samp_rate, '', True)
        self.audio_source_0 = audio.source(samp_rate, 'hw:RadioHatCodec,1', False)
        self.audio_sink_1 = audio.sink(48000, '', False)
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


    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.band_pass_filter_0.set_taps(firdes.band_pass(.4, self.samp_rate, 100, 3000, 100, firdes.WIN_BLACKMAN, 6.76))
        self.band_pass_filter_1.set_taps(firdes.band_pass(1, self.samp_rate, 100, 3000, 100, firdes.WIN_BLACKMAN, 6.76))
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate, 3000, 100, firdes.WIN_KAISER, 6.76))
        self.low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.samp_rate, 3000, 100, firdes.WIN_KAISER, 6.76))

    def get_LSB(self):
        return self.LSB

    def set_LSB(self, LSB):
        self.LSB = LSB
        self.blocks_multiply_const_vxx_0_0.set_k(self.LSB)
        self.blocks_multiply_const_vxx_0_0_1.set_k(self.LSB)





def main(top_block_cls=ssbio, options=None):
    tb = top_block_cls()

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
