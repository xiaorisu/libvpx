/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


extern "C" {
#include "./vpx_config.h"
#include "./vp9_rtcd.h"
}
#include "third_party/googletest/src/include/gtest/gtest.h"
#include "test/acm_random.h"
#include "test/register_state_check.h"
#include "test/util.h"

namespace {
typedef void (*convolve_fn_t)(const uint8_t *src, int src_stride,
                              uint8_t *dst, int dst_stride,
                              const int16_t *filter_x, int filter_x_stride,
                              const int16_t *filter_y, int filter_y_stride,
                              int w, int h);

struct ConvolveFunctions {
  ConvolveFunctions(convolve_fn_t h8, convolve_fn_t h8_avg,
                    convolve_fn_t v8, convolve_fn_t v8_avg,
                    convolve_fn_t hv8, convolve_fn_t hv8_avg)
      : h8_(h8), v8_(v8), hv8_(hv8), h8_avg_(h8_avg), v8_avg_(v8_avg),
        hv8_avg_(hv8_avg) {}

  convolve_fn_t h8_;
  convolve_fn_t v8_;
  convolve_fn_t hv8_;
  convolve_fn_t h8_avg_;
  convolve_fn_t v8_avg_;
  convolve_fn_t hv8_avg_;
};

// Reference 8-tap subpixel filter, slightly modified to fit into this test.
#define VP9_FILTER_WEIGHT 128
#define VP9_FILTER_SHIFT 7
static uint8_t clip_pixel(int x) {
  return x < 0 ? 0 :
         x > 255 ? 255 :
         x;
}

static void filter_block2d_8_c(const uint8_t *src_ptr,
                               const unsigned int src_stride,
                               const int16_t *HFilter,
                               const int16_t *VFilter,
                               uint8_t *dst_ptr,
                               unsigned int dst_stride,
                               unsigned int output_width,
                               unsigned int output_height) {
  // Between passes, we use an intermediate buffer whose height is extended to
  // have enough horizontally filtered values as input for the vertical pass.
  // This buffer is allocated to be big enough for the largest block type we
  // support.
  const int kInterp_Extend = 4;
  const unsigned int intermediate_height =
    (kInterp_Extend - 1) +     output_height + kInterp_Extend;

  /* Size of intermediate_buffer is max_intermediate_height * filter_max_width,
   * where max_intermediate_height = (kInterp_Extend - 1) + filter_max_height
   *                                 + kInterp_Extend
   *                               = 3 + 16 + 4
   *                               = 23
   * and filter_max_width = 16
   */
  uint8_t intermediate_buffer[23 * 16];
  const int intermediate_next_stride = 1 - intermediate_height * output_width;

  // Horizontal pass (src -> transposed intermediate).
  {
    uint8_t *output_ptr = intermediate_buffer;
    const int src_next_row_stride = src_stride - output_width;
    unsigned int i, j;
    src_ptr -= (kInterp_Extend - 1) * src_stride + (kInterp_Extend - 1);
    for (i = 0; i < intermediate_height; i++) {
      for (j = 0; j < output_width; j++) {
        // Apply filter...
        int temp = ((int)src_ptr[0] * HFilter[0]) +
                   ((int)src_ptr[1] * HFilter[1]) +
                   ((int)src_ptr[2] * HFilter[2]) +
                   ((int)src_ptr[3] * HFilter[3]) +
                   ((int)src_ptr[4] * HFilter[4]) +
                   ((int)src_ptr[5] * HFilter[5]) +
                   ((int)src_ptr[6] * HFilter[6]) +
                   ((int)src_ptr[7] * HFilter[7]) +
                   (VP9_FILTER_WEIGHT >> 1);  // Rounding

        // Normalize back to 0-255...
        *output_ptr = clip_pixel(temp >> VP9_FILTER_SHIFT);
        src_ptr++;
        output_ptr += intermediate_height;
      }
      src_ptr += src_next_row_stride;
      output_ptr += intermediate_next_stride;
    }
  }

  // Vertical pass (transposed intermediate -> dst).
  {
    uint8_t *src_ptr = intermediate_buffer;
    const int dst_next_row_stride = dst_stride - output_width;
    unsigned int i, j;
    for (i = 0; i < output_height; i++) {
      for (j = 0; j < output_width; j++) {
        // Apply filter...
        int temp = ((int)src_ptr[0] * VFilter[0]) +
                   ((int)src_ptr[1] * VFilter[1]) +
                   ((int)src_ptr[2] * VFilter[2]) +
                   ((int)src_ptr[3] * VFilter[3]) +
                   ((int)src_ptr[4] * VFilter[4]) +
                   ((int)src_ptr[5] * VFilter[5]) +
                   ((int)src_ptr[6] * VFilter[6]) +
                   ((int)src_ptr[7] * VFilter[7]) +
                   (VP9_FILTER_WEIGHT >> 1);  // Rounding

        // Normalize back to 0-255...
        *dst_ptr++ = clip_pixel(temp >> VP9_FILTER_SHIFT);
        src_ptr += intermediate_height;
      }
      src_ptr += intermediate_next_stride;
      dst_ptr += dst_next_row_stride;
    }
  }
}

static void block2d_average_c(uint8_t *src,
                              unsigned int src_stride,
                              uint8_t *output_ptr,
                              unsigned int output_stride,
                              unsigned int output_width,
                              unsigned int output_height) {
  unsigned int i, j;
  for (i = 0; i < output_height; i++) {
    for (j = 0; j < output_width; j++) {
      output_ptr[j] = (output_ptr[j] + src[i * src_stride + j] + 1) >> 1;
    }
    output_ptr += output_stride;
  }
}

static void filter_average_block2d_8_c(const uint8_t *src_ptr,
                                       const unsigned int src_stride,
                                       const int16_t *HFilter,
                                       const int16_t *VFilter,
                                       uint8_t *dst_ptr,
                                       unsigned int dst_stride,
                                       unsigned int output_width,
                                       unsigned int output_height) {
  uint8_t tmp[16*16];

  assert(output_width <= 16);
  assert(output_height <= 16);
  filter_block2d_8_c(src_ptr, src_stride, HFilter, VFilter, tmp, 16,
                     output_width, output_height);
  block2d_average_c(tmp, 16, dst_ptr, dst_stride,
                    output_width, output_height);
}

class ConvolveTest : public PARAMS(int, int, const ConvolveFunctions*) {
  protected:
    static const int kOuterBlockSize = 32;
    static const int kInputStride = kOuterBlockSize;
    static const int kOutputStride = kOuterBlockSize;
    static const int kMaxDimension = 16;

    int Width() const { return GET_PARAM(0); }
    int Height() const { return GET_PARAM(1); }
    int BorderLeft() const { return (kOuterBlockSize - Width()) / 2; }
    int BorderTop() const { return (kOuterBlockSize - Height()) / 2; }

    bool IsIndexInBorder(int i) {
      return (i < BorderTop() * kOuterBlockSize ||
              i >= (BorderTop() + Height()) * kOuterBlockSize ||
              i % kOuterBlockSize < BorderLeft() ||
              i % kOuterBlockSize >= (BorderLeft() + Width()));
    }

    virtual void SetUp() {
      UUT_ = GET_PARAM(2);
      memset(input_, 0, sizeof(input_));
      /* Set up guard blocks for an inner block cetered in the outer block */
      for (int i = 0; i < kOuterBlockSize * kOuterBlockSize; i++) {
        if (IsIndexInBorder(i))
          output_[i] = 255;
        else
          output_[i] = 0;
      }

      ::libvpx_test::ACMRandom prng;
      for (int i = 0; i < kOuterBlockSize * kOuterBlockSize; i++)
        input_[i] = prng.Rand8();
    }

    void CheckGuardBlocks() {
      for (int i = 0; i < kOuterBlockSize * kOuterBlockSize; i++) {
        if (IsIndexInBorder(i))
          EXPECT_EQ(255, output_[i]);
      }
    }

    uint8_t* input() {
      return input_ + BorderTop() * kOuterBlockSize + BorderLeft();
    }

    uint8_t* output() {
      return output_ + BorderTop() * kOuterBlockSize + BorderLeft();
    }

    const ConvolveFunctions* UUT_;
    uint8_t input_[kOuterBlockSize * kOuterBlockSize];
    uint8_t output_[kOuterBlockSize * kOuterBlockSize];
};

TEST_P(ConvolveTest, GuardBlocks) {
  CheckGuardBlocks();
}

TEST_P(ConvolveTest, CopyHoriz) {
  uint8_t* const in = input();
  uint8_t* const out = output();
  const int16_t filter8[8] = {0, 0, 0, 128, 0, 0, 0, 0};

  REGISTER_STATE_CHECK(
      UUT_->h8_(in, kInputStride, out, kOutputStride, filter8, 0, filter8, 0,
                Width(), Height()));

  CheckGuardBlocks();

  for (int y = 0; y < Height(); y++)
    for (int x = 0; x < Width(); x++)
      ASSERT_EQ(out[y * kOutputStride + x], in[y * kInputStride + x])
          << "(" << x << "," << y << ")";
}

TEST_P(ConvolveTest, CopyVert) {
  uint8_t* const in = input();
  uint8_t* const out = output();
  const int16_t filter8[8] = {0, 0, 0, 128, 0, 0, 0, 0};

  REGISTER_STATE_CHECK(
      UUT_->v8_(in, kInputStride, out, kOutputStride, filter8, 0, filter8, 0,
                Width(), Height()));

  CheckGuardBlocks();

  for (int y = 0; y < Height(); y++)
    for (int x = 0; x < Width(); x++)
      ASSERT_EQ(out[y * kOutputStride + x], in[y * kInputStride + x])
          << "(" << x << "," << y << ")";
}

TEST_P(ConvolveTest, Copy2D) {
  uint8_t* const in = input();
  uint8_t* const out = output();
  const int16_t filter8[8] = {0, 0, 0, 128, 0, 0, 0, 0};

  REGISTER_STATE_CHECK(
      UUT_->hv8_(in, kInputStride, out, kOutputStride, filter8, 0, filter8, 0,
                Width(), Height()));

  CheckGuardBlocks();

  for (int y = 0; y < Height(); y++)
    for (int x = 0; x < Width(); x++)
      ASSERT_EQ(out[y * kOutputStride + x], in[y * kInputStride + x])
          << "(" << x << "," << y << ")";
}

TEST_P(ConvolveTest, MatchesReferenceSubpixelFilter) {
  uint8_t* const in = input();
  uint8_t* const out = output();
  uint8_t ref[kOutputStride * kMaxDimension];

  const int16_t filters[][8] = {
    { 0,   0,   0, 128,   0,   0,   0,  0},
    { 0,   1,  -5, 126,   8,  -3,   1,  0},
    { -1,   3, -10, 122,  18,  -6,   2,  0},
    { -1,   4, -13, 118,  27,  -9,   3, -1},
    { -1,   4, -16, 112,  37, -11,   4, -1},
    { -1,   5, -18, 105,  48, -14,   4, -1},
    { -1,   5, -19,  97,  58, -16,   5, -1},
    { -1,   6, -19,  88,  68, -18,   5, -1},
    { -1,   6, -19,  78,  78, -19,   6, -1},
    { -1,   5, -18,  68,  88, -19,   6, -1},
    { -1,   5, -16,  58,  97, -19,   5, -1},
    { -1,   4, -14,  48, 105, -18,   5, -1},
    { -1,   4, -11,  37, 112, -16,   4, -1},
    { -1,   3,  -9,  27, 118, -13,   4, -1},
    { 0,   2,  -6,  18, 122, -10,   3, -1},
    { 0,   1,  -3,   8, 126,  -5,   1,  0}
  };

  const int kNumFilters = sizeof(filters) / sizeof(filters[0]);

  for (int filter_x = 0; filter_x < kNumFilters; filter_x++) {
    for (int filter_y = 0; filter_y < kNumFilters; filter_y++) {
      filter_block2d_8_c(in, kInputStride,
                         filters[filter_x], filters[filter_y],
                         ref, kOutputStride,
                         Width(), Height());

      if (filter_x && filter_y)
        REGISTER_STATE_CHECK(
            UUT_->hv8_(in, kInputStride, out, kOutputStride,
                       filters[filter_x], 0, filters[filter_y], 0,
                       Width(), Height()));
      else if (filter_y)
        REGISTER_STATE_CHECK(
            UUT_->v8_(in, kInputStride, out, kOutputStride,
                      filters[filter_x], 0, filters[filter_y], 0,
                      Width(), Height()));
      else
        REGISTER_STATE_CHECK(
            UUT_->h8_(in, kInputStride, out, kOutputStride,
                      filters[filter_x], 0, filters[filter_y], 0,
                      Width(), Height()));

      CheckGuardBlocks();

      for (int y = 0; y < Height(); y++)
        for (int x = 0; x < Width(); x++)
          ASSERT_EQ(ref[y * kOutputStride + x], out[y * kOutputStride + x])
              << "mismatch at (" << x << "," << y << "), "
              << "filters (" << filter_x << "," << filter_y << ")";
    }
  }
}

TEST_P(ConvolveTest, MatchesReferenceAveragingSubpixelFilter) {
  uint8_t* const in = input();
  uint8_t* const out = output();
  uint8_t ref[kOutputStride * kMaxDimension];

  // Populate ref and out with some random data
  ::libvpx_test::ACMRandom prng;
  for (int y = 0; y < Height(); y++) {
    for (int x = 0; x < Width(); x++) {
      const uint8_t r = prng.Rand8();

      out[y * kOutputStride + x] = r;
      ref[y * kOutputStride + x] = r;
    }
  }

  const int16_t filters[][8] = {
    { 0,   0,   0, 128,   0,   0,   0,  0},
    { 0,   1,  -5, 126,   8,  -3,   1,  0},
    { -1,   3, -10, 122,  18,  -6,   2,  0},
    { -1,   4, -13, 118,  27,  -9,   3, -1},
    { -1,   4, -16, 112,  37, -11,   4, -1},
    { -1,   5, -18, 105,  48, -14,   4, -1},
    { -1,   5, -19,  97,  58, -16,   5, -1},
    { -1,   6, -19,  88,  68, -18,   5, -1},
    { -1,   6, -19,  78,  78, -19,   6, -1},
    { -1,   5, -18,  68,  88, -19,   6, -1},
    { -1,   5, -16,  58,  97, -19,   5, -1},
    { -1,   4, -14,  48, 105, -18,   5, -1},
    { -1,   4, -11,  37, 112, -16,   4, -1},
    { -1,   3,  -9,  27, 118, -13,   4, -1},
    { 0,   2,  -6,  18, 122, -10,   3, -1},
    { 0,   1,  -3,   8, 126,  -5,   1,  0}
  };

  const int kNumFilters = sizeof(filters) / sizeof(filters[0]);

  for (int filter_x = 0; filter_x < kNumFilters; filter_x++) {
    for (int filter_y = 0; filter_y < kNumFilters; filter_y++) {
      filter_average_block2d_8_c(in, kInputStride,
                                 filters[filter_x], filters[filter_y],
                                 ref, kOutputStride,
                                 Width(), Height());

      if (filter_x && filter_y)
        REGISTER_STATE_CHECK(
            UUT_->hv8_avg_(in, kInputStride, out, kOutputStride,
                           filters[filter_x], 0, filters[filter_y], 0,
                           Width(), Height()));
      else if (filter_y)
        REGISTER_STATE_CHECK(
            UUT_->v8_avg_(in, kInputStride, out, kOutputStride,
                          filters[filter_x], 0, filters[filter_y], 0,
                          Width(), Height()));
      else
        REGISTER_STATE_CHECK(
            UUT_->h8_avg_(in, kInputStride, out, kOutputStride,
                          filters[filter_x], 0, filters[filter_y], 0,
                          Width(), Height()));

      CheckGuardBlocks();

      for (int y = 0; y < Height(); y++)
        for (int x = 0; x < Width(); x++)
          ASSERT_EQ(ref[y * kOutputStride + x], out[y * kOutputStride + x])
              << "mismatch at (" << x << "," << y << "), "
              << "filters (" << filter_x << "," << filter_y << ")";
    }
  }
}

TEST_P(ConvolveTest, ChangeFilterWorks) {
  uint8_t* const in = input();
  uint8_t* const out = output();

  const int16_t filters[][8] = {
    { 0,   0,   0,   0,   0,   0,   0, 128},
    { 0,   0,   0,   0,   0,   0, 128},
    { 0,   0,   0,   0,   0, 128},
    { 0,   0,   0,   0, 128},
    { 0,   0,   0, 128},
    { 0,   0, 128},
    { 0, 128},
    { 128},
    { 0,   0,   0,   0,   0,   0,   0, 128},
    { 0,   0,   0,   0,   0,   0, 128},
    { 0,   0,   0,   0,   0, 128},
    { 0,   0,   0,   0, 128},
    { 0,   0,   0, 128},
    { 0,   0, 128},
    { 0, 128},
    { 128},
    { 0,   0,   0,   0,   0,   0,   0, 128},
    { 0,   0,   0,   0,   0,   0, 128},
    { 0,   0,   0,   0,   0, 128},
    { 0,   0,   0,   0, 128},
    { 0,   0,   0, 128},
    { 0,   0, 128},
    { 0, 128},
    { 128},
  };

  REGISTER_STATE_CHECK(UUT_->h8_(in, kInputStride, out, kOutputStride,
                                 filters[0], 8, filters[4], 0,
                                 Width(), Height()));

  for (int x = 0; x < (Width() > 4 ? 8 : 4); x++) {
    ASSERT_EQ(in[4], out[x]) << "x == " << x;
  }

  REGISTER_STATE_CHECK(UUT_->v8_(in, kInputStride, out, kOutputStride,
                                 filters[4], 0, filters[0], 8,
                                 Width(), Height()));

  for (int y = 0; y < (Height() > 4 ? 8 : 4); y++) {
    ASSERT_EQ(in[4 * kInputStride], out[y * kOutputStride]) << "y == " << y;
  }

  REGISTER_STATE_CHECK(UUT_->hv8_(in, kInputStride, out, kOutputStride,
                                  filters[0], 8, filters[0], 8,
                                  Width(), Height()));

  for (int y = 0; y < (Height() > 4 ? 8 : 4); y++) {
    for (int x = 0; x < (Width() > 4 ? 8 : 4); x++) {
      ASSERT_EQ(in[4 * kInputStride + 4], out[y * kOutputStride + x])
          << "x == " << x << ", y == " << y;
    }
  }
}


using std::tr1::make_tuple;

const ConvolveFunctions convolve8_2d_only_c(
    vp9_convolve8_c, vp9_convolve8_avg_c,
    vp9_convolve8_c, vp9_convolve8_avg_c,
    vp9_convolve8_c, vp9_convolve8_avg_c);

const ConvolveFunctions convolve8_c(
    vp9_convolve8_horiz_c, vp9_convolve8_avg_horiz_c,
    vp9_convolve8_vert_c, vp9_convolve8_avg_vert_c,
    vp9_convolve8_c, vp9_convolve8_avg_c);

INSTANTIATE_TEST_CASE_P(C, ConvolveTest, ::testing::Values(
    make_tuple(4, 4, &convolve8_2d_only_c),
    make_tuple(8, 4, &convolve8_2d_only_c),
    make_tuple(8, 8, &convolve8_2d_only_c),
    make_tuple(16, 16, &convolve8_2d_only_c),
    make_tuple(4, 4, &convolve8_c),
    make_tuple(8, 4, &convolve8_c),
    make_tuple(8, 8, &convolve8_c),
    make_tuple(16, 16, &convolve8_c)));
}
