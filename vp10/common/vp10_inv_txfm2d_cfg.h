/*
 *  Copyright (c) 2015 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP10_INV_TXFM2D_CFG_H_
#define VP10_INV_TXFM2D_CFG_H_
#include "vp10/common/vp10_inv_txfm1d.h"

//  ---------------- config inv_dct_dct_4 ----------------
static const int8_t inv_shift_dct_dct_4[2] = {1, -5};
static const int8_t inv_stage_range_col_dct_dct_4[4] = {17, 17, 16, 16};
static const int8_t inv_stage_range_row_dct_dct_4[4] = {16, 16, 16, 16};
static const int8_t inv_cos_bit_col_dct_dct_4[4] = {15, 15, 15, 15};
static const int8_t inv_cos_bit_row_dct_dct_4[4] = {15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_dct_4 = {
    4,                              // .txfm_size
    4,                              // .stage_num_col
    4,                              // .stage_num_row
    inv_shift_dct_dct_4,            // .shift
    inv_stage_range_col_dct_dct_4,  // .stage_range_col
    inv_stage_range_row_dct_dct_4,  // .stage_range_row
    inv_cos_bit_col_dct_dct_4,      // .cos_bit_col
    inv_cos_bit_row_dct_dct_4,      // .cos_bit_row
    vp10_idct4_new,                 // .txfm_func_col
    vp10_idct4_new};                // .txfm_func_row;

//  ---------------- config inv_dct_dct_8 ----------------
static const int8_t inv_shift_dct_dct_8[2] = {0, -5};
static const int8_t inv_stage_range_col_dct_dct_8[6] = {17, 17, 17, 17, 16, 16};
static const int8_t inv_stage_range_row_dct_dct_8[6] = {17, 17, 17, 17, 17, 17};
static const int8_t inv_cos_bit_col_dct_dct_8[6] = {15, 15, 15, 15, 15, 15};
static const int8_t inv_cos_bit_row_dct_dct_8[6] = {15, 15, 15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_dct_8 = {
    8,                              // .txfm_size
    6,                              // .stage_num_col
    6,                              // .stage_num_row
    inv_shift_dct_dct_8,            // .shift
    inv_stage_range_col_dct_dct_8,  // .stage_range_col
    inv_stage_range_row_dct_dct_8,  // .stage_range_row
    inv_cos_bit_col_dct_dct_8,      // .cos_bit_col
    inv_cos_bit_row_dct_dct_8,      // .cos_bit_row
    vp10_idct8_new,                 // .txfm_func_col
    vp10_idct8_new};                // .txfm_func_row;

//  ---------------- config inv_dct_dct_16 ----------------
static const int8_t inv_shift_dct_dct_16[2] = {0, -6};
static const int8_t inv_stage_range_col_dct_dct_16[8] = {18, 18, 18, 18,
                                                         18, 18, 17, 17};
static const int8_t inv_stage_range_row_dct_dct_16[8] = {18, 18, 18, 18,
                                                         18, 18, 18, 18};
static const int8_t inv_cos_bit_col_dct_dct_16[8] = {14, 14, 14, 14,
                                                     14, 14, 14, 15};
static const int8_t inv_cos_bit_row_dct_dct_16[8] = {14, 14, 14, 14,
                                                     14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_dct_16 = {
    16,                              // .txfm_size
    8,                               // .stage_num_col
    8,                               // .stage_num_row
    inv_shift_dct_dct_16,            // .shift
    inv_stage_range_col_dct_dct_16,  // .stage_range_col
    inv_stage_range_row_dct_dct_16,  // .stage_range_row
    inv_cos_bit_col_dct_dct_16,      // .cos_bit_col
    inv_cos_bit_row_dct_dct_16,      // .cos_bit_row
    vp10_idct16_new,                 // .txfm_func_col
    vp10_idct16_new};                // .txfm_func_row;

//  ---------------- config inv_dct_dct_32 ----------------
static const int8_t inv_shift_dct_dct_32[2] = {-1, -6};
static const int8_t inv_stage_range_col_dct_dct_32[10] = {18, 18, 18, 18, 18,
                                                          18, 18, 18, 17, 17};
static const int8_t inv_stage_range_row_dct_dct_32[10] = {19, 19, 19, 19, 19,
                                                          19, 19, 19, 19, 19};
static const int8_t inv_cos_bit_col_dct_dct_32[10] = {14, 14, 14, 14, 14,
                                                      14, 14, 14, 14, 15};
static const int8_t inv_cos_bit_row_dct_dct_32[10] = {13, 13, 13, 13, 13,
                                                      13, 13, 13, 13, 13};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_dct_32 = {
    32,                              // .txfm_size
    10,                              // .stage_num_col
    10,                              // .stage_num_row
    inv_shift_dct_dct_32,            // .shift
    inv_stage_range_col_dct_dct_32,  // .stage_range_col
    inv_stage_range_row_dct_dct_32,  // .stage_range_row
    inv_cos_bit_col_dct_dct_32,      // .cos_bit_col
    inv_cos_bit_row_dct_dct_32,      // .cos_bit_row
    vp10_idct32_new,                 // .txfm_func_col
    vp10_idct32_new};                // .txfm_func_row;

//  ---------------- config inv_dct_dct_64 ----------------
static int8_t inv_shift_dct_dct_64[2] = {-1, -7};
static int8_t inv_stage_range_col_dct_dct_64[12] = {19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 18, 18};
static int8_t inv_stage_range_row_dct_dct_64[12] = {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20};
static int8_t inv_cos_bit_col_dct_dct_64[12] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14};
static int8_t inv_cos_bit_row_dct_dct_64[12] = {12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_dct_64 = {
    64,                              // .txfm_size
    12,                              // .stage_num_col
    12,                              // .stage_num_row
    inv_shift_dct_dct_64,            // .shift
    inv_stage_range_col_dct_dct_64,  // .stage_range_col
    inv_stage_range_row_dct_dct_64,  // .stage_range_row
    inv_cos_bit_col_dct_dct_64,      // .cos_bit_col
    inv_cos_bit_row_dct_dct_64,      // .cos_bit_row
    vp10_idct64_new,                 // .txfm_func_col
    vp10_idct64_new};                // .txfm_func_row;

//  ---------------- config inv_dct_adst_4 ----------------
static const int8_t inv_shift_dct_adst_4[2] = {1, -5};
static const int8_t inv_stage_range_col_dct_adst_4[4] = {17, 17, 16, 16};
static const int8_t inv_stage_range_row_dct_adst_4[6] = {16, 16, 16,
                                                         16, 16, 16};
static const int8_t inv_cos_bit_col_dct_adst_4[4] = {15, 15, 15, 15};
static const int8_t inv_cos_bit_row_dct_adst_4[6] = {15, 15, 15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_adst_4 = {
    4,                               // .txfm_size
    4,                               // .stage_num_col
    6,                               // .stage_num_row
    inv_shift_dct_adst_4,            // .shift
    inv_stage_range_col_dct_adst_4,  // .stage_range_col
    inv_stage_range_row_dct_adst_4,  // .stage_range_row
    inv_cos_bit_col_dct_adst_4,      // .cos_bit_col
    inv_cos_bit_row_dct_adst_4,      // .cos_bit_row
    vp10_idct4_new,                  // .txfm_func_col
    vp10_iadst4_new};                // .txfm_func_row;

//  ---------------- config inv_dct_adst_8 ----------------
static const int8_t inv_shift_dct_adst_8[2] = {-1, -4};
static const int8_t inv_stage_range_col_dct_adst_8[6] = {16, 16, 16,
                                                         16, 15, 15};
static const int8_t inv_stage_range_row_dct_adst_8[8] = {17, 17, 17, 17,
                                                         17, 17, 17, 17};
static const int8_t inv_cos_bit_col_dct_adst_8[6] = {15, 15, 15, 15, 15, 15};
static const int8_t inv_cos_bit_row_dct_adst_8[8] = {15, 15, 15, 15,
                                                     15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_adst_8 = {
    8,                               // .txfm_size
    6,                               // .stage_num_col
    8,                               // .stage_num_row
    inv_shift_dct_adst_8,            // .shift
    inv_stage_range_col_dct_adst_8,  // .stage_range_col
    inv_stage_range_row_dct_adst_8,  // .stage_range_row
    inv_cos_bit_col_dct_adst_8,      // .cos_bit_col
    inv_cos_bit_row_dct_adst_8,      // .cos_bit_row
    vp10_idct8_new,                  // .txfm_func_col
    vp10_iadst8_new};                // .txfm_func_row;

//  ---------------- config inv_dct_adst_16 ----------------
static const int8_t inv_shift_dct_adst_16[2] = {1, -7};
static const int8_t inv_stage_range_col_dct_adst_16[8] = {19, 19, 19, 19,
                                                          19, 19, 18, 18};
static const int8_t inv_stage_range_row_dct_adst_16[10] = {18, 18, 18, 18, 18,
                                                           18, 18, 18, 18, 18};
static const int8_t inv_cos_bit_col_dct_adst_16[8] = {13, 13, 13, 13,
                                                      13, 13, 13, 14};
static const int8_t inv_cos_bit_row_dct_adst_16[10] = {14, 14, 14, 14, 14,
                                                       14, 14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_adst_16 = {
    16,                               // .txfm_size
    8,                                // .stage_num_col
    10,                               // .stage_num_row
    inv_shift_dct_adst_16,            // .shift
    inv_stage_range_col_dct_adst_16,  // .stage_range_col
    inv_stage_range_row_dct_adst_16,  // .stage_range_row
    inv_cos_bit_col_dct_adst_16,      // .cos_bit_col
    inv_cos_bit_row_dct_adst_16,      // .cos_bit_row
    vp10_idct16_new,                  // .txfm_func_col
    vp10_iadst16_new};                // .txfm_func_row;

//  ---------------- config inv_dct_adst_32 ----------------
static const int8_t inv_shift_dct_adst_32[2] = {-1, -6};
static const int8_t inv_stage_range_col_dct_adst_32[10] = {18, 18, 18, 18, 18,
                                                           18, 18, 18, 17, 17};
static const int8_t inv_stage_range_row_dct_adst_32[12] = {
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19};
static const int8_t inv_cos_bit_col_dct_adst_32[10] = {14, 14, 14, 14, 14,
                                                       14, 14, 14, 14, 15};
static const int8_t inv_cos_bit_row_dct_adst_32[12] = {13, 13, 13, 13, 13, 13,
                                                       13, 13, 13, 13, 13, 13};

static const TXFM_2D_CFG inv_txfm_2d_cfg_dct_adst_32 = {
    32,                               // .txfm_size
    10,                               // .stage_num_col
    12,                               // .stage_num_row
    inv_shift_dct_adst_32,            // .shift
    inv_stage_range_col_dct_adst_32,  // .stage_range_col
    inv_stage_range_row_dct_adst_32,  // .stage_range_row
    inv_cos_bit_col_dct_adst_32,      // .cos_bit_col
    inv_cos_bit_row_dct_adst_32,      // .cos_bit_row
    vp10_idct32_new,                  // .txfm_func_col
    vp10_iadst32_new};                // .txfm_func_row;

//  ---------------- config inv_adst_adst_4 ----------------
static const int8_t inv_shift_adst_adst_4[2] = {0, -4};
static const int8_t inv_stage_range_col_adst_adst_4[6] = {16, 16, 16,
                                                          16, 15, 15};
static const int8_t inv_stage_range_row_adst_adst_4[6] = {16, 16, 16,
                                                          16, 16, 16};
static const int8_t inv_cos_bit_col_adst_adst_4[6] = {15, 15, 15, 15, 15, 15};
static const int8_t inv_cos_bit_row_adst_adst_4[6] = {15, 15, 15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_adst_4 = {
    4,                                // .txfm_size
    6,                                // .stage_num_col
    6,                                // .stage_num_row
    inv_shift_adst_adst_4,            // .shift
    inv_stage_range_col_adst_adst_4,  // .stage_range_col
    inv_stage_range_row_adst_adst_4,  // .stage_range_row
    inv_cos_bit_col_adst_adst_4,      // .cos_bit_col
    inv_cos_bit_row_adst_adst_4,      // .cos_bit_row
    vp10_iadst4_new,                  // .txfm_func_col
    vp10_iadst4_new};                 // .txfm_func_row;

//  ---------------- config inv_adst_adst_8 ----------------
static const int8_t inv_shift_adst_adst_8[2] = {-1, -4};
static const int8_t inv_stage_range_col_adst_adst_8[8] = {16, 16, 16, 16,
                                                          16, 16, 15, 15};
static const int8_t inv_stage_range_row_adst_adst_8[8] = {17, 17, 17, 17,
                                                          17, 17, 17, 17};
static const int8_t inv_cos_bit_col_adst_adst_8[8] = {15, 15, 15, 15,
                                                      15, 15, 15, 15};
static const int8_t inv_cos_bit_row_adst_adst_8[8] = {15, 15, 15, 15,
                                                      15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_adst_8 = {
    8,                                // .txfm_size
    8,                                // .stage_num_col
    8,                                // .stage_num_row
    inv_shift_adst_adst_8,            // .shift
    inv_stage_range_col_adst_adst_8,  // .stage_range_col
    inv_stage_range_row_adst_adst_8,  // .stage_range_row
    inv_cos_bit_col_adst_adst_8,      // .cos_bit_col
    inv_cos_bit_row_adst_adst_8,      // .cos_bit_row
    vp10_iadst8_new,                  // .txfm_func_col
    vp10_iadst8_new};                 // .txfm_func_row;

//  ---------------- config inv_adst_adst_16 ----------------
static const int8_t inv_shift_adst_adst_16[2] = {0, -6};
static const int8_t inv_stage_range_col_adst_adst_16[10] = {18, 18, 18, 18, 18,
                                                            18, 18, 18, 17, 17};
static const int8_t inv_stage_range_row_adst_adst_16[10] = {18, 18, 18, 18, 18,
                                                            18, 18, 18, 18, 18};
static const int8_t inv_cos_bit_col_adst_adst_16[10] = {14, 14, 14, 14, 14,
                                                        14, 14, 14, 14, 15};
static const int8_t inv_cos_bit_row_adst_adst_16[10] = {14, 14, 14, 14, 14,
                                                        14, 14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_adst_16 = {
    16,                                // .txfm_size
    10,                                // .stage_num_col
    10,                                // .stage_num_row
    inv_shift_adst_adst_16,            // .shift
    inv_stage_range_col_adst_adst_16,  // .stage_range_col
    inv_stage_range_row_adst_adst_16,  // .stage_range_row
    inv_cos_bit_col_adst_adst_16,      // .cos_bit_col
    inv_cos_bit_row_adst_adst_16,      // .cos_bit_row
    vp10_iadst16_new,                  // .txfm_func_col
    vp10_iadst16_new};                 // .txfm_func_row;

//  ---------------- config inv_adst_adst_32 ----------------
static const int8_t inv_shift_adst_adst_32[2] = {-1, -6};
static const int8_t inv_stage_range_col_adst_adst_32[12] = {
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 17, 17};
static const int8_t inv_stage_range_row_adst_adst_32[12] = {
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19};
static const int8_t inv_cos_bit_col_adst_adst_32[12] = {14, 14, 14, 14, 14, 14,
                                                        14, 14, 14, 14, 14, 15};
static const int8_t inv_cos_bit_row_adst_adst_32[12] = {13, 13, 13, 13, 13, 13,
                                                        13, 13, 13, 13, 13, 13};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_adst_32 = {
    32,                                // .txfm_size
    12,                                // .stage_num_col
    12,                                // .stage_num_row
    inv_shift_adst_adst_32,            // .shift
    inv_stage_range_col_adst_adst_32,  // .stage_range_col
    inv_stage_range_row_adst_adst_32,  // .stage_range_row
    inv_cos_bit_col_adst_adst_32,      // .cos_bit_col
    inv_cos_bit_row_adst_adst_32,      // .cos_bit_row
    vp10_iadst32_new,                  // .txfm_func_col
    vp10_iadst32_new};                 // .txfm_func_row;

//  ---------------- config inv_adst_dct_4 ----------------
static const int8_t inv_shift_adst_dct_4[2] = {1, -5};
static const int8_t inv_stage_range_col_adst_dct_4[6] = {17, 17, 17,
                                                         17, 16, 16};
static const int8_t inv_stage_range_row_adst_dct_4[4] = {16, 16, 16, 16};
static const int8_t inv_cos_bit_col_adst_dct_4[6] = {15, 15, 15, 15, 15, 15};
static const int8_t inv_cos_bit_row_adst_dct_4[4] = {15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_dct_4 = {
    4,                               // .txfm_size
    6,                               // .stage_num_col
    4,                               // .stage_num_row
    inv_shift_adst_dct_4,            // .shift
    inv_stage_range_col_adst_dct_4,  // .stage_range_col
    inv_stage_range_row_adst_dct_4,  // .stage_range_row
    inv_cos_bit_col_adst_dct_4,      // .cos_bit_col
    inv_cos_bit_row_adst_dct_4,      // .cos_bit_row
    vp10_iadst4_new,                 // .txfm_func_col
    vp10_idct4_new};                 // .txfm_func_row;

//  ---------------- config inv_adst_dct_8 ----------------
static const int8_t inv_shift_adst_dct_8[2] = {-1, -4};
static const int8_t inv_stage_range_col_adst_dct_8[8] = {16, 16, 16, 16,
                                                         16, 16, 15, 15};
static const int8_t inv_stage_range_row_adst_dct_8[6] = {17, 17, 17,
                                                         17, 17, 17};
static const int8_t inv_cos_bit_col_adst_dct_8[8] = {15, 15, 15, 15,
                                                     15, 15, 15, 15};
static const int8_t inv_cos_bit_row_adst_dct_8[6] = {15, 15, 15, 15, 15, 15};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_dct_8 = {
    8,                               // .txfm_size
    8,                               // .stage_num_col
    6,                               // .stage_num_row
    inv_shift_adst_dct_8,            // .shift
    inv_stage_range_col_adst_dct_8,  // .stage_range_col
    inv_stage_range_row_adst_dct_8,  // .stage_range_row
    inv_cos_bit_col_adst_dct_8,      // .cos_bit_col
    inv_cos_bit_row_adst_dct_8,      // .cos_bit_row
    vp10_iadst8_new,                 // .txfm_func_col
    vp10_idct8_new};                 // .txfm_func_row;

//  ---------------- config inv_adst_dct_16 ----------------
static const int8_t inv_shift_adst_dct_16[2] = {-1, -5};
static const int8_t inv_stage_range_col_adst_dct_16[10] = {17, 17, 17, 17, 17,
                                                           17, 17, 17, 16, 16};
static const int8_t inv_stage_range_row_adst_dct_16[8] = {18, 18, 18, 18,
                                                          18, 18, 18, 18};
static const int8_t inv_cos_bit_col_adst_dct_16[10] = {15, 15, 15, 15, 15,
                                                       15, 15, 15, 15, 15};
static const int8_t inv_cos_bit_row_adst_dct_16[8] = {14, 14, 14, 14,
                                                      14, 14, 14, 14};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_dct_16 = {
    16,                               // .txfm_size
    10,                               // .stage_num_col
    8,                                // .stage_num_row
    inv_shift_adst_dct_16,            // .shift
    inv_stage_range_col_adst_dct_16,  // .stage_range_col
    inv_stage_range_row_adst_dct_16,  // .stage_range_row
    inv_cos_bit_col_adst_dct_16,      // .cos_bit_col
    inv_cos_bit_row_adst_dct_16,      // .cos_bit_row
    vp10_iadst16_new,                 // .txfm_func_col
    vp10_idct16_new};                 // .txfm_func_row;

//  ---------------- config inv_adst_dct_32 ----------------
static const int8_t inv_shift_adst_dct_32[2] = {-1, -6};
static const int8_t inv_stage_range_col_adst_dct_32[12] = {
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 17, 17};
static const int8_t inv_stage_range_row_adst_dct_32[10] = {19, 19, 19, 19, 19,
                                                           19, 19, 19, 19, 19};
static const int8_t inv_cos_bit_col_adst_dct_32[12] = {14, 14, 14, 14, 14, 14,
                                                       14, 14, 14, 14, 14, 15};
static const int8_t inv_cos_bit_row_adst_dct_32[10] = {13, 13, 13, 13, 13,
                                                       13, 13, 13, 13, 13};

static const TXFM_2D_CFG inv_txfm_2d_cfg_adst_dct_32 = {
    32,                               // .txfm_size
    12,                               // .stage_num_col
    10,                               // .stage_num_row
    inv_shift_adst_dct_32,            // .shift
    inv_stage_range_col_adst_dct_32,  // .stage_range_col
    inv_stage_range_row_adst_dct_32,  // .stage_range_row
    inv_cos_bit_col_adst_dct_32,      // .cos_bit_col
    inv_cos_bit_row_adst_dct_32,      // .cos_bit_row
    vp10_iadst32_new,                 // .txfm_func_col
    vp10_idct32_new};                 // .txfm_func_row;

#endif  // VP10_INV_TXFM2D_CFG_H_
