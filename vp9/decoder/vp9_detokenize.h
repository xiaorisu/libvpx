/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef VP9_DECODER_VP9_DETOKENIZE_H_
#define VP9_DECODER_VP9_DETOKENIZE_H_

#include "vp9/decoder/vp9_onyxd_int.h"
#include "vp9/decoder/vp9_dboolhuff.h"

struct decode_block_args {
  VP9_COMMON *cm;
  MACROBLOCKD *xd;
  struct segmentation *seg;
  vp9_reader *r;
  int *eobtotal;
};

void v9_decode_block_tokens(int plane, int block, BLOCK_SIZE plane_bsize,
                                   TX_SIZE tx_size, void *argv);

int vp9_decode_tokens(VP9_COMMON *cm, MACROBLOCKD *xd,
                      struct segmentation *seg,
                      vp9_reader *r, BLOCK_SIZE bsize);

#endif  // VP9_DECODER_VP9_DETOKENIZE_H_
