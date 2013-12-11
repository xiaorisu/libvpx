/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_EXTERNAL_FRAME_BUFFER_H_
#define VPX_EXTERNAL_FRAME_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vpx/vpx_integer.h"

/*!\brief Stream properties
 *
 * This structure is used to hold external frame buffers passed into the
 * decoder by the application.
 */
typedef struct vpx_codec_frame_buffer {
  uint8_t *data;    /**< Pointer to the data buffer */
  size_t size;      /**< Size of data in bytes */
} vpx_codec_frame_buffer_t;

/*!\brief realloc frame buffer callback prototype
 *
 * This callback is invoked by the decoder to notify the application one
 * of the external frame buffers must increase the size, in order for the
 * decode call to complete. The callback must allocate at least new_size in
 * bytes and assign it to fb->data. Then the callback must set fb->size to
 * the allocated size. The application does not need to align the allocated
 * data. The callback is usually triggered by a frame size change. On success
 * the callback must return 0 (VPX_CODEC_OK). Any failure the callback must
 * return a value less than 0.
 *
 * \param[in] user_priv    User's private data
 * \param[in] new_size     Size in bytes needed by the buffer.
 * \param[in/out] fb       Pointer to frame buffer to increase size.
 */
typedef int32_t (*vpx_realloc_frame_buffer_cb_fn_t)(
    void *user_priv,
    size_t new_size,
    vpx_codec_frame_buffer_t *fb);

#ifdef __cplusplus
}
#endif

#endif  // VPX_EXTERNAL_FRAME_BUFFER_H_
