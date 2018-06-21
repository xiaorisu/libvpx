/*
 *  Copyright (c) 2018 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_dsp_rtcd.h"

#include "vpx_dsp/ppc/types_vsx.h"
#include "vpx_dsp/ppc/txfm_common_vsx.h"

static const int32x4_t vec_dct_const_rounding = { 8192, 8192, 8192, 8192 };
static const uint32x4_t vec_dct_const_bits = { 14, 14, 14, 14 };
static const uint16x8_t vec_dct_scale_log2 = { 2, 2, 2, 2, 2, 2, 2, 2 };

// Negate 16-bit integers
static INLINE int16x8_t vec_neg(int16x8_t a) {
  return vec_add(vec_xor(a, vec_splats((int16_t)-1)), vec_ones_s16);
}

static INLINE int16x8_t vec_sign_s16(int16x8_t a) {
  return vec_sr(a, vec_shift_sign_s16);
}

// Returns ((a + b) * c + (2 << 13)) >> 14, since a + b can overflow 16 bits,
// the multiplication is distributed a * c + b * c. Both multiplications are
// done in 32-bit lanes.
static INLINE void single_butterfly(int16x8_t a, int16x8_t b, int16x8_t c,
                                    int16x8_t *add, int16x8_t *sub) {
  const int32x4_t ac_e = vec_mule(a, c);
  const int32x4_t ac_o = vec_mulo(a, c);
  const int32x4_t bc_e = vec_mule(b, c);
  const int32x4_t bc_o = vec_mulo(b, c);

  const int32x4_t sum_e = vec_add(ac_e, bc_e);
  const int32x4_t sum_o = vec_add(ac_o, bc_o);
  const int32x4_t diff_e = vec_sub(ac_e, bc_e);
  const int32x4_t diff_o = vec_sub(ac_o, bc_o);

  const int32x4_t rsum_o = vec_add(sum_o, vec_dct_const_rounding);
  const int32x4_t rsum_e = vec_add(sum_e, vec_dct_const_rounding);
  const int32x4_t rdiff_o = vec_add(diff_o, vec_dct_const_rounding);
  const int32x4_t rdiff_e = vec_add(diff_e, vec_dct_const_rounding);

  const int32x4_t ssum_o = vec_sra(rsum_o, vec_dct_const_bits);
  const int32x4_t ssum_e = vec_sra(rsum_e, vec_dct_const_bits);
  const int32x4_t sdiff_o = vec_sra(rdiff_o, vec_dct_const_bits);
  const int32x4_t sdiff_e = vec_sra(rdiff_e, vec_dct_const_bits);

  const int16x8_t pack_sum = vec_pack(ssum_e, ssum_o);
  const int16x8_t pack_diff = vec_pack(sdiff_e, sdiff_o);

  *add = vec_perm(pack_sum, pack_sum, vec_perm_merge);
  *sub = vec_perm(pack_diff, pack_diff, vec_perm_merge);
}

static INLINE void double_butterfly(int16x8_t a, int16x8_t c1, int16x8_t b,
                                    int16x8_t c2, int16x8_t *add,
                                    int16x8_t *sub) {
  const int32x4_t ac1_o = vec_mulo(a, c1);
  const int32x4_t ac1_e = vec_mule(a, c1);
  const int32x4_t ac2_o = vec_mulo(a, c2);
  const int32x4_t ac2_e = vec_mule(a, c2);

  const int32x4_t bc1_o = vec_mulo(b, c1);
  const int32x4_t bc1_e = vec_mule(b, c1);
  const int32x4_t bc2_o = vec_mulo(b, c2);
  const int32x4_t bc2_e = vec_mule(b, c2);

  const int32x4_t sum_o = vec_add(ac1_o, bc2_o);
  const int32x4_t sum_e = vec_add(ac1_e, bc2_e);
  const int32x4_t diff_o = vec_sub(ac2_o, bc1_o);
  const int32x4_t diff_e = vec_sub(ac2_e, bc1_e);

  const int32x4_t rsum_o = vec_add(sum_o, vec_dct_const_rounding);
  const int32x4_t rsum_e = vec_add(sum_e, vec_dct_const_rounding);
  const int32x4_t rdiff_o = vec_add(diff_o, vec_dct_const_rounding);
  const int32x4_t rdiff_e = vec_add(diff_e, vec_dct_const_rounding);

  const int32x4_t ssum_o = vec_sra(rsum_o, vec_dct_const_bits);
  const int32x4_t ssum_e = vec_sra(rsum_e, vec_dct_const_bits);
  const int32x4_t sdiff_o = vec_sra(rdiff_o, vec_dct_const_bits);
  const int32x4_t sdiff_e = vec_sra(rdiff_e, vec_dct_const_bits);

  const int16x8_t pack_sum = vec_pack(ssum_e, ssum_o);
  const int16x8_t pack_diff = vec_pack(sdiff_e, sdiff_o);

  *add = vec_perm(pack_sum, pack_sum, vec_perm_merge);
  *sub = vec_perm(pack_diff, pack_diff, vec_perm_merge);
}

// TODO(ltrudeau) combine load and stage 1
static INLINE void load(const int16_t *a, int stride, int16x8_t *b) {
  // Tried out different combinations of load and shift instructions, this is
  // the fastest one.
  {
    const int16x8_t l0 = vec_vsx_ld(0, a);
    const int16x8_t l1 = vec_vsx_ld(0, a + stride);
    const int16x8_t l2 = vec_vsx_ld(0, a + 2 * stride);
    const int16x8_t l3 = vec_vsx_ld(0, a + 3 * stride);
    const int16x8_t l4 = vec_vsx_ld(0, a + 4 * stride);
    const int16x8_t l5 = vec_vsx_ld(0, a + 5 * stride);
    const int16x8_t l6 = vec_vsx_ld(0, a + 6 * stride);
    const int16x8_t l7 = vec_vsx_ld(0, a + 7 * stride);
    const int16x8_t l8 = vec_vsx_ld(0, a + 8 * stride);
    const int16x8_t l9 = vec_vsx_ld(0, a + 9 * stride);
    const int16x8_t l10 = vec_vsx_ld(0, a + 10 * stride);
    const int16x8_t l11 = vec_vsx_ld(0, a + 11 * stride);
    const int16x8_t l12 = vec_vsx_ld(0, a + 12 * stride);
    const int16x8_t l13 = vec_vsx_ld(0, a + 13 * stride);
    const int16x8_t l14 = vec_vsx_ld(0, a + 14 * stride);
    const int16x8_t l15 = vec_vsx_ld(0, a + 15 * stride);

    b[0] = vec_sl(l0, vec_dct_scale_log2);
    b[1] = vec_sl(l1, vec_dct_scale_log2);
    b[2] = vec_sl(l2, vec_dct_scale_log2);
    b[3] = vec_sl(l3, vec_dct_scale_log2);
    b[4] = vec_sl(l4, vec_dct_scale_log2);
    b[5] = vec_sl(l5, vec_dct_scale_log2);
    b[6] = vec_sl(l6, vec_dct_scale_log2);
    b[7] = vec_sl(l7, vec_dct_scale_log2);
    b[8] = vec_sl(l8, vec_dct_scale_log2);
    b[9] = vec_sl(l9, vec_dct_scale_log2);
    b[10] = vec_sl(l10, vec_dct_scale_log2);
    b[11] = vec_sl(l11, vec_dct_scale_log2);
    b[12] = vec_sl(l12, vec_dct_scale_log2);
    b[13] = vec_sl(l13, vec_dct_scale_log2);
    b[14] = vec_sl(l14, vec_dct_scale_log2);
    b[15] = vec_sl(l15, vec_dct_scale_log2);
  }
  {
    const int16x8_t l16 = vec_vsx_ld(0, a + 16 * stride);
    const int16x8_t l17 = vec_vsx_ld(0, a + 17 * stride);
    const int16x8_t l18 = vec_vsx_ld(0, a + 18 * stride);
    const int16x8_t l19 = vec_vsx_ld(0, a + 19 * stride);
    const int16x8_t l20 = vec_vsx_ld(0, a + 20 * stride);
    const int16x8_t l21 = vec_vsx_ld(0, a + 21 * stride);
    const int16x8_t l22 = vec_vsx_ld(0, a + 22 * stride);
    const int16x8_t l23 = vec_vsx_ld(0, a + 23 * stride);
    const int16x8_t l24 = vec_vsx_ld(0, a + 24 * stride);
    const int16x8_t l25 = vec_vsx_ld(0, a + 25 * stride);
    const int16x8_t l26 = vec_vsx_ld(0, a + 26 * stride);
    const int16x8_t l27 = vec_vsx_ld(0, a + 27 * stride);
    const int16x8_t l28 = vec_vsx_ld(0, a + 28 * stride);
    const int16x8_t l29 = vec_vsx_ld(0, a + 29 * stride);
    const int16x8_t l30 = vec_vsx_ld(0, a + 30 * stride);
    const int16x8_t l31 = vec_vsx_ld(0, a + 31 * stride);

    b[16] = vec_sl(l16, vec_dct_scale_log2);
    b[17] = vec_sl(l17, vec_dct_scale_log2);
    b[18] = vec_sl(l18, vec_dct_scale_log2);
    b[19] = vec_sl(l19, vec_dct_scale_log2);
    b[20] = vec_sl(l20, vec_dct_scale_log2);
    b[21] = vec_sl(l21, vec_dct_scale_log2);
    b[22] = vec_sl(l22, vec_dct_scale_log2);
    b[23] = vec_sl(l23, vec_dct_scale_log2);
    b[24] = vec_sl(l24, vec_dct_scale_log2);
    b[25] = vec_sl(l25, vec_dct_scale_log2);
    b[26] = vec_sl(l26, vec_dct_scale_log2);
    b[27] = vec_sl(l27, vec_dct_scale_log2);
    b[28] = vec_sl(l28, vec_dct_scale_log2);
    b[29] = vec_sl(l29, vec_dct_scale_log2);
    b[30] = vec_sl(l30, vec_dct_scale_log2);
    b[31] = vec_sl(l31, vec_dct_scale_log2);
  }
}

static INLINE void store(tran_low_t *a, const int16x8_t *b) {
  vec_vsx_st(b[0], 0, a);
  vec_vsx_st(b[8], 0, a + 8);
  vec_vsx_st(b[16], 0, a + 16);
  vec_vsx_st(b[24], 0, a + 24);

  vec_vsx_st(b[1], 0, a + 32);
  vec_vsx_st(b[9], 0, a + 40);
  vec_vsx_st(b[17], 0, a + 48);
  vec_vsx_st(b[25], 0, a + 56);

  vec_vsx_st(b[2], 0, a + 64);
  vec_vsx_st(b[10], 0, a + 72);
  vec_vsx_st(b[18], 0, a + 80);
  vec_vsx_st(b[26], 0, a + 88);

  vec_vsx_st(b[3], 0, a + 96);
  vec_vsx_st(b[11], 0, a + 104);
  vec_vsx_st(b[19], 0, a + 112);
  vec_vsx_st(b[27], 0, a + 120);

  vec_vsx_st(b[4], 0, a + 128);
  vec_vsx_st(b[12], 0, a + 136);
  vec_vsx_st(b[20], 0, a + 144);
  vec_vsx_st(b[28], 0, a + 152);

  vec_vsx_st(b[5], 0, a + 160);
  vec_vsx_st(b[13], 0, a + 168);
  vec_vsx_st(b[21], 0, a + 176);
  vec_vsx_st(b[29], 0, a + 184);

  vec_vsx_st(b[6], 0, a + 192);
  vec_vsx_st(b[14], 0, a + 200);
  vec_vsx_st(b[22], 0, a + 208);
  vec_vsx_st(b[30], 0, a + 216);

  vec_vsx_st(b[7], 0, a + 224);
  vec_vsx_st(b[15], 0, a + 232);
  vec_vsx_st(b[23], 0, a + 240);
  vec_vsx_st(b[31], 0, a + 248);
}

// Add 2 if positive, 1 if negative, and shift by 2.
static INLINE int16x8_t sub_round_shift(const int16x8_t a) {
  const int16x8_t sign = vec_sign_s16(a);
  return vec_sra(vec_sub(vec_add(a, vec_twos_s16), sign), vec_dct_scale_log2);
}

// Add 1 if positive, 2 if negative, and shift by 2.
// In practice, add 1, then add the sign bit, then shift without rounding.
static INLINE int16x8_t add_round_shift_s16(const int16x8_t a) {
  const int16x8_t sign = vec_sign_s16(a);
  return vec_sra(vec_add(vec_add(a, vec_ones_s16), sign), vec_dct_scale_log2);
}

// TODO(ltrudeau) 2 different versions of this function (pass 0 and pass1)
void vpx_fdct32_vsx(const int16x8_t *in, int16x8_t *out, int pass) {
  int16x8_t step[32];
  int16x8_t output[32];

  // Stage 1
  for (int i = 0; i < 16; i++) {
    step[i] = vec_add(in[i], in[31 - i]);
    step[i + 16] = vec_sub(in[15 - i], in[i + 16]);
  }

  // Stage 2
  for (int i = 0; i < 8; i++) {
    output[i] = vec_add(step[i], step[15 - i]);
    output[i + 8] = vec_sub(step[7 - i], step[i + 8]);
  }

  output[16] = step[16];
  output[17] = step[17];
  output[18] = step[18];
  output[19] = step[19];

  single_butterfly(step[27], step[20], cospi16_v, &output[27], &output[20]);
  single_butterfly(step[26], step[21], cospi16_v, &output[26], &output[21]);
  single_butterfly(step[25], step[22], cospi16_v, &output[25], &output[22]);
  single_butterfly(step[24], step[23], cospi16_v, &output[24], &output[23]);

  output[28] = step[28];
  output[29] = step[29];
  output[30] = step[30];
  output[31] = step[31];

  // dump the magnitude by 4, hence the intermediate values are within
  // the range of 16 bits.
  if (pass) {
    output[0] = add_round_shift_s16(output[0]);
    output[1] = add_round_shift_s16(output[1]);
    output[2] = add_round_shift_s16(output[2]);
    output[3] = add_round_shift_s16(output[3]);
    output[4] = add_round_shift_s16(output[4]);
    output[5] = add_round_shift_s16(output[5]);
    output[6] = add_round_shift_s16(output[6]);
    output[7] = add_round_shift_s16(output[7]);
    output[8] = add_round_shift_s16(output[8]);
    output[9] = add_round_shift_s16(output[9]);
    output[10] = add_round_shift_s16(output[10]);
    output[11] = add_round_shift_s16(output[11]);
    output[12] = add_round_shift_s16(output[12]);
    output[13] = add_round_shift_s16(output[13]);
    output[14] = add_round_shift_s16(output[14]);
    output[15] = add_round_shift_s16(output[15]);

    output[16] = add_round_shift_s16(output[16]);
    output[17] = add_round_shift_s16(output[17]);
    output[18] = add_round_shift_s16(output[18]);
    output[19] = add_round_shift_s16(output[19]);
    output[20] = add_round_shift_s16(output[20]);
    output[21] = add_round_shift_s16(output[21]);
    output[22] = add_round_shift_s16(output[22]);
    output[23] = add_round_shift_s16(output[23]);
    output[24] = add_round_shift_s16(output[24]);
    output[25] = add_round_shift_s16(output[25]);
    output[26] = add_round_shift_s16(output[26]);
    output[27] = add_round_shift_s16(output[27]);
    output[28] = add_round_shift_s16(output[28]);
    output[29] = add_round_shift_s16(output[29]);
    output[30] = add_round_shift_s16(output[30]);
    output[31] = add_round_shift_s16(output[31]);
  }

  // Stage 3
  step[0] = vec_add(output[0], output[7]);
  step[1] = vec_add(output[1], output[6]);
  step[2] = vec_add(output[2], output[5]);
  step[3] = vec_add(output[3], output[4]);
  step[4] = vec_sub(output[3], output[4]);
  step[5] = vec_sub(output[2], output[5]);
  step[6] = vec_sub(output[1], output[6]);
  step[7] = vec_sub(output[0], output[7]);
  step[8] = output[8];
  step[9] = output[9];

  single_butterfly(output[13], output[10], cospi16_v, &step[13], &step[10]);
  single_butterfly(output[12], output[11], cospi16_v, &step[12], &step[11]);
  step[14] = output[14];
  step[15] = output[15];

  step[16] = vec_add(output[16], output[23]);
  step[17] = vec_add(output[17], output[22]);
  step[18] = vec_add(output[18], output[21]);
  step[19] = vec_add(output[19], output[20]);

  step[20] = vec_sub(output[19], output[20]);
  step[21] = vec_sub(output[18], output[21]);
  step[22] = vec_sub(output[17], output[22]);
  step[23] = vec_sub(output[16], output[23]);
  step[24] = vec_sub(output[31], output[24]);
  step[25] = vec_sub(output[30], output[25]);
  step[26] = vec_sub(output[29], output[26]);
  step[27] = vec_sub(output[28], output[27]);

  step[28] = vec_add(output[28], output[27]);
  step[29] = vec_add(output[29], output[26]);
  step[30] = vec_add(output[30], output[25]);
  step[31] = vec_add(output[31], output[24]);

  // Stage 4
  output[0] = vec_add(step[0], step[3]);
  output[1] = vec_add(step[1], step[2]);
  output[2] = vec_sub(step[1], step[2]);
  output[3] = vec_sub(step[0], step[3]);
  output[4] = step[4];
  single_butterfly(step[6], step[5], cospi16_v, &output[6], &output[5]);
  output[7] = step[7];
  output[8] = vec_add(step[8], step[11]);
  output[9] = vec_add(step[9], step[10]);
  output[10] = vec_sub(step[9], step[10]);
  output[11] = vec_sub(step[8], step[11]);
  output[12] = vec_sub(step[15], step[12]);
  output[13] = vec_sub(step[14], step[13]);
  output[14] = vec_add(step[14], step[13]);
  output[15] = vec_add(step[15], step[12]);

  output[16] = step[16];
  output[17] = step[17];
  double_butterfly(step[29], cospi8_v, step[18], cospi24_v, &output[29],
                   &output[18]);
  double_butterfly(step[28], cospi8_v, step[19], cospi24_v, &output[28],
                   &output[19]);
  double_butterfly(step[27], cospi24_v, step[20], vec_neg(cospi8_v),
                   &output[27], &output[20]);
  double_butterfly(step[26], cospi24_v, step[21], vec_neg(cospi8_v),
                   &output[26], &output[21]);

  output[22] = step[22];
  output[23] = step[23];
  output[24] = step[24];
  output[25] = step[25];
  output[30] = step[30];
  output[31] = step[31];

  // Stage 5
  single_butterfly(output[0], output[1], cospi16_v, &step[0], &step[1]);

  double_butterfly(output[3], cospi8_v, output[2], cospi24_v, &step[2],
                   &step[3]);

  step[4] = vec_add(output[4], output[5]);
  step[5] = vec_sub(output[4], output[5]);
  step[6] = vec_sub(output[7], output[6]);
  step[7] = vec_add(output[7], output[6]);
  step[8] = output[8];

  double_butterfly(output[14], cospi8_v, output[9], cospi24_v, &step[14],
                   &step[9]);
  double_butterfly(output[13], cospi24_v, output[10], vec_neg(cospi8_v),
                   &step[13], &step[10]);
  step[11] = output[11];
  step[12] = output[12];
  step[15] = output[15];

  step[16] = vec_add(output[16], output[19]);
  step[17] = vec_add(output[17], output[18]);
  step[18] = vec_sub(output[17], output[18]);
  step[19] = vec_sub(output[16], output[19]);
  step[20] = vec_sub(output[23], output[20]);
  step[21] = vec_sub(output[22], output[21]);
  step[22] = vec_add(output[22], output[21]);
  step[23] = vec_add(output[23], output[20]);
  step[24] = vec_add(output[24], output[27]);
  step[25] = vec_add(output[25], output[26]);
  step[26] = vec_sub(output[25], output[26]);
  step[27] = vec_sub(output[24], output[27]);
  step[28] = vec_sub(output[31], output[28]);
  step[29] = vec_sub(output[30], output[29]);
  step[30] = vec_add(output[30], output[29]);
  step[31] = vec_add(output[31], output[28]);

  // Stage 6
  output[0] = step[0];
  output[1] = step[1];
  output[2] = step[2];
  output[3] = step[3];

  double_butterfly(step[7], cospi4_v, step[4], cospi28_v, &output[4],
                   &output[7]);
  double_butterfly(step[6], cospi20_v, step[5], cospi12_v, &output[5],
                   &output[6]);
  output[8] = vec_add(step[8], step[9]);
  output[9] = vec_sub(step[8], step[9]);
  output[10] = vec_sub(step[11], step[10]);
  output[11] = vec_add(step[11], step[10]);
  output[12] = vec_add(step[12], step[13]);
  output[13] = vec_sub(step[12], step[13]);
  output[14] = vec_sub(step[15], step[14]);
  output[15] = vec_add(step[15], step[14]);

  output[16] = step[16];

  double_butterfly(step[30], cospi4_v, step[17], cospi28_v, &output[30],
                   &output[17]);
  double_butterfly(step[29], cospi28_v, step[18], vec_neg(cospi4_v),
                   &output[29], &output[18]);
  output[19] = step[19];
  output[20] = step[20];
  double_butterfly(step[26], cospi20_v, step[21], cospi12_v, &output[26],
                   &output[21]);
  double_butterfly(step[25], cospi12_v, step[22], vec_neg(cospi20_v),
                   &output[25], &output[22]);
  output[23] = step[23];
  output[24] = step[24];
  output[27] = step[27];
  output[28] = step[28];
  output[31] = step[31];

  // Stage 7
  step[0] = output[0];
  step[1] = output[1];
  step[2] = output[2];
  step[3] = output[3];
  step[4] = output[4];
  step[5] = output[5];
  step[6] = output[6];
  step[7] = output[7];
  double_butterfly(output[15], cospi2_v, output[8], cospi30_v, &step[8],
                   &step[15]);
  double_butterfly(output[14], cospi18_v, output[9], cospi14_v, &step[9],
                   &step[14]);
  double_butterfly(output[13], cospi10_v, output[10], cospi22_v, &step[10],
                   &step[13]);
  double_butterfly(output[12], cospi26_v, output[11], cospi6_v, &step[11],
                   &step[12]);

  step[16] = vec_add(output[16], output[17]);
  step[17] = vec_sub(output[16], output[17]);
  step[18] = vec_sub(output[19], output[18]);
  step[19] = vec_add(output[19], output[18]);
  step[20] = vec_add(output[20], output[21]);
  step[21] = vec_sub(output[20], output[21]);
  step[22] = vec_sub(output[23], output[22]);
  step[23] = vec_add(output[23], output[22]);
  step[24] = vec_add(output[24], output[25]);
  step[25] = vec_sub(output[24], output[25]);
  step[26] = vec_sub(output[27], output[26]);
  step[27] = vec_add(output[27], output[26]);
  step[28] = vec_add(output[28], output[29]);
  step[29] = vec_sub(output[28], output[29]);
  step[30] = vec_sub(output[31], output[30]);
  step[31] = vec_add(output[31], output[30]);

  // Final stage --- outputs indices are bit-reversed.
  out[0] = step[0];
  out[16] = step[1];
  out[8] = step[2];
  out[24] = step[3];
  out[4] = step[4];
  out[20] = step[5];
  out[12] = step[6];
  out[28] = step[7];
  out[2] = step[8];
  out[18] = step[9];
  out[10] = step[10];
  out[26] = step[11];
  out[6] = step[12];
  out[22] = step[13];
  out[14] = step[14];
  out[30] = step[15];

  double_butterfly(step[31], cospi1_v, step[16], cospi31_v, &out[1], &out[31]);
  double_butterfly(step[30], cospi17_v, step[17], cospi15_v, &out[17],
                   &out[15]);
  double_butterfly(step[29], cospi9_v, step[18], cospi23_v, &out[9], &out[23]);
  double_butterfly(step[28], cospi25_v, step[19], cospi7_v, &out[25], &out[7]);
  double_butterfly(step[27], cospi5_v, step[20], cospi27_v, &out[5], &out[27]);
  double_butterfly(step[26], cospi21_v, step[21], cospi11_v, &out[21],
                   &out[11]);
  double_butterfly(step[25], cospi13_v, step[22], cospi19_v, &out[13],
                   &out[19]);
  double_butterfly(step[24], cospi29_v, step[23], cospi3_v, &out[29], &out[3]);

  if (pass == 0) {
    for (int i = 0; i < 32; i++) {
      out[i] = sub_round_shift(out[i]);
    }
  }
}

void vpx_fdct32x32_rd_vsx(const int16_t *input, tran_low_t *out, int stride) {
  int16x8_t temp0[32];
  int16x8_t temp1[32];
  int16x8_t temp2[32];
  int16x8_t temp3[32];
  int16x8_t temp4[32];
  int16x8_t temp5[32];
  int16x8_t temp6[32];

  // Process in 8x32 columns.
  load(input, stride, temp0);
  vpx_fdct32_vsx(temp0, temp1, 0);

  load(input + 8, stride, temp0);
  vpx_fdct32_vsx(temp0, temp2, 0);

  load(input + 16, stride, temp0);
  vpx_fdct32_vsx(temp0, temp3, 0);

  load(input + 24, stride, temp0);
  vpx_fdct32_vsx(temp0, temp4, 0);

  // Generate the top row by munging the first set of 8 from each one together.
  transpose_8x8(&temp1[0], &temp0[0]);
  transpose_8x8(&temp2[0], &temp0[8]);
  transpose_8x8(&temp3[0], &temp0[16]);
  transpose_8x8(&temp4[0], &temp0[24]);

  vpx_fdct32_vsx(temp0, temp5, 1);

  transpose_8x8(&temp5[0], &temp6[0]);
  transpose_8x8(&temp5[8], &temp6[8]);
  transpose_8x8(&temp5[16], &temp6[16]);
  transpose_8x8(&temp5[24], &temp6[24]);

  store(out, temp6);

  // Second row of 8x32.
  transpose_8x8(&temp1[8], &temp0[0]);
  transpose_8x8(&temp2[8], &temp0[8]);
  transpose_8x8(&temp3[8], &temp0[16]);
  transpose_8x8(&temp4[8], &temp0[24]);

  vpx_fdct32_vsx(temp0, temp5, 1);

  transpose_8x8(&temp5[0], &temp6[0]);
  transpose_8x8(&temp5[8], &temp6[8]);
  transpose_8x8(&temp5[16], &temp6[16]);
  transpose_8x8(&temp5[24], &temp6[24]);

  store(out + 8 * 32, temp6);

  // Third row of 8x32
  transpose_8x8(&temp1[16], &temp0[0]);
  transpose_8x8(&temp2[16], &temp0[8]);
  transpose_8x8(&temp3[16], &temp0[16]);
  transpose_8x8(&temp4[16], &temp0[24]);

  vpx_fdct32_vsx(temp0, temp5, 1);

  transpose_8x8(&temp5[0], &temp6[0]);
  transpose_8x8(&temp5[8], &temp6[8]);
  transpose_8x8(&temp5[16], &temp6[16]);
  transpose_8x8(&temp5[24], &temp6[24]);

  store(out + 16 * 32, temp6);

  // Final row of 8x32.
  transpose_8x8(&temp1[24], &temp0[0]);
  transpose_8x8(&temp2[24], &temp0[8]);
  transpose_8x8(&temp3[24], &temp0[16]);
  transpose_8x8(&temp4[24], &temp0[24]);

  vpx_fdct32_vsx(temp0, temp5, 1);

  transpose_8x8(&temp5[0], &temp6[0]);
  transpose_8x8(&temp5[8], &temp6[8]);
  transpose_8x8(&temp5[16], &temp6[16]);
  transpose_8x8(&temp5[24], &temp6[24]);

  store(out + 24 * 32, temp6);
}
