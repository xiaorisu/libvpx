/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <stdlib.h>
#include "vp9/common/vp9_filter.h"
#include "vpx_ports/mem.h"
#include "vp9_rtcd.h"
#include "vp9/common/vp9_common.h"

/* TODO(jkoleszar): We can avoid duplicating these tables 2X by forcing 256
 * byte alignment of the table's base address.
 */
DECLARE_ALIGNED(256, const int16_t, vp9_bilinear_filters[SUBPEL_SHIFTS][8]) = {
  { 0, 0, 0, 128,   0, 0, 0, 0 },
  { 0, 0, 0, 120,   8, 0, 0, 0 },
  { 0, 0, 0, 112,  16, 0, 0, 0 },
  { 0, 0, 0, 104,  24, 0, 0, 0 },
  { 0, 0, 0,  96,  32, 0, 0, 0 },
  { 0, 0, 0,  88,  40, 0, 0, 0 },
  { 0, 0, 0,  80,  48, 0, 0, 0 },
  { 0, 0, 0,  72,  56, 0, 0, 0 },
  { 0, 0, 0,  64,  64, 0, 0, 0 },
  { 0, 0, 0,  56,  72, 0, 0, 0 },
  { 0, 0, 0,  48,  80, 0, 0, 0 },
  { 0, 0, 0,  40,  88, 0, 0, 0 },
  { 0, 0, 0,  32,  96, 0, 0, 0 },
  { 0, 0, 0,  24, 104, 0, 0, 0 },
  { 0, 0, 0,  16, 112, 0, 0, 0 },
  { 0, 0, 0,   8, 120, 0, 0, 0 },
};

#define FILTER_ALPHA       0
#define FILTER_ALPHA_SHARP 1
DECLARE_ALIGNED(256, const int16_t, vp9_sub_pel_filters_8[SUBPEL_SHIFTS][8])
    = {
#if FILTER_ALPHA == 0
  /* Lagrangian interpolation filter */
  { 0,   0,   0, 128,   0,   0,   0,  0},
  { 0,   1,  -5, 126,   8,  -3,   1,  0},
  { -1,   3, -10, 122,  18,  -6,   2,  0},
  { -1,   4, -13, 118,  27,  -9,   3, -1},
  { -1,   4, -16, 112,  37, -11,   4, -1},
  { -1,   5, -18, 105,  48, -14,   4, -1},
  { -1,   5, -19,  97,  58, -16,   5, -1},
  { -1,   6, -19,  88,  68, -18,   5, -1},
  { -1,   6, -19,  78,  78, -19,   6, -1},
  { -1,   5, -18,  68,  88, -19,   6, -1},
  { -1,   5, -16,  58,  97, -19,   5, -1},
  { -1,   4, -14,  48, 105, -18,   5, -1},
  { -1,   4, -11,  37, 112, -16,   4, -1},
  { -1,   3,  -9,  27, 118, -13,   4, -1},
  { 0,   2,  -6,  18, 122, -10,   3, -1},
  { 0,   1,  -3,   8, 126,  -5,   1,  0},

#elif FILTER_ALPHA == 50
  /* Generated using MATLAB:
   * alpha = 0.5;
   * b=intfilt(8,4,alpha);
   * bi=round(128*b);
   * ba=flipud(reshape([bi 0], 8, 8));
   * disp(num2str(ba, '%d,'))
   */
  { 0,   0,   0, 128,   0,   0,   0,  0},
  { 0,   1,  -5, 126,   8,  -3,   1,  0},
  { 0,   2, -10, 122,  18,  -6,   2,  0},
  { -1,   3, -13, 118,  27,  -9,   3,  0},
  { -1,   4, -16, 112,  37, -11,   3,  0},
  { -1,   5, -17, 104,  48, -14,   4, -1},
  { -1,   5, -18,  96,  58, -16,   5, -1},
  { -1,   5, -19,  88,  68, -17,   5, -1},
  { -1,   5, -18,  78,  78, -18,   5, -1},
  { -1,   5, -17,  68,  88, -19,   5, -1},
  { -1,   5, -16,  58,  96, -18,   5, -1},
  { -1,   4, -14,  48, 104, -17,   5, -1},
  { 0,   3, -11,  37, 112, -16,   4, -1},
  { 0,   3,  -9,  27, 118, -13,   3, -1},
  { 0,   2,  -6,  18, 122, -10,   2,  0},
  { 0,   1,  -3,   8, 126,  -5,   1,  0}
#endif  /* FILTER_ALPHA */
};

DECLARE_ALIGNED(256, const int16_t, vp9_sub_pel_filters_8s[SUBPEL_SHIFTS][8])
    = {
#if FILTER_ALPHA_SHARP == 1
  /* dct based filter */
  {0,   0,   0, 128,   0,   0,   0, 0},
  {-1,   3,  -7, 127,   8,  -3,   1, 0},
  {-2,   5, -13, 125,  17,  -6,   3, -1},
  {-3,   7, -17, 121,  27, -10,   5, -2},
  {-4,   9, -20, 115,  37, -13,   6, -2},
  {-4,  10, -23, 108,  48, -16,   8, -3},
  {-4,  10, -24, 100,  59, -19,   9, -3},
  {-4,  11, -24,  90,  70, -21,  10, -4},
  {-4,  11, -23,  80,  80, -23,  11, -4},
  {-4,  10, -21,  70,  90, -24,  11, -4},
  {-3,   9, -19,  59, 100, -24,  10, -4},
  {-3,   8, -16,  48, 108, -23,  10, -4},
  {-2,   6, -13,  37, 115, -20,   9, -4},
  {-2,   5, -10,  27, 121, -17,   7, -3},
  {-1,   3,  -6,  17, 125, -13,   5, -2},
  {0,   1,  -3,   8, 127,  -7,   3, -1},

#elif FILTER_ALPHA_SHARP == 75
  /* alpha = 0.75 */
  {0,   0,   0, 128,   0,   0,   0, 0},
  {-1,   2,  -6, 126,   9,  -3,   2, -1},
  {-1,   4, -11, 123,  18,  -7,   3, -1},
  {-2,   6, -16, 119,  28, -10,   5, -2},
  {-2,   7, -19, 113,  38, -13,   6, -2},
  {-3,   8, -21, 106,  49, -16,   7, -2},
  {-3,   9, -22,  99,  59, -19,   8, -3},
  {-3,   9, -23,  90,  70, -21,   9, -3},
  {-3,   9, -22,  80,  80, -22,   9, -3},
  {-3,   9, -21,  70,  90, -23,   9, -3},
  {-3,   8, -19,  59,  99, -22,   9, -3},
  {-2,   7, -16,  49, 106, -21,   8, -3},
  {-2,   6, -13,  38, 113, -19,   7, -2},
  {-2,   5, -10,  28, 119, -16,   6, -2},
  {-1,   3,  -7,  18, 123, -11,   4, -1},
  {-1,   2,  -3,   9, 126,  -6,   2, -1}
#endif  /* FILTER_ALPHA_SHARP */
};

DECLARE_ALIGNED(256, const int16_t,
                vp9_sub_pel_filters_8lp[SUBPEL_SHIFTS][8]) = {
  /* 8-tap lowpass filter */
  /* Hamming window */
  {-1, -7, 32, 80, 32, -7, -1,  0},
  {-1, -8, 28, 80, 37, -7, -2,  1},
  { 0, -8, 24, 79, 41, -7, -2,  1},
  { 0, -8, 20, 78, 45, -5, -3,  1},
  { 0, -8, 16, 76, 50, -4, -3,  1},
  { 0, -7, 13, 74, 54, -3, -4,  1},
  { 1, -7,  9, 71, 58, -1, -4,  1},
  { 1, -6,  6, 68, 62,  1, -5,  1},
  { 1, -6,  4, 65, 65,  4, -6,  1},
  { 1, -5,  1, 62, 68,  6, -6,  1},
  { 1, -4, -1, 58, 71,  9, -7,  1},
  { 1, -4, -3, 54, 74, 13, -7,  0},
  { 1, -3, -4, 50, 76, 16, -8,  0},
  { 1, -3, -5, 45, 78, 20, -8,  0},
  { 1, -2, -7, 41, 79, 24, -8,  0},
  { 1, -2, -7, 37, 80, 28, -8, -1},
};

DECLARE_ALIGNED(256, const int16_t, vp9_sub_pel_filters_6[SUBPEL_SHIFTS][8])
    = {
  {0, 0,   0, 128,   0,   0, 0,  0},
  {0, 1,  -5, 125,   8,  -2, 1,  0},
  {0, 1,  -8, 122,  17,  -5, 1,  0},
  {0, 2, -11, 116,  27,  -8, 2,  0},
  {0, 3, -14, 110,  37, -10, 2,  0},
  {0, 3, -15, 103,  47, -12, 2,  0},
  {0, 3, -16,  95,  57, -14, 3,  0},
  {0, 3, -16,  86,  67, -15, 3,  0},
  {0, 3, -16,  77,  77, -16, 3,  0},
  {0, 3, -15,  67,  86, -16, 3,  0},
  {0, 3, -14,  57,  95, -16, 3,  0},
  {0, 2, -12,  47, 103, -15, 3,  0},
  {0, 2, -10,  37, 110, -14, 3,  0},
  {0, 2,  -8,  27, 116, -11, 2,  0},
  {0, 1,  -5,  17, 122,  -8, 1,  0},
  {0, 1,  -2,   8, 125,  -5, 1,  0},

};
