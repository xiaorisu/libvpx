/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <limits.h>
#include <math.h>

#include "vp9/common/vp9_blockd.h"
#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_skin_detection.h"

int vp9_compute_skin_block(const uint8_t *y, const uint8_t *u, const uint8_t *v,
                           int stride, int strideuv, int bsize,
                           int consec_zeromv, int curr_motion_magn) {
  // No skin if block has been zero/small motion for long consecutive time.
  if (consec_zeromv > 60 && curr_motion_magn == 0) {
    return 0;
  } else {
    int motion = 1;
    // Take center pixel in block to determine is_skin.
    const int y_width_shift = (4 << b_width_log2_lookup[bsize]) >> 1;
    const int y_height_shift = (4 << b_height_log2_lookup[bsize]) >> 1;
    const int uv_width_shift = y_width_shift >> 1;
    const int uv_height_shift = y_height_shift >> 1;
    const uint8_t ysource = y[y_height_shift * stride + y_width_shift];
    const uint8_t usource = u[uv_height_shift * strideuv + uv_width_shift];
    const uint8_t vsource = v[uv_height_shift * strideuv + uv_width_shift];
    if (consec_zeromv > 25 && curr_motion_magn == 0) motion = 0;
    return vpx_skin_pixel(ysource, usource, vsource, motion);
  }
}

#ifdef OUTPUT_YUV_SKINMAP
// For viewing skin map on input source.
void vp9_compute_skin_map(VP9_COMP *const cpi, FILE *yuv_skinmap_file) {
  int i, j, mi_row, mi_col, num_bl;
  VP9_COMMON *const cm = &cpi->common;
  uint8_t *y;
  const uint8_t *src_y = cpi->Source->y_buffer;
  const int src_ystride = cpi->Source->y_stride;

  const int y_bsize = 16;  // Use 8x8 or 16x16.
  const int shy = (y_bsize == 8) ? 3 : 4;
  const int fac = y_bsize / 8;

  YV12_BUFFER_CONFIG skinmap;
  memset(&skinmap, 0, sizeof(YV12_BUFFER_CONFIG));
  if (vpx_alloc_frame_buffer(&skinmap, cm->width, cm->height, cm->subsampling_x,
                             cm->subsampling_y, VP9_ENC_BORDER_IN_PIXELS,
                             cm->byte_alignment)) {
    vpx_free_frame_buffer(&skinmap);
    return;
  }
  memset(skinmap.buffer_alloc, 128, skinmap.frame_size);
  y = skinmap.y_buffer;
  // Loop through blocks and set skin map based on center pixel of block.
  // Set y to white for skin block, otherwise set to source with gray scale.
  // Ignore rightmost/bottom boundary blocks.
  for (mi_row = 0; mi_row < cm->mi_rows - 1; mi_row += fac) {
    num_bl = 0;
    for (mi_col = 0; mi_col < cm->mi_cols - 1; mi_col += fac) {
      const int block_index = mi_row * cm->mi_cols + mi_col;
      const int is_skin = cpi->skin_map[block_index];
      for (i = 0; i < y_bsize; i++) {
        for (j = 0; j < y_bsize; j++) {
          y[i * src_ystride + j] = is_skin ? 255 : src_y[i * src_ystride + j];
        }
      }
      num_bl++;
      y += y_bsize;
      src_y += y_bsize;
    }
    y += (src_ystride << shy) - (num_bl << shy);
    src_y += (src_ystride << shy) - (num_bl << shy);
  }
  vpx_write_yuv_frame(yuv_skinmap_file, &skinmap);
  vpx_free_frame_buffer(&skinmap);
}
#endif
