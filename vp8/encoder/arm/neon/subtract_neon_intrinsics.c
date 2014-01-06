/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/*
 * ARMv8 support by rewrite NEON assembly to NEON intrinsics.
 * Enable with -mfpu=neon compile option.
 *
 * This file includes below assembly code.
 * subtract_neon.asm
 */

#include <arm_neon.h>
#include "vp8/encoder/block.h"

void vp8_subtract_b_neon(
        BLOCK *be,
        BLOCKD *bd,
        int pitch) {
    unsigned char *src_ptr, *predictor;
    int src_stride;
    short *src_diff;
    uint8x8_t d0u8, d1u8, d2u8, d3u8, d4u8, d5u8, d6u8, d7u8;
    uint16x8_t q10u16, q11u16, q12u16, q13u16;

    src_ptr = *be->base_src + be->src;
    src_stride = be->src_stride;
    predictor = bd->predictor;
    d0u8 = vld1_u8(src_ptr); src_ptr += src_stride;
    d2u8 = vld1_u8(src_ptr); src_ptr += src_stride;
    d4u8 = vld1_u8(src_ptr); src_ptr += src_stride;
    d6u8 = vld1_u8(src_ptr);
    d1u8 = vld1_u8(predictor); predictor += pitch;
    d3u8 = vld1_u8(predictor); predictor += pitch;
    d5u8 = vld1_u8(predictor); predictor += pitch;
    d7u8 = vld1_u8(predictor);

    q10u16 = vsubl_u8(d0u8, d1u8);
    q11u16 = vsubl_u8(d2u8, d3u8);
    q12u16 = vsubl_u8(d4u8, d5u8);
    q13u16 = vsubl_u8(d6u8, d7u8);

    src_diff = be->src_diff;
    vst1_u16((uint16_t *)src_diff, vget_low_u16(q10u16)); src_diff += pitch;
    vst1_u16((uint16_t *)src_diff, vget_low_u16(q11u16)); src_diff += pitch;
    vst1_u16((uint16_t *)src_diff, vget_low_u16(q12u16)); src_diff += pitch;
    vst1_u16((uint16_t *)src_diff, vget_low_u16(q13u16));
    return;
}

void vp8_subtract_mby_neon(
        short *diff,
        unsigned char *src,
        int src_stride,
        unsigned char *pred,
        int pred_stride) {
    int i;
    uint8x16_t q0u8, q1u8, q2u8, q3u8, q4u8, q5u8, q6u8, q7u8;
    uint16x8_t q8u16, q9u16, q10u16, q11u16, q12u16, q13u16, q14u16, q15u16;

    for (i = 0; i < 4; i++) {  // subtract_mby_loop
        q0u8 = vld1q_u8(src); src += src_stride;
        q2u8 = vld1q_u8(src); src += src_stride;
        q4u8 = vld1q_u8(src); src += src_stride;
        q6u8 = vld1q_u8(src); src += src_stride;
        q1u8 = vld1q_u8(pred); pred += pred_stride;
        q3u8 = vld1q_u8(pred); pred += pred_stride;
        q5u8 = vld1q_u8(pred); pred += pred_stride;
        q7u8 = vld1q_u8(pred); pred += pred_stride;

        q8u16  = vsubl_u8(vget_low_u8(q0u8), vget_low_u8(q1u8));
        q9u16  = vsubl_u8(vget_high_u8(q0u8), vget_high_u8(q1u8));
        q10u16 = vsubl_u8(vget_low_u8(q2u8), vget_low_u8(q3u8));
        q11u16 = vsubl_u8(vget_high_u8(q2u8), vget_high_u8(q3u8));
        q12u16 = vsubl_u8(vget_low_u8(q4u8), vget_low_u8(q5u8));
        q13u16 = vsubl_u8(vget_high_u8(q4u8), vget_high_u8(q5u8));
        q14u16 = vsubl_u8(vget_low_u8(q6u8), vget_low_u8(q7u8));
        q15u16 = vsubl_u8(vget_high_u8(q6u8), vget_high_u8(q7u8));

        vst1q_u16((uint16_t *)diff, q8u16);  diff += 8;
        vst1q_u16((uint16_t *)diff, q9u16);  diff += 8;
        vst1q_u16((uint16_t *)diff, q10u16); diff += 8;
        vst1q_u16((uint16_t *)diff, q11u16); diff += 8;
        vst1q_u16((uint16_t *)diff, q12u16); diff += 8;
        vst1q_u16((uint16_t *)diff, q13u16); diff += 8;
        vst1q_u16((uint16_t *)diff, q14u16); diff += 8;
        vst1q_u16((uint16_t *)diff, q15u16); diff += 8;
    }
    return;
}

void vp8_subtract_mbuv_neon(
        short *diff,
        unsigned char *usrc,
        unsigned char *vsrc,
        int src_stride,
        unsigned char *upred,
        unsigned char *vpred,
        int pred_stride) {
    int i;
    unsigned char *src_ptr, *pred_ptr;
    uint8x8_t d0u8, d1u8, d2u8, d3u8, d4u8, d5u8, d6u8, d7u8;
    uint8x8_t d8u8, d9u8, d10u8, d11u8, d12u8, d13u8, d14u8, d15u8;
    uint16x8_t q8u16, q9u16, q10u16, q11u16, q12u16, q13u16, q14u16, q15u16;

    diff += 256;
    for (i = 0; i < 2; i++) {
        if (i == 0) {
            src_ptr = usrc;
            pred_ptr = upred;
        } else if (i == 1) {
            src_ptr = vsrc;
            pred_ptr = vpred;
        }

        d0u8  = vld1_u8(src_ptr); src_ptr += src_stride;
        d1u8  = vld1_u8(pred_ptr); pred_ptr += pred_stride;
        d2u8  = vld1_u8(src_ptr); src_ptr += src_stride;
        d3u8  = vld1_u8(pred_ptr); pred_ptr += pred_stride;
        d4u8  = vld1_u8(src_ptr); src_ptr += src_stride;
        d5u8  = vld1_u8(pred_ptr); pred_ptr += pred_stride;
        d6u8  = vld1_u8(src_ptr); src_ptr += src_stride;
        d7u8  = vld1_u8(pred_ptr); pred_ptr += pred_stride;
        d8u8  = vld1_u8(src_ptr); src_ptr += src_stride;
        d9u8  = vld1_u8(pred_ptr); pred_ptr += pred_stride;
        d10u8 = vld1_u8(src_ptr); src_ptr += src_stride;
        d11u8 = vld1_u8(pred_ptr); pred_ptr += pred_stride;
        d12u8 = vld1_u8(src_ptr); src_ptr += src_stride;
        d13u8 = vld1_u8(pred_ptr); pred_ptr += pred_stride;
        d14u8 = vld1_u8(src_ptr); src_ptr += src_stride;
        d15u8 = vld1_u8(pred_ptr); pred_ptr += pred_stride;

        q8u16  = vsubl_u8(d0u8, d1u8);
        q9u16  = vsubl_u8(d2u8, d3u8);
        q10u16 = vsubl_u8(d4u8, d5u8);
        q11u16 = vsubl_u8(d6u8, d7u8);
        q12u16 = vsubl_u8(d8u8, d9u8);
        q13u16 = vsubl_u8(d10u8, d11u8);
        q14u16 = vsubl_u8(d12u8, d13u8);
        q15u16 = vsubl_u8(d14u8, d15u8);

        vst1q_u16((uint16_t *)diff, q8u16);  diff += 8;
        vst1q_u16((uint16_t *)diff, q9u16);  diff += 8;
        vst1q_u16((uint16_t *)diff, q10u16); diff += 8;
        vst1q_u16((uint16_t *)diff, q11u16); diff += 8;
        vst1q_u16((uint16_t *)diff, q12u16); diff += 8;
        vst1q_u16((uint16_t *)diff, q13u16); diff += 8;
        vst1q_u16((uint16_t *)diff, q14u16); diff += 8;
        vst1q_u16((uint16_t *)diff, q15u16); diff += 8;
    }
    return;
}
