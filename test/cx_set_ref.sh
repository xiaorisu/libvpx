#!/bin/sh
##
##  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
##
##  Use of this source code is governed by a BSD-style license
##  that can be found in the LICENSE file in the root of the source
##  tree. An additional intellectual property rights grant can be found
##  in the file PATENTS.  All contributing project authors may
##  be found in the AUTHORS file in the root of the source tree.
##
##  This file tests the libaom cx_set_ref example. To add new tests to this
##  file, do the following:
##    1. Write a shell function (this is your test).
##    2. Add the function to cx_set_ref_tests (on a new line).
##
. $(dirname $0)/tools_common.sh

# Environment check: $YUV_RAW_INPUT is required.
cx_set_ref_verify_environment() {
  if [ ! -e "${YUV_RAW_INPUT}" ]; then
    echo "Libaom test data must exist in LIBVPX_TEST_DATA_PATH."
    return 1
  fi
}

# Runs cx_set_ref and updates the reference frame before encoding frame 90.
# $1 is the codec name.
aom_set_ref() {
  local codec="$1"
  local encoder="${LIBAOM_BIN_PATH}/aomcx_set_ref${AOM_TEST_EXE_SUFFIX}"

  local output_file="${AOM_TEST_OUTPUT_DIR}/${codec}cx_set_ref_${codec}.ivf"
  local ref_frame_num=90

  if [ ! -x "${encoder}" ]; then
    elog "${encoder} does not exist or is not executable."
    return 1
  fi

  if [ "$codec" = "vp8" ]; then
    eval "${AOM_TEST_PREFIX}" "${encoder}" "${YUV_RAW_INPUT_WIDTH}" \
        "${YUV_RAW_INPUT_HEIGHT}" "${YUV_RAW_INPUT}" "${output_file}" \
        "${ref_frame_num}" ${devnull}
  else
    eval "${AOM_TEST_PREFIX}" "${encoder}" "${codec}" "${YUV_RAW_INPUT_WIDTH}" \
        "${YUV_RAW_INPUT_HEIGHT}" "${YUV_RAW_INPUT}" "${output_file}" \
        "${ref_frame_num}" ${devnull}
  fi

  [ -e "${output_file}" ] || return 1
}

cx_set_ref_av1() {
  if [ "$(av1_encode_available)" = "yes" ]; then
    aom_set_ref av1 || return 1
  fi
}

cx_set_ref_tests="cx_set_ref_av1"

run_tests cx_set_ref_verify_environment "${cx_set_ref_tests}"
