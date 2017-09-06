/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "./vp8_rtcd.h"
#include "vpx_ports/asmdefs_mmi.h"

#define COPY_MEM_16X2 \
  "gsldlc1    %[ftmp0],   0x07(%[src])                    \n\t" \
  "gsldrc1    %[ftmp0],   0x00(%[src])                    \n\t" \
  "ldl        %[tmp0],    0x0f(%[src])                    \n\t" \
  "ldr        %[tmp0],    0x08(%[src])                    \n\t" \
  MMI_ADDU(%[src],     %[src],         %[src_stride])           \
  "gssdlc1    %[ftmp0],   0x07(%[dst])                    \n\t" \
  "gssdrc1    %[ftmp0],   0x00(%[dst])                    \n\t" \
  "sdl        %[tmp0],    0x0f(%[dst])                    \n\t" \
  "sdr        %[tmp0],    0x08(%[dst])                    \n\t" \
  MMI_ADDU(%[dst],      %[dst],        %[dst_stride])           \
  "gsldlc1    %[ftmp1],   0x07(%[src])                    \n\t" \
  "gsldrc1    %[ftmp1],   0x00(%[src])                    \n\t" \
  "ldl        %[tmp1],    0x0f(%[src])                    \n\t" \
  "ldr        %[tmp1],    0x08(%[src])                    \n\t" \
  MMI_ADDU(%[src],     %[src],         %[src_stride])           \
  "gssdlc1    %[ftmp1],   0x07(%[dst])                    \n\t" \
  "gssdrc1    %[ftmp1],   0x00(%[dst])                    \n\t" \
  "sdl        %[tmp1],    0x0f(%[dst])                    \n\t" \
  "sdr        %[tmp1],    0x08(%[dst])                    \n\t" \
  MMI_ADDU(%[dst],     %[dst],         %[dst_stride])

#define COPY_MEM_8X2 \
  "gsldlc1    %[ftmp0],   0x07(%[src])                    \n\t" \
  "gsldrc1    %[ftmp0],   0x00(%[src])                    \n\t" \
  MMI_ADDU(%[src],     %[src],         %[src_stride])           \
  "ldl        %[tmp0],    0x07(%[src])                    \n\t" \
  "ldr        %[tmp0],    0x00(%[src])                    \n\t" \
  MMI_ADDU(%[src],     %[src],         %[src_stride])           \
                                                                \
  "gssdlc1    %[ftmp0],   0x07(%[dst])                    \n\t" \
  "gssdrc1    %[ftmp0],   0x00(%[dst])                    \n\t" \
  MMI_ADDU(%[dst],      %[dst],        %[dst_stride])           \
  "sdl        %[tmp0],    0x07(%[dst])                    \n\t" \
  "sdr        %[tmp0],    0x00(%[dst])                    \n\t" \
  MMI_ADDU(%[dst],     %[dst],         %[dst_stride])

void vp8_copy_mem16x16_mmi(unsigned char *src, int src_stride,
                           unsigned char *dst, int dst_stride) {
  double ftmp[2];
  uint64_t tmp[3];

  __asm__ volatile (
    MMI_LI(%[tmp2], 0x04)
    "1:                                                     \n\t"
    COPY_MEM_16X2
    COPY_MEM_16X2
    MMI_ADDIU(%[tmp2], %[tmp2], -0x01)
    "bnez       %[tmp2],    1b                              \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [tmp2]"=&r"(tmp[2]),
      [dst]"+&r"(dst),                  [src]"+&r"(src)
    : [src_stride]"r"((mips_reg)src_stride),
      [dst_stride]"r"((mips_reg)dst_stride)
    : "memory"
  );
}

void vp8_copy_mem8x8_mmi(unsigned char *src, int src_stride, unsigned char *dst,
                         int dst_stride) {
  double ftmp[2];
  uint64_t tmp[2];

  __asm__ volatile (
    MMI_LI(%[tmp1], 0x04)
    "1:                                                     \n\t"
    COPY_MEM_8X2
    MMI_ADDIU(%[tmp1], %[tmp1], -0x01)
    "bnez       %[tmp1],    1b                              \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [dst]"+&r"(dst),                  [src]"+&r"(src)
    : [src_stride]"r"((mips_reg)src_stride),
      [dst_stride]"r"((mips_reg)dst_stride)
    : "memory"
  );
}

void vp8_copy_mem8x4_mmi(unsigned char *src, int src_stride, unsigned char *dst,
                         int dst_stride) {
  double ftmp[2];
  uint64_t tmp[2];

  __asm__ volatile (
    MMI_LI(%[tmp1], 0x02)
    "1:                                                     \n\t"
    COPY_MEM_8X2
    MMI_ADDIU(%[tmp1], %[tmp1], -0x01)
    "bnez       %[tmp1],    1b                              \n\t"
    : [ftmp0]"=&f"(ftmp[0]),            [ftmp1]"=&f"(ftmp[1]),
      [tmp0]"=&r"(tmp[0]),              [tmp1]"=&r"(tmp[1]),
      [dst]"+&r"(dst),                  [src]"+&r"(src)
    : [src_stride]"r"((mips_reg)src_stride),
      [dst_stride]"r"((mips_reg)dst_stride)
    : "memory"
  );
}
