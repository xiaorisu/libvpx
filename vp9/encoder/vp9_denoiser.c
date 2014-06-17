/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <limits.h>
#include "vpx_scale/yv12config.h"
#include "vpx/vpx_integer.h"
#include "vp9/common/vp9_reconinter.h"
#include "vp9/encoder/vp9_denoiser.h"

/* The VP9 denoiser is a work-in-progress. It currently is only designed to work
 * with speed 6, though it (inexplicably) seems to also work with speed 5 (one
 * would need to modify the source code in vp9_pickmode.c and vp9_encoder.c to
 * make the calls to the vp9_denoiser_* functions when in speed 5).
 *
 * The implementation is very similar to that of the VP8 denoiser. While
 * choosing the motion vectors / reference frames, the denoiser is run, and if
 * it did not modify the signal to much, the denoised block is copied to the
 * signal.
 */

#ifdef OUTPUT_YUV_DENOISED
static void make_grayscale(YV12_BUFFER_CONFIG *yuv);
#endif

static const int widths[]  = {4, 4, 8, 8,  8, 16, 16, 16, 32, 32, 32, 64, 64};
static const int heights[] = {4, 8, 4, 8, 16,  8, 16, 32, 16, 32, 64, 32, 64};

static int absdiff_thresh(BLOCK_SIZE bs, int increase_denoising) {
  (void)bs;
  return 3 + (increase_denoising ? 1 : 0);
}

static int delta_thresh(BLOCK_SIZE bs, int increase_denoising) {
  (void)bs;
  (void)increase_denoising;
  return 4;
}

static int noise_motion_thresh(BLOCK_SIZE bs, int increase_denoising) {
  (void)bs;
  (void)increase_denoising;
  return 25 * 25;
}

static unsigned int sse_thresh(BLOCK_SIZE bs, int increase_denoising) {
  return widths[bs] * heights[bs] * (increase_denoising ? 60 : 40);
}

static int sse_diff_thresh(BLOCK_SIZE bs, int increase_denoising,
                           int mv_row, int mv_col) {
  if (mv_row * mv_row + mv_col * mv_col >
      noise_motion_thresh(bs, increase_denoising)) {
    return 0;
  } else {
    return widths[bs] * heights[bs] * 20;
  }
}

static int total_adj_strong_thresh(BLOCK_SIZE bs, int increase_denoising) {
  return widths[bs] * heights[bs] * (increase_denoising ? 3 : 2);
}

static int total_adj_weak_thresh(BLOCK_SIZE bs, int increase_denoising) {
  return widths[bs] * heights[bs] * (increase_denoising ? 3 : 2);
}

static VP9_DENOISER_DECISION denoiser_filter(const uint8_t *sig, int sig_stride,
                                             const uint8_t *mc_avg,
                                             int mc_avg_stride,
                                             uint8_t *avg, int avg_stride,
                                             int increase_denoising,
                                             BLOCK_SIZE bs) {
  int r, c;
  const uint8_t *sig_start = sig;
  const uint8_t *mc_avg_start = mc_avg;
  uint8_t *avg_start = avg;
  int diff, adj, absdiff, delta;
  int adj_val[] = {3, 4, 6};
  int total_adj = 0;

  // First attempt to apply a strong temporal denoising filter.
  for (r = 0; r < heights[bs]; ++r) {
    for (c = 0; c < widths[bs]; ++c) {
      diff = mc_avg[c] - sig[c];
      absdiff = abs(diff);

      if (absdiff <= absdiff_thresh(bs, increase_denoising)) {
        avg[c] = mc_avg[c];
        total_adj += diff;
      } else {
        switch (absdiff) {
          case 4: case 5: case 6: case 7:
            adj = adj_val[0];
            break;
          case 8: case 9: case 10: case 11:
          case 12: case 13: case 14: case 15:
            adj = adj_val[1];
            break;
          default:
            adj = adj_val[2];
        }
        if (diff > 0) {
          avg[c] = MIN(UINT8_MAX, sig[c] + adj);
          total_adj += adj;
        } else {
          avg[c] = MAX(0, sig[c] - adj);
          total_adj -= adj;
        }
      }
    }
    sig += sig_stride;
    avg += avg_stride;
    mc_avg += mc_avg_stride;
  }

  // If the strong filter did not modify the signal too much, we're all set.
  if (abs(total_adj) <= total_adj_strong_thresh(bs, increase_denoising)) {
    return FILTER_BLOCK;
  }

  // Otherwise, we try to dampen the filter if the delta is not too high.
  delta = ((abs(total_adj) - total_adj_strong_thresh(bs, increase_denoising))
           >> 8) + 1;
  if (delta > delta_thresh(bs, increase_denoising)) {
    return COPY_BLOCK;
  }

  mc_avg =  mc_avg_start;
  avg = avg_start;
  sig = sig_start;
  for (r = 0; r < heights[bs]; ++r) {
    for (c = 0; c < widths[bs]; ++c) {
      diff = mc_avg[c] - sig[c];
      adj = abs(diff);
      if (adj > delta) {
        adj = delta;
      }
      if (diff > 0) {
        avg[c] = MAX(0, avg[c] - adj);
        total_adj += adj;
      } else {
        avg[c] = MIN(UINT8_MAX, avg[c] + adj);
        total_adj -= adj;
      }
    }
    sig += sig_stride;
    avg += avg_stride;
    mc_avg += mc_avg_stride;
  }

  // We can use the filter if it has been sufficiently dampened
  if (abs(total_adj) <= total_adj_weak_thresh(bs, increase_denoising)) {
    return FILTER_BLOCK;
  }
  return COPY_BLOCK;
}

static uint8_t *block_start(uint8_t *framebuf, int stride,
                            int mi_row, int mi_col) {
  return framebuf + (stride * mi_row * 8) + (mi_col * 8);
}

void copy_block(uint8_t *dest, int dest_stride,
                uint8_t *src, int src_stride, BLOCK_SIZE bs) {
  int r;
  for (r = 0; r < heights[bs]; ++r) {
    vpx_memcpy(dest, src, widths[bs]);
    dest += dest_stride;
    src += src_stride;
  }
}

static VP9_DENOISER_DECISION perform_motion_compensation(VP9_DENOISER *denoiser,
                                                         MACROBLOCK *mb,
                                                         BLOCK_SIZE bs,
                                                         int increase_denoising,
                                                         int mi_row,
                                                         int mi_col) {
  int mv_col, mv_row;
  int sse_diff = denoiser->zero_mv_sse - denoiser->best_sse;
  MV_REFERENCE_FRAME frame;
  MACROBLOCKD *filter_mbd = &mb->e_mbd;
  MB_MODE_INFO *mbmi = &filter_mbd->mi[0]->mbmi;

  // We will restore these after motion compensation.
  MB_MODE_INFO saved_mbmi = *mbmi;
  struct buf_2d saved_dst = filter_mbd->plane[0].dst;
  struct buf_2d saved_pre[2];
  saved_pre[0] = filter_mbd->plane[0].pre[0];
  saved_pre[1] = filter_mbd->plane[0].pre[1];

  mv_col = denoiser->best_sse_mv.as_mv.col;
  mv_row = denoiser->best_sse_mv.as_mv.row;

  frame = denoiser->best_reference_frame;

  // If the best reference frame uses inter-prediction and there is enough of a
  // difference in sum-squared-error, use it.
  if (frame != INTRA_FRAME &&
      sse_diff > sse_diff_thresh(bs, increase_denoising, mv_row, mv_col)) {
    mbmi->ref_frame[0] = denoiser->best_reference_frame;
    mbmi->mode = denoiser->best_sse_inter_mode;
    mbmi->mv[0] = denoiser->best_sse_mv;
  } else {
    // Otherwise, use the zero reference frame.
    frame = denoiser->best_zeromv_reference_frame;

    mbmi->ref_frame[0] = denoiser->best_zeromv_reference_frame;
    mbmi->mode = ZEROMV;
    mbmi->mv[0].as_int = 0;

    denoiser->best_sse_inter_mode = ZEROMV;
    denoiser->best_sse_mv.as_int = 0;
    denoiser->best_sse = denoiser->zero_mv_sse;
  }

  // Set the pointers in the MACROBLOCKD to point to the buffers in the denoiser
  // struct.
  filter_mbd->plane[0].pre[0].buf =
      block_start(denoiser->running_avg_y[frame].y_buffer,
                  denoiser->running_avg_y[frame].y_stride,
                  mi_row, mi_col);
  filter_mbd->plane[0].pre[0].stride = denoiser->running_avg_y[frame].y_stride;

  filter_mbd->plane[1].pre[0].buf =
      block_start(denoiser->running_avg_y[frame].u_buffer,
                  denoiser->running_avg_y[frame].uv_stride,
                  mi_row, mi_col);
  filter_mbd->plane[1].pre[0].stride = denoiser->running_avg_y[frame].uv_stride;

  filter_mbd->plane[2].pre[0].buf =
      block_start(denoiser->running_avg_y[frame].v_buffer,
                  denoiser->running_avg_y[frame].uv_stride,
                  mi_row, mi_col);
  filter_mbd->plane[2].pre[0].stride = denoiser->running_avg_y[frame].uv_stride;

  filter_mbd->plane[0].pre[1].buf =
      block_start(denoiser->running_avg_y[frame].y_buffer,
                  denoiser->running_avg_y[frame].y_stride,
                  mi_row, mi_col);
  filter_mbd->plane[0].pre[1].stride = denoiser->running_avg_y[frame].y_stride;

  filter_mbd->plane[1].pre[1].buf =
      block_start(denoiser->running_avg_y[frame].u_buffer,
                  denoiser->running_avg_y[frame].uv_stride,
                  mi_row, mi_col);
  filter_mbd->plane[1].pre[1].stride = denoiser->running_avg_y[frame].uv_stride;

  filter_mbd->plane[2].pre[1].buf =
      block_start(denoiser->running_avg_y[frame].v_buffer,
                  denoiser->running_avg_y[frame].uv_stride,
                  mi_row, mi_col);
  filter_mbd->plane[2].pre[1].stride = denoiser->running_avg_y[frame].uv_stride;

  filter_mbd->plane[0].dst.buf =
      block_start(denoiser->mc_running_avg_y.y_buffer,
                  denoiser->mc_running_avg_y.y_stride,
                  mi_row, mi_col);
  filter_mbd->plane[0].dst.stride = denoiser->mc_running_avg_y.y_stride;

  filter_mbd->plane[1].dst.buf =
      block_start(denoiser->mc_running_avg_y.u_buffer,
                  denoiser->mc_running_avg_y.uv_stride,
                  mi_row, mi_col);
  filter_mbd->plane[1].dst.stride = denoiser->mc_running_avg_y.y_stride;

  filter_mbd->plane[2].dst.buf =
      block_start(denoiser->mc_running_avg_y.v_buffer,
                  denoiser->mc_running_avg_y.uv_stride,
                  mi_row, mi_col);
  filter_mbd->plane[2].dst.stride = denoiser->mc_running_avg_y.y_stride;

  vp9_build_inter_predictors_sby(filter_mbd, mv_row, mv_col, bs);

  // Restore everything to its original state
  filter_mbd->plane[0].pre[0] = saved_pre[0];
  filter_mbd->plane[0].pre[1] = saved_pre[1];
  filter_mbd->plane[0].dst = saved_dst;
  *mbmi = saved_mbmi;

  mv_row = denoiser->best_sse_mv.as_mv.row;
  mv_col = denoiser->best_sse_mv.as_mv.col;

  if (denoiser->best_sse > sse_thresh(bs, increase_denoising)) {
    return COPY_BLOCK;
  }
  if (mv_row * mv_row + mv_col * mv_col >
      8 * noise_motion_thresh(bs, increase_denoising)) {
    return COPY_BLOCK;
  }
  return FILTER_BLOCK;
}

void vp9_denoiser_denoise(VP9_DENOISER *denoiser, MACROBLOCK *mb,
                          int mi_row, int mi_col, BLOCK_SIZE bs) {
  VP9_DENOISER_DECISION decision = FILTER_BLOCK;
  YV12_BUFFER_CONFIG avg = denoiser->running_avg_y[INTRA_FRAME];
  YV12_BUFFER_CONFIG mc_avg = denoiser->mc_running_avg_y;
  uint8_t *avg_start = block_start(avg.y_buffer, avg.y_stride, mi_row, mi_col);
  uint8_t *mc_avg_start = block_start(mc_avg.y_buffer, mc_avg.y_stride,
                                          mi_row, mi_col);
  struct buf_2d src = mb->plane[0].src;

  decision = perform_motion_compensation(denoiser, mb, bs,
                                         denoiser->increase_denoising,
                                         mi_row, mi_col);

  if (decision == FILTER_BLOCK) {
    decision = denoiser_filter(src.buf, src.stride,
                               mc_avg_start, mc_avg.y_stride,
                               avg_start, avg.y_stride,
                               0, bs);
  }

  if (decision == FILTER_BLOCK) {
    copy_block(src.buf, src.stride, avg_start, avg.y_stride, bs);
  } else {  // COPY_BLOCK
    copy_block(avg_start, avg.y_stride, src.buf, src.stride, bs);
  }
}

static void copy_frame(YV12_BUFFER_CONFIG dest, const YV12_BUFFER_CONFIG src) {
  int r;
  const uint8_t *srcbuf = src.y_buffer;
  uint8_t *destbuf = dest.y_buffer;
  assert(dest.y_width == src.y_width);
  assert(dest.y_height == src.y_height);

  for (r = 0; r < dest.y_height; ++r) {
    vpx_memcpy(destbuf, srcbuf, dest.y_width);
    destbuf += dest.y_stride;
    srcbuf += src.y_stride;
  }
}

void vp9_denoiser_update_frame_info(VP9_DENOISER *denoiser,
                                    YV12_BUFFER_CONFIG src,
                                    FRAME_TYPE frame_type,
                                    int refresh_alt_ref_frame,
                                    int refresh_golden_frame,
                                    int refresh_last_frame) {
  if (frame_type == KEY_FRAME) {
    int i;
    copy_frame(denoiser->running_avg_y[LAST_FRAME], src);
    for (i = 2; i < MAX_REF_FRAMES - 1; i++) {
      copy_frame(denoiser->running_avg_y[i],
                 denoiser->running_avg_y[LAST_FRAME]);
    }
  } else {  /* For non key frames */
    if (refresh_alt_ref_frame) {
      copy_frame(denoiser->running_avg_y[ALTREF_FRAME],
                 denoiser->running_avg_y[INTRA_FRAME]);
    }
    if (refresh_golden_frame) {
      copy_frame(denoiser->running_avg_y[GOLDEN_FRAME],
                 denoiser->running_avg_y[INTRA_FRAME]);
    }
    if (refresh_last_frame) {
      copy_frame(denoiser->running_avg_y[LAST_FRAME],
                 denoiser->running_avg_y[INTRA_FRAME]);
    }
  }
}

void vp9_denoiser_reset_frame_stats(VP9_DENOISER *denoiser) {
  denoiser->zero_mv_sse = UINT_MAX;
  denoiser->best_sse = UINT_MAX;
}

void vp9_denoiser_update_frame_stats(VP9_DENOISER *denoiser, MB_MODE_INFO *mbmi,
                                     unsigned int sse, PREDICTION_MODE mode) {
  // TODO(tkopp): Use both MVs if possible
  if (mbmi->mv[0].as_int == 0 && sse < denoiser->zero_mv_sse) {
    denoiser->zero_mv_sse = sse;
    denoiser->best_zeromv_reference_frame = mbmi->ref_frame[0];
  }

  if (mbmi->mv[0].as_int != 0 && sse < denoiser->best_sse) {
    denoiser->best_sse = sse;
    denoiser->best_sse_inter_mode = mode;
    denoiser->best_sse_mv = mbmi->mv[0];
    denoiser->best_reference_frame = mbmi->ref_frame[0];
  }
}

int vp9_denoiser_alloc(VP9_DENOISER *denoiser, int width, int height,
                       int ssx, int ssy, int border) {
  int i, fail;
  assert(denoiser != NULL);

  for (i = 0; i < MAX_REF_FRAMES; ++i) {
    fail = vp9_alloc_frame_buffer(&denoiser->running_avg_y[i], width, height,
                                  ssx, ssy, border);
    if (fail) {
      vp9_denoiser_free(denoiser);
      return 1;
    }
#ifdef OUTPUT_YUV_DENOISED
    make_grayscale(&denoiser->running_avg_y[i]);
#endif
  }

  fail = vp9_alloc_frame_buffer(&denoiser->mc_running_avg_y, width, height,
                                ssx, ssy, border);
  if (fail) {
    vp9_denoiser_free(denoiser);
    return 1;
  }
#ifdef OUTPUT_YUV_DENOISED
  make_grayscale(&denoiser->running_avg_y[i]);
#endif
  denoiser->increase_denoising = 0;

  return 0;
}

void vp9_denoiser_free(VP9_DENOISER *denoiser) {
  int i;
  if (denoiser == NULL) {
    return;
  }
  for (i = 0; i < MAX_REF_FRAMES; ++i) {
    if (&denoiser->running_avg_y[i] != NULL) {
      vp9_free_frame_buffer(&denoiser->running_avg_y[i]);
    }
  }
  if (&denoiser->mc_running_avg_y != NULL) {
    vp9_free_frame_buffer(&denoiser->mc_running_avg_y);
  }
}

#ifdef OUTPUT_YUV_DENOISED
static void make_grayscale(YV12_BUFFER_CONFIG *yuv) {
  int r, c;
  uint8_t *u = yuv->u_buffer;
  uint8_t *v = yuv->v_buffer;

  // The '/2's are there because we have a 440 buffer, but we want to output
  // 420.
  for (r = 0; r < yuv->uv_height / 2; ++r) {
    for (c = 0; c < yuv->uv_width / 2; ++c) {
      u[c] = UINT8_MAX / 2;
      v[c] = UINT8_MAX / 2;
    }
    u += yuv->uv_stride + yuv->uv_width / 2;
    v += yuv->uv_stride + yuv->uv_width / 2;
  }
}
#endif
