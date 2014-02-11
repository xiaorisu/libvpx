/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


// VP8 Set Active and ROI Maps
// ===========================
//
// This is an example demonstrating how to control the VP8 encoder's
// ROI and Active maps.
//
// ROI (Reigon of Interest) maps are a way for the application to assign
// each macroblock in the image to a region, and then set quantizer and
// filtering parameters on that image.
//
// Active maps are a way for the application to specify on a
// macroblock-by-macroblock basis whether there is any activity in that
// macroblock.
//
//
// Configuration
// -------------
// An ROI map is set on frame 22. If the width of the image in macroblocks
// is evenly divisble by 4, then the output will appear to have distinct
// columns, where the quantizer, loopfilter, and static threshold differ
// from column to column.
//
// An active map is set on frame 33. If the width of the image in macroblocks
// is evenly divisble by 4, then the output will appear to have distinct
// columns, where one column will have motion and the next will not.
//
// The active map is cleared on frame 44.
//
// Observing The Effects
// ---------------------
// Use the `simple_decoder` example to decode this sample, and observe
// the change in the image at frames 22, 33, and 44.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VPX_CODEC_DISABLE_COMPAT 1
#include "vpx/vp8cx.h"
#include "vpx/vpx_encoder.h"

#include "./tools_common.h"
#include "./video_writer.h"

#define interface (vpx_codec_vp8_cx())

static const char *exec_name;

void usage_exit() {
  fprintf(stderr, "Usage: %s <width> <height> <infile> <outfile>\n", exec_name);
  exit(EXIT_FAILURE);
}

static void set_roi_map(const vpx_codec_enc_cfg_t *cfg,
                        vpx_codec_ctx_t *codec) {
  int i;
  vpx_roi_map_t roi;

  roi.rows = cfg->g_h / 16;
  roi.cols = cfg->g_w / 16;

  roi.delta_q[0] = 0;
  roi.delta_q[1] = -2;
  roi.delta_q[2] = -4;
  roi.delta_q[3] = -6;

  roi.delta_lf[0] = 0;
  roi.delta_lf[1] = 1;
  roi.delta_lf[2] = 2;
  roi.delta_lf[3] = 3;

  roi.static_threshold[0] = 1500;
  roi.static_threshold[1] = 1000;
  roi.static_threshold[2] = 500;
  roi.static_threshold[3] = 0;

  roi.roi_map = (unsigned char *)malloc(roi.rows * roi.cols);
  for (i = 0; i < roi.rows * roi.cols; ++i)
    roi.roi_map[i] = i & 3;

  if (vpx_codec_control(&codec, VP8E_SET_ROI_MAP, &roi))
    die_codec(&codec, "Failed to set ROI map");

  free(roi.roi_map);
}

static void set_active_map(const vpx_codec_enc_cfg_t *cfg,
                           vpx_codec_ctx_t *codec) {
  int i;
  vpx_active_map_t  map;

  map.rows = cfg->g_h / 16;
  map.cols = cfg->g_w / 16;

  map.active_map = (unsigned char *)malloc(map.rows * map.cols);
  for (i = 0; i < map.rows * map.cols; ++i)
    map.active_map[i] = i & 1;

  if (vpx_codec_control(&codec, VP8E_SET_ACTIVEMAP, &map))
    die_codec(&codec, "Failed to set active map");

  free(map.active_map);
}

static void unset_active_map(const vpx_codec_enc_cfg_t *cfg,
                             vpx_codec_ctx_t *codec) {
  vpx_active_map_t map;

  map.rows = cfg->g_h / 16;
  map.cols = cfg->g_w / 16;
  map.active_map = NULL;

  if (vpx_codec_control(&codec, VP8E_SET_ACTIVEMAP, &map))
    die_codec(&codec, "Failed to set active map");
}


int main(int argc, char **argv) {
  FILE *infile = NULL;
  vpx_codec_ctx_t codec;
  vpx_codec_enc_cfg_t cfg;
  int frame_count = 0;
  vpx_image_t raw;
  vpx_codec_err_t res;
  VpxVideoInfo info = {0};
  VpxVideoWriter *writer = NULL;
  const int fps = 30;        // TODO(dkovalev) add command line argument
  const int bitrate = 200;   // kbit/s TODO(dkovalev) add command line argument

  exec_name = argv[0];

  if (argc != 5)
    die("Invalid number of arguments");

  info.codec_fourcc = VP8_FOURCC;
  info.frame_width = strtol(argv[1], NULL, 0);
  info.frame_height = strtol(argv[2], NULL, 0);
  info.time_base.numerator = 1;
  info.time_base.denominator = fps;

  if (info.frame_width <= 0 ||
      info.frame_height <= 0 ||
      (info.frame_width % 2) != 0 ||
      (info.frame_height % 2) != 0) {
    die("Invalid frame size: %dx%d", info.frame_width, info.frame_height);
  }

  if (!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, info.frame_width,
                                             info.frame_height, 1)) {
    die("Failed to allocate image.");
  }

  printf("Using %s\n", vpx_codec_iface_name(interface));

  res = vpx_codec_enc_config_default(interface, &cfg, 0);
  if (res)
    die_codec(&codec, "Failed to get default codec config.");

  cfg.g_w = info.frame_width;
  cfg.g_h = info.frame_height;
  cfg.g_timebase.num = info.time_base.numerator;
  cfg.g_timebase.den = info.time_base.denominator;
  cfg.rc_target_bitrate = bitrate;

  writer = vpx_video_writer_open(argv[4], kContainerIVF, &info);
  if (!writer)
    die("Failed to open %s for writing.", argv[4]);

  if (!(infile = fopen(argv[3], "rb")))
    die("Failed to open %s for reading.", argv[3]);

  if (vpx_codec_enc_init(&codec, interface, &cfg, 0))
    die_codec(&codec, "Failed to initialize encoder");

  while (vpx_img_read(&raw, infile)) {
    vpx_codec_iter_t iter = NULL;
    const vpx_codec_cx_pkt_t *pkt = NULL;

    ++frame_count;

    if (frame_count == 22) {
      set_roi_map(&cfg, &codec);
    } else if (frame_count == 33) {
      set_active_map(&cfg, &codec);
    } else if (frame_count == 44) {
      unset_active_map(&cfg, &codec);
    }

    res = vpx_codec_encode(&codec, &raw, frame_count, 1, 0,
                           VPX_DL_GOOD_QUALITY);
    if (res != VPX_CODEC_OK)
      die_codec(&codec, "Failed to encode frame");

    while ((pkt = vpx_codec_get_cx_data(&codec, &iter)) != NULL) {
      if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
        const int keyframe = (pkt->data.frame.flags & VPX_FRAME_IS_KEY) != 0;
        if (!vpx_video_writer_write_frame(writer,
                                          pkt->data.frame.buf,
                                          pkt->data.frame.sz,
                                          pkt->data.frame.pts))
          die_codec(&codec, "Failed to write compressed frame.");
        printf(keyframe ? "K" : ".");
        fflush(stdout);
      }
    }
  }
  printf("\n");
  fclose(infile);
  printf("Processed %d frames.\n", frame_count);

  vpx_img_free(&raw);
  if (vpx_codec_destroy(&codec))
    die_codec(&codec, "Failed to destroy codec.");

  vpx_video_writer_close(writer);

  return EXIT_SUCCESS;
}
















