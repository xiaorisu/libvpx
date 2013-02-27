/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <emmintrin.h>  // SSE2
#include "vp9/common/vp9_idct.h" // for cospi constants

#define pair_set_epi16(a,b) \
  _mm_set1_epi32(((uint16_t)(a)) + (((uint16_t)(b)) << 16))

void vp9_short_fdct8x8_sse2(int16_t *input, int16_t *output, int pitch) {
  const int stride = pitch >> 1;
  int pass;
  // Constants
  //    When we use them, in one case, they are all the same. In all others
  //    it's a pair of them that we need to repeat four times. This is done
  //    by constructing the 32 bit constant corresponding to that pair.
  const __m128i k__cospi_p16_p16 = _mm_set1_epi16(cospi_16_64);
  const __m128i k__cospi_p16_m16 = pair_set_epi16(+cospi_16_64, -cospi_16_64);
  const __m128i k__cospi_p24_p08 = pair_set_epi16(+cospi_24_64,  +cospi_8_64);
  const __m128i k__cospi_m08_p24 = pair_set_epi16( -cospi_8_64, +cospi_24_64);
  const __m128i k__cospi_p28_p04 = pair_set_epi16(+cospi_28_64,  +cospi_4_64);
  const __m128i k__cospi_m04_p28 = pair_set_epi16( -cospi_4_64, +cospi_28_64);
  const __m128i k__cospi_p12_p20 = pair_set_epi16(+cospi_12_64, +cospi_20_64);
  const __m128i k__cospi_m20_p12 = pair_set_epi16(-cospi_20_64, +cospi_12_64);
  const __m128i k__DCT_CONST_ROUNDING = _mm_set1_epi32(DCT_CONST_ROUNDING);
  // Load input
  __m128i in0  = _mm_loadu_si128((const __m128i *)(input + 0 * stride));
  __m128i in1  = _mm_loadu_si128((const __m128i *)(input + 1 * stride));
  __m128i in2  = _mm_loadu_si128((const __m128i *)(input + 2 * stride));
  __m128i in3  = _mm_loadu_si128((const __m128i *)(input + 3 * stride));
  __m128i in4  = _mm_loadu_si128((const __m128i *)(input + 4 * stride));
  __m128i in5  = _mm_loadu_si128((const __m128i *)(input + 5 * stride));
  __m128i in6  = _mm_loadu_si128((const __m128i *)(input + 6 * stride));
  __m128i in7  = _mm_loadu_si128((const __m128i *)(input + 7 * stride));
  // Pre-condition input (shift by two)
  in0 = _mm_slli_epi16 (in0, 2);
  in1 = _mm_slli_epi16 (in1, 2);
  in2 = _mm_slli_epi16 (in2, 2);
  in3 = _mm_slli_epi16 (in3, 2);
  in4 = _mm_slli_epi16 (in4, 2);
  in5 = _mm_slli_epi16 (in5, 2);
  in6 = _mm_slli_epi16 (in6, 2);
  in7 = _mm_slli_epi16 (in7, 2);

  // We do two passes, first the columns, then the rows. The results of the
  // first pass are transposed so that the same column code can be reused. The
  // results of the second pass are also transposed so that the rows (processed
  // as columns) are put back in row positions.
  for (pass = 0; pass < 2; pass++) {
    // To store results of each pass before the transpose.
    __m128i res0, res1, res2, res3, res4, res5, res6, res7;
    // Add/substract
    const __m128i q0 = _mm_add_epi16(in0, in7);
    const __m128i q1 = _mm_add_epi16(in1, in6);
    const __m128i q2 = _mm_add_epi16(in2, in5);
    const __m128i q3 = _mm_add_epi16(in3, in4);
    const __m128i q4 = _mm_sub_epi16(in3, in4);
    const __m128i q5 = _mm_sub_epi16(in2, in5);
    const __m128i q6 = _mm_sub_epi16(in1, in6);
    const __m128i q7 = _mm_sub_epi16(in0, in7);
    // Work on first four results
    {
      // Add/substract
      const __m128i r0 = _mm_add_epi16(q0, q3);
      const __m128i r1 = _mm_add_epi16(q1, q2);
      const __m128i r2 = _mm_sub_epi16(q1, q2);
      const __m128i r3 = _mm_sub_epi16(q0, q3);
      // Interleave to do the multiply by constants which gets us into 32bits
      const __m128i t0 = _mm_unpacklo_epi16(r0, r1);
      const __m128i t1 = _mm_unpackhi_epi16(r0, r1);
      const __m128i t2 = _mm_unpacklo_epi16(r2, r3);
      const __m128i t3 = _mm_unpackhi_epi16(r2, r3);
      const __m128i u0 = _mm_madd_epi16(t0, k__cospi_p16_p16);
      const __m128i u1 = _mm_madd_epi16(t1, k__cospi_p16_p16);
      const __m128i u2 = _mm_madd_epi16(t0, k__cospi_p16_m16);
      const __m128i u3 = _mm_madd_epi16(t1, k__cospi_p16_m16);
      const __m128i u4 = _mm_madd_epi16(t2, k__cospi_p24_p08);
      const __m128i u5 = _mm_madd_epi16(t3, k__cospi_p24_p08);
      const __m128i u6 = _mm_madd_epi16(t2, k__cospi_m08_p24);
      const __m128i u7 = _mm_madd_epi16(t3, k__cospi_m08_p24);
      // dct_const_round_shift
      const __m128i v0 = _mm_add_epi32(u0, k__DCT_CONST_ROUNDING);
      const __m128i v1 = _mm_add_epi32(u1, k__DCT_CONST_ROUNDING);
      const __m128i v2 = _mm_add_epi32(u2, k__DCT_CONST_ROUNDING);
      const __m128i v3 = _mm_add_epi32(u3, k__DCT_CONST_ROUNDING);
      const __m128i v4 = _mm_add_epi32(u4, k__DCT_CONST_ROUNDING);
      const __m128i v5 = _mm_add_epi32(u5, k__DCT_CONST_ROUNDING);
      const __m128i v6 = _mm_add_epi32(u6, k__DCT_CONST_ROUNDING);
      const __m128i v7 = _mm_add_epi32(u7, k__DCT_CONST_ROUNDING);
      const __m128i w0 = _mm_srai_epi32(v0, DCT_CONST_BITS);
      const __m128i w1 = _mm_srai_epi32(v1, DCT_CONST_BITS);
      const __m128i w2 = _mm_srai_epi32(v2, DCT_CONST_BITS);
      const __m128i w3 = _mm_srai_epi32(v3, DCT_CONST_BITS);
      const __m128i w4 = _mm_srai_epi32(v4, DCT_CONST_BITS);
      const __m128i w5 = _mm_srai_epi32(v5, DCT_CONST_BITS);
      const __m128i w6 = _mm_srai_epi32(v6, DCT_CONST_BITS);
      const __m128i w7 = _mm_srai_epi32(v7, DCT_CONST_BITS);
      // Combine
      res0 = _mm_packs_epi32(w0, w1);
      res4 = _mm_packs_epi32(w2, w3);
      res2 = _mm_packs_epi32(w4, w5);
      res6 = _mm_packs_epi32(w6, w7);
    }
    // Work on next four results
    {
      // Interleave to do the multiply by constants which gets us into 32bits
      const __m128i d0 = _mm_unpacklo_epi16(q6, q5);
      const __m128i d1 = _mm_unpackhi_epi16(q6, q5);
      const __m128i e0 = _mm_madd_epi16(d0, k__cospi_p16_m16);
      const __m128i e1 = _mm_madd_epi16(d1, k__cospi_p16_m16);
      const __m128i e2 = _mm_madd_epi16(d0, k__cospi_p16_p16);
      const __m128i e3 = _mm_madd_epi16(d1, k__cospi_p16_p16);
      // dct_const_round_shift
      const __m128i f0 = _mm_add_epi32(e0, k__DCT_CONST_ROUNDING);
      const __m128i f1 = _mm_add_epi32(e1, k__DCT_CONST_ROUNDING);
      const __m128i f2 = _mm_add_epi32(e2, k__DCT_CONST_ROUNDING);
      const __m128i f3 = _mm_add_epi32(e3, k__DCT_CONST_ROUNDING);
      const __m128i s0 = _mm_srai_epi32(f0, DCT_CONST_BITS);
      const __m128i s1 = _mm_srai_epi32(f1, DCT_CONST_BITS);
      const __m128i s2 = _mm_srai_epi32(f2, DCT_CONST_BITS);
      const __m128i s3 = _mm_srai_epi32(f3, DCT_CONST_BITS);
      // Combine
      const __m128i r0 = _mm_packs_epi32 (s0, s1);
      const __m128i r1 = _mm_packs_epi32 (s2, s3);
      // Add/substract
      const __m128i x0 = _mm_add_epi16(q4, r0);
      const __m128i x1 = _mm_sub_epi16(q4, r0);
      const __m128i x2 = _mm_sub_epi16(q7, r1);
      const __m128i x3 = _mm_add_epi16(q7, r1);
      // Interleave to do the multiply by constants which gets us into 32bits
      const __m128i t0 = _mm_unpacklo_epi16(x0, x3);
      const __m128i t1 = _mm_unpackhi_epi16(x0, x3);
      const __m128i t2 = _mm_unpacklo_epi16(x1, x2);
      const __m128i t3 = _mm_unpackhi_epi16(x1, x2);
      const __m128i u0 = _mm_madd_epi16(t0, k__cospi_p28_p04);
      const __m128i u1 = _mm_madd_epi16(t1, k__cospi_p28_p04);
      const __m128i u2 = _mm_madd_epi16(t0, k__cospi_m04_p28);
      const __m128i u3 = _mm_madd_epi16(t1, k__cospi_m04_p28);
      const __m128i u4 = _mm_madd_epi16(t2, k__cospi_p12_p20);
      const __m128i u5 = _mm_madd_epi16(t3, k__cospi_p12_p20);
      const __m128i u6 = _mm_madd_epi16(t2, k__cospi_m20_p12);
      const __m128i u7 = _mm_madd_epi16(t3, k__cospi_m20_p12);
      // dct_const_round_shift
      const __m128i v0 = _mm_add_epi32(u0, k__DCT_CONST_ROUNDING);
      const __m128i v1 = _mm_add_epi32(u1, k__DCT_CONST_ROUNDING);
      const __m128i v2 = _mm_add_epi32(u2, k__DCT_CONST_ROUNDING);
      const __m128i v3 = _mm_add_epi32(u3, k__DCT_CONST_ROUNDING);
      const __m128i v4 = _mm_add_epi32(u4, k__DCT_CONST_ROUNDING);
      const __m128i v5 = _mm_add_epi32(u5, k__DCT_CONST_ROUNDING);
      const __m128i v6 = _mm_add_epi32(u6, k__DCT_CONST_ROUNDING);
      const __m128i v7 = _mm_add_epi32(u7, k__DCT_CONST_ROUNDING);
      const __m128i w0 = _mm_srai_epi32(v0, DCT_CONST_BITS);
      const __m128i w1 = _mm_srai_epi32(v1, DCT_CONST_BITS);
      const __m128i w2 = _mm_srai_epi32(v2, DCT_CONST_BITS);
      const __m128i w3 = _mm_srai_epi32(v3, DCT_CONST_BITS);
      const __m128i w4 = _mm_srai_epi32(v4, DCT_CONST_BITS);
      const __m128i w5 = _mm_srai_epi32(v5, DCT_CONST_BITS);
      const __m128i w6 = _mm_srai_epi32(v6, DCT_CONST_BITS);
      const __m128i w7 = _mm_srai_epi32(v7, DCT_CONST_BITS);
      // Combine
      res1 = _mm_packs_epi32(w0, w1);
      res7 = _mm_packs_epi32(w2, w3);
      res5 = _mm_packs_epi32(w4, w5);
      res3 = _mm_packs_epi32(w6, w7);
    }
    // Transpose the 8x8.
    {
      // 00 01 02 03 04 05 06 07
      // 10 11 12 13 14 15 16 17
      // 20 21 22 23 24 25 26 27
      // 30 31 32 33 34 35 36 37
      // 40 41 42 43 44 45 46 47
      // 50 51 52 53 54 55 56 57
      // 60 61 62 63 64 65 66 67
      // 70 71 72 73 74 75 76 77
      const __m128i tr0_0 = _mm_unpacklo_epi16(res0, res1);
      const __m128i tr0_1 = _mm_unpacklo_epi16(res2, res3);
      const __m128i tr0_2 = _mm_unpackhi_epi16(res0, res1);
      const __m128i tr0_3 = _mm_unpackhi_epi16(res2, res3);
      const __m128i tr0_4 = _mm_unpacklo_epi16(res4, res5);
      const __m128i tr0_5 = _mm_unpacklo_epi16(res6, res7);
      const __m128i tr0_6 = _mm_unpackhi_epi16(res4, res5);
      const __m128i tr0_7 = _mm_unpackhi_epi16(res6, res7);
      // 00 10 01 11 02 12 03 13
      // 20 30 21 31 22 32 23 33
      // 04 14 05 15 06 16 07 17
      // 24 34 25 35 26 36 27 37
      // 40 50 41 51 42 52 43 53
      // 60 70 61 71 62 72 63 73
      // 54 54 55 55 56 56 57 57
      // 64 74 65 75 66 76 67 77
      const __m128i tr1_0 = _mm_unpacklo_epi32(tr0_0, tr0_1);
      const __m128i tr1_1 = _mm_unpacklo_epi32(tr0_2, tr0_3);
      const __m128i tr1_2 = _mm_unpackhi_epi32(tr0_0, tr0_1);
      const __m128i tr1_3 = _mm_unpackhi_epi32(tr0_2, tr0_3);
      const __m128i tr1_4 = _mm_unpacklo_epi32(tr0_4, tr0_5);
      const __m128i tr1_5 = _mm_unpacklo_epi32(tr0_6, tr0_7);
      const __m128i tr1_6 = _mm_unpackhi_epi32(tr0_4, tr0_5);
      const __m128i tr1_7 = _mm_unpackhi_epi32(tr0_6, tr0_7);
      // 00 10 20 30 01 11 21 31
      // 40 50 60 70 41 51 61 71
      // 02 12 22 32 03 13 23 33
      // 42 52 62 72 43 53 63 73
      // 04 14 24 34 05 15 21 36
      // 44 54 64 74 45 55 61 76
      // 06 16 26 36 07 17 27 37
      // 46 56 66 76 47 57 67 77
      in0 = _mm_unpacklo_epi64(tr1_0, tr1_4);
      in1 = _mm_unpackhi_epi64(tr1_0, tr1_4);
      in2 = _mm_unpacklo_epi64(tr1_2, tr1_6);
      in3 = _mm_unpackhi_epi64(tr1_2, tr1_6);
      in4 = _mm_unpacklo_epi64(tr1_1, tr1_5);
      in5 = _mm_unpackhi_epi64(tr1_1, tr1_5);
      in6 = _mm_unpacklo_epi64(tr1_3, tr1_7);
      in7 = _mm_unpackhi_epi64(tr1_3, tr1_7);
      // 00 10 20 30 40 50 60 70
      // 01 11 21 31 41 51 61 71
      // 02 12 22 32 42 52 62 72
      // 03 13 23 33 43 53 63 73
      // 04 14 24 34 44 54 64 74
      // 05 15 25 35 45 55 65 75
      // 06 16 26 36 46 56 66 76
      // 07 17 27 37 47 57 67 77
    }
  }
  // Post-condition output and store it
  {
    // Post-condition (division by two)
    //    division of two 16 bits signed numbers using shifts
    //    n / 2 = (n - (n >> 15)) >> 1
    const __m128i sign_in0 = _mm_srai_epi16(in0, 15);
    const __m128i sign_in1 = _mm_srai_epi16(in1, 15);
    const __m128i sign_in2 = _mm_srai_epi16(in2, 15);
    const __m128i sign_in3 = _mm_srai_epi16(in3, 15);
    const __m128i sign_in4 = _mm_srai_epi16(in4, 15);
    const __m128i sign_in5 = _mm_srai_epi16(in5, 15);
    const __m128i sign_in6 = _mm_srai_epi16(in6, 15);
    const __m128i sign_in7 = _mm_srai_epi16(in7, 15);
    in0 = _mm_sub_epi16(in0, sign_in0);
    in1 = _mm_sub_epi16(in1, sign_in1);
    in2 = _mm_sub_epi16(in2, sign_in2);
    in3 = _mm_sub_epi16(in3, sign_in3);
    in4 = _mm_sub_epi16(in4, sign_in4);
    in5 = _mm_sub_epi16(in5, sign_in5);
    in6 = _mm_sub_epi16(in6, sign_in6);
    in7 = _mm_sub_epi16(in7, sign_in7);
    in0 = _mm_srai_epi16(in0, 1);
    in1 = _mm_srai_epi16(in1, 1);
    in2 = _mm_srai_epi16(in2, 1);
    in3 = _mm_srai_epi16(in3, 1);
    in4 = _mm_srai_epi16(in4, 1);
    in5 = _mm_srai_epi16(in5, 1);
    in6 = _mm_srai_epi16(in6, 1);
    in7 = _mm_srai_epi16(in7, 1);
    // store results
    _mm_storeu_si128 ((__m128i *)(output + 0 * 8), in0);
    _mm_storeu_si128 ((__m128i *)(output + 1 * 8), in1);
    _mm_storeu_si128 ((__m128i *)(output + 2 * 8), in2);
    _mm_storeu_si128 ((__m128i *)(output + 3 * 8), in3);
    _mm_storeu_si128 ((__m128i *)(output + 4 * 8), in4);
    _mm_storeu_si128 ((__m128i *)(output + 5 * 8), in5);
    _mm_storeu_si128 ((__m128i *)(output + 6 * 8), in6);
    _mm_storeu_si128 ((__m128i *)(output + 7 * 8), in7);
  }
}
