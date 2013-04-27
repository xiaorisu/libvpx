/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_seg_common.h"

static const int seg_feature_data_signed[SEG_LVL_MAX] = { 1, 1, 0, 0 };
static const int seg_feature_data_max[SEG_LVL_MAX] = { MAXQ, 63, 15, 15 };

// These functions provide access to new segment level features.
// Eventually these function may be "optimized out" but for the moment,
// the coding mechanism is still subject to change so these provide a
// convenient single point of change.

int vp9_segfeature_active(const MACROBLOCKD *xd, int segment_id,
                          SEG_LVL_FEATURES feature_id) {
  return xd->segmentation_enabled &&
         (xd->segment_feature_mask[segment_id] & (1 << feature_id));
}

void vp9_clearall_segfeatures(MACROBLOCKD *xd) {
  vpx_memset(xd->segment_feature_data, 0, sizeof(xd->segment_feature_data));
  vpx_memset(xd->segment_feature_mask, 0, sizeof(xd->segment_feature_mask));
}

void vp9_enable_segfeature(MACROBLOCKD *xd, int segment_id,
                           SEG_LVL_FEATURES feature_id) {
  xd->segment_feature_mask[segment_id] |= 1 << feature_id;
}

void vp9_disable_segfeature(MACROBLOCKD *xd, int segment_id,
                            SEG_LVL_FEATURES feature_id) {
  xd->segment_feature_mask[segment_id] &= ~(1 << feature_id);
}

int vp9_seg_feature_data_max(SEG_LVL_FEATURES feature_id) {
  return seg_feature_data_max[feature_id];
}

int vp9_is_segfeature_signed(SEG_LVL_FEATURES feature_id) {
  return seg_feature_data_signed[feature_id];
}

void vp9_clear_segdata(MACROBLOCKD *xd, int segment_id,
                       SEG_LVL_FEATURES feature_id) {
  xd->segment_feature_data[segment_id][feature_id] = 0;
}

void vp9_set_segdata(MACROBLOCKD *xd, int segment_id,
                     SEG_LVL_FEATURES feature_id, int seg_data) {
  assert(seg_data <= seg_feature_data_max[feature_id]);
  if (seg_data < 0) {
    assert(seg_feature_data_signed[feature_id]);
    assert(-seg_data <= seg_feature_data_max[feature_id]);
  }

  xd->segment_feature_data[segment_id][feature_id] = seg_data;
}

int vp9_get_segdata(const MACROBLOCKD *xd, int segment_id,
                    SEG_LVL_FEATURES feature_id) {
  return xd->segment_feature_data[segment_id][feature_id];
}

void vp9_clear_segref(MACROBLOCKD *xd, int segment_id) {
  xd->segment_feature_data[segment_id][SEG_LVL_REF_FRAME] = 0;
}

void vp9_set_segref(MACROBLOCKD *xd, int segment_id,
                    MV_REFERENCE_FRAME ref_frame) {
  xd->segment_feature_data[segment_id][SEG_LVL_REF_FRAME] |= 1 << ref_frame;
}

int vp9_check_segref(const MACROBLOCKD *xd, int segment_id,
                     MV_REFERENCE_FRAME ref_frame) {
  return (xd->segment_feature_data[segment_id][SEG_LVL_REF_FRAME] &
         (1 << ref_frame)) ? 1 : 0;
}


#if CONFIG_IMPLICIT_SEGMENTATION
// This function defines an implicit segmentation for the next frame based
// on predcition and transform decisions in the current frame.
// For test purposes at the moment it uses ref frame and prediction size
void vp9_implicit_segment_map_update(VP9_COMMON * cm) {
  int row, col;
  MODE_INFO *mi, *mi_ptr = cm->mi;
  unsigned char * map_ptr = cm->last_frame_seg_map;

  for (row = 0; row < cm->mb_rows; row++) {
    mi = mi_ptr;

    for (col = 0; col < cm->mb_cols; ++col, ++mi) {
      // Inter prediction
      if (mi->mbmi.ref_frame != INTRA_FRAME) {
        // Zero motion and prediction block size >= 16
        if ((mi->mbmi.sb_type >= BLOCK_SIZE_MB16X16) &&
            (mi->mbmi.mv[0].as_int == 0))
          map_ptr[col] = 1;
        else if (mi->mbmi.sb_type >= BLOCK_SIZE_SB32X32)
          map_ptr[col] = 2;
        else if (mi->mbmi.sb_type >= BLOCK_SIZE_MB16X16)
          map_ptr[col] = 3;
        else
          map_ptr[col] = 6;

      // Intra prediction
      } else {
        if (mi->mbmi.sb_type >= BLOCK_SIZE_SB32X32)
          map_ptr[col] = 4;
        else if (mi->mbmi.sb_type >= BLOCK_SIZE_MB16X16)
          map_ptr[col] = 5;
        else
          map_ptr[col] = 7;
      }
    }
    mi_ptr += cm->mode_info_stride;
    map_ptr += cm->mb_cols;
  }
}

// This function defines an implicit segmentation for the next frame based
// on predcition and transform decisions in the current frame.
// For test purposes at the moment only TX size is used.
void vp9_implicit_segment_map_update_tx(VP9_COMMON * cm) {
  int row, col;
  MODE_INFO *mi, *mi_ptr = cm->mi;
  unsigned char * map_ptr = cm->last_frame_seg_map;

  for (row = 0; row < cm->mb_rows; row++) {
    mi = mi_ptr;
    for (col = 0; col < cm->mb_cols; ++col, ++mi) {
      // Intra modes
      if (mi->mbmi.ref_frame == INTRA_FRAME) {
        if (mi->mbmi.txfm_size == TX_4X4)
          map_ptr[col] = 7;
        else if (mi->mbmi.txfm_size <= TX_16X16)
          map_ptr[col] = 5;
        else
          map_ptr[col] = 4;
      } else {
        // Inter Modes
        if (mi->mbmi.txfm_size == TX_4X4)
          map_ptr[col] = 6;
        else if (mi->mbmi.txfm_size == TX_8X8)
          map_ptr[col] = 3;
        else if (mi->mbmi.txfm_size == TX_16X16)
          map_ptr[col] = 2;
        else
          map_ptr[col] = 1;
      }
    }
    mi_ptr += cm->mode_info_stride;
    map_ptr += cm->mb_cols;
  }
}
#endif


const vp9_tree_index vp9_segment_tree[14] = {
  2,  4,  6,  8, 10, 12,
  0, -1, -2, -3, -4, -5, -6, -7
};


// TBD? Functions to read and write segment data with range / validity checking
