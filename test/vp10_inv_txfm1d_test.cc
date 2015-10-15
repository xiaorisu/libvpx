/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "test/vp10_txfm_test.h"

using libvpx_test::ACMRandom;

namespace {
static int txfm_type_num = 2;
static int txfm_size_num = 4;
static int txfm_size_ls[4] = {4, 8, 16, 32};

static TxfmFunc fwd_txfm_func_ls[2][4] = {
    {vp10_fdct4_new, vp10_fdct8_new, vp10_fdct16_new, vp10_fdct32_new},
    {vp10_fadst4_new, vp10_fadst8_new, vp10_fadst16_new, vp10_fadst32_new}};

static TxfmFunc inv_txfm_func_ls[2][4] = {
    {vp10_idct4_new, vp10_idct8_new, vp10_idct16_new, vp10_idct32_new},
    {vp10_iadst4_new, vp10_iadst8_new, vp10_iadst16_new, vp10_iadst32_new}};

// the maximum stage number of fwd/inv 1d dct/adst txfm is 12
static int8_t cos_bit[12] = {14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14};
static int8_t range_bit[12] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};

TEST(vp10_inv_txfm1d, round_trip) {
  ACMRandom rnd(ACMRandom::DeterministicSeed());
  for (int si = 0; si < txfm_size_num; ++si) {
    int txfm_size = txfm_size_ls[si];
    int32_t *input = new int32_t[txfm_size];
    int32_t *output = new int32_t[txfm_size];
    int32_t *round_trip_output = new int32_t[txfm_size];

    for (int ti = 0; ti < txfm_type_num; ++ti) {
      TxfmFunc fwd_txfm_func = fwd_txfm_func_ls[ti][si];
      TxfmFunc inv_txfm_func = inv_txfm_func_ls[ti][si];
      int max_error = 2;

      const int count_test_block = 5000;
      for (int ci = 0; ci < count_test_block; ++ci) {
        for (int ni = 0; ni < txfm_size; ++ni) {
          input[ni] = rnd.Rand16() % base - rnd.Rand16() % base;
        }

        fwd_txfm_func(input, output, cos_bit, range_bit);
        inv_txfm_func(output, round_trip_output, cos_bit, range_bit);

        for (int ni = 0; ni < txfm_size; ++ni) {
          EXPECT_LE(abs(input[ni] - round_shift(round_trip_output[ni],
                                                get_max_bit(txfm_size) - 1)),
                    max_error);
        }
      }
    }
    delete[] input;
    delete[] output;
    delete[] round_trip_output;
  }
}

}  // namespace
