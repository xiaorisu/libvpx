/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef VP10_ENCODER_SUBEXP_H_
#define VP10_ENCODER_SUBEXP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vpx_dsp/prob.h"

struct vpx_writer;

void vp10_write_prob_diff_update(struct vpx_writer *w,
                                vpx_prob newp, vpx_prob oldp);

void vp10_cond_prob_diff_update(struct vpx_writer *w, vpx_prob *oldp,
                               const unsigned int ct[2]);

int vp10_prob_diff_update_savings_search(const unsigned int *ct,
                                        vpx_prob oldp, vpx_prob *bestp,
                                        vpx_prob upd);


int vp10_prob_diff_update_savings_search_model(const unsigned int *ct,
                                              const vpx_prob *oldp,
                                              vpx_prob *bestp,
                                              vpx_prob upd,
                                              int stepsize);

int vp10_cond_prob_diff_update_savings(vpx_prob *oldp,
                                       const unsigned int ct[2]);
#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // VP10_ENCODER_SUBEXP_H_
