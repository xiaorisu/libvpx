/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_DSP_INTRAPRED_H_
#define VPX_DSP_INTRAPRED_H_

#include "./vpx_config.h"
#include "./vpx_dsp_rtcd.h"
#include "vpx_mem/vpx_mem.h"

INLINE void d207_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left);
INLINE void d63_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                          const uint8_t *above, const uint8_t *left);
INLINE void d45_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                          const uint8_t *above, const uint8_t *left);
INLINE void d117_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left);
INLINE void d135_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left);
INLINE void d153_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                           const uint8_t *above, const uint8_t *left);
INLINE void v_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                        const uint8_t *above, const uint8_t *left);
INLINE void h_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                        const uint8_t *above, const uint8_t *left);
INLINE void tm_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                         const uint8_t *above, const uint8_t *left);
INLINE void dc_128_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                             const uint8_t *above, const uint8_t *left);
INLINE void dc_left_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                              const uint8_t *above,
                              const uint8_t *left);
INLINE void dc_top_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                             const uint8_t *above, const uint8_t *left);
INLINE void dc_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                         const uint8_t *above, const uint8_t *left);

void vp9_d207_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                              const uint8_t *above, const uint8_t *left);
void vp9_d63_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                             const uint8_t *above, const uint8_t *left);
void vp9_d45_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                             const uint8_t *above, const uint8_t *left);
void vp9_d117_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                              const uint8_t *above, const uint8_t *left);
void vp9_d135_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                              const uint8_t *above, const uint8_t *left);
void vp9_d153_predictor_4x4_c(uint8_t *dst, ptrdiff_t stride,
                              const uint8_t *above, const uint8_t *left);

#if CONFIG_VP9_HIGHBITDEPTH
INLINE void highbd_d207_predictor(uint16_t *dst, ptrdiff_t stride,
                                  int bs, const uint16_t *above,
                                  const uint16_t *left, int bd);
INLINE void highbd_d63_predictor(uint16_t *dst, ptrdiff_t stride,
                                 int bs, const uint16_t *above,
                                 const uint16_t *left, int bd);
INLINE void highbd_d45_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                 const uint16_t *above,
                                 const uint16_t *left, int bd);
INLINE void highbd_d117_predictor(uint16_t *dst, ptrdiff_t stride,
                                  int bs, const uint16_t *above,
                                  const uint16_t *left, int bd);
INLINE void highbd_d135_predictor(uint16_t *dst, ptrdiff_t stride,
                                  int bs, const uint16_t *above,
                                  const uint16_t *left, int bd);
INLINE void highbd_d153_predictor(uint16_t *dst, ptrdiff_t stride,
                                  int bs, const uint16_t *above,
                                  const uint16_t *left, int bd);
INLINE void highbd_v_predictor(uint16_t *dst, ptrdiff_t stride,
                               int bs, const uint16_t *above,
                               const uint16_t *left, int bd);
INLINE void highbd_h_predictor(uint16_t *dst, ptrdiff_t stride,
                               int bs, const uint16_t *above,
                               const uint16_t *left, int bd);
INLINE void highbd_tm_predictor(uint16_t *dst, ptrdiff_t stride,
                                int bs, const uint16_t *above,
                                const uint16_t *left, int bd);
INLINE void highbd_dc_128_predictor(uint16_t *dst, ptrdiff_t stride,
                                    int bs, const uint16_t *above,
                                    const uint16_t *left, int bd);
INLINE void highbd_dc_left_predictor(uint16_t *dst, ptrdiff_t stride,
                                     int bs, const uint16_t *above,
                                     const uint16_t *left, int bd);
INLINE void highbd_dc_top_predictor(uint16_t *dst, ptrdiff_t stride,
                                    int bs, const uint16_t *above,
                                    const uint16_t *left, int bd);
INLINE void highbd_dc_predictor(uint16_t *dst, ptrdiff_t stride,
                                int bs, const uint16_t *above,
                                const uint16_t *left, int bd);
#endif  // CONFIG_VP9_HIGHBITDEPTH


// This serves as a wrapper function, so that all the prediction functions
// can be unified and accessed as a pointer array. Note that the boundary
// above and left are not necessarily used all the time.
#define declare_intra_pred_sized(type, size) \
  void vp9_##type##_predictor_##size##x##size##_c(uint8_t *dst, \
                                                  ptrdiff_t stride, \
                                                  const uint8_t *above, \
                                                  const uint8_t *left);

#if CONFIG_VP9_HIGHBITDEPTH
#define declare_intra_pred_highbd_sized(type, size) \
  void vp9_highbd_##type##_predictor_##size##x##size##_c( \
      uint16_t *dst, ptrdiff_t stride, const uint16_t *above, \
      const uint16_t *left, int bd);

#define declare_intra_pred_allsizes(type) \
  declare_intra_pred_sized(type, 4) \
  declare_intra_pred_sized(type, 8) \
  declare_intra_pred_sized(type, 16) \
  declare_intra_pred_sized(type, 32) \
  declare_intra_pred_highbd_sized(type, 4) \
  declare_intra_pred_highbd_sized(type, 8) \
  declare_intra_pred_highbd_sized(type, 16) \
  declare_intra_pred_highbd_sized(type, 32)

#define declare_intra_pred_no_4x4(type) \
  declare_intra_pred_sized(type, 8) \
  declare_intra_pred_sized(type, 16) \
  declare_intra_pred_sized(type, 32) \
  declare_intra_pred_highbd_sized(type, 4) \
  declare_intra_pred_highbd_sized(type, 8) \
  declare_intra_pred_highbd_sized(type, 16) \
  declare_intra_pred_highbd_sized(type, 32)

#else
#define declare_intra_pred_allsizes(type) \
  declare_intra_pred_sized(type, 4) \
  declare_intra_pred_sized(type, 8) \
  declare_intra_pred_sized(type, 16) \
  declare_intra_pred_sized(type, 32)

#define declare_intra_pred_no_4x4(type) \
  declare_intra_pred_sized(type, 8) \
  declare_intra_pred_sized(type, 16) \
  declare_intra_pred_sized(type, 32)
#endif  // CONFIG_VP9_HIGHBITDEPTH

#define DST(x, y) dst[(x) + (y) * stride]
#define AVG3(a, b, c) (((a) + 2 * (b) + (c) + 2) >> 2)
#define AVG2(a, b) (((a) + (b) + 1) >> 1)

declare_intra_pred_no_4x4(d207)
declare_intra_pred_no_4x4(d63)
declare_intra_pred_no_4x4(d45)
declare_intra_pred_no_4x4(d117)
declare_intra_pred_no_4x4(d135)
declare_intra_pred_no_4x4(d153)
declare_intra_pred_allsizes(v)
declare_intra_pred_allsizes(h)
declare_intra_pred_allsizes(tm)
declare_intra_pred_allsizes(dc_128)
declare_intra_pred_allsizes(dc_left)
declare_intra_pred_allsizes(dc_top)
declare_intra_pred_allsizes(dc)
#undef declare_intra_pred_allsizes

#endif  // VPX_DSP_INTRAPRED_H_
