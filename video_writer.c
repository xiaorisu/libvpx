/*
 *  Copyright (c) 2014 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>

#include "./ivfenc.h"
#include "./video_writer.h"
#include "aom/aom_encoder.h"

struct AvxVideoWriterStruct {
  AvxVideoInfo info;
  FILE *file;
  int frame_count;
};

static void write_header(FILE *file, const AvxVideoInfo *info,
                         int frame_count) {
  struct aom_codec_enc_cfg cfg;
  cfg.g_w = info->frame_width;
  cfg.g_h = info->frame_height;
  cfg.g_timebase.num = info->time_base.numerator;
  cfg.g_timebase.den = info->time_base.denominator;

  ivf_write_file_header(file, &cfg, info->codec_fourcc, frame_count);
}

AvxVideoWriter *aom_video_writer_open(const char *filename,
                                      AvxContainer container,
                                      const AvxVideoInfo *info) {
  if (container == kContainerIVF) {
    AvxVideoWriter *writer = NULL;
    FILE *const file = fopen(filename, "wb");
    if (!file) return NULL;

    writer = malloc(sizeof(*writer));
    if (!writer) return NULL;

    writer->frame_count = 0;
    writer->info = *info;
    writer->file = file;

    write_header(writer->file, info, 0);

    return writer;
  }

  return NULL;
}

void aom_video_writer_close(AvxVideoWriter *writer) {
  if (writer) {
    // Rewriting frame header with real frame count
    rewind(writer->file);
    write_header(writer->file, &writer->info, writer->frame_count);

    fclose(writer->file);
    free(writer);
  }
}

int aom_video_writer_write_frame(AvxVideoWriter *writer, const uint8_t *buffer,
                                 size_t size, int64_t pts) {
  ivf_write_frame_header(writer->file, pts, size);
  if (fwrite(buffer, 1, size, writer->file) != size) return 0;

  ++writer->frame_count;

  return 1;
}
