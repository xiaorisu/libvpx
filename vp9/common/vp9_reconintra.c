/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vpx_config.h"

#include "vpx_mem/vpx_mem.h"
#include "vpx_ports/vpx_once.h"

#include "./vp9_rtcd.h"

#include "vp9/common/vp9_reconintra.h"
#include "vp9/common/vp9_onyxc_int.h"
#include "vpx_config.h"

const TX_TYPE intra_mode_to_tx_type_lookup[INTRA_MODES] = {
  DCT_DCT,    // DC
  ADST_DCT,   // V
  DCT_ADST,   // H
  DCT_DCT,    // D45
  ADST_ADST,  // D135
  ADST_DCT,   // D117
  DCT_ADST,   // D153
  DCT_ADST,   // D207
  ADST_DCT,   // D63
  ADST_ADST,  // TM
};

#if CONFIG_B10_EXT
#define intra_pred_sized(type, size) \
  void vp9_##type##_predictor_##size##x##size##_c(uint16_t *dst, \
                                                  ptrdiff_t stride, \
                                                  const uint16_t *above, \
                                                  const uint16_t *left) { \
    type##_predictor(dst, stride, size, above, left); \
  }
#else
#define intra_pred_sized(type, size) \
  void vp9_##type##_predictor_##size##x##size##_c(uint8_t *dst, \
                                                  ptrdiff_t stride, \
                                                  const uint8_t *above, \
                                                  const uint8_t *left) { \
    type##_predictor(dst, stride, size, above, left); \
  }
#endif

#define intra_pred_allsizes(type) \
  intra_pred_sized(type, 4) \
  intra_pred_sized(type, 8) \
  intra_pred_sized(type, 16) \
  intra_pred_sized(type, 32)

#if CONFIG_B10_EXT
static INLINE void d207_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                  const uint16_t *above, const uint16_t *left) {
#else
static INLINE void d207_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
#endif
  int r, c;

  // first column
  for (r = 0; r < bs - 1; ++r)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r] + left[r + 1], 1);
  dst[(bs - 1) * stride] = left[bs - 1];
  dst++;

  // second column
  for (r = 0; r < bs - 2; ++r)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r] + left[r + 1] * 2 +
                                         left[r + 2], 2);
  dst[(bs - 2) * stride] = ROUND_POWER_OF_TWO(left[bs - 2] +
                                              left[bs - 1] * 3, 2);
  dst[(bs - 1) * stride] = left[bs - 1];
  dst++;

  // rest of last row
  for (c = 0; c < bs - 2; ++c)
    dst[(bs - 1) * stride + c] = left[bs - 1];

  for (r = bs - 2; r >= 0; --r)
    for (c = 0; c < bs - 2; ++c)
      dst[r * stride + c] = dst[(r + 1) * stride + c - 2];
}
intra_pred_allsizes(d207)

#if CONFIG_B10_EXT
static INLINE void d63_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                 const uint16_t *above, const uint16_t *left) {
#else
static INLINE void d63_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                 const uint8_t *above, const uint8_t *left) {
#endif
  int r, c;
  for (r = 0; r < bs; ++r) {
    for (c = 0; c < bs; ++c)
      dst[c] = r & 1 ? ROUND_POWER_OF_TWO(above[r/2 + c] +
                                          above[r/2 + c + 1] * 2 +
                                          above[r/2 + c + 2], 2)
                     : ROUND_POWER_OF_TWO(above[r/2 + c] +
                                          above[r/2 + c + 1], 1);
    dst += stride;
  }
}
intra_pred_allsizes(d63)

#if CONFIG_B10_EXT
static INLINE void d45_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                 const uint16_t *above, const uint16_t *left) {
#else
static INLINE void d45_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                 const uint8_t *above, const uint8_t *left) {
#endif
  int r, c;
  for (r = 0; r < bs; ++r) {
    for (c = 0; c < bs; ++c)
      dst[c] = r + c + 2 < bs * 2 ?  ROUND_POWER_OF_TWO(above[r + c] +
                                                        above[r + c + 1] * 2 +
                                                        above[r + c + 2], 2)
                                  : above[bs * 2 - 1];
    dst += stride;
  }
}
intra_pred_allsizes(d45)

#if CONFIG_B10_EXT
static INLINE void d117_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                  const uint16_t *above, const uint16_t *left) {
#else
static INLINE void d117_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
#endif
  int r, c;

  // first row
  for (c = 0; c < bs; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 1] + above[c], 1);
  dst += stride;

  // second row
  dst[0] = ROUND_POWER_OF_TWO(left[0] + above[-1] * 2 + above[0], 2);
  for (c = 1; c < bs; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 2] + above[c - 1] * 2 + above[c], 2);
  dst += stride;

  // the rest of first col
  dst[0] = ROUND_POWER_OF_TWO(above[-1] + left[0] * 2 + left[1], 2);
  for (r = 3; r < bs; ++r)
    dst[(r - 2) * stride] = ROUND_POWER_OF_TWO(left[r - 3] + left[r - 2] * 2 +
                                               left[r - 1], 2);

  // the rest of the block
  for (r = 2; r < bs; ++r) {
    for (c = 1; c < bs; c++)
      dst[c] = dst[-2 * stride + c - 1];
    dst += stride;
  }
}
intra_pred_allsizes(d117)

#if CONFIG_B10_EXT
static INLINE void d135_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                  const uint16_t *above, const uint16_t *left) {
#else
static INLINE void d135_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
#endif
  int r, c;
  dst[0] = ROUND_POWER_OF_TWO(left[0] + above[-1] * 2 + above[0], 2);
  for (c = 1; c < bs; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 2] + above[c - 1] * 2 + above[c], 2);

  dst[stride] = ROUND_POWER_OF_TWO(above[-1] + left[0] * 2 + left[1], 2);
  for (r = 2; r < bs; ++r)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r - 2] + left[r - 1] * 2 +
                                         left[r], 2);

  dst += stride;
  for (r = 1; r < bs; ++r) {
    for (c = 1; c < bs; c++)
      dst[c] = dst[-stride + c - 1];
    dst += stride;
  }
}
intra_pred_allsizes(d135)

#if CONFIG_B10_EXT
static INLINE void d153_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                  const uint16_t *above, const uint16_t *left) {
#else
static INLINE void d153_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                  const uint8_t *above, const uint8_t *left) {
#endif
  int r, c;
  dst[0] = ROUND_POWER_OF_TWO(above[-1] + left[0], 1);
  for (r = 1; r < bs; r++)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r - 1] + left[r], 1);
  dst++;

  dst[0] = ROUND_POWER_OF_TWO(left[0] + above[-1] * 2 + above[0], 2);
  dst[stride] = ROUND_POWER_OF_TWO(above[-1] + left[0] * 2 + left[1], 2);
  for (r = 2; r < bs; r++)
    dst[r * stride] = ROUND_POWER_OF_TWO(left[r - 2] + left[r - 1] * 2 +
                                         left[r], 2);
  dst++;

  for (c = 0; c < bs - 2; c++)
    dst[c] = ROUND_POWER_OF_TWO(above[c - 1] + above[c] * 2 + above[c + 1], 2);
  dst += stride;

  for (r = 1; r < bs; ++r) {
    for (c = 0; c < bs - 2; c++)
      dst[c] = dst[-stride + c - 2];
    dst += stride;
  }
}
intra_pred_allsizes(d153)

#if CONFIG_B10_EXT
static INLINE void v_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                               const uint16_t *above, const uint16_t *left) {
  int r, c;
#else
static INLINE void v_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                               const uint8_t *above, const uint8_t *left) {
  int r;
#endif

  for (r = 0; r < bs; r++) {
#if CONFIG_B10_EXT
   // vpx_memcpy(dst, above,2*bs);
    for(c = 0; c < bs; c++) { dst[c] = above[c]; }
#else
    vpx_memcpy(dst, above, bs);
#endif
    dst += stride;
  }
}
intra_pred_allsizes(v)

#if CONFIG_B10_EXT
static INLINE void h_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                               const uint16_t *above, const uint16_t *left) {
  int r, c;
#else
static INLINE void h_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                               const uint8_t *above, const uint8_t *left) {
  int r;
#endif

  for (r = 0; r < bs; r++) {
#if CONFIG_B10_EXT
    for(c=0; c < bs; c++) { dst[c]=left[r]; }
#else
    vpx_memset(dst, left[r], bs);
#endif
    dst += stride;
  }
}
intra_pred_allsizes(h)

#if CONFIG_B10_EXT
static INLINE void tm_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                const uint16_t *above, const uint16_t *left) {
#else
static INLINE void tm_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                const uint8_t *above, const uint8_t *left) {
#endif
  int r, c;
  int ytop_left = above[-1];

  for (r = 0; r < bs; r++) {
    for (c = 0; c < bs; c++)
#if CONFIG_B10_EXT
      dst[c] = clamp(left[r] + above[c] - ytop_left, 0, 255);
#else
      dst[c] = clip_pixel(left[r] + above[c] - ytop_left);
#endif
    dst += stride;
  }
}
intra_pred_allsizes(tm)

#if CONFIG_B10_EXT
static INLINE void dc_128_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                    const uint16_t *above, const uint16_t *left) {
  int r, c;
#else
static INLINE void dc_128_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                    const uint8_t *above, const uint8_t *left) {
  int r;
#endif

  for (r = 0; r < bs; r++) {
#if CONFIG_B10_EXT
     for(c=0; c < bs; c++) { dst[c]=128; }
#else
    vpx_memset(dst, 128, bs);
#endif
    dst += stride;
  }
}
intra_pred_allsizes(dc_128)

#if CONFIG_B10_EXT
static INLINE void dc_left_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                     const uint16_t *above,
                                     const uint16_t *left) {
  int i, r, c, expected_dc, sum = 0;
#else
static INLINE void dc_left_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                     const uint8_t *above,
                                     const uint8_t *left) {
  int i, r, expected_dc, sum = 0;
#endif

  for (i = 0; i < bs; i++)
    sum += left[i];
  expected_dc = (sum + (bs >> 1)) / bs;

  for (r = 0; r < bs; r++) {
#if CONFIG_B10_EXT
    for(c=0; c < bs; c++) { dst[c]=expected_dc;}
#else
    vpx_memset(dst, expected_dc, bs);
#endif
    dst += stride;
  }
}
intra_pred_allsizes(dc_left)

#if CONFIG_B10_EXT
static INLINE void dc_top_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                    const uint16_t *above, const uint16_t *left) {
  int i, r, c, expected_dc, sum = 0;
#else
static INLINE void dc_top_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                    const uint8_t *above, const uint8_t *left) {
  int i, r, expected_dc, sum = 0;
#endif

  for (i = 0; i < bs; i++)
    sum += above[i];
  expected_dc = (sum + (bs >> 1)) / bs;

  for (r = 0; r < bs; r++) {
#if CONFIG_B10_EXT
     for(c=0; c < bs; c++) { dst[c]=expected_dc; }
#else
    vpx_memset(dst, expected_dc, bs);
#endif
    dst += stride;
  }
}
intra_pred_allsizes(dc_top)

#if CONFIG_B10_EXT
static INLINE void dc_predictor(uint16_t *dst, ptrdiff_t stride, int bs,
                                const uint16_t *above, const uint16_t *left) {
  int i, r, c, expected_dc, sum = 0;
#else
static INLINE void dc_predictor(uint8_t *dst, ptrdiff_t stride, int bs,
                                const uint8_t *above, const uint8_t *left) {
  int i, r, expected_dc, sum = 0;
#endif
  const int count = 2 * bs;

  for (i = 0; i < bs; i++) {
    sum += above[i];
    sum += left[i];
  }

  expected_dc = (sum + (count >> 1)) / count;

  for (r = 0; r < bs; r++) {
#if CONFIG_B10_EXT
    for(c=0; c < bs; c++) { dst[c]=expected_dc; }
#else
    vpx_memset(dst, expected_dc, bs);
#endif
    dst += stride;
  }
}
intra_pred_allsizes(dc)
#undef intra_pred_allsizes

#if CONFIG_B10_EXT
typedef void (*intra_pred_fn)(uint16_t *dst, ptrdiff_t stride,
                              const uint16_t *above, const uint16_t *left);
#else
typedef void (*intra_pred_fn)(uint8_t *dst, ptrdiff_t stride,
                              const uint8_t *above, const uint8_t *left);
#endif

static intra_pred_fn pred[INTRA_MODES][4];
static intra_pred_fn dc_pred[2][2][4];

static void init_intra_pred_fn_ptrs(void) {
#define intra_pred_allsizes(l, type) \
  l[0] = vp9_##type##_predictor_4x4; \
  l[1] = vp9_##type##_predictor_8x8; \
  l[2] = vp9_##type##_predictor_16x16; \
  l[3] = vp9_##type##_predictor_32x32

  intra_pred_allsizes(pred[V_PRED], v);
  intra_pred_allsizes(pred[H_PRED], h);
  intra_pred_allsizes(pred[D207_PRED], d207);
  intra_pred_allsizes(pred[D45_PRED], d45);
  intra_pred_allsizes(pred[D63_PRED], d63);
  intra_pred_allsizes(pred[D117_PRED], d117);
  intra_pred_allsizes(pred[D135_PRED], d135);
  intra_pred_allsizes(pred[D153_PRED], d153);
  intra_pred_allsizes(pred[TM_PRED], tm);

  intra_pred_allsizes(dc_pred[0][0], dc_128);
  intra_pred_allsizes(dc_pred[0][1], dc_top);
  intra_pred_allsizes(dc_pred[1][0], dc_left);
  intra_pred_allsizes(dc_pred[1][1], dc);

#undef intra_pred_allsizes
}

#if CONFIG_B10_EXT
static void build_intra_predictors(const MACROBLOCKD *xd, const uint16_t *ref,
                                   int ref_stride, uint16_t *dst, int dst_stride,
#else
static void build_intra_predictors(const MACROBLOCKD *xd, const uint8_t *ref,
                                   int ref_stride, uint8_t *dst, int dst_stride,
#endif
                                   MB_PREDICTION_MODE mode, TX_SIZE tx_size,
                                   int up_available, int left_available,
                                   int right_available, int x, int y,
                                   int plane) {
  int i;
#if CONFIG_B10_EXT
  DECLARE_ALIGNED_ARRAY(16, uint16_t, left_col, 64);
  DECLARE_ALIGNED_ARRAY(16, uint16_t, above_data, 128 + 16);
  uint16_t *above_row = above_data + 16;
  const uint16_t *const_above_row = above_row;
#else
  DECLARE_ALIGNED_ARRAY(16, uint8_t, left_col, 64);
  DECLARE_ALIGNED_ARRAY(16, uint8_t, above_data, 128 + 16);
  uint8_t *above_row = above_data + 16;
  const uint8_t *const_above_row = above_row;
#endif
  const int bs = 4 << tx_size;
  int frame_width, frame_height;
  int x0, y0;
  const struct macroblockd_plane *const pd = &xd->plane[plane];

  // 127 127 127 .. 127 127 127 127 127 127
  // 129  A   B  ..  Y   Z
  // 129  C   D  ..  W   X
  // 129  E   F  ..  U   V
  // 129  G   H  ..  S   T   T   T   T   T
  // ..

  once(init_intra_pred_fn_ptrs);

  // Get current frame pointer, width and height.
  if (plane == 0) {
    frame_width = xd->cur_buf->y_width;
    frame_height = xd->cur_buf->y_height;
  } else {
    frame_width = xd->cur_buf->uv_width;
    frame_height = xd->cur_buf->uv_height;
  }

  // Get block position in current frame.
  x0 = (-xd->mb_to_left_edge >> (3 + pd->subsampling_x)) + x;
  y0 = (-xd->mb_to_top_edge >> (3 + pd->subsampling_y)) + y;

  // left
  if (left_available) {
    if (xd->mb_to_bottom_edge < 0) {
      /* slower path if the block needs border extension */
      if (y0 + bs <= frame_height) {
        for (i = 0; i < bs; ++i)
          left_col[i] = ref[i * ref_stride - 1];
      } else {
        const int extend_bottom = frame_height - y0;
        for (i = 0; i < extend_bottom; ++i)
          left_col[i] = ref[i * ref_stride - 1];
        for (; i < bs; ++i)
          left_col[i] = ref[(extend_bottom - 1) * ref_stride - 1];
      }
    } else {
      /* faster path if the block does not need extension */
      for (i = 0; i < bs; ++i)
        left_col[i] = ref[i * ref_stride - 1];
    }
  } else {
#if CONFIG_B10_EXT
   for (i = 0; i < bs; ++i) left_col[i]=129;
#else
    vpx_memset(left_col, 129, bs);
#endif
  }

  // TODO(hkuang) do not extend 2*bs pixels for all modes.
  // above
  if (up_available) {
#if CONFIG_B10_EXT
    const uint16_t *above_ref = ref - ref_stride;
#else
    const uint8_t *above_ref = ref - ref_stride;
#endif
    if (xd->mb_to_right_edge < 0) {
      /* slower path if the block needs border extension */
      if (x0 + 2 * bs <= frame_width) {
        if (right_available && bs == 4) {
#if CONFIG_B10_EXT
          for(i=0; i<2*bs; i++){ above_row[i]=above_ref[i];}
#else
          vpx_memcpy(above_row, above_ref, 2 * bs);
#endif
        } else {
#if CONFIG_B10_EXT
         for(i=0;i<bs;i++){ 
            above_row[i]=above_ref[i];
            above_row[bs+i]=above_ref[bs-1];
         }
#else
          vpx_memcpy(above_row, above_ref, bs);
          vpx_memset(above_row + bs, above_row[bs - 1], bs);
#endif
        }
      } else if (x0 + bs <= frame_width) {
        const int r = frame_width - x0;
        if (right_available && bs == 4) {
#if CONFIG_B10_EXT
          for(i=0; i<r; i++){ above_row[i] = above_ref[i];}
          for(i=0; i<x0+2*bs-frame_width; i++){
            above_row[r+i] = above_row[r-1];
          }
#else
          vpx_memcpy(above_row, above_ref, r);
          vpx_memset(above_row + r, above_row[r - 1],
                     x0 + 2 * bs - frame_width);
#endif
        } else {
#if CONFIG_B10_EXT
          for(i=0; i<bs; i++){
            above_row[i]=above_ref[i];
            above_row[bs+i]=above_ref[bs-1];
          }
#else
          vpx_memcpy(above_row, above_ref, bs);
          vpx_memset(above_row + bs, above_row[bs - 1], bs);
#endif
        }
      } else if (x0 <= frame_width) {
        const int r = frame_width - x0;
        if (right_available && bs == 4) {
#if CONFIG_B10_EXT
          for(i=0; i<r; i++){ above_row[i]=above_ref[i]; }
          for(i=0; i<x0+2*bs-frame_width; i++){ above_row[r+i]=above_row[r-1]; }
#else
          vpx_memcpy(above_row, above_ref, r);
          vpx_memset(above_row + r, above_row[r - 1],
                     x0 + 2 * bs - frame_width);
#endif
        } else {
#if CONFIG_B10_EXT
          for(i=0; i<r; i++){ above_row[i]=above_ref[i]; }
          for(i=0; i<x0+ 2*bs - frame_width; i++){
            above_row[r+i]=above_row[r-1];
          }
#else
          vpx_memcpy(above_row, above_ref, r);
          vpx_memset(above_row + r, above_row[r - 1],
                     x0 + 2 * bs - frame_width);
#endif
        }
      }
      above_row[-1] = left_available ? above_ref[-1] : 129;
    } else {
      /* faster path if the block does not need extension */
      if (bs == 4 && right_available && left_available) {
        const_above_row = above_ref;
      } else {
#if CONFIG_B10_EXT
        for(i=0; i<bs; i++){ above_row[i]=above_ref[i]; }
#else 
        vpx_memcpy(above_row, above_ref, bs);
#endif
        if (bs == 4 && right_available)
#if CONFIG_B10_EXT
        { for(i=0; i<bs; i++){ above_row[bs+i]=above_ref[bs+i]; } }
#else
          vpx_memcpy(above_row + bs, above_ref + bs, bs);
#endif
        else
#if CONFIG_B10_EXT
        { for(i=0; i<bs; i++){ above_row[bs+i]=above_row[bs-1];} }
#else
          vpx_memset(above_row + bs, above_row[bs - 1], bs);
#endif
        above_row[-1] = left_available ? above_ref[-1] : 129;
      }
    }
  } else {
#if CONFIG_B10_EXT
    for(i=0; i<bs*2; i++){ above_row[i]=127;} 
#else
    vpx_memset(above_row, 127, bs * 2);
#endif
    above_row[-1] = 127;
  }

  // predict
  if (mode == DC_PRED) {
    dc_pred[left_available][up_available][tx_size](dst, dst_stride,
                                                   const_above_row, left_col);
  } else {
    pred[mode][tx_size](dst, dst_stride, const_above_row, left_col);
  }
}

void vp9_predict_intra_block(const MACROBLOCKD *xd, int block_idx, int bwl_in,
                             TX_SIZE tx_size, MB_PREDICTION_MODE mode,
#if CONFIG_B10_EXT
                             const uint16_t *ref, int ref_stride,
                             uint16_t *dst, int dst_stride,
#else
                             const uint8_t *ref, int ref_stride,
                             uint8_t *dst, int dst_stride,
#endif
                             int aoff, int loff, int plane) {
  const int bwl = bwl_in - tx_size;
  const int wmask = (1 << bwl) - 1;
  const int have_top = (block_idx >> bwl) || xd->up_available;
  const int have_left = (block_idx & wmask) || xd->left_available;
  const int have_right = ((block_idx & wmask) != wmask);
  const int x = aoff * 4;
  const int y = loff * 4;

  assert(bwl >= 0);
  build_intra_predictors(xd, ref, ref_stride, dst, dst_stride, mode, tx_size,
                         have_top, have_left, have_right, x, y, plane);
}
