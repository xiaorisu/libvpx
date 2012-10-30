/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "vpx_ports/config.h"
#include "vp8/common/blockd.h"
#include "vp8/common/reconintra.h"
#include "vpx_mem/vpx_mem.h"
#include "vp8/common/recon.h"

#if HAVE_ARMV7
extern void vp9_build_intra_predictors_mby_neon_func(
  unsigned char *y_buffer,
  unsigned char *ypred_ptr,
  int y_stride,
  int mode,
  int Up,
  int Left);

void vp9_build_intra_predictors_mby_neon(MACROBLOCKD *xd) {
  unsigned char *y_buffer = xd->dst.y_buffer;
  unsigned char *ypred_ptr = xd->predictor;
  int y_stride = xd->dst.y_stride;
  int mode = xd->mode_info_context->mbmi.mode;
  int Up = xd->up_available;
  int Left = xd->left_available;

  vp9_build_intra_predictors_mby_neon_func(y_buffer, ypred_ptr,
                                           y_stride, mode, Up, Left);
}
#endif


#if HAVE_ARMV7
extern void vp9_build_intra_predictors_mby_s_neon_func(
  unsigned char *y_buffer,
  unsigned char *ypred_ptr,
  int y_stride,
  int mode,
  int Up,
  int Left);

void vp9_build_intra_predictors_mby_s_neon(MACROBLOCKD *xd) {
  unsigned char *y_buffer = xd->dst.y_buffer;
  unsigned char *ypred_ptr = xd->predictor;
  int y_stride = xd->dst.y_stride;
  int mode = xd->mode_info_context->mbmi.mode;
  int Up = xd->up_available;
  int Left = xd->left_available;

  vp9_build_intra_predictors_mby_s_neon_func(y_buffer, ypred_ptr,
                                             y_stride, mode, Up, Left);
}

#endif
