/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef __INC_BLOCKD_H
#define __INC_BLOCKD_H

void vpx_log(const char *format, ...);

#include "vpx_ports/config.h"
#include "vpx_scale/yv12config.h"
#include "mv.h"
#include "treecoder.h"
#include "subpixel.h"
#include "vpx_ports/mem.h"
#include "common.h"

#define TRUE    1
#define FALSE   0

// #define MODE_STATS

/*#define DCPRED 1*/
#define DCPREDSIMTHRESH 0
#define DCPREDCNTTHRESH 3

#define MB_FEATURE_TREE_PROBS   3
#define PREDICTION_PROBS 3

#define MBSKIP_CONTEXTS 3

#define MAX_MB_SEGMENTS         4

#define MAX_REF_LF_DELTAS       4
#define MAX_MODE_LF_DELTAS      4

/* Segment Feature Masks */
#define SEGMENT_DELTADATA   0
#define SEGMENT_ABSDATA     1
#if CONFIG_NEWBESTREFMV || CONFIG_NEW_MVREF
#define MAX_MV_REFS 19
#endif

typedef struct {
  int r, c;
} POS;

typedef enum PlaneType {
  PLANE_TYPE_Y_NO_DC = 0,
  PLANE_TYPE_Y2,
  PLANE_TYPE_UV,
  PLANE_TYPE_Y_WITH_DC,
} PLANE_TYPE;

typedef char ENTROPY_CONTEXT;
typedef struct {
  ENTROPY_CONTEXT y1[4];
  ENTROPY_CONTEXT u[2];
  ENTROPY_CONTEXT v[2];
  ENTROPY_CONTEXT y2;
} ENTROPY_CONTEXT_PLANES;

extern const unsigned char vp8_block2left[25];
extern const unsigned char vp8_block2above[25];
extern const unsigned char vp8_block2left_8x8[25];
extern const unsigned char vp8_block2above_8x8[25];

#define VP8_COMBINEENTROPYCONTEXTS( Dest, A, B) \
  Dest = ((A)!=0) + ((B)!=0);

typedef enum {
  KEY_FRAME = 0,
  INTER_FRAME = 1
} FRAME_TYPE;

typedef enum
{
  SIXTAP   = 0,
  BILINEAR = 1,
  EIGHTTAP = 2,
  EIGHTTAP_SHARP = 3,
#if CONFIG_SWITCHABLE_INTERP
  SWITCHABLE  /* should be the last one */
#endif
} INTERPOLATIONFILTERTYPE;

typedef enum
{
  DC_PRED,            /* average of above and left pixels */
  V_PRED,             /* vertical prediction */
  H_PRED,             /* horizontal prediction */
  D45_PRED,           /* Directional 45 deg prediction  [anti-clockwise from 0 deg hor] */
  D135_PRED,          /* Directional 135 deg prediction [anti-clockwise from 0 deg hor] */
  D117_PRED,          /* Directional 112 deg prediction [anti-clockwise from 0 deg hor] */
  D153_PRED,          /* Directional 157 deg prediction [anti-clockwise from 0 deg hor] */
  D27_PRED,           /* Directional 22 deg prediction  [anti-clockwise from 0 deg hor] */
  D63_PRED,           /* Directional 67 deg prediction  [anti-clockwise from 0 deg hor] */
  TM_PRED,            /* Truemotion prediction */
  I8X8_PRED,          /* 8x8 based prediction, each 8x8 has its own prediction mode */
  B_PRED,             /* block based prediction, each block has its own prediction mode */

  NEARESTMV,
  NEARMV,
  ZEROMV,
  NEWMV,
  SPLITMV,

  MB_MODE_COUNT
} MB_PREDICTION_MODE;

// Segment level features.
typedef enum {
  SEG_LVL_ALT_Q = 0,               // Use alternate Quantizer ....
  SEG_LVL_ALT_LF = 1,              // Use alternate loop filter value...
  SEG_LVL_REF_FRAME = 2,           // Optional Segment reference frame
  SEG_LVL_MODE = 3,                // Optional Segment mode
  SEG_LVL_EOB = 4,                 // EOB end stop marker.
  SEG_LVL_TRANSFORM = 5,           // Block transform size.
  SEG_LVL_MAX = 6                  // Number of MB level features supported

} SEG_LVL_FEATURES;

// Segment level features.
typedef enum {
  TX_4X4,                      // 4x4 dct transform
  TX_8X8,                      // 8x8 dct transform
  TX_16X16,                    // 16x16 dct transform
  TX_SIZE_MAX                  // Number of different transforms available
} TX_SIZE;

#if CONFIG_HYBRIDTRANSFORM8X8 || CONFIG_HYBRIDTRANSFORM || CONFIG_HYBRIDTRANSFORM16X16
typedef enum {
  DCT_DCT   = 0,                      // DCT  in both horizontal and vertical
  ADST_DCT  = 1,                      // ADST in horizontal, DCT in vertical
  DCT_ADST  = 2,                      // DCT  in horizontal, ADST in vertical
  ADST_ADST = 3                       // ADST in both directions
} TX_TYPE;
#endif

#define VP8_YMODES  (B_PRED + 1)
#define VP8_UV_MODES (TM_PRED + 1)
#define VP8_I8X8_MODES (TM_PRED + 1)
#define VP8_I32X32_MODES (TM_PRED + 1)

#define VP8_MVREFS (1 + SPLITMV - NEARESTMV)

typedef enum {
  B_DC_PRED,          /* average of above and left pixels */
  B_TM_PRED,

  B_VE_PRED,           /* vertical prediction */
  B_HE_PRED,           /* horizontal prediction */

  B_LD_PRED,
  B_RD_PRED,

  B_VR_PRED,
  B_VL_PRED,
  B_HD_PRED,
  B_HU_PRED,

  LEFT4X4,
  ABOVE4X4,
  ZERO4X4,
  NEW4X4,

  B_MODE_COUNT
} B_PREDICTION_MODE;

#define VP8_BINTRAMODES (B_HU_PRED + 1)  /* 10 */
#define VP8_SUBMVREFS (1 + NEW4X4 - LEFT4X4)

/* For keyframes, intra block modes are predicted by the (already decoded)
   modes for the Y blocks to the left and above us; for interframes, there
   is a single probability table. */

union b_mode_info {
  struct {
    B_PREDICTION_MODE first;
#if CONFIG_HYBRIDTRANSFORM8X8 || CONFIG_HYBRIDTRANSFORM || CONFIG_HYBRIDTRANSFORM16X16
    TX_TYPE           tx_type;
#endif

#if CONFIG_COMP_INTRA_PRED
    B_PREDICTION_MODE second;
#endif
  } as_mode;
  struct {
    int_mv first;
    int_mv second;
  } as_mv;
};

typedef enum {
  INTRA_FRAME = 0,
  LAST_FRAME = 1,
  GOLDEN_FRAME = 2,
  ALTREF_FRAME = 3,
  MAX_REF_FRAMES = 4
} MV_REFERENCE_FRAME;

typedef struct {
  MB_PREDICTION_MODE mode, uv_mode;
#if CONFIG_COMP_INTRA_PRED
  MB_PREDICTION_MODE second_mode, second_uv_mode;
#endif
  MV_REFERENCE_FRAME ref_frame, second_ref_frame;
  TX_SIZE txfm_size;
  int_mv mv[2]; // for each reference frame used
#if CONFIG_NEWBESTREFMV
  int_mv ref_mv, second_ref_mv;
  int_mv ref_mvs[MAX_REF_FRAMES][MAX_MV_REFS];
  int mv_ref_index[MAX_REF_FRAMES];
#endif

  unsigned char partitioning;
  unsigned char mb_skip_coeff;                                /* does this mb has coefficients at all, 1=no coefficients, 0=need decode tokens */
  unsigned char need_to_clamp_mvs;
  unsigned char need_to_clamp_secondmv;
  unsigned char segment_id;                  /* Which set of segmentation parameters should be used for this MB */

  // Flags used for prediction status of various bistream signals
  unsigned char seg_id_predicted;
  unsigned char ref_predicted;

  // Indicates if the mb is part of the image (1) vs border (0)
  // This can be useful in determining whether the MB provides
  // a valid predictor
  unsigned char mb_in_image;

#if CONFIG_PRED_FILTER
  // Flag to turn prediction signal filter on(1)/off(0 ) at the MB level
  unsigned int pred_filter_enabled;
#endif
#if CONFIG_SWITCHABLE_INTERP
    INTERPOLATIONFILTERTYPE interp_filter;
#endif

#if CONFIG_SUPERBLOCKS
  // FIXME need a SB array of 4 MB_MODE_INFOs that
  // only needs one encoded_as_sb.
  unsigned char encoded_as_sb;
#endif
} MB_MODE_INFO;

typedef struct {
  MB_MODE_INFO mbmi;
  union b_mode_info bmi[16];
} MODE_INFO;

typedef struct {
  short *qcoeff;
  short *dqcoeff;
  unsigned char  *predictor;
  short *diff;
  short *dequant;

  /* 16 Y blocks, 4 U blocks, 4 V blocks each with 16 entries */
  unsigned char **base_pre;
  unsigned char **base_second_pre;
  int pre;
  int pre_stride;

  unsigned char **base_dst;
  int dst;
  int dst_stride;

  int eob;

  union b_mode_info bmi;
} BLOCKD;

typedef struct MacroBlockD {
  DECLARE_ALIGNED(16, short, diff[400]);      /* from idct diff */
  DECLARE_ALIGNED(16, unsigned char,  predictor[384]);
  DECLARE_ALIGNED(16, short, qcoeff[400]);
  DECLARE_ALIGNED(16, short, dqcoeff[400]);
  DECLARE_ALIGNED(16, char,  eobs[25]);

  /* 16 Y blocks, 4 U, 4 V, 1 DC 2nd order block, each with 16 entries. */
  BLOCKD block[25];
  int fullpixel_mask;

  YV12_BUFFER_CONFIG pre; /* Filtered copy of previous frame reconstruction */
  struct {
    uint8_t *y_buffer, *u_buffer, *v_buffer;
  } second_pre;
  YV12_BUFFER_CONFIG dst;

  MODE_INFO *prev_mode_info_context;
  MODE_INFO *mode_info_context;
  int mode_info_stride;

  FRAME_TYPE frame_type;

  int up_available;
  int left_available;

  /* Y,U,V,Y2 */
  ENTROPY_CONTEXT_PLANES *above_context;
  ENTROPY_CONTEXT_PLANES *left_context;

  /* 0 indicates segmentation at MB level is not enabled. Otherwise the individual bits indicate which features are active. */
  unsigned char segmentation_enabled;

  /* 0 (do not update) 1 (update) the macroblock segmentation map. */
  unsigned char update_mb_segmentation_map;

  /* 0 (do not update) 1 (update) the macroblock segmentation feature data. */
  unsigned char update_mb_segmentation_data;

  /* 0 (do not update) 1 (update) the macroblock segmentation feature data. */
  unsigned char mb_segment_abs_delta;

  /* Per frame flags that define which MB level features (such as quantizer or loop filter level) */
  /* are enabled and when enabled the proabilities used to decode the per MB flags in MB_MODE_INFO */

  // Probability Tree used to code Segment number
  vp8_prob mb_segment_tree_probs[MB_FEATURE_TREE_PROBS];


  // Segment features
  signed char segment_feature_data[MAX_MB_SEGMENTS][SEG_LVL_MAX];
  unsigned int segment_feature_mask[MAX_MB_SEGMENTS];

#if CONFIG_FEATUREUPDATES
  // keep around the last set so we can figure out what updates...
  unsigned int old_segment_feature_mask[MAX_MB_SEGMENTS];
  signed char old_segment_feature_data[MAX_MB_SEGMENTS][SEG_LVL_MAX];
#endif

  /* mode_based Loop filter adjustment */
  unsigned char mode_ref_lf_delta_enabled;
  unsigned char mode_ref_lf_delta_update;

  /* Delta values have the range +/- MAX_LOOP_FILTER */
  signed char last_ref_lf_deltas[MAX_REF_LF_DELTAS];                /* 0 = Intra, Last, GF, ARF */
  signed char ref_lf_deltas[MAX_REF_LF_DELTAS];                     /* 0 = Intra, Last, GF, ARF */
  signed char last_mode_lf_deltas[MAX_MODE_LF_DELTAS];              /* 0 = BPRED, ZERO_MV, MV, SPLIT */
  signed char mode_lf_deltas[MAX_MODE_LF_DELTAS];                   /* 0 = BPRED, ZERO_MV, MV, SPLIT */

  /* Distance of MB away from frame edges */
  int mb_to_left_edge;
  int mb_to_right_edge;
  int mb_to_top_edge;
  int mb_to_bottom_edge;

  unsigned int frames_since_golden;
  unsigned int frames_till_alt_ref_frame;
  vp8_subpix_fn_t  subpixel_predict;
  vp8_subpix_fn_t  subpixel_predict8x4;
  vp8_subpix_fn_t  subpixel_predict8x8;
  vp8_subpix_fn_t  subpixel_predict16x16;
  vp8_subpix_fn_t  subpixel_predict_avg;
  vp8_subpix_fn_t  subpixel_predict_avg8x4;
  vp8_subpix_fn_t  subpixel_predict_avg8x8;
  vp8_subpix_fn_t  subpixel_predict_avg16x16;
  int allow_high_precision_mv;

  void *current_bc;

  int corrupted;

#if !CONFIG_SUPERBLOCKS && (ARCH_X86 || ARCH_X86_64)
  /* This is an intermediate buffer currently used in sub-pixel motion search
   * to keep a copy of the reference area. This buffer can be used for other
   * purpose.
   */
  DECLARE_ALIGNED(32, unsigned char, y_buf[22 * 32]);
#endif

#if CONFIG_RUNTIME_CPU_DETECT
  struct VP8_COMMON_RTCD  *rtcd;
#endif

  int mb_index;   // Index of the MB in the SB (0..3)

#if CONFIG_NEWBESTREFMV
  int_mv ref_mv[MAX_MV_REFS];
#endif

#if CONFIG_HYBRIDTRANSFORM || CONFIG_HYBRIDTRANSFORM8X8 || CONFIG_HYBRIDTRANSFORM16X16
  int q_index;
#endif

} MACROBLOCKD;

#if CONFIG_HYBRIDTRANSFORM || CONFIG_HYBRIDTRANSFORM16X16
#define ACTIVE_HT 110                // quantization stepsize threshold
#endif

#if CONFIG_HYBRIDTRANSFORM16X16
#define ACTIVE_HT16 300
#endif

#if CONFIG_HYBRIDTRANSFORM8X8 || CONFIG_HYBRIDTRANSFORM16X16
// convert MB_PREDICTION_MODE to B_PREDICTION_MODE
static B_PREDICTION_MODE pred_mode_conv(MB_PREDICTION_MODE mode) {
  B_PREDICTION_MODE b_mode;
  switch (mode) {
    case DC_PRED:
      b_mode = B_DC_PRED;
      break;
    case V_PRED:
      b_mode = B_VE_PRED;
      break;
    case H_PRED:
      b_mode = B_HE_PRED;
      break;
    case TM_PRED:
      b_mode = B_TM_PRED;
      break;
    case D45_PRED:
      b_mode = B_LD_PRED;
      break;
    case D135_PRED:
      b_mode = B_RD_PRED;
      break;
    case D117_PRED:
      b_mode = B_VR_PRED;
      break;
    case D153_PRED:
      b_mode = B_HD_PRED;
      break;
    case D27_PRED:
      b_mode = B_HU_PRED;
      break;
    case D63_PRED:
      b_mode = B_VL_PRED;
      break;
    default :
      // for debug purpose, to be removed after full testing
      assert(0);
      break;
  }
  return b_mode;
}
#endif

#if CONFIG_HYBRIDTRANSFORM || CONFIG_HYBRIDTRANSFORM8X8 || CONFIG_HYBRIDTRANSFORM16X16
// transform mapping
static TX_TYPE txfm_map(B_PREDICTION_MODE bmode) {
  // map transform type
  TX_TYPE tx_type;
  switch (bmode) {
    case B_TM_PRED :
    case B_RD_PRED :
      tx_type = ADST_ADST;
      break;

    case B_VE_PRED :
    case B_VR_PRED :
      tx_type = ADST_DCT;
      break;

    case B_HE_PRED :
    case B_HD_PRED :
    case B_HU_PRED :
      tx_type = DCT_ADST;
      break;

    default :
      tx_type = DCT_DCT;
      break;
  }
  return tx_type;
}
#endif

#if CONFIG_HYBRIDTRANSFORM
static TX_TYPE get_tx_type_4x4(const MACROBLOCKD *xd, const BLOCKD *b) {
  TX_TYPE tx_type = DCT_DCT;
  if (xd->mode_info_context->mbmi.mode == B_PRED &&
      xd->q_index < ACTIVE_HT) {
    tx_type = txfm_map(b->bmi.as_mode.first);
  }
  return tx_type;
}
#endif

#if CONFIG_HYBRIDTRANSFORM8X8
static TX_TYPE get_tx_type_8x8(const MACROBLOCKD *xd, const BLOCKD *b) {
  TX_TYPE tx_type = DCT_DCT;
  if (xd->mode_info_context->mbmi.mode == I8X8_PRED) {
    tx_type = txfm_map(pred_mode_conv(b->bmi.as_mode.first));
  }
  return tx_type;
}
#endif

#if CONFIG_HYBRIDTRANSFORM16X16
static TX_TYPE get_tx_type_16x16(const MACROBLOCKD *xd, const BLOCKD *b) {
  TX_TYPE tx_type = DCT_DCT;
  if (xd->mode_info_context->mbmi.mode < I8X8_PRED &&
      xd->q_index < ACTIVE_HT16) {
    tx_type = txfm_map(pred_mode_conv(xd->mode_info_context->mbmi.mode));
  }
  return tx_type;
}
#endif

#if CONFIG_HYBRIDTRANSFORM || CONFIG_HYBRIDTRANSFORM8X8 || CONFIG_HYBRIDTRANSFORM16X16
static TX_TYPE get_tx_type(const MACROBLOCKD *xd, const BLOCKD *b) {
  TX_TYPE tx_type = DCT_DCT;
  int ib = (b - xd->block);
  if (ib >= 16)
    return tx_type;
#if CONFIG_HYBRIDTRANSFORM16X16
  if (xd->mode_info_context->mbmi.txfm_size == TX_16X16) {
    tx_type = get_tx_type_16x16(xd, b);
  }
#endif
#if CONFIG_HYBRIDTRANSFORM8X8
  if (xd->mode_info_context->mbmi.txfm_size  == TX_8X8) {
    ib = (ib & 8) + ((ib & 4) >> 1);
    tx_type = get_tx_type_8x8(xd, &xd->block[ib]);
  }
#endif
#if CONFIG_HYBRIDTRANSFORM
  if (xd->mode_info_context->mbmi.txfm_size  == TX_4X4) {
    tx_type = get_tx_type_4x4(xd, b);
  }
#endif
  return tx_type;
}
#endif

extern void vp8_build_block_doffsets(MACROBLOCKD *xd);
extern void vp8_setup_block_dptrs(MACROBLOCKD *xd);

static void update_blockd_bmi(MACROBLOCKD *xd) {
  int i;
  int is_4x4;
  is_4x4 = (xd->mode_info_context->mbmi.mode == SPLITMV) ||
           (xd->mode_info_context->mbmi.mode == I8X8_PRED) ||
           (xd->mode_info_context->mbmi.mode == B_PRED);

  if (is_4x4) {
    for (i = 0; i < 16; i++) {
      xd->block[i].bmi = xd->mode_info_context->bmi[i];
    }
  }
}
#endif  /* __INC_BLOCKD_H */
