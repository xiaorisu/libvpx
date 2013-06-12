/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "third_party/googletest/src/include/gtest/gtest.h"

extern "C" {
#include "vp9/encoder/vp9_boolhuff.h"
#include "vp9/decoder/vp9_dboolhuff.h"
}

#include "acm_random.h"
#include "vpx/vpx_integer.h"

using libvpx_test::ACMRandom;

namespace {
const int num_tests = 10;
}  // namespace

TEST(VP9, TestBitIO) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());
  for (int n = 0; n < num_tests; ++n) {
    for (int method = 0; method <= 7; ++method) {   // we generate various proba
      const int bits_to_test = 1000;
      uint8_t probas[bits_to_test];

      for (int i = 0; i < bits_to_test; ++i) {
        const int parity = i & 1;
        probas[i] =
          (method == 0) ? 0 : (method == 1) ? 255 :
          (method == 2) ? 128 :
          (method == 3) ? rnd.Rand8() :
          (method == 4) ? (parity ? 0 : 255) :
            // alternate between low and high proba:
            (method == 5) ? (parity ? rnd(128) : 255 - rnd(128)) :
            (method == 6) ?
            (parity ? rnd(64) : 255 - rnd(64)) :
            (parity ? rnd(32) : 255 - rnd(32));
      }
      for (int bit_method = 0; bit_method <= 3; ++bit_method) {
        const int random_seed = 6432;
        const int buffer_size = 10000;
        ACMRandom bit_rnd(random_seed);
        vp9_writer bw;
        uint8_t bw_buffer[buffer_size];
        vp9_start_encode(&bw, bw_buffer);

        int bit = (bit_method == 0) ? 0 : (bit_method == 1) ? 1 : 0;
        for (int i = 0; i < bits_to_test; ++i) {
          if (bit_method == 2) {
            bit = (i & 1);
          } else if (bit_method == 3) {
            bit = bit_rnd(2);
          }
          vp9_write(&bw, bit, static_cast<int>(probas[i]));
        }

        vp9_stop_encode(&bw);

        // First bit should be zero
        GTEST_ASSERT_EQ(bw_buffer[0] & 0x80, 0);

        vp9_reader br;
        vp9_reader_init(&br, bw_buffer, buffer_size);
        bit_rnd.Reset(random_seed);
        for (int i = 0; i < bits_to_test; ++i) {
          if (bit_method == 2) {
            bit = (i & 1);
          } else if (bit_method == 3) {
            bit = bit_rnd(2);
          }
          GTEST_ASSERT_EQ(vp9_read(&br, probas[i]), bit)
              << "pos: " << i << " / " << bits_to_test
              << " bit_method: " << bit_method
              << " method: " << method;
        }
      }
    }
  }
}
