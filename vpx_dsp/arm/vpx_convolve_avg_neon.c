/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <arm_neon.h>

#include "./vpx_dsp_rtcd.h"
#include "vpx/vpx_integer.h"

void vpx_convolve_avg_neon_org(const uint8_t *src,    // r0
                               ptrdiff_t src_stride,  // r1
                               uint8_t *dst,          // r2
                               ptrdiff_t dst_stride,  // r3
                               const int16_t *filter_x, int filter_x_stride,
                               const int16_t *filter_y, int filter_y_stride,
                               int w, int h) {
  uint8_t *d;
  uint8x8_t d0u8, d1u8, d2u8, d3u8;
  uint32x2_t d0u32, d2u32;
  uint8x16_t q0u8, q1u8, q2u8, q3u8, q8u8, q9u8, q10u8, q11u8;
  (void)filter_x;
  (void)filter_x_stride;
  (void)filter_y;
  (void)filter_y_stride;

  d = dst;
  if (w > 32) {  // avg64
    for (; h > 0; h -= 1) {
      q0u8 = vld1q_u8(src);
      q1u8 = vld1q_u8(src + 16);
      q2u8 = vld1q_u8(src + 32);
      q3u8 = vld1q_u8(src + 48);
      src += src_stride;
      q8u8 = vld1q_u8(d);
      q9u8 = vld1q_u8(d + 16);
      q10u8 = vld1q_u8(d + 32);
      q11u8 = vld1q_u8(d + 48);
      d += dst_stride;

      q0u8 = vrhaddq_u8(q0u8, q8u8);
      q1u8 = vrhaddq_u8(q1u8, q9u8);
      q2u8 = vrhaddq_u8(q2u8, q10u8);
      q3u8 = vrhaddq_u8(q3u8, q11u8);

      vst1q_u8(dst, q0u8);
      vst1q_u8(dst + 16, q1u8);
      vst1q_u8(dst + 32, q2u8);
      vst1q_u8(dst + 48, q3u8);
      dst += dst_stride;
    }
  } else if (w == 32) {  // avg32
    for (; h > 0; h -= 2) {
      q0u8 = vld1q_u8(src);
      q1u8 = vld1q_u8(src + 16);
      src += src_stride;
      q2u8 = vld1q_u8(src);
      q3u8 = vld1q_u8(src + 16);
      src += src_stride;
      q8u8 = vld1q_u8(d);
      q9u8 = vld1q_u8(d + 16);
      d += dst_stride;
      q10u8 = vld1q_u8(d);
      q11u8 = vld1q_u8(d + 16);
      d += dst_stride;

      q0u8 = vrhaddq_u8(q0u8, q8u8);
      q1u8 = vrhaddq_u8(q1u8, q9u8);
      q2u8 = vrhaddq_u8(q2u8, q10u8);
      q3u8 = vrhaddq_u8(q3u8, q11u8);

      vst1q_u8(dst, q0u8);
      vst1q_u8(dst + 16, q1u8);
      dst += dst_stride;
      vst1q_u8(dst, q2u8);
      vst1q_u8(dst + 16, q3u8);
      dst += dst_stride;
    }
  } else if (w > 8) {  // avg16
    for (; h > 0; h -= 2) {
      q0u8 = vld1q_u8(src);
      src += src_stride;
      q1u8 = vld1q_u8(src);
      src += src_stride;
      q2u8 = vld1q_u8(d);
      d += dst_stride;
      q3u8 = vld1q_u8(d);
      d += dst_stride;

      q0u8 = vrhaddq_u8(q0u8, q2u8);
      q1u8 = vrhaddq_u8(q1u8, q3u8);

      vst1q_u8(dst, q0u8);
      dst += dst_stride;
      vst1q_u8(dst, q1u8);
      dst += dst_stride;
    }
  } else if (w == 8) {  // avg8
    for (; h > 0; h -= 2) {
      d0u8 = vld1_u8(src);
      src += src_stride;
      d1u8 = vld1_u8(src);
      src += src_stride;
      d2u8 = vld1_u8(d);
      d += dst_stride;
      d3u8 = vld1_u8(d);
      d += dst_stride;

      q0u8 = vcombine_u8(d0u8, d1u8);
      q1u8 = vcombine_u8(d2u8, d3u8);
      q0u8 = vrhaddq_u8(q0u8, q1u8);

      vst1_u8(dst, vget_low_u8(q0u8));
      dst += dst_stride;
      vst1_u8(dst, vget_high_u8(q0u8));
      dst += dst_stride;
    }
  } else {  // avg4
    for (; h > 0; h -= 2) {
      d0u32 = vld1_lane_u32((const uint32_t *)src, d0u32, 0);
      src += src_stride;
      d0u32 = vld1_lane_u32((const uint32_t *)src, d0u32, 1);
      src += src_stride;
      d2u32 = vld1_lane_u32((const uint32_t *)d, d2u32, 0);
      d += dst_stride;
      d2u32 = vld1_lane_u32((const uint32_t *)d, d2u32, 1);
      d += dst_stride;

      d0u8 = vrhadd_u8(vreinterpret_u8_u32(d0u32), vreinterpret_u8_u32(d2u32));

      d0u32 = vreinterpret_u32_u8(d0u8);
      vst1_lane_u32((uint32_t *)dst, d0u32, 0);
      dst += dst_stride;
      vst1_lane_u32((uint32_t *)dst, d0u32, 1);
      dst += dst_stride;
    }
  }
  return;
}

void vpx_convolve_avg_neon(const uint8_t *src, ptrdiff_t src_stride,
                           uint8_t *dst, ptrdiff_t dst_stride,
                           const int16_t *filter_x, int filter_x_stride,
                           const int16_t *filter_y, int filter_y_stride, int w,
                           int h) {
  uint8x8_t ss0, ss1, dd0, dd1;
  uint8x16_t s0, s1, s2, s3, d0, d1, d2, d3;
  (void)filter_x;
  (void)filter_x_stride;
  (void)filter_y;
  (void)filter_y_stride;

  if (w < 8) {  // avg4
    uint32x2x2_t s01;
    do {
      ss0 = vld1_u8(src);
      src += src_stride;
      ss1 = vld1_u8(src);
      src += src_stride;
      s01 = vzip_u32(vreinterpret_u32_u8(ss0), vreinterpret_u32_u8(ss1));
      dd0 = vreinterpret_u8_u32(
          vld1_lane_u32((const uint32_t *)dst, vreinterpret_u32_u8(dd0), 0));
      dd0 = vreinterpret_u8_u32(vld1_lane_u32(
          (const uint32_t *)(dst + dst_stride), vreinterpret_u32_u8(dd0), 1));
      dd0 = vrhadd_u8(vreinterpret_u8_u32(s01.val[0]), dd0);
      vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(dd0), 0);
      dst += dst_stride;
      vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(dd0), 1);
      dst += dst_stride;
      h -= 2;
    } while (h > 0);
  } else if (w == 8) {  // avg8
    do {
      ss0 = vld1_u8(src);
      src += src_stride;
      ss1 = vld1_u8(src);
      src += src_stride;
      dd0 = vld1_u8(dst);
      dd1 = vld1_u8(dst + dst_stride);

      s0 = vcombine_u8(ss0, ss1);
      d0 = vcombine_u8(dd0, dd1);
      d0 = vrhaddq_u8(s0, d0);

      vst1_u8(dst, vget_low_u8(d0));
      dst += dst_stride;
      vst1_u8(dst, vget_high_u8(d0));
      dst += dst_stride;
      h -= 2;
    } while (h > 0);
  } else if (w < 32) {  // avg16
    do {
      s0 = vld1q_u8(src);
      src += src_stride;
      s1 = vld1q_u8(src);
      src += src_stride;
      d0 = vld1q_u8(dst);
      d1 = vld1q_u8(dst + dst_stride);

      d0 = vrhaddq_u8(s0, d0);
      d1 = vrhaddq_u8(s1, d1);

      vst1q_u8(dst, d0);
      dst += dst_stride;
      vst1q_u8(dst, d1);
      dst += dst_stride;
      h -= 2;
    } while (h > 0);
  } else if (w == 32) {  // avg32
    do {
      s0 = vld1q_u8(src);
      s1 = vld1q_u8(src + 16);
      src += src_stride;
      s2 = vld1q_u8(src);
      s3 = vld1q_u8(src + 16);
      src += src_stride;
      d0 = vld1q_u8(dst);
      d1 = vld1q_u8(dst + 16);
      d2 = vld1q_u8(dst + dst_stride);
      d3 = vld1q_u8(dst + dst_stride + 16);

      d0 = vrhaddq_u8(s0, d0);
      d1 = vrhaddq_u8(s1, d1);
      d2 = vrhaddq_u8(s2, d2);
      d3 = vrhaddq_u8(s3, d3);

      vst1q_u8(dst, d0);
      vst1q_u8(dst + 16, d1);
      dst += dst_stride;
      vst1q_u8(dst, d2);
      vst1q_u8(dst + 16, d3);
      dst += dst_stride;
      h -= 2;
    } while (h > 0);
  } else {  // avg64
    do {
      s0 = vld1q_u8(src);
      s1 = vld1q_u8(src + 16);
      s2 = vld1q_u8(src + 32);
      s3 = vld1q_u8(src + 48);
      src += src_stride;
      d0 = vld1q_u8(dst);
      d1 = vld1q_u8(dst + 16);
      d2 = vld1q_u8(dst + 32);
      d3 = vld1q_u8(dst + 48);

      d0 = vrhaddq_u8(s0, d0);
      d1 = vrhaddq_u8(s1, d1);
      d2 = vrhaddq_u8(s2, d2);
      d3 = vrhaddq_u8(s3, d3);

      vst1q_u8(dst, d0);
      vst1q_u8(dst + 16, d1);
      vst1q_u8(dst + 32, d2);
      vst1q_u8(dst + 48, d3);
      dst += dst_stride;
    } while (--h);
  }
}
