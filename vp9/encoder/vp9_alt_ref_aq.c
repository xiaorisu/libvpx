/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed  by a BSD-style license that can be
 *  found in the LICENSE file in the root of the source tree. An additional
 *  intellectual property  rights grant can  be found in the  file PATENTS.
 *  All contributing  project authors may be  found in the AUTHORS  file in
 *  the root of the source tree.
 */

#include <assert.h>
#include <string.h>

#include "vpx_ports/system_state.h"
#include "vpx_dsp/vpx_dsp_common.h"
#include "vpx/vpx_integer.h"
#include "vpx_mem/vpx_mem.h"

#include "vp9/common/vp9_onyxc_int.h"
#include "vp9/common/vp9_seg_common.h"
#include "vp9/common/vp9_blockd.h"

#include "vp9/encoder/vp9_encoder.h"
#include "vp9/encoder/vp9_segmentation.h"

#include "vp9/common/vp9_matx_enums.h"
#include "vp9/common/vp9_matx.h"
#include "vp9/common/vp9_matx_functions.h"

#include "vp9/encoder/vp9_alt_ref_aq_private.h"
#include "vp9/encoder/vp9_alt_ref_aq.h"


#define array_zerofill(array, nelems) \
  memset((array), 0, (nelems)*sizeof((array)[0]));

static void vp9_alt_ref_aq_update_number_of_segments(struct ALT_REF_AQ *self) {
  int i, j;

  for (i = 0, j = 0; i < self->nsegments; ++i)
    j = VPXMAX(j, self->__segment_changes[i]);

  self->nsegments = j + 1;
}

static void vp9_alt_ref_aq_setup_segment_deltas(struct ALT_REF_AQ *self) {
  int i;

  array_zerofill(&self->segment_deltas[1], self->nsegments);

  for (i = 0; i < self->nsegments; ++i)
    ++self->segment_deltas[self->__segment_changes[i] + 1];

  for (i = 2; i < self->nsegments; ++i)
    self->segment_deltas[i] += self->segment_deltas[i - 1];
}

static void vp9_alt_ref_aq_compress_segment_hist(struct ALT_REF_AQ *self) {
  int i, j;

  // invert segment ids and drop out segments
  // with zero so we can save few segments
  // for other staff and save segmentation overhead
  for (i = self->nsegments - 1, j = 0; i >= 0; --i)
    if (self->segment_hist[i] > 0)
       self->__segment_changes[i] = j++;

  // in the case we drop out zero segment, update
  // segment changes from zero to the first non-dropped segment id
  for (i = 0; self->segment_hist[i] <= 0; ++i)
    self->__segment_changes[i] = self->__segment_changes[i] - 1;

  // I only keep it for the case of the single segment
  self->segment_deltas[0] = self->nsegments - i;

  // join several largest segments
  i = VPXMAX(self->MIN_NONZERO_SEGMENTS, i + 1);
  i = VPXMAX(self->nsegments - self->NUM_ZERO_SEGMENTS + 1, i);

  for (; i < self->nsegments; ++i) {
      int last = self->nsegments - 1;
      self->__segment_changes[i] = self->__segment_changes[last];
  }

  // compress segment id's range
  for (i = self->nsegments - 2; i >= 0; --i) {
    int previous_id = self->__segment_changes[i + 1];
    int diff = self->__segment_changes[i] - previous_id;

    self->__segment_changes[i] = previous_id + !!diff;
  }
}

static void vp9_alt_ref_aq_update_segmentation_map(struct ALT_REF_AQ *self) {
  int i, j;

  array_zerofill(self->segment_hist, self->nsegments);

  for (i = 0; i < self->segmentation_map.rows; ++i) {
    uint8_t* data = self->segmentation_map.data;
    uint8_t* row_data = data + i*self->segmentation_map.stride;

    for (j = 0; j < self->segmentation_map.cols; ++j) {
      row_data[j] = self->__segment_changes[row_data[j]];
      ++self->segment_hist[row_data[j]];
    }
  }
}

struct ALT_REF_AQ* vp9_alt_ref_aq_create() {
  struct ALT_REF_AQ* self = vpx_malloc(sizeof(struct ALT_REF_AQ));

  self->aq_mode = LOOKAHEAD_AQ;
  self->nsegments = -1;

  memset(self->segment_hist, 0, sizeof(self->segment_hist));

  // these parameters may have noticeable
  // influence on the quality because
  // of the overhead for storing segmentation map
  self->NUM_ZERO_SEGMENTS = INT_MAX/2;
  self->MIN_NONZERO_SEGMENTS = 1;

  // This is just initiallization, allocation
  // is going to happen on the first request
  vp9_matx_init(&self->segmentation_map);

  return self; /* ------------------------------ */
}

// set up histogram (I use it for filtering zero-area segments out)
static void vp9_alt_ref_aq_setup_histogram(struct ALT_REF_AQ* const self) {
  struct MATX_8U segm_map;
  int i, j;

  segm_map = self->segmentation_map;
  array_zerofill(self->segment_hist, self->nsegments);

  for (i = 0; i < segm_map.rows; ++i) {
    const uint8_t* const data = segm_map.data;
    int offset = i*segm_map.stride;

    for (j = 0; j < segm_map.cols; ++j)
      ++self->segment_hist[data[offset + j]];
  }
}

static void vp9_alt_ref_aq_process_map(struct ALT_REF_AQ* const self) {
  assert(self->nsegments != -1);

  vp9_alt_ref_aq_setup_histogram(self);

  vp9_alt_ref_aq_compress_segment_hist(self);
  vp9_alt_ref_aq_setup_segment_deltas(self);
  vp9_alt_ref_aq_update_number_of_segments(self);

  if (self->nsegments == 1)
    return;

  vp9_alt_ref_aq_update_segmentation_map(self);
}

void vp9_alt_ref_aq_destroy(struct ALT_REF_AQ* const self) {
  vp9_matx_destroy(&self->segmentation_map);
  vpx_free(self); /* -------------------- */
}

struct MATX_8U* vp9_alt_ref_aq_segm_map(struct ALT_REF_AQ* const self) {
  return &self->segmentation_map;
}

void vp9_alt_ref_aq_set_nsegments(struct ALT_REF_AQ* const self,
                                  int nsegments) {
  self->nsegments = nsegments;
}

void vp9_alt_ref_aq_setup_mode(struct ALT_REF_AQ* const self,
                               struct VP9_COMP* const cpi) {
  (void) cpi;
  vp9_alt_ref_aq_process_map(self);
}

// set basic segmentation to the altref's one
void vp9_alt_ref_aq_setup_map(struct ALT_REF_AQ* const self,
                              struct VP9_COMP* const cpi) {
  (void) cpi;
  (void) self;
}

// restore cpi->aq_mode
void vp9_alt_ref_aq_unset_all(struct ALT_REF_AQ* const self,
                              struct VP9_COMP* const cpi) {
  (void) cpi;
  (void) self;
}
