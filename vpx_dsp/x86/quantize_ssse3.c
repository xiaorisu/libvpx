/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <tmmintrin.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx/vpx_integer.h"
#include "vpx_dsp/x86/bitdepth_conversion_sse2.h"

void vpx_quantize_b_ssse3(const tran_low_t *coeff_ptr, intptr_t n_coeffs,
                          int skip_block, const int16_t *zbin_ptr,
                          const int16_t *round_ptr, const int16_t *quant_ptr,
                          const int16_t *quant_shift_ptr,
                          tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr,
                          const int16_t *dequant_ptr, uint16_t *eob_ptr,
                          const int16_t *scan_ptr, const int16_t *iscan_ptr) {
  const __m128i zero = _mm_setzero_si128();
  intptr_t index = 16;

  __m128i zbin, round, quant, dequant, shift;
  __m128i coeff0, coeff1;
  __m128i qcoeff0, qcoeff1;
  __m128i cmp_mask0, cmp_mask1;
  __m128i qtmp0, qtmp1;
  __m128i zero_coeff0, zero_coeff1, iscan0, iscan1;
  __m128i eob, eob0, eob1;

  (void)scan_ptr;
  (void)skip_block;
  assert(!skip_block);

  // Setup global values.
  zbin = _mm_load_si128((const __m128i *)zbin_ptr);
  // x86 has no "greater *or equal* comparison. Subtract 1 from zbin so
  // it is a strict "greater" comparison.
  zbin = _mm_sub_epi16(zbin, _mm_set1_epi16(1));
  round = _mm_load_si128((const __m128i *)round_ptr);
  quant = _mm_load_si128((const __m128i *)quant_ptr);
  dequant = _mm_load_si128((const __m128i *)dequant_ptr);
  shift = _mm_load_si128((const __m128i *)quant_shift_ptr);

  // Do DC and first 15 AC.
  coeff0 = load_tran_low(coeff_ptr);
  coeff1 = load_tran_low(coeff_ptr + 8);

  qcoeff0 = _mm_abs_epi16(coeff0);
  qcoeff1 = _mm_abs_epi16(coeff1);

  cmp_mask0 = _mm_cmpgt_epi16(qcoeff0, zbin);
  zbin = _mm_unpackhi_epi64(zbin, zbin);  // Switch DC to AC
  cmp_mask1 = _mm_cmpgt_epi16(qcoeff1, zbin);

  qcoeff0 = _mm_adds_epi16(qcoeff0, round);
  round = _mm_unpackhi_epi64(round, round);
  qcoeff1 = _mm_adds_epi16(qcoeff1, round);

  qtmp0 = _mm_mulhi_epi16(qcoeff0, quant);
  quant = _mm_unpackhi_epi64(quant, quant);
  qtmp1 = _mm_mulhi_epi16(qcoeff1, quant);

  qtmp0 = _mm_add_epi16(qtmp0, qcoeff0);
  qtmp1 = _mm_add_epi16(qtmp1, qcoeff1);

  qcoeff0 = _mm_mulhi_epi16(qtmp0, shift);
  shift = _mm_unpackhi_epi64(shift, shift);
  qcoeff1 = _mm_mulhi_epi16(qtmp1, shift);

  // Reinsert signs
  qcoeff0 = _mm_sign_epi16(qcoeff0, coeff0);
  qcoeff1 = _mm_sign_epi16(qcoeff1, coeff1);

  // Mask out zbin threshold coeffs
  qcoeff0 = _mm_and_si128(qcoeff0, cmp_mask0);
  qcoeff1 = _mm_and_si128(qcoeff1, cmp_mask1);

  store_tran_low(qcoeff0, qcoeff_ptr);
  store_tran_low(qcoeff1, qcoeff_ptr + 8);

  coeff0 = _mm_mullo_epi16(qcoeff0, dequant);
  dequant = _mm_unpackhi_epi64(dequant, dequant);
  coeff1 = _mm_mullo_epi16(qcoeff1, dequant);

  store_tran_low(coeff0, dqcoeff_ptr);
  store_tran_low(coeff1, dqcoeff_ptr + 8);

  // Scan for eob.
  zero_coeff0 = _mm_cmpeq_epi16(coeff0, zero);
  zero_coeff1 = _mm_cmpeq_epi16(coeff1, zero);
  iscan0 = _mm_load_si128((const __m128i *)(iscan_ptr));
  iscan1 = _mm_load_si128((const __m128i *)(iscan_ptr + 8));
  // Add one to convert from indices to counts
  iscan0 = _mm_sub_epi16(iscan0, cmp_mask0);
  iscan1 = _mm_sub_epi16(iscan1, cmp_mask1);
  eob = _mm_andnot_si128(zero_coeff0, iscan0);
  eob1 = _mm_andnot_si128(zero_coeff1, iscan1);
  eob = _mm_max_epi16(eob, eob1);

  // AC only loop.
  while (index < n_coeffs) {
    coeff0 = load_tran_low(coeff_ptr + index);
    coeff1 = load_tran_low(coeff_ptr + index + 8);

    qcoeff0 = _mm_abs_epi16(coeff0);
    qcoeff1 = _mm_abs_epi16(coeff1);

    cmp_mask0 = _mm_cmpgt_epi16(qcoeff0, zbin);
    cmp_mask1 = _mm_cmpgt_epi16(qcoeff1, zbin);

    qcoeff0 = _mm_adds_epi16(qcoeff0, round);
    qcoeff1 = _mm_adds_epi16(qcoeff1, round);

    qtmp0 = _mm_mulhi_epi16(qcoeff0, quant);
    qtmp1 = _mm_mulhi_epi16(qcoeff1, quant);

    qtmp0 = _mm_add_epi16(qtmp0, qcoeff0);
    qtmp1 = _mm_add_epi16(qtmp1, qcoeff1);

    qcoeff0 = _mm_mulhi_epi16(qtmp0, shift);
    qcoeff1 = _mm_mulhi_epi16(qtmp1, shift);

    qcoeff0 = _mm_sign_epi16(qcoeff0, coeff0);
    qcoeff1 = _mm_sign_epi16(qcoeff1, coeff1);

    qcoeff0 = _mm_and_si128(qcoeff0, cmp_mask0);
    qcoeff1 = _mm_and_si128(qcoeff1, cmp_mask1);

    store_tran_low(qcoeff0, qcoeff_ptr + index);
    store_tran_low(qcoeff1, qcoeff_ptr + index + 8);

    coeff0 = _mm_mullo_epi16(qcoeff0, dequant);
    coeff1 = _mm_mullo_epi16(qcoeff1, dequant);

    store_tran_low(coeff0, dqcoeff_ptr + index);
    store_tran_low(coeff1, dqcoeff_ptr + index + 8);

    zero_coeff0 = _mm_cmpeq_epi16(coeff0, zero);
    zero_coeff1 = _mm_cmpeq_epi16(coeff1, zero);
    iscan0 = _mm_load_si128((const __m128i *)(iscan_ptr + index));
    iscan1 = _mm_load_si128((const __m128i *)(iscan_ptr + index + 8));
    iscan0 = _mm_sub_epi16(iscan0, cmp_mask0);
    iscan1 = _mm_sub_epi16(iscan1, cmp_mask1);
    eob0 = _mm_andnot_si128(zero_coeff0, iscan0);
    eob1 = _mm_andnot_si128(zero_coeff1, iscan1);
    eob0 = _mm_max_epi16(eob0, eob1);
    eob = _mm_max_epi16(eob, eob0);

    index += 16;
  }

  // Accumulate eob.
  {
    __m128i eob_shuffled;
    eob_shuffled = _mm_shuffle_epi32(eob, 0xe);
    eob = _mm_max_epi16(eob, eob_shuffled);
    eob_shuffled = _mm_shufflelo_epi16(eob, 0xe);
    eob = _mm_max_epi16(eob, eob_shuffled);
    eob_shuffled = _mm_shufflelo_epi16(eob, 0x1);
    eob = _mm_max_epi16(eob, eob_shuffled);
    //*eob_ptr = _mm_extract_epi16(eob, 1);
    *eob_ptr = _mm_cvtsi128_si32(eob);
  }
}

void new_vpx_quantize_b_32x32_ssse3(
    const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block,
    const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr,
    const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr,
    tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, uint16_t *eob_ptr,
    const int16_t *scan_ptr, const int16_t *iscan_ptr) {
  const __m128i zero = _mm_setzero_si128();
  const __m128i one = _mm_set1_epi16(1);
  intptr_t index = 16;

  __m128i zbin, round, quant, dequant, shift;
  __m128i coeff0, coeff1;
  __m128i qcoeff0, qcoeff1;
  __m128i cmp_mask0, cmp_mask1;
  __m128i qtmp0, qtmp1;
  __m128i zero_coeff0, zero_coeff1, iscan0, iscan1;
  __m128i eob, eob0, eob1;

  (void)scan_ptr;
  (void)n_coeffs;
  (void)skip_block;
  assert(!skip_block);

  // Setup global values
  // The 32x32 halves zbin and round.
  zbin = _mm_load_si128((const __m128i *)zbin_ptr);
  // Shift with rounding.
  zbin = _mm_add_epi16(zbin, one);
  zbin = _mm_srli_epi16(zbin, 1);
  // x86 has no "greater *or equal*" comparison. Subtract 1 from zbin so
  // it is a strict "greater" comparison.
  zbin = _mm_sub_epi16(zbin, one);

  round = _mm_load_si128((const __m128i *)round_ptr);
  round = _mm_add_epi16(round, one);
  round = _mm_srli_epi16(round, 1);

  quant = _mm_load_si128((const __m128i *)quant_ptr);
  dequant = _mm_load_si128((const __m128i *)dequant_ptr);
  shift = _mm_load_si128((const __m128i *)quant_shift_ptr);
  // I suspect this is not technically OK because quant_shift can be up
  // to 1 << 16 and shifting up again will outrange that, but the test is not
  // comprehensive enough to catch that and "it's been that way forever"
  shift = _mm_slli_epi16(shift, 1);

  // Do DC and first 15 AC
  coeff0 = load_tran_low(coeff_ptr);
  coeff1 = load_tran_low(coeff_ptr + 8);

  qcoeff0 = _mm_abs_epi16(coeff0);
  qcoeff1 = _mm_abs_epi16(coeff1);

  cmp_mask0 = _mm_cmpgt_epi16(qcoeff0, zbin);
  zbin = _mm_unpackhi_epi64(zbin, zbin);  // Switch DC to AC.
  cmp_mask1 = _mm_cmpgt_epi16(qcoeff1, zbin);

  qcoeff0 = _mm_adds_epi16(qcoeff0, round);
  round = _mm_unpackhi_epi64(round, round);
  qcoeff1 = _mm_adds_epi16(qcoeff1, round);

  qtmp0 = _mm_mulhi_epi16(qcoeff0, quant);
  quant = _mm_unpackhi_epi64(quant, quant);
  qtmp1 = _mm_mulhi_epi16(qcoeff1, quant);

  qtmp0 = _mm_add_epi16(qtmp0, qcoeff0);
  qtmp1 = _mm_add_epi16(qtmp1, qcoeff1);

  qcoeff0 = _mm_mulhi_epi16(qtmp0, shift);
  shift = _mm_unpackhi_epi64(shift, shift);
  qcoeff1 = _mm_mulhi_epi16(qtmp1, shift);

  // Reinsert signs
  qcoeff0 = _mm_sign_epi16(qcoeff0, coeff0);
  qcoeff1 = _mm_sign_epi16(qcoeff1, coeff1);

  // Mask out zbin threshold coeffs
  qcoeff0 = _mm_and_si128(qcoeff0, cmp_mask0);
  qcoeff1 = _mm_and_si128(qcoeff1, cmp_mask1);

  store_tran_low(qcoeff0, qcoeff_ptr);
  store_tran_low(qcoeff1, qcoeff_ptr + 8);

  {
    // Extend to 32 bits
    __m128i coeff01_lo = _mm_mullo_epi16(qcoeff0, dequant);
    __m128i coeff01_hi = _mm_mulhi_epi16(qcoeff0, dequant);
    __m128i coeff23_lo, coeff23_hi;
    __m128i coeff32_0, coeff32_1, coeff32_2, coeff32_3;
    __m128i coeff32_0_sign_bit, coeff32_1_sign_bit, coeff32_2_sign_bit,
        coeff32_3_sign_bit;
    dequant = _mm_unpackhi_epi64(dequant, dequant);
    coeff23_lo = _mm_mullo_epi16(qcoeff1, dequant);
    coeff23_hi = _mm_mulhi_epi16(qcoeff1, dequant);

    coeff32_0 = _mm_unpacklo_epi16(coeff01_lo, coeff01_hi);
    coeff32_1 = _mm_unpackhi_epi16(coeff01_lo, coeff01_hi);
    coeff32_2 = _mm_unpacklo_epi16(coeff23_lo, coeff23_hi);
    coeff32_3 = _mm_unpackhi_epi16(coeff23_lo, coeff23_hi);

    coeff32_0_sign_bit = _mm_slli_epi32(coeff32_0, 31);
    coeff32_1_sign_bit = _mm_slli_epi32(coeff32_1, 31);
    coeff32_2_sign_bit = _mm_slli_epi32(coeff32_2, 31);
    coeff32_3_sign_bit = _mm_slli_epi32(coeff32_3, 31);

    // Add 1 if negative to bias rounding towards 0.
    coeff32_0 = _mm_add_epi32(coeff32_0, coeff32_0_sign_bit);
    coeff32_1 = _mm_add_epi32(coeff32_1, coeff32_1_sign_bit);
    coeff32_2 = _mm_add_epi32(coeff32_2, coeff32_2_sign_bit);
    coeff32_3 = _mm_add_epi32(coeff32_3, coeff32_3_sign_bit);

    // Attempt to emulate truncation.
    coeff32_0 = _mm_slli_epi32(coeff32_0, 14);
    coeff32_1 = _mm_slli_epi32(coeff32_1, 14);
    coeff32_2 = _mm_slli_epi32(coeff32_2, 14);
    coeff32_3 = _mm_slli_epi32(coeff32_3, 14);

    // Logical shift.
    coeff32_0 = _mm_srai_epi32(coeff32_0, 15);
    coeff32_1 = _mm_srai_epi32(coeff32_1, 15);
    coeff32_2 = _mm_srai_epi32(coeff32_2, 15);
    coeff32_3 = _mm_srai_epi32(coeff32_3, 15);

    coeff0 = _mm_packs_epi32(coeff32_0, coeff32_1);
    coeff1 = _mm_packs_epi32(coeff32_2, coeff32_3);
  }

  store_tran_low(coeff0, dqcoeff_ptr);
  store_tran_low(coeff1, dqcoeff_ptr + 8);

  // Scan for eob
  zero_coeff0 = _mm_cmpeq_epi16(coeff0, zero);
  zero_coeff1 = _mm_cmpeq_epi16(coeff1, zero);
  iscan0 = _mm_load_si128((const __m128i *)(iscan_ptr));
  iscan1 = _mm_load_si128((const __m128i *)(iscan_ptr + 8));
  // Add one to convert from indices to counts
  iscan0 = _mm_sub_epi16(iscan0, cmp_mask0);
  iscan1 = _mm_sub_epi16(iscan1, cmp_mask1);
  eob = _mm_andnot_si128(zero_coeff0, iscan0);
  eob1 = _mm_andnot_si128(zero_coeff1, iscan1);
  eob = _mm_max_epi16(eob, eob1);

  // AC only loop
  do {
    coeff0 = load_tran_low(coeff_ptr + index);
    coeff1 = load_tran_low(coeff_ptr + index + 8);

    qcoeff0 = _mm_abs_epi16(coeff0);
    qcoeff1 = _mm_abs_epi16(coeff1);

    cmp_mask0 = _mm_cmpgt_epi16(qcoeff0, zbin);
    cmp_mask1 = _mm_cmpgt_epi16(qcoeff1, zbin);

    qcoeff0 = _mm_adds_epi16(qcoeff0, round);
    qcoeff1 = _mm_adds_epi16(qcoeff1, round);

    qtmp0 = _mm_mulhi_epi16(qcoeff0, quant);
    qtmp1 = _mm_mulhi_epi16(qcoeff1, quant);

    qtmp0 = _mm_add_epi16(qtmp0, qcoeff0);
    qtmp1 = _mm_add_epi16(qtmp1, qcoeff1);

    qcoeff0 = _mm_mulhi_epi16(qtmp0, shift);
    qcoeff1 = _mm_mulhi_epi16(qtmp1, shift);

    qcoeff0 = _mm_sign_epi16(qcoeff0, coeff0);
    qcoeff1 = _mm_sign_epi16(qcoeff1, coeff1);

    qcoeff0 = _mm_and_si128(qcoeff0, cmp_mask0);
    qcoeff1 = _mm_and_si128(qcoeff1, cmp_mask1);

    store_tran_low(qcoeff0, qcoeff_ptr + index);
    store_tran_low(qcoeff1, qcoeff_ptr + index + 8);

    {
      // Extend to 32 bits
      __m128i coeff01_lo = _mm_mullo_epi16(qcoeff0, dequant);
      __m128i coeff01_hi = _mm_mulhi_epi16(qcoeff0, dequant);
      __m128i coeff23_lo, coeff23_hi;
      __m128i coeff32_0, coeff32_1, coeff32_2, coeff32_3;
      __m128i coeff32_0_sign_bit, coeff32_1_sign_bit, coeff32_2_sign_bit,
          coeff32_3_sign_bit;
      dequant = _mm_unpackhi_epi64(dequant, dequant);
      coeff23_lo = _mm_mullo_epi16(qcoeff1, dequant);
      coeff23_hi = _mm_mulhi_epi16(qcoeff1, dequant);

      coeff32_0 = _mm_unpacklo_epi16(coeff01_lo, coeff01_hi);
      coeff32_1 = _mm_unpackhi_epi16(coeff01_lo, coeff01_hi);
      coeff32_2 = _mm_unpacklo_epi16(coeff23_lo, coeff23_hi);
      coeff32_3 = _mm_unpackhi_epi16(coeff23_lo, coeff23_hi);

      coeff32_0_sign_bit = _mm_slli_epi32(coeff32_0, 31);
      coeff32_1_sign_bit = _mm_slli_epi32(coeff32_1, 31);
      coeff32_2_sign_bit = _mm_slli_epi32(coeff32_2, 31);
      coeff32_3_sign_bit = _mm_slli_epi32(coeff32_3, 31);

      coeff32_0 = _mm_add_epi32(coeff32_0, coeff32_0_sign_bit);
      coeff32_1 = _mm_add_epi32(coeff32_1, coeff32_1_sign_bit);
      coeff32_2 = _mm_add_epi32(coeff32_2, coeff32_2_sign_bit);
      coeff32_3 = _mm_add_epi32(coeff32_3, coeff32_3_sign_bit);

      coeff32_0 = _mm_slli_epi32(coeff32_0, 14);
      coeff32_1 = _mm_slli_epi32(coeff32_1, 14);
      coeff32_2 = _mm_slli_epi32(coeff32_2, 14);
      coeff32_3 = _mm_slli_epi32(coeff32_3, 14);

      coeff32_0 = _mm_srai_epi32(coeff32_0, 15);
      coeff32_1 = _mm_srai_epi32(coeff32_1, 15);
      coeff32_2 = _mm_srai_epi32(coeff32_2, 15);
      coeff32_3 = _mm_srai_epi32(coeff32_3, 15);

      coeff0 = _mm_packs_epi32(coeff32_0, coeff32_1);
      coeff1 = _mm_packs_epi32(coeff32_2, coeff32_3);
    }

    store_tran_low(coeff0, dqcoeff_ptr + index);
    store_tran_low(coeff1, dqcoeff_ptr + index + 8);

    // Scan for eob
    zero_coeff0 = _mm_cmpeq_epi16(qcoeff0, zero);
    zero_coeff1 = _mm_cmpeq_epi16(qcoeff1, zero);
    iscan0 = _mm_load_si128((const __m128i *)(iscan_ptr + index));
    iscan1 = _mm_load_si128((const __m128i *)(iscan_ptr + index + 8));
    // Add one to convert from indices to counts
    iscan0 = _mm_sub_epi16(iscan0, cmp_mask0);
    iscan1 = _mm_sub_epi16(iscan1, cmp_mask1);
    eob0 = _mm_andnot_si128(zero_coeff0, iscan0);
    eob1 = _mm_andnot_si128(zero_coeff1, iscan1);
    eob0 = _mm_max_epi16(eob0, eob1);
    eob = _mm_max_epi16(eob, eob0);

    index += 16;
  } while (index < 32 * 32);

  // Accumulate EOB
  {
    __m128i eob_shuffled;
    eob_shuffled = _mm_shuffle_epi32(eob, 0xe);
    eob = _mm_max_epi16(eob, eob_shuffled);
    eob_shuffled = _mm_shufflelo_epi16(eob, 0xe);
    eob = _mm_max_epi16(eob, eob_shuffled);
    eob_shuffled = _mm_shufflelo_epi16(eob, 0x1);
    eob = _mm_max_epi16(eob, eob_shuffled);
    *eob_ptr = _mm_extract_epi16(eob, 1);
  }
}
