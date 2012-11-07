/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdio.h>
#include "vpx_ports/config.h"
#include "vpx_rtcd.h"
#include "reconintra.h"
#include "vpx_mem/vpx_mem.h"

/* For skip_recon_mb(), add vp9_build_intra_predictors_mby_s(MACROBLOCKD *xd)
 * and vp9_build_intra_predictors_mbuv_s(MACROBLOCKD *xd).
 */

static void d27_predictor(uint8_t *ypred_ptr, int y_stride, int n,
                          uint8_t *yabove_row, uint8_t *yleft_col) {
  int r, c, h, w, v;
  int a, b;
  r = 0;
  for (c = 0; c < n - 2; c++) {
    if (c & 1)
      a = yleft_col[r + 1];
    else
      a = (yleft_col[r] + yleft_col[r + 1] + 1) >> 1;
    b = yabove_row[c + 2];
    ypred_ptr[c] = (2 * a + (c + 1) * b + (c + 3) / 2) / (c + 3);
  }
  for (r = 1; r < n / 2 - 1; r++) {
    for (c = 0; c < n - 2 - 2 * r; c++) {
      if (c & 1)
        a = yleft_col[r + 1];
      else
        a = (yleft_col[r] + yleft_col[r + 1] + 1) >> 1;
      b = ypred_ptr[(r - 1) * y_stride + c + 2];
      ypred_ptr[r * y_stride + c] = (2 * a + (c + 1) * b + (c + 3) / 2) / (c + 3);
    }
  }
  for (; r < n - 1; ++r) {
    for (c = 0; c < n; c++) {
      v = (c & 1 ? yleft_col[r + 1] : (yleft_col[r] + yleft_col[r + 1] + 1) >> 1);
      h = r - c / 2;
      ypred_ptr[h * y_stride + c] = v;
    }
  }
  c = 0;
  r = n - 1;
  ypred_ptr[r * y_stride] = (ypred_ptr[(r - 1) * y_stride] +
                             yleft_col[r] + 1) >> 1;
  for (r = n - 2; r >= n / 2; --r) {
    w = c + (n - 1 - r) * 2;
    ypred_ptr[r * y_stride + w] = (ypred_ptr[(r - 1) * y_stride + w] +
                                   ypred_ptr[r * y_stride + w - 1] + 1) >> 1;
  }
  for (c = 1; c < n; c++) {
    for (r = n - 1; r >= n / 2 + c / 2; --r) {
      w = c + (n - 1 - r) * 2;
      ypred_ptr[r * y_stride + w] = (ypred_ptr[(r - 1) * y_stride + w] +
                                     ypred_ptr[r * y_stride + w - 1] + 1) >> 1;
    }
  }
}

static void d63_predictor(uint8_t *ypred_ptr, int y_stride, int n,
                          uint8_t *yabove_row, uint8_t *yleft_col) {
  int r, c, h, w, v;
  int a, b;
  c = 0;
  for (r = 0; r < n - 2; r++) {
    if (r & 1)
      a = yabove_row[c + 1];
    else
      a = (yabove_row[c] + yabove_row[c + 1] + 1) >> 1;
    b = yleft_col[r + 2];
    ypred_ptr[r * y_stride] = (2 * a + (r + 1) * b + (r + 3) / 2) / (r + 3);
  }
  for (c = 1; c < n / 2 - 1; c++) {
    for (r = 0; r < n - 2 - 2 * c; r++) {
      if (r & 1)
        a = yabove_row[c + 1];
      else
        a = (yabove_row[c] + yabove_row[c + 1] + 1) >> 1;
      b = ypred_ptr[(r + 2) * y_stride + c - 1];
      ypred_ptr[r * y_stride + c] = (2 * a + (c + 1) * b + (c + 3) / 2) / (c + 3);
    }
  }
  for (; c < n - 1; ++c) {
    for (r = 0; r < n; r++) {
      v = (r & 1 ? yabove_row[c + 1] : (yabove_row[c] + yabove_row[c + 1] + 1) >> 1);
      w = c - r / 2;
      ypred_ptr[r * y_stride + w] = v;
    }
  }
  r = 0;
  c = n - 1;
  ypred_ptr[c] = (ypred_ptr[(c - 1)] + yabove_row[c] + 1) >> 1;
  for (c = n - 2; c >= n / 2; --c) {
    h = r + (n - 1 - c) * 2;
    ypred_ptr[h * y_stride + c] = (ypred_ptr[h * y_stride + c - 1] +
                                   ypred_ptr[(h - 1) * y_stride + c] + 1) >> 1;
  }
  for (r = 1; r < n; r++) {
    for (c = n - 1; c >= n / 2 + r / 2; --c) {
      h = r + (n - 1 - c) * 2;
      ypred_ptr[h * y_stride + c] = (ypred_ptr[h * y_stride + c - 1] +
                                     ypred_ptr[(h - 1) * y_stride + c] + 1) >> 1;
    }
  }
}

static void d45_predictor(uint8_t *ypred_ptr, int y_stride, int n,
                          uint8_t *yabove_row, uint8_t *yleft_col) {
  int r, c;
  for (r = 0; r < n - 1; ++r) {
    for (c = 0; c <= r; ++c) {
      ypred_ptr[(r - c) * y_stride + c] =
        (yabove_row[r + 1] * (c + 1) +
         yleft_col[r + 1] * (r - c + 1) + r / 2 + 1) / (r + 2);
    }
  }
  for (c = 0; c <= r; ++c) {
    int yabove_ext = yabove_row[r]; // 2*yabove_row[r] - yabove_row[r-1];
    int yleft_ext = yleft_col[r]; // 2*yleft_col[r] - yleft_col[r-1];
    yabove_ext = (yabove_ext > 255 ? 255 : (yabove_ext < 0 ? 0 : yabove_ext));
    yleft_ext = (yleft_ext > 255 ? 255 : (yleft_ext < 0 ? 0 : yleft_ext));
    ypred_ptr[(r - c) * y_stride + c] =
      (yabove_ext * (c + 1) +
       yleft_ext * (r - c + 1) + r / 2 + 1) / (r + 2);
  }
  for (r = 1; r < n; ++r) {
    for (c = n - r; c < n; ++c)
      ypred_ptr[r * y_stride + c] = (ypred_ptr[(r - 1) * y_stride + c] +
                                     ypred_ptr[r * y_stride + c - 1] + 1) >> 1;
  }
}

static void d117_predictor(uint8_t *ypred_ptr, int y_stride, int n,
                           uint8_t *yabove_row, uint8_t *yleft_col) {
  int r, c;
  for (c = 0; c < n; c++)
    ypred_ptr[c] = (yabove_row[c - 1] + yabove_row[c] + 1) >> 1;
  ypred_ptr += y_stride;
  for (c = 0; c < n; c++)
    ypred_ptr[c] = yabove_row[c - 1];
  ypred_ptr += y_stride;
  for (r = 2; r < n; ++r) {
    ypred_ptr[0] = yleft_col[r - 2];
    for (c = 1; c < n; c++)
      ypred_ptr[c] = ypred_ptr[-2 * y_stride + c - 1];
    ypred_ptr += y_stride;
  }
}

static void d135_predictor(uint8_t *ypred_ptr, int y_stride, int n,
                           uint8_t *yabove_row, uint8_t *yleft_col) {
  int r, c;
  ypred_ptr[0] = yabove_row[-1];
  for (c = 1; c < n; c++)
    ypred_ptr[c] = yabove_row[c - 1];
  for (r = 1; r < n; ++r)
    ypred_ptr[r * y_stride] = yleft_col[r - 1];

  ypred_ptr += y_stride;
  for (r = 1; r < n; ++r) {
    for (c = 1; c < n; c++) {
      ypred_ptr[c] = ypred_ptr[-y_stride + c - 1];
    }
    ypred_ptr += y_stride;
  }
}

static void d153_predictor(uint8_t *ypred_ptr, int y_stride, int n,
                           uint8_t *yabove_row, uint8_t *yleft_col) {
  int r, c;
  ypred_ptr[0] = (yabove_row[-1] + yleft_col[0] + 1) >> 1;
  for (r = 1; r < n; r++)
    ypred_ptr[r * y_stride] = (yleft_col[r - 1] + yleft_col[r] + 1) >> 1;
  ypred_ptr++;
  ypred_ptr[0] = yabove_row[-1];
  for (r = 1; r < n; r++)
    ypred_ptr[r * y_stride] = yleft_col[r - 1];
  ypred_ptr++;

  for (c = 0; c < n - 2; c++)
    ypred_ptr[c] = yabove_row[c];
  ypred_ptr += y_stride;
  for (r = 1; r < n; ++r) {
    for (c = 0; c < n - 2; c++)
      ypred_ptr[c] = ypred_ptr[-y_stride + c - 2];
    ypred_ptr += y_stride;
  }
}

static void corner_predictor(unsigned char *ypred_ptr, int y_stride, int n,
                             unsigned char *yabove_row,
                             unsigned char *yleft_col) {
  int h[32], v[32], mh, mv, maxgradh, maxgradv, x, y, nx, ny;
  int i, j;
  int top_left = yabove_row[-1];
  mh = mv = 0;
  maxgradh = yabove_row[1] - top_left;
  maxgradv = yleft_col[1] - top_left;
  for (i = 2; i < n; ++i) {
    int gh = yabove_row[i] - yabove_row[i - 2];
    int gv = yleft_col[i] - yleft_col[i - 2];
    if (gh > maxgradh) {
      maxgradh = gh;
      mh = i - 1;
    }
    if (gv > maxgradv) {
      maxgradv = gv;
      mv = i - 1;
    }
  }
  nx = mh + mv + 3;
  ny = 2 * n + 1 - nx;

  x = top_left;
  for (i = 0; i <= mh; ++i) x += yabove_row[i];
  for (i = 0; i <= mv; ++i) x += yleft_col[i];
  x += (nx >> 1);
  x /= nx;
  y = 0;
  for (i = mh + 1; i < n; ++i) y += yabove_row[i];
  for (i = mv + 1; i < n; ++i) y += yleft_col[i];
  y += (ny >> 1);
  y /= ny;

  for (i = 0; i < n; ++i) {
    for (j = 0; j < n; ++j)
      ypred_ptr[j] = (i <= mh && j <= mv ? x : y);
    ypred_ptr += y_stride;
  }
}

void vp9_recon_intra_mbuv(MACROBLOCKD *xd) {
  int i;
  for (i = 16; i < 24; i += 2) {
    BLOCKD *b = &xd->block[i];
    vp9_recon2b(b->predictor, b->diff, *(b->base_dst) + b->dst, b->dst_stride);
  }
}

void vp9_build_intra_predictors_internal(unsigned char *src, int src_stride,
                                         unsigned char *ypred_ptr,
                                         int y_stride, int mode, int bsize,
                                         int up_available, int left_available) {

  unsigned char *yabove_row = src - src_stride;
  unsigned char yleft_col[32];
  unsigned char ytop_left = yabove_row[-1];
  int r, c, i;

  for (i = 0; i < bsize; i++) {
    yleft_col[i] = src[i * src_stride - 1];
  }

  /* for Y */
  switch (mode) {
    case DC_PRED: {
      int expected_dc;
      int i;
      int shift;
      int average = 0;
      int log2_bsize_minus_1;

      assert(bsize == 4 || bsize == 8 || bsize == 16 || bsize == 32);
      if (bsize == 4) {
        log2_bsize_minus_1 = 1;
      } else if (bsize == 8) {
        log2_bsize_minus_1 = 2;
      } else if (bsize == 16) {
        log2_bsize_minus_1 = 3;
      } else /* bsize == 32 */ {
        log2_bsize_minus_1 = 4;
      }

      if (up_available || left_available) {
        if (up_available) {
          for (i = 0; i < bsize; i++) {
            average += yabove_row[i];
          }
        }

        if (left_available) {
          for (i = 0; i < bsize; i++) {
            average += yleft_col[i];
          }
        }
        shift = log2_bsize_minus_1 + up_available + left_available;
        expected_dc = (average + (1 << (shift - 1))) >> shift;
      } else {
        expected_dc = 128;
      }

      for (r = 0; r < bsize; r++) {
        vpx_memset(ypred_ptr, expected_dc, bsize);
        ypred_ptr += y_stride;
      }
    }
    break;
    case V_PRED: {
      for (r = 0; r < bsize; r++) {
        memcpy(ypred_ptr, yabove_row, bsize);
        ypred_ptr += y_stride;
      }
    }
    break;
    case H_PRED: {
      for (r = 0; r < bsize; r++) {
        vpx_memset(ypred_ptr, yleft_col[r], bsize);
        ypred_ptr += y_stride;
      }
    }
    break;
    case TM_PRED: {
      for (r = 0; r < bsize; r++) {
        for (c = 0; c < bsize; c++) {
          int pred =  yleft_col[r] + yabove_row[ c] - ytop_left;

          if (pred < 0)
            pred = 0;

          if (pred > 255)
            pred = 255;

          ypred_ptr[c] = pred;
        }

        ypred_ptr += y_stride;
      }
    }
    break;
    case D45_PRED: {
      d45_predictor(ypred_ptr, y_stride, bsize,  yabove_row, yleft_col);
    }
    break;
    case D135_PRED: {
      d135_predictor(ypred_ptr, y_stride, bsize,  yabove_row, yleft_col);
    }
    break;
    case D117_PRED: {
      d117_predictor(ypred_ptr, y_stride, bsize,  yabove_row, yleft_col);
    }
    break;
    case D153_PRED: {
      d153_predictor(ypred_ptr, y_stride, bsize,  yabove_row, yleft_col);
    }
    break;
    case D27_PRED: {
      d27_predictor(ypred_ptr, y_stride, bsize,  yabove_row, yleft_col);
    }
    break;
    case D63_PRED: {
      d63_predictor(ypred_ptr, y_stride, bsize,  yabove_row, yleft_col);
    }
    break;
    case I8X8_PRED:
    case B_PRED:
    case NEARESTMV:
    case NEARMV:
    case ZEROMV:
    case NEWMV:
    case SPLITMV:
    case MB_MODE_COUNT:
      break;
  }
}

#if CONFIG_COMP_INTERINTRA_PRED
static void combine_interintra(MB_PREDICTION_MODE mode,
                               unsigned char *interpred,
                               int interstride,
                               unsigned char *intrapred,
                               int intrastride,
                               int size) {
  // TODO(debargha): Explore different ways of combining predictors
  //                 or designing the tables below
  static const int scale_bits = 8;
  static int weights1d[32] = {
    128, 122, 116, 111, 107, 103,  99,  96,
    93, 90, 88, 85, 83, 81, 80, 78,
    77, 76, 75, 74, 73, 72, 71, 70,
    70, 69, 69, 68, 68, 68, 67, 67,
  };
  static int weights2d[1024] = {
    128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128,
    128, 122, 120, 118, 116, 115, 114, 113,
    112, 111, 111, 110, 109, 109, 108, 107,
    107, 106, 106, 105, 105, 104, 104, 104,
    103, 103, 102, 102, 102, 101, 101, 101,
    128, 120, 116, 114, 112, 111, 109, 108,
    107, 106, 105, 104, 103, 102, 102, 101,
    100, 100,  99,  99,  98,  97,  97,  96,
    96,  96,  95,  95,  94,  94,  93,  93,
    128, 118, 114, 111, 109, 107, 106, 104,
    103, 102, 101, 100,  99,  98,  97,  97,
    96,  95,  95,  94,  93,  93,  92,  92,
    91,  91,  90,  90,  90,  89,  89,  88,
    128, 116, 112, 109, 107, 105, 103, 102,
    100,  99,  98,  97,  96,  95,  94,  93,
    93,  92,  91,  91,  90,  90,  89,  89,
    88,  88,  87,  87,  86,  86,  85,  85,
    128, 115, 111, 107, 105, 103, 101,  99,
    98,  97,  96,  94,  93,  93,  92,  91,
    90,  89,  89,  88,  88,  87,  86,  86,
    85,  85,  84,  84,  84,  83,  83,  82,
    128, 114, 109, 106, 103, 101,  99,  97,
    96,  95,  93,  92,  91,  90,  90,  89,
    88,  87,  87,  86,  85,  85,  84,  84,
    83,  83,  82,  82,  82,  81,  81,  80,
    128, 113, 108, 104, 102,  99,  97,  96,
    94,  93,  92,  91,  90,  89,  88,  87,
    86,  85,  85,  84,  84,  83,  83,  82,
    82,  81,  81,  80,  80,  79,  79,  79,
    128, 112, 107, 103, 100,  98,  96,  94,
    93,  91,  90,  89,  88,  87,  86,  85,
    85,  84,  83,  83,  82,  82,  81,  80,
    80,  80,  79,  79,  78,  78,  78,  77,
    128, 111, 106, 102,  99,  97,  95,  93,
    91,  90,  89,  88,  87,  86,  85,  84,
    83,  83,  82,  81,  81,  80,  80,  79,
    79,  78,  78,  77,  77,  77,  76,  76,
    128, 111, 105, 101,  98,  96,  93,  92,
    90,  89,  88,  86,  85,  84,  84,  83,
    82,  81,  81,  80,  80,  79,  79,  78,
    78,  77,  77,  76,  76,  76,  75,  75,
    128, 110, 104, 100,  97,  94,  92,  91,
    89,  88,  86,  85,  84,  83,  83,  82,
    81,  80,  80,  79,  79,  78,  78,  77,
    77,  76,  76,  75,  75,  75,  74,  74,
    128, 109, 103,  99,  96,  93,  91,  90,
    88,  87,  85,  84,  83,  82,  82,  81,
    80,  79,  79,  78,  78,  77,  77,  76,
    76,  75,  75,  75,  74,  74,  74,  73,
    128, 109, 102,  98,  95,  93,  90,  89,
    87,  86,  84,  83,  82,  81,  81,  80,
    79,  78,  78,  77,  77,  76,  76,  75,
    75,  75,  74,  74,  73,  73,  73,  73,
    128, 108, 102,  97,  94,  92,  90,  88,
    86,  85,  84,  83,  82,  81,  80,  79,
    78,  78,  77,  77,  76,  76,  75,  75,
    74,  74,  73,  73,  73,  73,  72,  72,
    128, 107, 101,  97,  93,  91,  89,  87,
    85,  84,  83,  82,  81,  80,  79,  78,
    78,  77,  76,  76,  75,  75,  74,  74,
    74,  73,  73,  73,  72,  72,  72,  71,
    128, 107, 100,  96,  93,  90,  88,  86,
    85,  83,  82,  81,  80,  79,  78,  78,
    77,  76,  76,  75,  75,  74,  74,  73,
    73,  73,  72,  72,  72,  71,  71,  71,
    128, 106, 100,  95,  92,  89,  87,  85,
    84,  83,  81,  80,  79,  78,  78,  77,
    76,  76,  75,  75,  74,  74,  73,  73,
    72,  72,  72,  72,  71,  71,  71,  70,
    128, 106,  99,  95,  91,  89,  87,  85,
    83,  82,  81,  80,  79,  78,  77,  76,
    76,  75,  75,  74,  74,  73,  73,  72,
    72,  72,  71,  71,  71,  71,  70,  70,
    128, 105,  99,  94,  91,  88,  86,  84,
    83,  81,  80,  79,  78,  77,  77,  76,
    75,  75,  74,  74,  73,  73,  72,  72,
    72,  71,  71,  71,  70,  70,  70,  70,
    128, 105,  98,  93,  90,  88,  85,  84,
    82,  81,  80,  79,  78,  77,  76,  75,
    75,  74,  74,  73,  73,  72,  72,  71,
    71,  71,  71,  70,  70,  70,  70,  69,
    128, 104,  97,  93,  90,  87,  85,  83,
    82,  80,  79,  78,  77,  76,  76,  75,
    74,  74,  73,  73,  72,  72,  71,  71,
    71,  70,  70,  70,  70,  69,  69,  69,
    128, 104,  97,  92,  89,  86,  84,  83,
    81,  80,  79,  78,  77,  76,  75,  74,
    74,  73,  73,  72,  72,  71,  71,  71,
    70,  70,  70,  70,  69,  69,  69,  69,
    128, 104,  96,  92,  89,  86,  84,  82,
    80,  79,  78,  77,  76,  75,  75,  74,
    73,  73,  72,  72,  71,  71,  71,  70,
    70,  70,  70,  69,  69,  69,  69,  68,
    128, 103,  96,  91,  88,  85,  83,  82,
    80,  79,  78,  77,  76,  75,  74,  74,
    73,  72,  72,  72,  71,  71,  70,  70,
    70,  70,  69,  69,  69,  69,  68,  68,
    128, 103,  96,  91,  88,  85,  83,  81,
    80,  78,  77,  76,  75,  75,  74,  73,
    73,  72,  72,  71,  71,  70,  70,  70,
    70,  69,  69,  69,  69,  68,  68,  68,
    128, 102,  95,  90,  87,  84,  82,  81,
    79,  78,  77,  76,  75,  74,  73,  73,
    72,  72,  71,  71,  71,  70,  70,  70,
    69,  69,  69,  69,  68,  68,  68,  68,
    128, 102,  95,  90,  87,  84,  82,  80,
    79,  77,  76,  75,  75,  74,  73,  73,
    72,  72,  71,  71,  70,  70,  70,  69,
    69,  69,  69,  68,  68,  68,  68,  68,
    128, 102,  94,  90,  86,  84,  82,  80,
    78,  77,  76,  75,  74,  73,  73,  72,
    72,  71,  71,  70,  70,  70,  69,  69,
    69,  69,  68,  68,  68,  68,  68,  67,
    128, 101,  94,  89,  86,  83,  81,  79,
    78,  77,  76,  75,  74,  73,  73,  72,
    71,  71,  71,  70,  70,  69,  69,  69,
    69,  68,  68,  68,  68,  68,  67,  67,
    128, 101,  93,  89,  85,  83,  81,  79,
    78,  76,  75,  74,  74,  73,  72,  72,
    71,  71,  70,  70,  70,  69,  69,  69,
    68,  68,  68,  68,  68,  67,  67,  67,
    128, 101,  93,  88,  85,  82,  80,  79,
    77,  76,  75,  74,  73,  73,  72,  71,
    71,  70,  70,  70,  69,  69,  69,  68,
    68,  68,  68,  68,  67,  67,  67,  67,
  };
  int size_scale = (size == 32 ? 1 :
                    size == 16 ? 2 :
                    size == 8  ? 4 : 8);
  int i, j;
  switch (mode) {
    case V_PRED:
      {
        for (i = 0; i < size; ++i) {
          for (j = 0; j < size; ++j) {
            int scale = weights1d[i * size_scale];
            interpred[i * interstride + j] =
                (((1 << scale_bits) - scale) * interpred[i * interstride + j] +
                 (scale) * intrapred[i * intrastride + j] +
                 (1 << scale_bits) / 2 - 1) >> (scale_bits);
          }
        }
      }
      break;

    case H_PRED:
      {
        for (i = 0; i < size; ++i) {
          for (j = 0; j < size; ++j) {
            int scale = weights1d[j * size_scale];
            interpred[i * interstride + j] =
                (((1 << scale_bits) - scale) * interpred[i * interstride + j] +
                 (scale) * intrapred[i * intrastride + j] +
                 (1 << scale_bits) / 2 - 1) >> (scale_bits);
          }
        }
      }
      break;

    case D117_PRED:
      {
        for (i = 0; i < size; ++i) {
          for (j = 0; j < size; ++j) {
            int scale = (weights2d[i * size_scale * 32 + j * size_scale] +
                         weights1d[i * size_scale]) >> 1;
            interpred[i * interstride + j] =
                (((1 << scale_bits) - scale) * interpred[i * interstride + j] +
                 (scale) * intrapred[i * intrastride + j] +
                 (1 << scale_bits) / 2 - 1) >> (scale_bits);
          }
        }
      }
      break;

    case D153_PRED:
      {
        for (i = 0; i < size; ++i) {
          for (j = 0; j < size; ++j) {
            int scale = (weights2d[i * size_scale * 32 + j * size_scale] +
                         weights1d[j * size_scale]) >> 1;
            interpred[i * interstride + j] =
                (((1 << scale_bits) - scale) * interpred[i * interstride + j] +
                 (scale) * intrapred[i * intrastride + j] +
                 (1 << scale_bits) / 2 - 1) >> (scale_bits);
          }
        }
      }
      break;

    case D135_PRED:
      {
        for (i = 0; i < size; ++i) {
          for (j = 0; j < size; ++j) {
            int scale = weights2d[i * size_scale * 32 + j * size_scale];
            interpred[i * interstride + j] =
                (((1 << scale_bits) - scale) * interpred[i * interstride + j] +
                 (scale) * intrapred[i * intrastride + j] +
                 (1 << scale_bits) / 2 - 1) >> (scale_bits);
          }
        }
      }
      break;

    case D45_PRED:
    case D63_PRED:
    case D27_PRED:
    case DC_PRED:
    case TM_PRED:
    default:
      // simple average
      {
        for (i = 0; i < size; ++i) {
          for (j = 0; j < size; ++j) {
            interpred[i * interstride + j] =
                (interpred[i * interstride + j] +
                 intrapred[i * intrastride + j]) >> 1;
          }
        }
      }
      break;
  }
}

void vp9_build_interintra_16x16_predictors_mb(MACROBLOCKD *xd,
                                              unsigned char *ypred,
                                              unsigned char *upred,
                                              unsigned char *vpred,
                                              int ystride, int uvstride) {
  vp9_build_interintra_16x16_predictors_mby(xd, ypred, ystride);
  vp9_build_interintra_16x16_predictors_mbuv(xd, upred, vpred, uvstride);
}

void vp9_build_interintra_16x16_predictors_mby(MACROBLOCKD *xd,
                                               unsigned char *ypred,
                                               int ystride) {
  static const int scale_bits = 6;
  unsigned char intrapredictor[256];
  int i, j;
  vp9_build_intra_predictors_internal(
      xd->dst.y_buffer, xd->dst.y_stride,
      intrapredictor, 16,
      xd->mode_info_context->mbmi.interintra_mode, 16,
      xd->up_available, xd->left_available);
  combine_interintra(xd->mode_info_context->mbmi.interintra_mode,
                     ypred, ystride, intrapredictor, 16, 16);
}

void vp9_build_interintra_16x16_predictors_mbuv(MACROBLOCKD *xd,
                                                unsigned char *upred,
                                                unsigned char *vpred,
                                                int uvstride) {
  int i, j;
  unsigned char uintrapredictor[64];
  unsigned char vintrapredictor[64];
  vp9_build_intra_predictors_internal(
      xd->dst.u_buffer, xd->dst.uv_stride,
      uintrapredictor, 8,
      xd->mode_info_context->mbmi.interintra_uv_mode, 8,
      xd->up_available, xd->left_available);
  vp9_build_intra_predictors_internal(
      xd->dst.v_buffer, xd->dst.uv_stride,
      vintrapredictor, 8,
      xd->mode_info_context->mbmi.interintra_uv_mode, 8,
      xd->up_available, xd->left_available);
  combine_interintra(xd->mode_info_context->mbmi.interintra_uv_mode,
                     upred, uvstride, uintrapredictor, 8, 8);
  combine_interintra(xd->mode_info_context->mbmi.interintra_uv_mode,
                     vpred, uvstride, vintrapredictor, 8, 8);
}
#endif

void vp9_build_intra_predictors_mby(MACROBLOCKD *xd) {
  vp9_build_intra_predictors_internal(xd->dst.y_buffer, xd->dst.y_stride,
                                      xd->predictor, 16,
                                      xd->mode_info_context->mbmi.mode, 16,
                                      xd->up_available, xd->left_available);
}

void vp9_build_intra_predictors_mby_s(MACROBLOCKD *xd) {
  vp9_build_intra_predictors_internal(xd->dst.y_buffer, xd->dst.y_stride,
                                      xd->dst.y_buffer, xd->dst.y_stride,
                                      xd->mode_info_context->mbmi.mode, 16,
                                      xd->up_available, xd->left_available);
}

#if CONFIG_SUPERBLOCKS
void vp9_build_intra_predictors_sby_s(MACROBLOCKD *xd) {
  vp9_build_intra_predictors_internal(xd->dst.y_buffer, xd->dst.y_stride,
                                      xd->dst.y_buffer, xd->dst.y_stride,
                                      xd->mode_info_context->mbmi.mode, 32,
                                      xd->up_available, xd->left_available);
}
#endif

#if CONFIG_COMP_INTRA_PRED
void vp9_build_comp_intra_predictors_mby(MACROBLOCKD *xd) {
  unsigned char predictor[2][256];
  int i;

  vp9_build_intra_predictors_internal(xd->dst.y_buffer, xd->dst.y_stride,
                                      predictor[0], 16,
                                      xd->mode_info_context->mbmi.mode,
                                      16, xd->up_available,
                                      xd->left_available);
  vp9_build_intra_predictors_internal(xd->dst.y_buffer, xd->dst.y_stride,
                                      predictor[1], 16,
                                      xd->mode_info_context->mbmi.second_mode,
                                      16, xd->up_available,
                                      xd->left_available);

  for (i = 0; i < 256; i++) {
    xd->predictor[i] = (predictor[0][i] + predictor[1][i] + 1) >> 1;
  }
}
#endif

void vp9_build_intra_predictors_mbuv_internal(MACROBLOCKD *xd,
                                              unsigned char *upred_ptr,
                                              unsigned char *vpred_ptr,
                                              int uv_stride,
                                              int mode, int bsize) {
  vp9_build_intra_predictors_internal(xd->dst.u_buffer, xd->dst.uv_stride,
                                      upred_ptr, uv_stride, mode, bsize,
                                      xd->up_available, xd->left_available);
  vp9_build_intra_predictors_internal(xd->dst.v_buffer, xd->dst.uv_stride,
                                      vpred_ptr, uv_stride, mode, bsize,
                                      xd->up_available, xd->left_available);
}

void vp9_build_intra_predictors_mbuv(MACROBLOCKD *xd) {
  vp9_build_intra_predictors_mbuv_internal(xd, &xd->predictor[256],
                                           &xd->predictor[320], 8,
                                           xd->mode_info_context->mbmi.uv_mode,
                                           8);
}

void vp9_build_intra_predictors_mbuv_s(MACROBLOCKD *xd) {
  vp9_build_intra_predictors_mbuv_internal(xd, xd->dst.u_buffer,
                                           xd->dst.v_buffer,
                                           xd->dst.uv_stride,
                                           xd->mode_info_context->mbmi.uv_mode,
                                           8);
}

#if CONFIG_SUPERBLOCKS
void vp9_build_intra_predictors_sbuv_s(MACROBLOCKD *xd) {
  vp9_build_intra_predictors_mbuv_internal(xd, xd->dst.u_buffer,
                                           xd->dst.v_buffer, xd->dst.uv_stride,
                                           xd->mode_info_context->mbmi.uv_mode,
                                           16);
}
#endif

#if CONFIG_COMP_INTRA_PRED
void vp9_build_comp_intra_predictors_mbuv(MACROBLOCKD *xd) {
  unsigned char predictor[2][2][64];
  int i;

  vp9_build_intra_predictors_mbuv_internal(
      xd, predictor[0][0], predictor[1][0], 8,
      xd->mode_info_context->mbmi.uv_mode, 8);
  vp9_build_intra_predictors_mbuv_internal(
      xd, predictor[0][1], predictor[1][1], 8,
      xd->mode_info_context->mbmi.second_uv_mode, 8);
  for (i = 0; i < 64; i++) {
    xd->predictor[256 + i] = (predictor[0][0][i] + predictor[0][1][i] + 1) >> 1;
    xd->predictor[256 + 64 + i] = (predictor[1][0][i] +
                                   predictor[1][1][i] + 1) >> 1;
  }
}
#endif

void vp9_intra8x8_predict(BLOCKD *xd,
                          int mode,
                          unsigned char *predictor) {
  vp9_build_intra_predictors_internal(*(xd->base_dst) + xd->dst,
                                      xd->dst_stride, predictor, 16,
                                      mode, 8, 1, 1);
}

#if CONFIG_COMP_INTRA_PRED
void vp9_comp_intra8x8_predict(BLOCKD *xd,
                               int mode, int second_mode,
                               unsigned char *out_predictor) {
  unsigned char predictor[2][8 * 16];
  int i, j;

  vp9_intra8x8_predict(xd, mode, predictor[0]);
  vp9_intra8x8_predict(xd, second_mode, predictor[1]);

  for (i = 0; i < 8 * 16; i += 16) {
    for (j = i; j < i + 8; j++) {
      out_predictor[j] = (predictor[0][j] + predictor[1][j] + 1) >> 1;
    }
  }
}
#endif

void vp9_intra_uv4x4_predict(BLOCKD *xd,
                             int mode,
                             unsigned char *predictor) {
  vp9_build_intra_predictors_internal(*(xd->base_dst) + xd->dst,
                                      xd->dst_stride, predictor, 8,
                                      mode, 4, 1, 1);
}

#if CONFIG_COMP_INTRA_PRED
void vp9_comp_intra_uv4x4_predict(BLOCKD *xd,
                                  int mode, int mode2,
                                  unsigned char *out_predictor) {
  unsigned char predictor[2][8 * 4];
  int i, j;

  vp9_intra_uv4x4_predict(xd, mode, predictor[0]);
  vp9_intra_uv4x4_predict(xd, mode2, predictor[1]);

  for (i = 0; i < 4 * 8; i += 8) {
    for (j = i; j < i + 4; j++) {
      out_predictor[j] = (predictor[0][j] + predictor[1][j] + 1) >> 1;
    }
  }
}
#endif

/* TODO: try different ways of use Y-UV mode correlation
   Current code assumes that a uv 4x4 block use same mode
   as corresponding Y 8x8 area
   */
