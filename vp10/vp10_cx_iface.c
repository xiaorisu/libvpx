/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>
#include <string.h>

#include "./vpx_config.h"
#include "vpx/vpx_encoder.h"
#include "vpx_ports/vpx_once.h"
#include "vpx_ports/system_state.h"
#include "vpx/internal/vpx_codec_internal.h"
#include "./vpx_version.h"
#include "vp10/encoder/encoder.h"
#include "vpx/vp8cx.h"
#include "vp10/encoder/firstpass.h"
#include "vp10/vp10_iface_common.h"

struct vp10_extracfg {
  int                         cpu_used;  // available cpu percentage in 1/16
  unsigned int                enable_auto_alt_ref;
  unsigned int                noise_sensitivity;
  unsigned int                sharpness;
  unsigned int                static_thresh;
  unsigned int                tile_columns;
  unsigned int                tile_rows;
  unsigned int                arnr_max_frames;
  unsigned int                arnr_strength;
  unsigned int                min_gf_interval;
  unsigned int                max_gf_interval;
  vp8e_tuning                 tuning;
  unsigned int                cq_level;  // constrained quality level
  unsigned int                rc_max_intra_bitrate_pct;
  unsigned int                rc_max_inter_bitrate_pct;
  unsigned int                gf_cbr_boost_pct;
  unsigned int                lossless;
  unsigned int                frame_parallel_decoding_mode;
  AQ_MODE                     aq_mode;
  unsigned int                frame_periodic_boost;
  vpx_bit_depth_t             bit_depth;
  vp9e_tune_content           content;
  vpx_color_space_t           color_space;
  int                         color_range;
  int                         render_width;
  int                         render_height;
  vpx_superblock_size_t       superblock_size;
};

static struct vp10_extracfg default_extra_cfg = {
  0,                            // cpu_used
  1,                            // enable_auto_alt_ref
  0,                            // noise_sensitivity
  0,                            // sharpness
  0,                            // static_thresh
#if CONFIG_EXT_TILE
  UINT_MAX,                     // tile_columns
  UINT_MAX,                     // tile_rows
#else
  0,                            // tile_columns
  0,                            // tile_rows
#endif  // CONFIG_EXT_TILE
  7,                            // arnr_max_frames
  5,                            // arnr_strength
  0,                            // min_gf_interval; 0 -> default decision
  0,                            // max_gf_interval; 0 -> default decision
  VP8_TUNE_PSNR,                // tuning
  10,                           // cq_level
  0,                            // rc_max_intra_bitrate_pct
  0,                            // rc_max_inter_bitrate_pct
  0,                            // gf_cbr_boost_pct
  0,                            // lossless
  1,                            // frame_parallel_decoding_mode
  NO_AQ,                        // aq_mode
  0,                            // frame_periodic_delta_q
  VPX_BITS_8,                   // Bit depth
  VP9E_CONTENT_DEFAULT,         // content
  VPX_CS_UNKNOWN,               // color space
  0,                            // color range
  0,                            // render width
  0,                            // render height
  VPX_SUPERBLOCK_SIZE_DYNAMIC   // superblock_size
};

struct vpx_codec_alg_priv {
  vpx_codec_priv_t        base;
  vpx_codec_enc_cfg_t     cfg;
  struct vp10_extracfg    extra_cfg;
  VP10EncoderConfig       oxcf;
  VP10_COMP               *cpi;
  unsigned char           *cx_data;
  size_t                  cx_data_sz;
  unsigned char           *pending_cx_data;
  size_t                  pending_cx_data_sz;
  int                     pending_frame_count;
  size_t                  pending_frame_sizes[8];
  vpx_image_t             preview_img;
  vpx_enc_frame_flags_t   next_frame_flags;
  vp8_postproc_cfg_t      preview_ppcfg;
  vpx_codec_pkt_list_decl(256) pkt_list;
  unsigned int            fixed_kf_cntr;
  vpx_codec_priv_output_cx_pkt_cb_pair_t output_cx_pkt_cb;
  // BufferPool that holds all reference frames.
  BufferPool              *buffer_pool;
};

static vpx_codec_err_t update_error_state(vpx_codec_alg_priv_t *ctx,
    const struct vpx_internal_error_info *error) {
  const vpx_codec_err_t res = error->error_code;

  if (res != VPX_CODEC_OK)
    ctx->base.err_detail = error->has_detail ? error->detail : NULL;

  return res;
}


#undef ERROR
#define ERROR(str) do {\
    ctx->base.err_detail = str;\
    return VPX_CODEC_INVALID_PARAM;\
  } while (0)

#define RANGE_CHECK(p, memb, lo, hi) do {\
    if (!(((p)->memb == lo || (p)->memb > (lo)) && (p)->memb <= hi)) \
      ERROR(#memb " out of range ["#lo".."#hi"]");\
  } while (0)

#define RANGE_CHECK_HI(p, memb, hi) do {\
    if (!((p)->memb <= (hi))) \
      ERROR(#memb " out of range [.."#hi"]");\
  } while (0)

#define RANGE_CHECK_LO(p, memb, lo) do {\
    if (!((p)->memb >= (lo))) \
      ERROR(#memb " out of range ["#lo"..]");\
  } while (0)

#define RANGE_CHECK_BOOL(p, memb) do {\
    if (!!((p)->memb) != (p)->memb) ERROR(#memb " expected boolean");\
  } while (0)

static vpx_codec_err_t validate_config(vpx_codec_alg_priv_t *ctx,
                                       const vpx_codec_enc_cfg_t *cfg,
                                       const struct vp10_extracfg *extra_cfg) {
  RANGE_CHECK(cfg, g_w,                   1, 65535);  // 16 bits available
  RANGE_CHECK(cfg, g_h,                   1, 65535);  // 16 bits available
  RANGE_CHECK(cfg, g_timebase.den,        1, 1000000000);
  RANGE_CHECK(cfg, g_timebase.num,        1, cfg->g_timebase.den);
  RANGE_CHECK_HI(cfg, g_profile,          3);

  RANGE_CHECK_HI(cfg, rc_max_quantizer,   63);
  RANGE_CHECK_HI(cfg, rc_min_quantizer,   cfg->rc_max_quantizer);
  RANGE_CHECK_BOOL(extra_cfg, lossless);
  RANGE_CHECK(extra_cfg, aq_mode,           0, AQ_MODE_COUNT - 1);
  RANGE_CHECK(extra_cfg, frame_periodic_boost, 0, 1);
  RANGE_CHECK_HI(cfg, g_threads,          64);
  RANGE_CHECK_HI(cfg, g_lag_in_frames,    MAX_LAG_BUFFERS);
  RANGE_CHECK(cfg, rc_end_usage,          VPX_VBR, VPX_Q);
  RANGE_CHECK_HI(cfg, rc_undershoot_pct,  100);
  RANGE_CHECK_HI(cfg, rc_overshoot_pct,   100);
  RANGE_CHECK_HI(cfg, rc_2pass_vbr_bias_pct, 100);
  RANGE_CHECK(cfg, kf_mode,               VPX_KF_DISABLED, VPX_KF_AUTO);
  RANGE_CHECK_BOOL(cfg,                   rc_resize_allowed);
  RANGE_CHECK_HI(cfg, rc_dropframe_thresh,   100);
  RANGE_CHECK_HI(cfg, rc_resize_up_thresh,   100);
  RANGE_CHECK_HI(cfg, rc_resize_down_thresh, 100);
  RANGE_CHECK(cfg,        g_pass,         VPX_RC_ONE_PASS, VPX_RC_LAST_PASS);
  RANGE_CHECK(extra_cfg, min_gf_interval, 0, (MAX_LAG_BUFFERS - 1));
  RANGE_CHECK(extra_cfg, max_gf_interval, 0, (MAX_LAG_BUFFERS - 1));
  if (extra_cfg->max_gf_interval > 0) {
    RANGE_CHECK(extra_cfg, max_gf_interval, 2, (MAX_LAG_BUFFERS - 1));
  }
  if (extra_cfg->min_gf_interval > 0 && extra_cfg->max_gf_interval > 0) {
    RANGE_CHECK(extra_cfg, max_gf_interval, extra_cfg->min_gf_interval,
      (MAX_LAG_BUFFERS - 1));
  }

  if (cfg->rc_resize_allowed == 1) {
    RANGE_CHECK(cfg, rc_scaled_width, 0, cfg->g_w);
    RANGE_CHECK(cfg, rc_scaled_height, 0, cfg->g_h);
  }

  // Spatial/temporal scalability are not yet supported in VP10.
  // Only accept the default value for range checking.
  RANGE_CHECK(cfg, ss_number_layers, 1, 1);
  RANGE_CHECK(cfg, ts_number_layers, 1, 1);
  // VP9 does not support a lower bound on the keyframe interval in
  // automatic keyframe placement mode.
  if (cfg->kf_mode != VPX_KF_DISABLED &&
      cfg->kf_min_dist != cfg->kf_max_dist &&
      cfg->kf_min_dist > 0)
    ERROR("kf_min_dist not supported in auto mode, use 0 "
          "or kf_max_dist instead.");

  RANGE_CHECK(extra_cfg, enable_auto_alt_ref, 0, 2);
  RANGE_CHECK(extra_cfg, cpu_used, -8, 8);
  RANGE_CHECK_HI(extra_cfg, noise_sensitivity, 6);
  RANGE_CHECK(extra_cfg, superblock_size,
              VPX_SUPERBLOCK_SIZE_64X64, VPX_SUPERBLOCK_SIZE_DYNAMIC);
#if CONFIG_EXT_TILE
  // TODO(any): Waring. If CONFIG_EXT_TILE is true, tile_columns really
  // means tile_width, and tile_rows really means tile_hight. The interface
  // should be sanitized.
#if CONFIG_EXT_PARTITION
  if (extra_cfg->superblock_size != VPX_SUPERBLOCK_SIZE_64X64) {
    if (extra_cfg->tile_columns != UINT_MAX)
      RANGE_CHECK(extra_cfg, tile_columns, 1, 32);
    if (extra_cfg->tile_rows != UINT_MAX)
      RANGE_CHECK(extra_cfg, tile_rows, 1, 32);
  } else
#endif  // CONFIG_EXT_PARTITION
  {
    if (extra_cfg->tile_columns != UINT_MAX)
      RANGE_CHECK(extra_cfg, tile_columns, 1, 64);
    if (extra_cfg->tile_rows != UINT_MAX)
      RANGE_CHECK(extra_cfg, tile_rows, 1, 64);
  }
#else
  RANGE_CHECK(extra_cfg, tile_columns, 0, 6);
  RANGE_CHECK(extra_cfg, tile_rows, 0, 2);
#endif  // CONFIG_EXT_TILE
  RANGE_CHECK_HI(extra_cfg, sharpness, 7);
  RANGE_CHECK(extra_cfg, arnr_max_frames, 0, 15);
  RANGE_CHECK_HI(extra_cfg, arnr_strength, 6);
  RANGE_CHECK(extra_cfg, cq_level, 0, 63);
  RANGE_CHECK(cfg, g_bit_depth, VPX_BITS_8, VPX_BITS_12);
  RANGE_CHECK(cfg, g_input_bit_depth, 8, 12);
  RANGE_CHECK(extra_cfg, content,
              VP9E_CONTENT_DEFAULT, VP9E_CONTENT_INVALID - 1);

  // TODO(yaowu): remove this when ssim tuning is implemented for vp9
  if (extra_cfg->tuning == VP8_TUNE_SSIM)
      ERROR("Option --tune=ssim is not currently supported in VP9.");

  if (cfg->g_pass == VPX_RC_LAST_PASS) {
    const size_t packet_sz = sizeof(FIRSTPASS_STATS);
    const int n_packets = (int)(cfg->rc_twopass_stats_in.sz / packet_sz);
    const FIRSTPASS_STATS *stats;

    if (cfg->rc_twopass_stats_in.buf == NULL)
      ERROR("rc_twopass_stats_in.buf not set.");

    if (cfg->rc_twopass_stats_in.sz % packet_sz)
      ERROR("rc_twopass_stats_in.sz indicates truncated packet.");

    if (cfg->rc_twopass_stats_in.sz < 2 * packet_sz)
      ERROR("rc_twopass_stats_in requires at least two packets.");

    stats =
        (const FIRSTPASS_STATS *)cfg->rc_twopass_stats_in.buf + n_packets - 1;

    if ((int)(stats->count + 0.5) != n_packets - 1)
      ERROR("rc_twopass_stats_in missing EOS stats packet");
  }

#if !CONFIG_VP9_HIGHBITDEPTH
  if (cfg->g_profile > (unsigned int)PROFILE_1) {
    ERROR("Profile > 1 not supported in this build configuration");
  }
#endif
  if (cfg->g_profile <= (unsigned int)PROFILE_1 &&
      cfg->g_bit_depth > VPX_BITS_8) {
    ERROR("Codec high bit-depth not supported in profile < 2");
  }
  if (cfg->g_profile <= (unsigned int)PROFILE_1 &&
      cfg->g_input_bit_depth > 8) {
    ERROR("Source high bit-depth not supported in profile < 2");
  }
  if (cfg->g_profile > (unsigned int)PROFILE_1 &&
      cfg->g_bit_depth == VPX_BITS_8) {
    ERROR("Codec bit-depth 8 not supported in profile > 1");
  }
  RANGE_CHECK(extra_cfg, color_space, VPX_CS_UNKNOWN, VPX_CS_SRGB);
  RANGE_CHECK(extra_cfg, color_range, 0, 1);
  return VPX_CODEC_OK;
}

static vpx_codec_err_t validate_img(vpx_codec_alg_priv_t *ctx,
                                    const vpx_image_t *img) {
  switch (img->fmt) {
    case VPX_IMG_FMT_YV12:
    case VPX_IMG_FMT_I420:
    case VPX_IMG_FMT_I42016:
      break;
    case VPX_IMG_FMT_I422:
    case VPX_IMG_FMT_I444:
    case VPX_IMG_FMT_I440:
      if (ctx->cfg.g_profile != (unsigned int)PROFILE_1) {
        ERROR("Invalid image format. I422, I444, I440 images are "
              "not supported in profile.");
      }
      break;
    case VPX_IMG_FMT_I42216:
    case VPX_IMG_FMT_I44416:
    case VPX_IMG_FMT_I44016:
      if (ctx->cfg.g_profile != (unsigned int)PROFILE_1 &&
          ctx->cfg.g_profile != (unsigned int)PROFILE_3) {
        ERROR("Invalid image format. 16-bit I422, I444, I440 images are "
              "not supported in profile.");
      }
      break;
    default:
      ERROR("Invalid image format. Only YV12, I420, I422, I444 images are "
            "supported.");
      break;
  }

  if (img->d_w != ctx->cfg.g_w || img->d_h != ctx->cfg.g_h)
    ERROR("Image size must match encoder init configuration size");

  return VPX_CODEC_OK;
}

static int get_image_bps(const vpx_image_t *img) {
  switch (img->fmt) {
    case VPX_IMG_FMT_YV12:
    case VPX_IMG_FMT_I420: return 12;
    case VPX_IMG_FMT_I422: return 16;
    case VPX_IMG_FMT_I444: return 24;
    case VPX_IMG_FMT_I440: return 16;
    case VPX_IMG_FMT_I42016: return 24;
    case VPX_IMG_FMT_I42216: return 32;
    case VPX_IMG_FMT_I44416: return 48;
    case VPX_IMG_FMT_I44016: return 32;
    default: assert(0 && "Invalid image format"); break;
  }
  return 0;
}

static vpx_codec_err_t set_encoder_config(
  VP10EncoderConfig *oxcf,
  const vpx_codec_enc_cfg_t *cfg,
  const struct vp10_extracfg *extra_cfg) {
  const int is_vbr = cfg->rc_end_usage == VPX_VBR;
  oxcf->profile = cfg->g_profile;
  oxcf->max_threads = (int)cfg->g_threads;
  oxcf->width   = cfg->g_w;
  oxcf->height  = cfg->g_h;
  oxcf->bit_depth = cfg->g_bit_depth;
  oxcf->input_bit_depth = cfg->g_input_bit_depth;
  // guess a frame rate if out of whack, use 30
  oxcf->init_framerate = (double)cfg->g_timebase.den / cfg->g_timebase.num;
  if (oxcf->init_framerate > 180)
    oxcf->init_framerate = 30;

  oxcf->mode = GOOD;

  switch (cfg->g_pass) {
    case VPX_RC_ONE_PASS:
      oxcf->pass = 0;
      break;
    case VPX_RC_FIRST_PASS:
      oxcf->pass = 1;
      break;
    case VPX_RC_LAST_PASS:
      oxcf->pass = 2;
      break;
  }

  oxcf->lag_in_frames = cfg->g_pass == VPX_RC_FIRST_PASS ? 0
                                                         : cfg->g_lag_in_frames;
  oxcf->rc_mode = cfg->rc_end_usage;

  // Convert target bandwidth from Kbit/s to Bit/s
  oxcf->target_bandwidth = 1000 * cfg->rc_target_bitrate;
  oxcf->rc_max_intra_bitrate_pct = extra_cfg->rc_max_intra_bitrate_pct;
  oxcf->rc_max_inter_bitrate_pct = extra_cfg->rc_max_inter_bitrate_pct;
  oxcf->gf_cbr_boost_pct = extra_cfg->gf_cbr_boost_pct;

  oxcf->best_allowed_q =
      extra_cfg->lossless ? 0 : vp10_quantizer_to_qindex(cfg->rc_min_quantizer);
  oxcf->worst_allowed_q =
      extra_cfg->lossless ? 0 : vp10_quantizer_to_qindex(cfg->rc_max_quantizer);
  oxcf->cq_level        = vp10_quantizer_to_qindex(extra_cfg->cq_level);
  oxcf->fixed_q = -1;

  oxcf->under_shoot_pct         = cfg->rc_undershoot_pct;
  oxcf->over_shoot_pct          = cfg->rc_overshoot_pct;

  oxcf->scaled_frame_width  = cfg->rc_scaled_width;
  oxcf->scaled_frame_height = cfg->rc_scaled_height;
  if (cfg->rc_resize_allowed == 1) {
    oxcf->resize_mode =
        (oxcf->scaled_frame_width == 0 || oxcf->scaled_frame_height == 0) ?
            RESIZE_DYNAMIC : RESIZE_FIXED;
  } else {
    oxcf->resize_mode = RESIZE_NONE;
  }

  oxcf->maximum_buffer_size_ms   = is_vbr ? 240000 : cfg->rc_buf_sz;
  oxcf->starting_buffer_level_ms = is_vbr ? 60000 : cfg->rc_buf_initial_sz;
  oxcf->optimal_buffer_level_ms  = is_vbr ? 60000 : cfg->rc_buf_optimal_sz;

  oxcf->drop_frames_water_mark   = cfg->rc_dropframe_thresh;

  oxcf->two_pass_vbrbias         = cfg->rc_2pass_vbr_bias_pct;
  oxcf->two_pass_vbrmin_section  = cfg->rc_2pass_vbr_minsection_pct;
  oxcf->two_pass_vbrmax_section  = cfg->rc_2pass_vbr_maxsection_pct;

  oxcf->auto_key               = cfg->kf_mode == VPX_KF_AUTO &&
                                 cfg->kf_min_dist != cfg->kf_max_dist;

  oxcf->key_freq               = cfg->kf_max_dist;

  oxcf->speed                  =  abs(extra_cfg->cpu_used);
  oxcf->encode_breakout        =  extra_cfg->static_thresh;
  oxcf->enable_auto_arf        =  extra_cfg->enable_auto_alt_ref;
  oxcf->noise_sensitivity      =  extra_cfg->noise_sensitivity;
  oxcf->sharpness              =  extra_cfg->sharpness;

  oxcf->two_pass_stats_in      =  cfg->rc_twopass_stats_in;

#if CONFIG_FP_MB_STATS
  oxcf->firstpass_mb_stats_in  = cfg->rc_firstpass_mb_stats_in;
#endif

  oxcf->color_space = extra_cfg->color_space;
  oxcf->color_range = extra_cfg->color_range;
  oxcf->render_width  = extra_cfg->render_width;
  oxcf->render_height = extra_cfg->render_height;
  oxcf->arnr_max_frames = extra_cfg->arnr_max_frames;
  oxcf->arnr_strength   = extra_cfg->arnr_strength;
  oxcf->min_gf_interval = extra_cfg->min_gf_interval;
  oxcf->max_gf_interval = extra_cfg->max_gf_interval;

  oxcf->tuning = extra_cfg->tuning;
  oxcf->content = extra_cfg->content;

#if CONFIG_EXT_PARTITION
  oxcf->superblock_size = extra_cfg->superblock_size;
#endif  // CONFIG_EXT_PARTITION

#if CONFIG_EXT_TILE
  {
#if CONFIG_EXT_PARTITION
    const unsigned int max =
      extra_cfg->superblock_size == VPX_SUPERBLOCK_SIZE_64X64 ? 64 : 32;
#else
    const unsigned int max = 64;
#endif  // CONFIG_EXT_PARTITION
    oxcf->tile_columns = VPXMIN(extra_cfg->tile_columns, max);
    oxcf->tile_rows    = VPXMIN(extra_cfg->tile_rows, max);
  }
#else
  oxcf->tile_columns = extra_cfg->tile_columns;
  oxcf->tile_rows    = extra_cfg->tile_rows;
#endif  // CONFIG_EXT_TILE

  oxcf->error_resilient_mode         = cfg->g_error_resilient;
  oxcf->frame_parallel_decoding_mode = extra_cfg->frame_parallel_decoding_mode;

  oxcf->aq_mode = extra_cfg->aq_mode;

  oxcf->frame_periodic_boost =  extra_cfg->frame_periodic_boost;

  /*
  printf("Current VP9 Settings: \n");
  printf("target_bandwidth: %d\n", oxcf->target_bandwidth);
  printf("noise_sensitivity: %d\n", oxcf->noise_sensitivity);
  printf("sharpness: %d\n",    oxcf->sharpness);
  printf("cpu_used: %d\n",  oxcf->cpu_used);
  printf("Mode: %d\n",     oxcf->mode);
  printf("auto_key: %d\n",  oxcf->auto_key);
  printf("key_freq: %d\n", oxcf->key_freq);
  printf("end_usage: %d\n", oxcf->end_usage);
  printf("under_shoot_pct: %d\n", oxcf->under_shoot_pct);
  printf("over_shoot_pct: %d\n", oxcf->over_shoot_pct);
  printf("starting_buffer_level: %d\n", oxcf->starting_buffer_level);
  printf("optimal_buffer_level: %d\n",  oxcf->optimal_buffer_level);
  printf("maximum_buffer_size: %d\n", oxcf->maximum_buffer_size);
  printf("fixed_q: %d\n",  oxcf->fixed_q);
  printf("worst_allowed_q: %d\n", oxcf->worst_allowed_q);
  printf("best_allowed_q: %d\n", oxcf->best_allowed_q);
  printf("allow_spatial_resampling: %d\n", oxcf->allow_spatial_resampling);
  printf("scaled_frame_width: %d\n", oxcf->scaled_frame_width);
  printf("scaled_frame_height: %d\n", oxcf->scaled_frame_height);
  printf("two_pass_vbrbias: %d\n",  oxcf->two_pass_vbrbias);
  printf("two_pass_vbrmin_section: %d\n", oxcf->two_pass_vbrmin_section);
  printf("two_pass_vbrmax_section: %d\n", oxcf->two_pass_vbrmax_section);
  printf("lag_in_frames: %d\n", oxcf->lag_in_frames);
  printf("enable_auto_arf: %d\n", oxcf->enable_auto_arf);
  printf("Version: %d\n", oxcf->Version);
  printf("encode_breakout: %d\n", oxcf->encode_breakout);
  printf("error resilient: %d\n", oxcf->error_resilient_mode);
  printf("frame parallel detokenization: %d\n",
         oxcf->frame_parallel_decoding_mode);
  */
  return VPX_CODEC_OK;
}

static vpx_codec_err_t encoder_set_config(vpx_codec_alg_priv_t *ctx,
                                          const vpx_codec_enc_cfg_t  *cfg) {
  vpx_codec_err_t res;
  int force_key = 0;

  if (cfg->g_w != ctx->cfg.g_w || cfg->g_h != ctx->cfg.g_h) {
    if (cfg->g_lag_in_frames > 1 || cfg->g_pass != VPX_RC_ONE_PASS)
      ERROR("Cannot change width or height after initialization");
    if (!valid_ref_frame_size(ctx->cfg.g_w, ctx->cfg.g_h, cfg->g_w, cfg->g_h) ||
        (ctx->cpi->initial_width && (int)cfg->g_w > ctx->cpi->initial_width) ||
        (ctx->cpi->initial_height && (int)cfg->g_h > ctx->cpi->initial_height))
      force_key = 1;
  }

  // Prevent increasing lag_in_frames. This check is stricter than it needs
  // to be -- the limit is not increasing past the first lag_in_frames
  // value, but we don't track the initial config, only the last successful
  // config.
  if (cfg->g_lag_in_frames > ctx->cfg.g_lag_in_frames)
    ERROR("Cannot increase lag_in_frames");

  res = validate_config(ctx, cfg, &ctx->extra_cfg);

  if (res == VPX_CODEC_OK) {
    ctx->cfg = *cfg;
    set_encoder_config(&ctx->oxcf, &ctx->cfg, &ctx->extra_cfg);
    // On profile change, request a key frame
    force_key |= ctx->cpi->common.profile != ctx->oxcf.profile;
    vp10_change_config(ctx->cpi, &ctx->oxcf);
  }

  if (force_key)
    ctx->next_frame_flags |= VPX_EFLAG_FORCE_KF;

  return res;
}

static vpx_codec_err_t ctrl_get_quantizer(vpx_codec_alg_priv_t *ctx,
                                          va_list args) {
  int *const arg = va_arg(args, int *);
  if (arg == NULL)
    return VPX_CODEC_INVALID_PARAM;
  *arg = vp10_get_quantizer(ctx->cpi);
  return VPX_CODEC_OK;
}

static vpx_codec_err_t ctrl_get_quantizer64(vpx_codec_alg_priv_t *ctx,
                                            va_list args) {
  int *const arg = va_arg(args, int *);
  if (arg == NULL)
    return VPX_CODEC_INVALID_PARAM;
  *arg = vp10_qindex_to_quantizer(vp10_get_quantizer(ctx->cpi));
  return VPX_CODEC_OK;
}

static vpx_codec_err_t update_extra_cfg(vpx_codec_alg_priv_t *ctx,
                                        const struct vp10_extracfg *extra_cfg) {
  const vpx_codec_err_t res = validate_config(ctx, &ctx->cfg, extra_cfg);
  if (res == VPX_CODEC_OK) {
    ctx->extra_cfg = *extra_cfg;
    set_encoder_config(&ctx->oxcf, &ctx->cfg, &ctx->extra_cfg);
    vp10_change_config(ctx->cpi, &ctx->oxcf);
  }
  return res;
}

static vpx_codec_err_t ctrl_set_cpuused(vpx_codec_alg_priv_t *ctx,
                                        va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.cpu_used = CAST(VP8E_SET_CPUUSED, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_enable_auto_alt_ref(vpx_codec_alg_priv_t *ctx,
                                                    va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.enable_auto_alt_ref = CAST(VP8E_SET_ENABLEAUTOALTREF, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_noise_sensitivity(vpx_codec_alg_priv_t *ctx,
                                                  va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.noise_sensitivity = CAST(VP9E_SET_NOISE_SENSITIVITY, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_sharpness(vpx_codec_alg_priv_t *ctx,
                                          va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.sharpness = CAST(VP8E_SET_SHARPNESS, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_static_thresh(vpx_codec_alg_priv_t *ctx,
                                              va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.static_thresh = CAST(VP8E_SET_STATIC_THRESHOLD, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_tile_columns(vpx_codec_alg_priv_t *ctx,
                                             va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.tile_columns = CAST(VP9E_SET_TILE_COLUMNS, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_tile_rows(vpx_codec_alg_priv_t *ctx,
                                          va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.tile_rows = CAST(VP9E_SET_TILE_ROWS, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_arnr_max_frames(vpx_codec_alg_priv_t *ctx,
                                                va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.arnr_max_frames = CAST(VP8E_SET_ARNR_MAXFRAMES, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_arnr_strength(vpx_codec_alg_priv_t *ctx,
                                              va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.arnr_strength = CAST(VP8E_SET_ARNR_STRENGTH, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_arnr_type(vpx_codec_alg_priv_t *ctx,
                                          va_list args) {
  (void)ctx;
  (void)args;
  return VPX_CODEC_OK;
}

static vpx_codec_err_t ctrl_set_tuning(vpx_codec_alg_priv_t *ctx,
                                       va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.tuning = CAST(VP8E_SET_TUNING, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_cq_level(vpx_codec_alg_priv_t *ctx,
                                         va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.cq_level = CAST(VP8E_SET_CQ_LEVEL, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_rc_max_intra_bitrate_pct(
    vpx_codec_alg_priv_t *ctx, va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.rc_max_intra_bitrate_pct =
      CAST(VP8E_SET_MAX_INTRA_BITRATE_PCT, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_rc_max_inter_bitrate_pct(
    vpx_codec_alg_priv_t *ctx, va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.rc_max_inter_bitrate_pct =
      CAST(VP8E_SET_MAX_INTER_BITRATE_PCT, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_rc_gf_cbr_boost_pct(
    vpx_codec_alg_priv_t *ctx, va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.gf_cbr_boost_pct =
      CAST(VP9E_SET_GF_CBR_BOOST_PCT, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_lossless(vpx_codec_alg_priv_t *ctx,
                                         va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.lossless = CAST(VP9E_SET_LOSSLESS, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_frame_parallel_decoding_mode(
    vpx_codec_alg_priv_t *ctx, va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.frame_parallel_decoding_mode =
      CAST(VP9E_SET_FRAME_PARALLEL_DECODING, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_aq_mode(vpx_codec_alg_priv_t *ctx,
                                        va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.aq_mode = CAST(VP9E_SET_AQ_MODE, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_min_gf_interval(vpx_codec_alg_priv_t *ctx,
                                                va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.min_gf_interval = CAST(VP9E_SET_MIN_GF_INTERVAL, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_max_gf_interval(vpx_codec_alg_priv_t *ctx,
                                                va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.max_gf_interval = CAST(VP9E_SET_MAX_GF_INTERVAL, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_frame_periodic_boost(vpx_codec_alg_priv_t *ctx,
                                                     va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.frame_periodic_boost = CAST(VP9E_SET_FRAME_PERIODIC_BOOST, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t encoder_init(vpx_codec_ctx_t *ctx,
                                    vpx_codec_priv_enc_mr_cfg_t *data) {
  vpx_codec_err_t res = VPX_CODEC_OK;
  (void)data;

  if (ctx->priv == NULL) {
    vpx_codec_alg_priv_t *const priv = vpx_calloc(1, sizeof(*priv));
    if (priv == NULL)
      return VPX_CODEC_MEM_ERROR;

    ctx->priv = (vpx_codec_priv_t *)priv;
    ctx->priv->init_flags = ctx->init_flags;
    ctx->priv->enc.total_encoders = 1;
    priv->buffer_pool =
        (BufferPool *)vpx_calloc(1, sizeof(BufferPool));
    if (priv->buffer_pool == NULL)
      return VPX_CODEC_MEM_ERROR;

#if CONFIG_MULTITHREAD
    if (pthread_mutex_init(&priv->buffer_pool->pool_mutex, NULL)) {
      return VPX_CODEC_MEM_ERROR;
    }
#endif

    if (ctx->config.enc) {
      // Update the reference to the config structure to an internal copy.
      priv->cfg = *ctx->config.enc;
      ctx->config.enc = &priv->cfg;
    }

    priv->extra_cfg = default_extra_cfg;
    once(vp10_initialize_enc);

    res = validate_config(priv, &priv->cfg, &priv->extra_cfg);

    if (res == VPX_CODEC_OK) {
      set_encoder_config(&priv->oxcf, &priv->cfg, &priv->extra_cfg);
#if CONFIG_VP9_HIGHBITDEPTH
      priv->oxcf.use_highbitdepth =
          (ctx->init_flags & VPX_CODEC_USE_HIGHBITDEPTH) ? 1 : 0;
#endif
      priv->cpi = vp10_create_compressor(&priv->oxcf, priv->buffer_pool);
      if (priv->cpi == NULL)
        res = VPX_CODEC_MEM_ERROR;
      else
        priv->cpi->output_pkt_list = &priv->pkt_list.head;
    }
  }

  return res;
}

static vpx_codec_err_t encoder_destroy(vpx_codec_alg_priv_t *ctx) {
  free(ctx->cx_data);
  vp10_remove_compressor(ctx->cpi);
#if CONFIG_MULTITHREAD
  pthread_mutex_destroy(&ctx->buffer_pool->pool_mutex);
#endif
  vpx_free(ctx->buffer_pool);
  vpx_free(ctx);
  return VPX_CODEC_OK;
}

static void pick_quickcompress_mode(vpx_codec_alg_priv_t *ctx,
                                    unsigned long duration,
                                    unsigned long deadline) {
  MODE new_mode = BEST;

  switch (ctx->cfg.g_pass) {
    case VPX_RC_ONE_PASS:
      if (deadline > 0) {
        const vpx_codec_enc_cfg_t *const cfg = &ctx->cfg;

        // Convert duration parameter from stream timebase to microseconds.
        const uint64_t duration_us = (uint64_t)duration * 1000000 *
           (uint64_t)cfg->g_timebase.num /(uint64_t)cfg->g_timebase.den;

        // If the deadline is more that the duration this frame is to be shown,
        // use good quality mode. Otherwise use realtime mode.
        new_mode = (deadline > duration_us) ? GOOD : REALTIME;
      } else {
        new_mode = BEST;
      }
      break;
    case VPX_RC_FIRST_PASS:
      break;
    case VPX_RC_LAST_PASS:
      new_mode = deadline > 0 ? GOOD : BEST;
      break;
  }

  if (ctx->oxcf.mode != new_mode) {
    ctx->oxcf.mode = new_mode;
    vp10_change_config(ctx->cpi, &ctx->oxcf);
  }
}

// Turn on to test if supplemental superframe data breaks decoding
// #define TEST_SUPPLEMENTAL_SUPERFRAME_DATA
static int write_superframe_index(vpx_codec_alg_priv_t *ctx) {
  uint8_t marker = 0xc0;
  unsigned int mask;
  int mag, index_sz;
  int i;
  size_t max_frame_sz = 0;

  assert(ctx->pending_frame_count);
  assert(ctx->pending_frame_count <= 8);

  // Add the number of frames to the marker byte
  marker |= ctx->pending_frame_count - 1;
  for (i = 0; i < ctx->pending_frame_count - 1; i++) {
    const size_t frame_sz = (unsigned int) ctx->pending_frame_sizes[i] - 1;
    max_frame_sz = frame_sz > max_frame_sz ? frame_sz : max_frame_sz;
  }

  // Choose the magnitude
  for (mag = 0, mask = 0xff; mag < 4; mag++) {
    if (max_frame_sz <= mask)
      break;
    mask <<= 8;
    mask |= 0xff;
  }
  marker |= mag << 3;

  // Write the index
  index_sz = 2 + (mag + 1) * (ctx->pending_frame_count - 1);
  if (ctx->pending_cx_data_sz + index_sz < ctx->cx_data_sz) {
    uint8_t *x = ctx->pending_cx_data + ctx->pending_cx_data_sz;
    int i, j;
#ifdef TEST_SUPPLEMENTAL_SUPERFRAME_DATA
    uint8_t marker_test = 0xc0;
    int mag_test = 2;     // 1 - 4
    int frames_test = 4;  // 1 - 8
    int index_sz_test = 2 + mag_test * frames_test;
    marker_test |= frames_test - 1;
    marker_test |= (mag_test - 1) << 3;
    *x++ = marker_test;
    for (i = 0; i < mag_test * frames_test; ++i)
      *x++ = 0;  // fill up with arbitrary data
    *x++ = marker_test;
    ctx->pending_cx_data_sz += index_sz_test;
    printf("Added supplemental superframe data\n");
#endif

    *x++ = marker;
    for (i = 0; i < ctx->pending_frame_count - 1; i++) {
      unsigned int this_sz;

      assert(ctx->pending_frame_sizes[i] > 0);
      this_sz = (unsigned int)ctx->pending_frame_sizes[i] - 1;
      for (j = 0; j <= mag; j++) {
        *x++ = this_sz & 0xff;
        this_sz >>= 8;
      }
    }
    *x++ = marker;
    ctx->pending_cx_data_sz += index_sz;
#ifdef TEST_SUPPLEMENTAL_SUPERFRAME_DATA
    index_sz += index_sz_test;
#endif
  }
  return index_sz;
}

// vp9 uses 10,000,000 ticks/second as time stamp
#define TICKS_PER_SEC 10000000LL

static int64_t timebase_units_to_ticks(const vpx_rational_t *timebase,
                                       int64_t n) {
  return n * TICKS_PER_SEC * timebase->num / timebase->den;
}

static int64_t ticks_to_timebase_units(const vpx_rational_t *timebase,
                                       int64_t n) {
  const int64_t round = TICKS_PER_SEC * timebase->num / 2 - 1;
  return (n * timebase->den + round) / timebase->num / TICKS_PER_SEC;
}

static vpx_codec_frame_flags_t get_frame_pkt_flags(const VP10_COMP *cpi,
                                                   unsigned int lib_flags) {
  vpx_codec_frame_flags_t flags = lib_flags << 16;

  if (lib_flags & FRAMEFLAGS_KEY)
    flags |= VPX_FRAME_IS_KEY;

  if (cpi->droppable)
    flags |= VPX_FRAME_IS_DROPPABLE;

  return flags;
}

static vpx_codec_err_t encoder_encode(vpx_codec_alg_priv_t  *ctx,
                                      const vpx_image_t *img,
                                      vpx_codec_pts_t pts,
                                      unsigned long duration,
                                      vpx_enc_frame_flags_t enc_flags,
                                      unsigned long deadline) {
  volatile vpx_codec_err_t res = VPX_CODEC_OK;
  volatile vpx_enc_frame_flags_t flags = enc_flags;
  VP10_COMP *const cpi = ctx->cpi;
  const vpx_rational_t *const timebase = &ctx->cfg.g_timebase;
  size_t data_sz;

  if (cpi == NULL) return VPX_CODEC_INVALID_PARAM;

  if (img != NULL) {
    res = validate_img(ctx, img);
    // TODO(jzern) the checks related to cpi's validity should be treated as a
    // failure condition, encoder setup is done fully in init() currently.
    if (res == VPX_CODEC_OK) {
      // There's no codec control for multiple alt-refs so check the encoder
      // instance for its status to determine the compressed data size.
      data_sz = ctx->cfg.g_w * ctx->cfg.g_h * get_image_bps(img) / 8 *
                (cpi->multi_arf_allowed ? 8 : 2);
      if (data_sz < 4096)
        data_sz = 4096;
      if (ctx->cx_data == NULL || ctx->cx_data_sz < data_sz) {
        ctx->cx_data_sz = data_sz;
        free(ctx->cx_data);
        ctx->cx_data = (unsigned char*)malloc(ctx->cx_data_sz);
        if (ctx->cx_data == NULL) {
          return VPX_CODEC_MEM_ERROR;
        }
      }
    }
  }

  pick_quickcompress_mode(ctx, duration, deadline);
  vpx_codec_pkt_list_init(&ctx->pkt_list);

  // Handle Flags
  if (((flags & VP8_EFLAG_NO_UPD_GF) && (flags & VP8_EFLAG_FORCE_GF)) ||
       ((flags & VP8_EFLAG_NO_UPD_ARF) && (flags & VP8_EFLAG_FORCE_ARF))) {
    ctx->base.err_detail = "Conflicting flags.";
    return VPX_CODEC_INVALID_PARAM;
  }

  if (setjmp(cpi->common.error.jmp)) {
    cpi->common.error.setjmp = 0;
    res = update_error_state(ctx, &cpi->common.error);
    vpx_clear_system_state();
    return res;
  }
  cpi->common.error.setjmp = 1;

  vp10_apply_encoding_flags(cpi, flags);

  // Handle fixed keyframe intervals
  if (ctx->cfg.kf_mode == VPX_KF_AUTO &&
      ctx->cfg.kf_min_dist == ctx->cfg.kf_max_dist) {
    if (++ctx->fixed_kf_cntr > ctx->cfg.kf_min_dist) {
      flags |= VPX_EFLAG_FORCE_KF;
      ctx->fixed_kf_cntr = 1;
    }
  }

  if (res == VPX_CODEC_OK) {
    unsigned int lib_flags = 0;
    YV12_BUFFER_CONFIG sd;
    int64_t dst_time_stamp = timebase_units_to_ticks(timebase, pts);
    int64_t dst_end_time_stamp =
        timebase_units_to_ticks(timebase, pts + duration);
    size_t size, cx_data_sz;
    unsigned char *cx_data;

    // Set up internal flags
    if (ctx->base.init_flags & VPX_CODEC_USE_PSNR)
      cpi->b_calculate_psnr = 1;

    if (img != NULL) {
      res = image2yuvconfig(img, &sd);

      // Store the original flags in to the frame buffer. Will extract the
      // key frame flag when we actually encode this frame.
      if (vp10_receive_raw_frame(cpi, flags | ctx->next_frame_flags,
                                &sd, dst_time_stamp, dst_end_time_stamp)) {
        res = update_error_state(ctx, &cpi->common.error);
      }
      ctx->next_frame_flags = 0;
    }

    cx_data = ctx->cx_data;
    cx_data_sz = ctx->cx_data_sz;

    /* Any pending invisible frames? */
    if (ctx->pending_cx_data) {
      memmove(cx_data, ctx->pending_cx_data, ctx->pending_cx_data_sz);
      ctx->pending_cx_data = cx_data;
      cx_data += ctx->pending_cx_data_sz;
      cx_data_sz -= ctx->pending_cx_data_sz;

      /* TODO: this is a minimal check, the underlying codec doesn't respect
       * the buffer size anyway.
       */
      if (cx_data_sz < ctx->cx_data_sz / 2) {
        vpx_internal_error(&cpi->common.error, VPX_CODEC_ERROR,
                           "Compressed data buffer too small");
        return VPX_CODEC_ERROR;
      }
    }

    while (cx_data_sz >= ctx->cx_data_sz / 2 &&
           -1 != vp10_get_compressed_data(cpi, &lib_flags, &size,
                                         cx_data, &dst_time_stamp,
                                         &dst_end_time_stamp, !img)) {
      if (size) {
        vpx_codec_cx_pkt_t pkt;

        // Pack invisible frames with the next visible frame
        if (!cpi->common.show_frame) {
          if (ctx->pending_cx_data == 0)
            ctx->pending_cx_data = cx_data;
          ctx->pending_cx_data_sz += size;
          ctx->pending_frame_sizes[ctx->pending_frame_count++] = size;
          cx_data += size;
          cx_data_sz -= size;

          if (ctx->output_cx_pkt_cb.output_cx_pkt) {
            pkt.kind = VPX_CODEC_CX_FRAME_PKT;
            pkt.data.frame.pts = ticks_to_timebase_units(timebase,
                                                         dst_time_stamp);
            pkt.data.frame.duration =
               (unsigned long)ticks_to_timebase_units(timebase,
                   dst_end_time_stamp - dst_time_stamp);
            pkt.data.frame.flags = get_frame_pkt_flags(cpi, lib_flags);
            pkt.data.frame.buf = ctx->pending_cx_data;
            pkt.data.frame.sz  = size;
            ctx->pending_cx_data = NULL;
            ctx->pending_cx_data_sz = 0;
            ctx->pending_frame_count = 0;
            ctx->output_cx_pkt_cb.output_cx_pkt(
                &pkt, ctx->output_cx_pkt_cb.user_priv);
          }
          continue;
        }

        // Add the frame packet to the list of returned packets.
        pkt.kind = VPX_CODEC_CX_FRAME_PKT;
        pkt.data.frame.pts = ticks_to_timebase_units(timebase, dst_time_stamp);
        pkt.data.frame.duration =
           (unsigned long)ticks_to_timebase_units(timebase,
               dst_end_time_stamp - dst_time_stamp);
        pkt.data.frame.flags = get_frame_pkt_flags(cpi, lib_flags);

        if (ctx->pending_cx_data) {
          ctx->pending_frame_sizes[ctx->pending_frame_count++] = size;
          ctx->pending_cx_data_sz += size;
          // write the superframe only for the case when
          if (!ctx->output_cx_pkt_cb.output_cx_pkt)
            size += write_superframe_index(ctx);
          pkt.data.frame.buf = ctx->pending_cx_data;
          pkt.data.frame.sz  = ctx->pending_cx_data_sz;
          ctx->pending_cx_data = NULL;
          ctx->pending_cx_data_sz = 0;
          ctx->pending_frame_count = 0;
        } else {
          pkt.data.frame.buf = cx_data;
          pkt.data.frame.sz  = size;
        }
        pkt.data.frame.partition_id = -1;

        if(ctx->output_cx_pkt_cb.output_cx_pkt)
          ctx->output_cx_pkt_cb.output_cx_pkt(&pkt,
                                              ctx->output_cx_pkt_cb.user_priv);
        else
          vpx_codec_pkt_list_add(&ctx->pkt_list.head, &pkt);

        cx_data += size;
        cx_data_sz -= size;
      }
    }
  }

  cpi->common.error.setjmp = 0;
  return res;
}

static const vpx_codec_cx_pkt_t *encoder_get_cxdata(vpx_codec_alg_priv_t *ctx,
                                                    vpx_codec_iter_t *iter) {
  return vpx_codec_pkt_list_get(&ctx->pkt_list.head, iter);
}

static vpx_codec_err_t ctrl_set_reference(vpx_codec_alg_priv_t *ctx,
                                          va_list args) {
  vpx_ref_frame_t *const frame = va_arg(args, vpx_ref_frame_t *);

  if (frame != NULL) {
    YV12_BUFFER_CONFIG sd;

    image2yuvconfig(&frame->img, &sd);
    vp10_set_reference_enc(ctx->cpi, ref_frame_to_vp10_reframe(frame->frame_type),
                          &sd);
    return VPX_CODEC_OK;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t ctrl_copy_reference(vpx_codec_alg_priv_t *ctx,
                                           va_list args) {
  vpx_ref_frame_t *const frame = va_arg(args, vpx_ref_frame_t *);

  if (frame != NULL) {
    YV12_BUFFER_CONFIG sd;

    image2yuvconfig(&frame->img, &sd);
    vp10_copy_reference_enc(ctx->cpi,
                           ref_frame_to_vp10_reframe(frame->frame_type), &sd);
    return VPX_CODEC_OK;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t ctrl_get_reference(vpx_codec_alg_priv_t *ctx,
                                          va_list args) {
  vp9_ref_frame_t *const frame = va_arg(args, vp9_ref_frame_t *);

  if (frame != NULL) {
    YV12_BUFFER_CONFIG *fb = get_ref_frame(&ctx->cpi->common, frame->idx);
    if (fb == NULL) return VPX_CODEC_ERROR;

    yuvconfig2image(&frame->img, fb, NULL);
    return VPX_CODEC_OK;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t ctrl_get_new_frame_image(vpx_codec_alg_priv_t *ctx,
                                                va_list args) {
  vpx_image_t *const new_img = va_arg(args, vpx_image_t *);

  if (new_img != NULL) {
    VP10_COMMON *cm = &ctx->cpi->common;
    YV12_BUFFER_CONFIG *new_fb =
        &cm->buffer_pool->frame_bufs[ctx->cpi->last_show_frame_buf_idx].buf;
    if (new_fb == NULL) return VPX_CODEC_ERROR;

    fprintf(stdout, "\nENCODER***Frame=%d, show_frame=%d, frame_type=%d, "
            "new_fb_idx=%d, last_show_frame_buf_idx=%d, "
            "ref_frame_map[%d]=%d, ref_frame_map[%d]=%d, ref_frame_map[%d]=%d\n",
            cm->current_video_frame, cm->show_frame, cm->frame_type,
            cm->new_fb_idx, ctx->cpi->last_show_frame_buf_idx,
            ctx->cpi->lst_fb_idx, cm->ref_frame_map[ctx->cpi->lst_fb_idx],
            ctx->cpi->gld_fb_idx, cm->ref_frame_map[ctx->cpi->gld_fb_idx],
            ctx->cpi->alt_fb_idx, cm->ref_frame_map[ctx->cpi->alt_fb_idx]);
    fflush(stdout);

    yuvconfig2image(new_img, new_fb, NULL);
    return VPX_CODEC_OK;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t ctrl_set_previewpp(vpx_codec_alg_priv_t *ctx,
                                          va_list args) {
#if CONFIG_VP9_POSTPROC
  vp8_postproc_cfg_t *config = va_arg(args, vp8_postproc_cfg_t *);
  if (config != NULL) {
    ctx->preview_ppcfg = *config;
    return VPX_CODEC_OK;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
#else
  (void)ctx;
  (void)args;
  return VPX_CODEC_INCAPABLE;
#endif
}


static vpx_image_t *encoder_get_preview(vpx_codec_alg_priv_t *ctx) {
  YV12_BUFFER_CONFIG sd;
  vp10_ppflags_t flags;
  vp10_zero(flags);

  if (ctx->preview_ppcfg.post_proc_flag) {
    flags.post_proc_flag   = ctx->preview_ppcfg.post_proc_flag;
    flags.deblocking_level = ctx->preview_ppcfg.deblocking_level;
    flags.noise_level      = ctx->preview_ppcfg.noise_level;
  }

  if (vp10_get_preview_raw_frame(ctx->cpi, &sd, &flags) == 0) {
    yuvconfig2image(&ctx->preview_img, &sd, NULL);
    return &ctx->preview_img;
  } else {
    return NULL;
  }
}

static vpx_codec_err_t ctrl_use_reference(vpx_codec_alg_priv_t *ctx,
                                          va_list args) {
  const int reference_flag = va_arg(args, int);

  vp10_use_as_reference(ctx->cpi, reference_flag);
  return VPX_CODEC_OK;
}

static vpx_codec_err_t ctrl_set_roi_map(vpx_codec_alg_priv_t *ctx,
                                        va_list args) {
  (void)ctx;
  (void)args;

  // TODO(yaowu): Need to re-implement and test for VP9.
  return VPX_CODEC_INVALID_PARAM;
}


static vpx_codec_err_t ctrl_set_active_map(vpx_codec_alg_priv_t *ctx,
                                           va_list args) {
  vpx_active_map_t *const map = va_arg(args, vpx_active_map_t *);

  if (map) {
    if (!vp10_set_active_map(ctx->cpi, map->active_map,
                            (int)map->rows, (int)map->cols))
      return VPX_CODEC_OK;
    else
      return VPX_CODEC_INVALID_PARAM;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t ctrl_get_active_map(vpx_codec_alg_priv_t *ctx,
                                           va_list args) {
  vpx_active_map_t *const map = va_arg(args, vpx_active_map_t *);

  if (map) {
    if (!vp10_get_active_map(ctx->cpi, map->active_map,
                            (int)map->rows, (int)map->cols))
      return VPX_CODEC_OK;
    else
      return VPX_CODEC_INVALID_PARAM;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t ctrl_set_scale_mode(vpx_codec_alg_priv_t *ctx,
                                           va_list args) {
  vpx_scaling_mode_t *const mode = va_arg(args, vpx_scaling_mode_t *);

  if (mode) {
    const int res = vp10_set_internal_size(ctx->cpi,
                                          (VPX_SCALING)mode->h_scaling_mode,
                                          (VPX_SCALING)mode->v_scaling_mode);
    return (res == 0) ? VPX_CODEC_OK : VPX_CODEC_INVALID_PARAM;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t ctrl_register_cx_callback(vpx_codec_alg_priv_t *ctx,
                                                 va_list args) {
  vpx_codec_priv_output_cx_pkt_cb_pair_t *cbp =
      (vpx_codec_priv_output_cx_pkt_cb_pair_t *)va_arg(args, void *);
  ctx->output_cx_pkt_cb.output_cx_pkt = cbp->output_cx_pkt;
  ctx->output_cx_pkt_cb.user_priv = cbp->user_priv;

  return VPX_CODEC_OK;
}

static vpx_codec_err_t ctrl_set_tune_content(vpx_codec_alg_priv_t *ctx,
                                             va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.content = CAST(VP9E_SET_TUNE_CONTENT, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_color_space(vpx_codec_alg_priv_t *ctx,
                                            va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.color_space = CAST(VP9E_SET_COLOR_SPACE, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_color_range(vpx_codec_alg_priv_t *ctx,
                                            va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.color_range = CAST(VP9E_SET_COLOR_RANGE, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_render_size(vpx_codec_alg_priv_t *ctx,
                                            va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  int *const render_size = va_arg(args, int *);
  extra_cfg.render_width  = render_size[0];
  extra_cfg.render_height = render_size[1];
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_err_t ctrl_set_superblock_size(vpx_codec_alg_priv_t *ctx,
                                            va_list args) {
  struct vp10_extracfg extra_cfg = ctx->extra_cfg;
  extra_cfg.superblock_size = CAST(VP10E_SET_SUPERBLOCK_SIZE, args);
  return update_extra_cfg(ctx, &extra_cfg);
}

static vpx_codec_ctrl_fn_map_t encoder_ctrl_maps[] = {
  {VP8_COPY_REFERENCE,                ctrl_copy_reference},
  {VP8E_USE_REFERENCE,                ctrl_use_reference},

  // Setters
  {VP8_SET_REFERENCE,                 ctrl_set_reference},
  {VP8_SET_POSTPROC,                  ctrl_set_previewpp},
  {VP8E_SET_ROI_MAP,                  ctrl_set_roi_map},
  {VP8E_SET_ACTIVEMAP,                ctrl_set_active_map},
  {VP8E_SET_SCALEMODE,                ctrl_set_scale_mode},
  {VP8E_SET_CPUUSED,                  ctrl_set_cpuused},
  {VP8E_SET_ENABLEAUTOALTREF,         ctrl_set_enable_auto_alt_ref},
  {VP8E_SET_SHARPNESS,                ctrl_set_sharpness},
  {VP8E_SET_STATIC_THRESHOLD,         ctrl_set_static_thresh},
  {VP9E_SET_TILE_COLUMNS,             ctrl_set_tile_columns},
  {VP9E_SET_TILE_ROWS,                ctrl_set_tile_rows},
  {VP8E_SET_ARNR_MAXFRAMES,           ctrl_set_arnr_max_frames},
  {VP8E_SET_ARNR_STRENGTH,            ctrl_set_arnr_strength},
  {VP8E_SET_ARNR_TYPE,                ctrl_set_arnr_type},
  {VP8E_SET_TUNING,                   ctrl_set_tuning},
  {VP8E_SET_CQ_LEVEL,                 ctrl_set_cq_level},
  {VP8E_SET_MAX_INTRA_BITRATE_PCT,    ctrl_set_rc_max_intra_bitrate_pct},
  {VP9E_SET_MAX_INTER_BITRATE_PCT,    ctrl_set_rc_max_inter_bitrate_pct},
  {VP9E_SET_GF_CBR_BOOST_PCT,         ctrl_set_rc_gf_cbr_boost_pct},
  {VP9E_SET_LOSSLESS,                 ctrl_set_lossless},
  {VP9E_SET_FRAME_PARALLEL_DECODING,  ctrl_set_frame_parallel_decoding_mode},
  {VP9E_SET_AQ_MODE,                  ctrl_set_aq_mode},
  {VP9E_SET_FRAME_PERIODIC_BOOST,     ctrl_set_frame_periodic_boost},
  {VP9E_REGISTER_CX_CALLBACK,         ctrl_register_cx_callback},
  {VP9E_SET_TUNE_CONTENT,             ctrl_set_tune_content},
  {VP9E_SET_COLOR_SPACE,              ctrl_set_color_space},
  {VP9E_SET_COLOR_RANGE,              ctrl_set_color_range},
  {VP9E_SET_NOISE_SENSITIVITY,        ctrl_set_noise_sensitivity},
  {VP9E_SET_MIN_GF_INTERVAL,          ctrl_set_min_gf_interval},
  {VP9E_SET_MAX_GF_INTERVAL,          ctrl_set_max_gf_interval},
  {VP9E_SET_RENDER_SIZE,              ctrl_set_render_size},
  {VP10E_SET_SUPERBLOCK_SIZE,         ctrl_set_superblock_size},

  // Getters
  {VP8E_GET_LAST_QUANTIZER,           ctrl_get_quantizer},
  {VP8E_GET_LAST_QUANTIZER_64,        ctrl_get_quantizer64},
  {VP9_GET_REFERENCE,                 ctrl_get_reference},
  {VP9E_GET_ACTIVEMAP,                ctrl_get_active_map},
  {VP10_GET_NEW_FRAME_IMAGE,          ctrl_get_new_frame_image},

  { -1, NULL},
};

static vpx_codec_enc_cfg_map_t encoder_usage_cfg_map[] = {
  {
    0,
    {  // NOLINT
      0,                  // g_usage
      8,                  // g_threads
      0,                  // g_profile

      320,                // g_width
      240,                // g_height
      VPX_BITS_8,         // g_bit_depth
      8,                  // g_input_bit_depth

      {1, 30},            // g_timebase

      0,                  // g_error_resilient

      VPX_RC_ONE_PASS,    // g_pass

      25,                 // g_lag_in_frames

      0,                  // rc_dropframe_thresh
      0,                  // rc_resize_allowed
      0,                  // rc_scaled_width
      0,                  // rc_scaled_height
      60,                 // rc_resize_down_thresold
      30,                 // rc_resize_up_thresold

      VPX_VBR,            // rc_end_usage
      {NULL, 0},          // rc_twopass_stats_in
      {NULL, 0},          // rc_firstpass_mb_stats_in
      256,                // rc_target_bandwidth
      0,                  // rc_min_quantizer
      63,                 // rc_max_quantizer
      25,                 // rc_undershoot_pct
      25,                 // rc_overshoot_pct

      6000,               // rc_max_buffer_size
      4000,               // rc_buffer_initial_size
      5000,               // rc_buffer_optimal_size

      50,                 // rc_two_pass_vbrbias
      0,                  // rc_two_pass_vbrmin_section
      2000,               // rc_two_pass_vbrmax_section

      // keyframing settings (kf)
      VPX_KF_AUTO,        // g_kfmode
      0,                  // kf_min_dist
      9999,               // kf_max_dist

      // TODO(yunqingwang): Spatial/temporal scalability are not supported
      // in VP10. The following 10 parameters are not used, which should
      // be removed later.
      1,                      // ss_number_layers
      {0},
      {0},                    // ss_target_bitrate
      1,                      // ts_number_layers
      {0},                    // ts_target_bitrate
      {0},                    // ts_rate_decimator
      0,                      // ts_periodicity
      {0},                    // ts_layer_id
      {0},                  // layer_taget_bitrate
      0                     // temporal_layering_mode
    }
  },
};

#ifndef VERSION_STRING
#define VERSION_STRING
#endif
CODEC_INTERFACE(vpx_codec_vp10_cx) = {
  "WebM Project VP10 Encoder" VERSION_STRING,
  VPX_CODEC_INTERNAL_ABI_VERSION,
#if CONFIG_VP9_HIGHBITDEPTH
  VPX_CODEC_CAP_HIGHBITDEPTH |
#endif
  VPX_CODEC_CAP_ENCODER | VPX_CODEC_CAP_PSNR,  // vpx_codec_caps_t
  encoder_init,       // vpx_codec_init_fn_t
  encoder_destroy,    // vpx_codec_destroy_fn_t
  encoder_ctrl_maps,  // vpx_codec_ctrl_fn_map_t
  {  // NOLINT
    NULL,  // vpx_codec_peek_si_fn_t
    NULL,  // vpx_codec_get_si_fn_t
    NULL,  // vpx_codec_decode_fn_t
    NULL,  // vpx_codec_frame_get_fn_t
    NULL   // vpx_codec_set_fb_fn_t
  },
  {  // NOLINT
    1,                      // 1 cfg map
    encoder_usage_cfg_map,  // vpx_codec_enc_cfg_map_t
    encoder_encode,         // vpx_codec_encode_fn_t
    encoder_get_cxdata,     // vpx_codec_get_cx_data_fn_t
    encoder_set_config,     // vpx_codec_enc_config_set_fn_t
    NULL,        // vpx_codec_get_global_headers_fn_t
    encoder_get_preview,    // vpx_codec_get_preview_frame_fn_t
    NULL         // vpx_codec_enc_mr_get_mem_loc_fn_t
  }
};
