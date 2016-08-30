/*
 *  Copyright (c) 2012 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/codec_factory.h"
#include "test/encode_test_driver.h"
#include "test/i420_video_source.h"
#include "test/util.h"
#include "test/y4m_video_source.h"

namespace {

const int kMaxPSNR = 100;

class CpuSpeedTest
    : public ::libaom_test::EncoderTest,
      public ::libaom_test::CodecTestWith2Params<libaom_test::TestMode, int> {
 protected:
  CpuSpeedTest()
      : EncoderTest(GET_PARAM(0)), encoding_mode_(GET_PARAM(1)),
        set_cpu_used_(GET_PARAM(2)), min_psnr_(kMaxPSNR),
        tune_content_(AOM_CONTENT_DEFAULT) {}
  virtual ~CpuSpeedTest() {}

  virtual void SetUp() {
    InitializeConfig();
    SetMode(encoding_mode_);
    if (encoding_mode_ != ::libaom_test::kRealTime) {
      cfg_.g_lag_in_frames = 25;
      cfg_.rc_end_usage = AOM_VBR;
    } else {
      cfg_.g_lag_in_frames = 0;
      cfg_.rc_end_usage = AOM_CBR;
    }
  }

  virtual void BeginPassHook(unsigned int /*pass*/) { min_psnr_ = kMaxPSNR; }

  virtual void PreEncodeFrameHook(::libaom_test::VideoSource *video,
                                  ::libaom_test::Encoder *encoder) {
    if (video->frame() == 1) {
      encoder->Control(AOME_SET_CPUUSED, set_cpu_used_);
      encoder->Control(AV1E_SET_TUNE_CONTENT, tune_content_);
      if (encoding_mode_ != ::libaom_test::kRealTime) {
        encoder->Control(AOME_SET_ENABLEAUTOALTREF, 1);
        encoder->Control(AOME_SET_ARNR_MAXFRAMES, 7);
        encoder->Control(AOME_SET_ARNR_STRENGTH, 5);
        encoder->Control(AOME_SET_ARNR_TYPE, 3);
      }
    }
  }

  virtual void PSNRPktHook(const aom_codec_cx_pkt_t *pkt) {
    if (pkt->data.psnr.psnr[0] < min_psnr_) min_psnr_ = pkt->data.psnr.psnr[0];
  }

  void TestQ0();
  void TestScreencastQ0();
  void TestTuneScreen();
  void TestEncodeHighBitrate();
  void TestLowBitrate();

  ::libaom_test::TestMode encoding_mode_;
  int set_cpu_used_;
  double min_psnr_;
  int tune_content_;
};

void CpuSpeedTest::TestQ0() {
  // Validate that this non multiple of 64 wide clip encodes and decodes
  // without a mismatch when passing in a very low max q.  This pushes
  // the encoder to producing lots of big partitions which will likely
  // extend into the border and test the border condition.
  cfg_.rc_2pass_vbr_minsection_pct = 5;
  cfg_.rc_2pass_vbr_maxsection_pct = 2000;
  cfg_.rc_target_bitrate = 400;
  cfg_.rc_max_quantizer = 0;
  cfg_.rc_min_quantizer = 0;

  ::libaom_test::I420VideoSource video("hantro_odd.yuv", 208, 144, 30, 1, 0,
                                       10);

  init_flags_ = AOM_CODEC_USE_PSNR;

  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  EXPECT_GE(min_psnr_, kMaxPSNR);
}

void CpuSpeedTest::TestScreencastQ0() {
  ::libaom_test::Y4mVideoSource video("screendata.y4m", 0, 10);
  cfg_.g_timebase = video.timebase();
  cfg_.rc_2pass_vbr_minsection_pct = 5;
  cfg_.rc_2pass_vbr_maxsection_pct = 2000;
  cfg_.rc_target_bitrate = 400;
  cfg_.rc_max_quantizer = 0;
  cfg_.rc_min_quantizer = 0;

  init_flags_ = AOM_CODEC_USE_PSNR;

  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  EXPECT_GE(min_psnr_, kMaxPSNR);
}

void CpuSpeedTest::TestTuneScreen() {
  ::libaom_test::Y4mVideoSource video("screendata.y4m", 0, 10);
  cfg_.g_timebase = video.timebase();
  cfg_.rc_2pass_vbr_minsection_pct = 5;
  cfg_.rc_2pass_vbr_minsection_pct = 2000;
  cfg_.rc_target_bitrate = 2000;
  cfg_.rc_max_quantizer = 63;
  cfg_.rc_min_quantizer = 0;
  tune_content_ = AOM_CONTENT_SCREEN;

  init_flags_ = AOM_CODEC_USE_PSNR;

  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
}

void CpuSpeedTest::TestEncodeHighBitrate() {
  // Validate that this non multiple of 64 wide clip encodes and decodes
  // without a mismatch when passing in a very low max q.  This pushes
  // the encoder to producing lots of big partitions which will likely
  // extend into the border and test the border condition.
  cfg_.rc_2pass_vbr_minsection_pct = 5;
  cfg_.rc_2pass_vbr_maxsection_pct = 2000;
  cfg_.rc_target_bitrate = 12000;
  cfg_.rc_max_quantizer = 10;
  cfg_.rc_min_quantizer = 0;

  ::libaom_test::I420VideoSource video("hantro_odd.yuv", 208, 144, 30, 1, 0,
                                       10);

  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
}

void CpuSpeedTest::TestLowBitrate() {
  // Validate that this clip encodes and decodes without a mismatch
  // when passing in a very high min q.  This pushes the encoder to producing
  // lots of small partitions which might will test the other condition.
  cfg_.rc_2pass_vbr_minsection_pct = 5;
  cfg_.rc_2pass_vbr_maxsection_pct = 2000;
  cfg_.rc_target_bitrate = 200;
  cfg_.rc_min_quantizer = 40;

  ::libaom_test::I420VideoSource video("hantro_odd.yuv", 208, 144, 30, 1, 0,
                                       10);

  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
}

TEST_P(CpuSpeedTest, TestQ0) { TestQ0(); }
TEST_P(CpuSpeedTest, TestScreencastQ0) { TestScreencastQ0(); }
TEST_P(CpuSpeedTest, TestTuneScreen) { TestTuneScreen(); }
TEST_P(CpuSpeedTest, TestEncodeHighBitrate) { TestEncodeHighBitrate(); }
TEST_P(CpuSpeedTest, TestLowBitrate) { TestLowBitrate(); }

class CpuSpeedTestLarge : public CpuSpeedTest {};

TEST_P(CpuSpeedTestLarge, TestQ0) { TestQ0(); }
TEST_P(CpuSpeedTestLarge, TestScreencastQ0) { TestScreencastQ0(); }
TEST_P(CpuSpeedTestLarge, TestTuneScreen) { TestTuneScreen(); }
TEST_P(CpuSpeedTestLarge, TestEncodeHighBitrate) { TestEncodeHighBitrate(); }
TEST_P(CpuSpeedTestLarge, TestLowBitrate) { TestLowBitrate(); }

AV1_INSTANTIATE_TEST_CASE(CpuSpeedTest,
                          ::testing::Values(::libaom_test::kTwoPassGood,
                                            ::libaom_test::kOnePassGood),
                          ::testing::Range(1, 3));
AV1_INSTANTIATE_TEST_CASE(CpuSpeedTestLarge,
                          ::testing::Values(::libaom_test::kTwoPassGood,
                                            ::libaom_test::kOnePassGood),
                          ::testing::Range(0, 1));
}  // namespace
