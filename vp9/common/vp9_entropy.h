/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_COMMON_VP9_ENTROPY_H_
#define VP9_COMMON_VP9_ENTROPY_H_

#include "vpx/vpx_integer.h"
#include "vp9/common/vp9_treecoder.h"
#include "vp9/common/vp9_blockd.h"
#include "vp9/common/vp9_common.h"
#include "vp9/common/vp9_coefupdateprobs.h"

extern const int vp9_i8x8_block[4];

/* Coefficient token alphabet */

#define ZERO_TOKEN              0       /* 0         Extra Bits 0+0 */
#define ONE_TOKEN               1       /* 1         Extra Bits 0+1 */
#define TWO_TOKEN               2       /* 2         Extra Bits 0+1 */
#define THREE_TOKEN             3       /* 3         Extra Bits 0+1 */
#define FOUR_TOKEN              4       /* 4         Extra Bits 0+1 */
#define DCT_VAL_CATEGORY1       5       /* 5-6       Extra Bits 1+1 */
#define DCT_VAL_CATEGORY2       6       /* 7-10      Extra Bits 2+1 */
#define DCT_VAL_CATEGORY3       7       /* 11-18     Extra Bits 3+1 */
#define DCT_VAL_CATEGORY4       8       /* 19-34     Extra Bits 4+1 */
#define DCT_VAL_CATEGORY5       9       /* 35-66     Extra Bits 5+1 */
#define DCT_VAL_CATEGORY6       10      /* 67+       Extra Bits 13+1 */
#define DCT_EOB_TOKEN           11      /* EOB       Extra Bits 0+0 */
#define MAX_ENTROPY_TOKENS 12
#define ENTROPY_NODES 11
#define EOSB_TOKEN              127     /* Not signalled, encoder only */

#define INTER_MODE_CONTEXTS     7

extern const vp9_tree_index vp9_coef_tree[];

extern struct vp9_token_struct vp9_coef_encodings[MAX_ENTROPY_TOKENS];

typedef struct {
  vp9_tree_p tree;
  const vp9_prob *prob;
  int Len;
  int base_val;
} vp9_extra_bit_struct;

extern vp9_extra_bit_struct vp9_extra_bits[12];    /* indexed by token value */

#define PROB_UPDATE_BASELINE_COST   7

#define MAX_PROB                255
#define DCT_MAX_VALUE           16384

/* Coefficients are predicted via a 3-dimensional probability table. */

/* Outside dimension.  0 = Y with DC, 1 = UV */
#define BLOCK_TYPES_4X4 2
#define BLOCK_TYPES_4X4_HYBRID 1

#define BLOCK_TYPES_8X8 2
#define BLOCK_TYPES_8X8_HYBRID 1

#define BLOCK_TYPES_16X16 2
#define BLOCK_TYPES_16X16_HYBRID 1

#define BLOCK_TYPES_32X32 1

/* Middle dimension is a coarsening of the coefficient's
   position within the 4x4 DCT. */

#define COEF_BANDS 8

/* Inside dimension is 3-valued measure of nearby complexity, that is,
   the extent to which nearby coefficients are nonzero.  For the first
   coefficient (DC, unless block type is 0), we look at the (already encoded)
   blocks above and to the left of the current block.  The context index is
   then the number (0,1,or 2) of these blocks having nonzero coefficients.
   After decoding a coefficient, the measure is roughly the size of the
   most recently decoded coefficient (0 for 0, 1 for 1, 2 for >1).
   Note that the intuitive meaning of this measure changes as coefficients
   are decoded, e.g., prior to the first token, a zero means that my neighbors
   are empty while, after the first token, because of the use of end-of-block,
   a zero means we just decoded a zero and hence guarantees that a non-zero
   coefficient will appear later in this block.  However, this shift
   in meaning is perfectly OK because our context depends also on the
   coefficient band (and since zigzag positions 0, 1, and 2 are in
   distinct bands). */

/*# define DC_TOKEN_CONTEXTS        3*/ /* 00, 0!0, !0!0 */
#define PREV_COEF_CONTEXTS          4

typedef unsigned int vp9_coeff_count[COEF_BANDS][PREV_COEF_CONTEXTS]
                                    [MAX_ENTROPY_TOKENS];
typedef unsigned int vp9_coeff_stats[COEF_BANDS][PREV_COEF_CONTEXTS]
                                    [ENTROPY_NODES][2];
typedef vp9_prob vp9_coeff_probs[COEF_BANDS][PREV_COEF_CONTEXTS]
                                [ENTROPY_NODES];

#define SUBEXP_PARAM                4   /* Subexponential code parameter */
#define MODULUS_PARAM               13  /* Modulus parameter */

struct VP9Common;
void vp9_default_coef_probs(struct VP9Common *);
extern DECLARE_ALIGNED(16, const int, vp9_default_zig_zag1d_4x4[16]);

extern DECLARE_ALIGNED(16, const int, vp9_col_scan_4x4[16]);
extern DECLARE_ALIGNED(16, const int, vp9_row_scan_4x4[16]);

extern DECLARE_ALIGNED(64, const int, vp9_default_zig_zag1d_8x8[64]);
extern DECLARE_ALIGNED(16, const int, vp9_default_zig_zag1d_16x16[256]);
extern DECLARE_ALIGNED(16, const int, vp9_default_zig_zag1d_32x32[1024]);

void vp9_coef_tree_initialize(void);
void vp9_adapt_coef_probs(struct VP9Common *);

static void vp9_reset_mb_tokens_context(MACROBLOCKD* const xd) {
  /* Clear entropy contexts */
  vpx_memset(xd->above_context, 0, sizeof(ENTROPY_CONTEXT_PLANES));
  vpx_memset(xd->left_context, 0, sizeof(ENTROPY_CONTEXT_PLANES));
}

extern const int vp9_coef_bands[32];
static int get_coef_band(int coef_index) {
  if (coef_index < 32)
    return vp9_coef_bands[coef_index];
  else
    return 7;
}
extern int vp9_get_coef_context(int * recent_energy, int token);

#if CONFIG_CODE_NONZEROCOUNT
/* Alphabet for number of non-zero symbols in block */
#define NZC_0                   0       /* Used for all blocks */
#define NZC_1                   1       /* Used for all blocks */
#define NZC_2                   2       /* Used for all blocks */
#define NZC_3TO4                3       /* Used for all blocks */
#define NZC_5TO8                4       /* Used for all blocks */
#define NZC_9TO16               5       /* Used for all blocks */
#define NZC_17TO32              6       /* Used for 8x8 and larger blocks */
#define NZC_33TO64              7       /* Used for 8x8 and larger blocks */
#define NZC_65TO128             8       /* Used for 16x16 and larger blocks */
#define NZC_129TO256            9       /* Used for 16x16 and larger blocks */
#define NZC_257TO512           10       /* Used for 32x32 and larger blocks */
#define NZC_513TO1024          11       /* Used for 32x32 and larger blocks */

/* Number of tokens for each block size */
#define NZC4X4_TOKENS           6
#define NZC8X8_TOKENS           8
#define NZC16X16_TOKENS        10
#define NZC32X32_TOKENS        12

/* Number of nodes for each block size */
#define NZC4X4_NODES            5
#define NZC8X8_NODES            7
#define NZC16X16_NODES          9
#define NZC32X32_NODES         11

/* Max number of tokens with extra bits */
#define NZC_TOKENS_EXTRA        9

/* Max number of extra bits */
#define NZC_BITS_EXTRA          9

#define MAX_NZC_CONTEXTS        3

/* nzc trees */
extern const vp9_tree_index    vp9_nzc4x4_tree[];
extern const vp9_tree_index    vp9_nzc8x8_tree[];
extern const vp9_tree_index    vp9_nzc16x16_tree[];
extern const vp9_tree_index    vp9_nzc32x32_tree[];

/* nzc encodings */
extern struct vp9_token_struct  vp9_nzc4x4_encodings[NZC4X4_TOKENS];
extern struct vp9_token_struct  vp9_nzc8x8_encodings[NZC8X8_TOKENS];
extern struct vp9_token_struct  vp9_nzc16x16_encodings[NZC16X16_TOKENS];
extern struct vp9_token_struct  vp9_nzc32x32_encodings[NZC32X32_TOKENS];

#define codenzc(x) (\
  (x) <= 3 ? (x) : (x) <= 4 ? 3 : (x) <= 8 ? 4 : \
  (x) <= 16 ? 5 : (x) <= 32 ? 6 : (x) <= 64 ? 7 :\
  (x) <= 128 ? 8 : (x) <= 256 ? 9 : (x) <= 512 ? 10 : 11)
#define extranzcbits(c) ((c) <= 2 ? 0 : (c) - 2)
#define basenzcvalue(c) ((c) <= 2 ? (c) : (1 << ((c) - 2)) + 1)

int vp9_get_nzc_context(MB_MODE_INFO *cur, TX_SIZE txfm_size, int block,
                        MB_MODE_INFO *left, MB_MODE_INFO *above);

/* Extra bit probabilities - block size agnostic */
extern const vp9_prob Pcat_nzc[MAX_NZC_CONTEXTS][NZC_TOKENS_EXTRA]
                              [NZC_BITS_EXTRA];

#endif  // CONFIG_CODE_NONZEROCOUNT
#endif  // VP9_COMMON_VP9_ENTROPY_H_
