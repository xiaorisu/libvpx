/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <string.h>
#include "third_party/googletest/src/include/gtest/gtest.h"
extern "C" {
#include "vpx_config.h"
#include "vpx_rtcd.h"
#include "vp8/common/blockd.h"
}

namespace {

class IntraPredBase {
 protected:
  void SetupMacroblock(uint8_t *data, int blockSize, int stride,
                       int numPlanes) {
    memset(&mb_, 0, sizeof(mb_));
    memset(&mi_, 0, sizeof(mi_));
    mb_.up_available = 1;
    mb_.left_available = 1;
    mb_.mode_info_context = &mi_;
    stride_ = stride;
    blockSize_ = blockSize;
    numPlanes_ = numPlanes;
    for (int p = 0; p < numPlanes; p++)
      dataPtr_[p] = data + stride * (blockSize + 1) * p +
                    stride + blockSize;
  }

  void FillRandom() {
    // Fill edges with random data
    for (int p = 0; p < numPlanes_; p++) {
      for (int x = -1 ; x <= blockSize_; x++)
        dataPtr_[p][x - stride_] = rand();
      for (int y = 0; y < blockSize_; y++)
        dataPtr_[p][y * stride_ - 1] = rand();
    }
  }

  virtual void Predict(MB_PREDICTION_MODE mode);

  void SetLeftUnavailable() {
    mb_.left_available = 0;
    for (int p = 0; p < numPlanes_; p++)
      for (int i = -1; i < blockSize_; ++i) {
        dataPtr_[p][stride_ * i - 1] = 129;
      }
  }
    
  void SetTopUnavailable() {
    mb_.up_available = 0;
    for (int p = 0; p < numPlanes_; p++)
      memset(&dataPtr_[p][-1 - stride_], 127, blockSize_ + 2);
  }

  void SetTopLeftUnavailable() {
    SetLeftUnavailable();
    SetTopUnavailable();
  }

  // check DC prediction output against a reference
  int BlockSizeLog2Min1() {
    switch (blockSize_) {
    case 16:
      return 3;
    case 8:
      return 2;
    default:
      abort();
    }
  }
  void CheckDCPrediction() {
    for (int p = 0; p < numPlanes_; p++) {
      // calculate expected DC
      int expected;
      if (mb_.up_available || mb_.left_available) {
        int sum = 0, shift = BlockSizeLog2Min1() + mb_.up_available +
                             mb_.left_available;
        if (mb_.up_available)
          for (int x = 0; x < blockSize_; x++)
            sum += dataPtr_[p][x - stride_];
        if (mb_.left_available)
          for (int y = 0; y < blockSize_; y++)
            sum += dataPtr_[p][y * stride_ - 1];
        expected = (sum + (1 << (shift - 1))) >> shift;
      } else {
        expected = 0x80;
      }

      // check that all subsequent lines are equal to the first
      for (int y = 1; y < blockSize_; ++y)
        EXPECT_EQ(0, memcmp(dataPtr_[p], &dataPtr_[p][y * stride_],
                            blockSize_));
      // within the first line, ensure that each pixel has the same value
      for (int x = 1; x < blockSize_; ++x)
        EXPECT_EQ(dataPtr_[p][0], dataPtr_[p][x]);
      // now ensure that that pixel has the expected (DC) value
      EXPECT_EQ(expected, dataPtr_[p][0]);
    }
  }

  // check V prediction output against a reference
  void CheckVPrediction() {
    // check that all lines equal the top border
    for (int p = 0; p < numPlanes_; p++)
      for (int y = 0; y < blockSize_; y++)
        EXPECT_EQ(0, memcmp(&dataPtr_[p][-stride_],
                            &dataPtr_[p][y * stride_], blockSize_));
  }

  // check H prediction output against a reference
  void CheckHPrediction() {
    // for each line, ensure that each pixel is equal to the left border
    for (int p = 0; p < numPlanes_; p++)
      for (int y = 0; y < blockSize_; y++)
        for (int x = 0; x < blockSize_; x++)
          EXPECT_EQ(dataPtr_[p][-1 + y * stride_],
                    dataPtr_[p][x + y * stride_]);
  }

  // check TM prediction output against a reference
  static int ClipByte(int value) {
    if (value > 255)
      return 255;
    else if (value < 0)
      return 0;
    return value;
  }
  void CheckTMPrediction() {
    for (int p = 0; p < numPlanes_; p++)
      for (int y = 0; y < blockSize_; y++)
        for (int x = 0; x < blockSize_; x++) {
          int expected = ClipByte(dataPtr_[p][x - stride_]
                                + dataPtr_[p][stride_ * y - 1]
                                - dataPtr_[p][-1 - stride_]);
          EXPECT_EQ(expected, dataPtr_[p][y * stride_ + x]);
       }
  }

  // Actual test
  void RunTest() {
    // DC
    FillRandom();
    Predict(DC_PRED);
    CheckDCPrediction();

    // DC from a left edge
    FillRandom();
    SetLeftUnavailable();
    Predict(DC_PRED);
    CheckDCPrediction();

    // DC from a top edge
    FillRandom();
    SetTopUnavailable();
    Predict(DC_PRED);
    CheckDCPrediction();

    // DC from top/left edge
    FillRandom();
    SetTopLeftUnavailable();
    Predict(DC_PRED);
    CheckDCPrediction();

    // Horizontal
    FillRandom();
    Predict(H_PRED);
    CheckHPrediction();

    // Vertical
    FillRandom();
    Predict(V_PRED);
    CheckVPrediction();

    // TM
    FillRandom();
    Predict(TM_PRED);
    CheckTMPrediction();
  }

  MACROBLOCKD mb_;
  MODE_INFO mi_;
  uint8_t *dataPtr_[2]; // in the case of Y, only [0] is used
  int stride_;
  int blockSize_;
  int numPlanes_;
};

typedef void (*intra_pred_y_fn_t)(MACROBLOCKD *x,
                                  uint8_t *yabove_row,
                                  uint8_t *yleft,
                                  int left_stride,
                                  uint8_t *ypred_ptr,
                                  int y_stride);

class IntraPredYTest : public ::testing::TestWithParam<intra_pred_y_fn_t>,
    protected IntraPredBase {
 protected:
  static const int kBlockSize = 16;
  static const int kStride = kBlockSize * 3;

  virtual void SetUp() {
    pred_fn_ = GetParam();
    SetupMacroblock(data_array_, kBlockSize, kStride, 1);
  }

  virtual void Predict(MB_PREDICTION_MODE mode) {
    mb_.mode_info_context->mbmi.mode = mode;
    pred_fn_(&mb_, dataPtr_[0] - kStride, dataPtr_[0] - 1, kStride,
             dataPtr_[0], kStride);
  }

  intra_pred_y_fn_t pred_fn_;
  // We use 48 so that the data pointer of the first pixel in each row of
  // each macroblock is 16-byte aligned, and this gives us access to the
  // top-left and top-right corner pixels belonging to the top-left/right
  // macroblocks.
  // We use 17 lines so we have one line above us for top-prediction.
  DECLARE_ALIGNED(16, uint8_t, data_array_[kStride * (kBlockSize + 1)]);
};

TEST_P(IntraPredYTest, IntraPredTests) {
  RunTest();
}

INSTANTIATE_TEST_CASE_P(C, IntraPredYTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mby_s_c));
#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, IntraPredYTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mby_s_sse2));
#endif
#if HAVE_SSSE3
INSTANTIATE_TEST_CASE_P(SSSE3, IntraPredYTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mby_s_ssse3));
#endif

typedef void (*intra_pred_uv_fn_t)(MACROBLOCKD *x,
                                   uint8_t *uabove_row,
                                   uint8_t *vabove_row,
                                   uint8_t *uleft,
                                   uint8_t *vleft,
                                   int left_stride,
                                   uint8_t *upred_ptr,
                                   uint8_t *vpred_ptr,
                                   int pred_stride);

class IntraPredUVTest : public ::testing::TestWithParam<intra_pred_uv_fn_t>,
    protected IntraPredBase {
 protected:
  static const int kBlockSize = 8;
  static const int kStride = kBlockSize * 3;

  virtual void SetUp() {
    pred_fn_ = GetParam();
    SetupMacroblock(data_array_, kBlockSize, kStride, 2);
  }

  virtual void Predict(MB_PREDICTION_MODE mode) {
    mb_.mode_info_context->mbmi.uv_mode = mode;
    pred_fn_(&mb_, dataPtr_[0] - kStride, dataPtr_[1] - kStride,
             dataPtr_[0] - 1, dataPtr_[1] - 1, kStride,
             dataPtr_[0], dataPtr_[1], kStride);
  }

  intra_pred_uv_fn_t pred_fn_;
  // We use 24 so that the data pointer of the first pixel in each row of
  // each macroblock is 8-byte aligned, and this gives us access to the
  // top-left and top-right corner pixels belonging to the top-left/right
  // macroblocks.
  // We use 9 lines so we have one line above us for top-prediction.
  // [0] = U, [1] = V
  DECLARE_ALIGNED(8, uint8_t, data_array_[2 * kStride * (kBlockSize + 1)]);
};

TEST_P(IntraPredUVTest, IntraPredTests) {
  RunTest();
}

INSTANTIATE_TEST_CASE_P(C, IntraPredUVTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mbuv_s_c));
#if HAVE_SSE2
INSTANTIATE_TEST_CASE_P(SSE2, IntraPredUVTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mbuv_s_sse2));
#endif
#if HAVE_SSSE3
INSTANTIATE_TEST_CASE_P(SSSE3, IntraPredUVTest,
                        ::testing::Values(
                            vp8_build_intra_predictors_mbuv_s_ssse3));
#endif

}  // namespace
