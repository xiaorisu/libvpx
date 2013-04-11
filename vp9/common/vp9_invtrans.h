/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_INVTRANS_H_
#define VP9_COMMON_VP9_INVTRANS_H_

#include "./vpx_config.h"
#include "vpx/vpx_integer.h"
#include "vp9/common/vp9_blockd.h"

void vp9_inverse_transform_b_4x4(MACROBLOCKD *xd, int eob,
                                 int16_t *dqcoeff, int16_t *diff,
                                 int pitch);

void vp9_inverse_transform_b_8x8(int16_t *input_dqcoeff,
                                 int16_t *output_coeff, int pitch);

void vp9_inverse_transform_b_16x16(int16_t *input_dqcoeff,
                                   int16_t *output_coeff, int pitch);

void vp9_inverse_transform_sby_32x32(MACROBLOCKD *xd, BLOCK_SIZE_TYPE bsize);
void vp9_inverse_transform_sby_16x16(MACROBLOCKD *xd, BLOCK_SIZE_TYPE bsize);
void vp9_inverse_transform_sby_8x8(MACROBLOCKD *xd, BLOCK_SIZE_TYPE bsize);
void vp9_inverse_transform_sby_4x4(MACROBLOCKD *xd, BLOCK_SIZE_TYPE bsize);
void vp9_inverse_transform_sbuv_32x32(MACROBLOCKD *xd, BLOCK_SIZE_TYPE bsize);
void vp9_inverse_transform_sbuv_16x16(MACROBLOCKD *xd, BLOCK_SIZE_TYPE bsize);
void vp9_inverse_transform_sbuv_8x8(MACROBLOCKD *xd, BLOCK_SIZE_TYPE bsize);
void vp9_inverse_transform_sbuv_4x4(MACROBLOCKD *xd, BLOCK_SIZE_TYPE bsize);

#endif  // VP9_COMMON_VP9_INVTRANS_H_
