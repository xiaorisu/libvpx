/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "vp8_rtcd.h"
#include "vpx/vpx_codec.h"
#include "vpx/internal/vpx_codec_internal.h"
#include "vpx_version.h"
#include "vp8/encoder/onyx_int.h"
#include "vpx/vp8cx.h"
#include "vp8/encoder/firstpass.h"
#include "vp8/common/onyx.h"
#include <stdlib.h>
#include <string.h>

struct vp8_extracfg
{
    struct vpx_codec_pkt_list *pkt_list;
    int                         cpu_used;                    /** available cpu percentage in 1/16*/
    unsigned int                enable_auto_alt_ref;           /** if encoder decides to uses alternate reference frame */
    unsigned int                noise_sensitivity;
    unsigned int                Sharpness;
    unsigned int                static_thresh;
    unsigned int                token_partitions;
    unsigned int                arnr_max_frames;    /* alt_ref Noise Reduction Max Frame Count */
    unsigned int                arnr_strength;    /* alt_ref Noise Reduction Strength */
    vp8e_tuning                 tuning;
    unsigned int                cq_level;         /* constrained quality level */
    unsigned int                rc_max_intra_bitrate_pct;

};

struct extraconfig_map
{
    int                 usage;
    struct vp8_extracfg cfg;
};

static const struct extraconfig_map extracfg_map[] =
{
    {
        0,
        {
            NULL,
#if !(CONFIG_REALTIME_ONLY)
            0,                          /* cpu_used      */
#else
            4,                          /* cpu_used      */
#endif
            0,                          /* enable_auto_alt_ref */
            0,                          /* noise_sensitivity */
            0,                          /* Sharpness */
            0,                          /* static_thresh */
#if (CONFIG_REALTIME_ONLY & CONFIG_ONTHEFLY_BITPACKING)
            VP8_EIGHT_TOKENPARTITION,
#else
            VP8_ONE_TOKENPARTITION,     /* token_partitions */
#endif
            0,                          /* arnr_max_frames */
            3,                          /* arnr_strength */
            0,                          /* tuning*/
            10,                         /* cq_level */
            0,                          /* rc_max_intra_bitrate_pct */
        }
    }
};

struct vpx_codec_alg_priv
{
    vpx_codec_priv_t        base;
    vpx_codec_enc_cfg_t     cfg;
    struct vp8_extracfg     vp8_cfg;
    VP8_CONFIG              oxcf;
    struct VP8_COMP        *cpi;
    unsigned char          *cx_data;
    unsigned int            cx_data_sz;
    vpx_image_t             preview_img;
    unsigned int            next_frame_flag;
    vp8_postproc_cfg_t      preview_ppcfg;
    /* pkt_list size depends on the maximum number of lagged frames allowed. */
    vpx_codec_pkt_list_decl(64) pkt_list;
    unsigned int                fixed_kf_cntr;
};


static vpx_codec_err_t
update_error_state(vpx_codec_alg_priv_t                 *ctx,
                   const struct vpx_internal_error_info *error)
{
    vpx_codec_err_t res;

    if ((res = error->error_code))
        ctx->base.err_detail = error->has_detail
                               ? error->detail
                               : NULL;

    return res;
}


#undef ERROR
#define ERROR(str) do {\
        ctx->base.err_detail = str;\
        return VPX_CODEC_INVALID_PARAM;\
    } while(0)

#define RANGE_CHECK(p,memb,lo,hi) do {\
        if(!(((p)->memb == lo || (p)->memb > (lo)) && (p)->memb <= hi)) \
            ERROR(#memb " out of range ["#lo".."#hi"]");\
    } while(0)

#define RANGE_CHECK_HI(p,memb,hi) do {\
        if(!((p)->memb <= (hi))) \
            ERROR(#memb " out of range [.."#hi"]");\
    } while(0)

#define RANGE_CHECK_LO(p,memb,lo) do {\
        if(!((p)->memb >= (lo))) \
            ERROR(#memb " out of range ["#lo"..]");\
    } while(0)

#define RANGE_CHECK_BOOL(p,memb) do {\
        if(!!((p)->memb) != (p)->memb) ERROR(#memb " expected boolean");\
    } while(0)

static vpx_codec_err_t validate_config(vpx_codec_alg_priv_t      *ctx,
                                       const vpx_codec_enc_cfg_t *cfg,
                                       const struct vp8_extracfg *vp8_cfg,
                                       int                        finalize)
{
    RANGE_CHECK(cfg, g_w,                   1, 16383); /* 14 bits available */
    RANGE_CHECK(cfg, g_h,                   1, 16383); /* 14 bits available */
    RANGE_CHECK(cfg, g_timebase.den,        1, 1000000000);
    RANGE_CHECK(cfg, g_timebase.num,        1, cfg->g_timebase.den);
    RANGE_CHECK_HI(cfg, g_profile,          3);
    RANGE_CHECK_HI(cfg, rc_max_quantizer,   63);
    RANGE_CHECK_HI(cfg, rc_min_quantizer,   cfg->rc_max_quantizer);
    RANGE_CHECK_HI(cfg, g_threads,          64);
#if CONFIG_REALTIME_ONLY
    RANGE_CHECK_HI(cfg, g_lag_in_frames,    0);
#elif CONFIG_MULTI_RES_ENCODING
    if (ctx->base.enc.total_encoders > 1)
        RANGE_CHECK_HI(cfg, g_lag_in_frames,    0);
#else
    RANGE_CHECK_HI(cfg, g_lag_in_frames,    25);
#endif
    RANGE_CHECK(cfg, rc_end_usage,          VPX_VBR, VPX_Q);
    RANGE_CHECK_HI(cfg, rc_undershoot_pct,  1000);
    RANGE_CHECK_HI(cfg, rc_overshoot_pct,   1000);
    RANGE_CHECK_HI(cfg, rc_2pass_vbr_bias_pct, 100);
    RANGE_CHECK(cfg, kf_mode,               VPX_KF_DISABLED, VPX_KF_AUTO);

/* TODO: add spatial re-sampling support and frame dropping in
 * multi-res-encoder.*/
#if CONFIG_MULTI_RES_ENCODING
    if (ctx->base.enc.total_encoders > 1)
        RANGE_CHECK_HI(cfg, rc_resize_allowed,     0);
#else
    RANGE_CHECK_BOOL(cfg, rc_resize_allowed);
#endif
    RANGE_CHECK_HI(cfg, rc_dropframe_thresh,   100);
    RANGE_CHECK_HI(cfg, rc_resize_up_thresh,   100);
    RANGE_CHECK_HI(cfg, rc_resize_down_thresh, 100);

#if CONFIG_REALTIME_ONLY
    RANGE_CHECK(cfg,        g_pass,         VPX_RC_ONE_PASS, VPX_RC_ONE_PASS);
#elif CONFIG_MULTI_RES_ENCODING
    if (ctx->base.enc.total_encoders > 1)
        RANGE_CHECK(cfg,    g_pass,         VPX_RC_ONE_PASS, VPX_RC_ONE_PASS);
#else
    RANGE_CHECK(cfg,        g_pass,         VPX_RC_ONE_PASS, VPX_RC_LAST_PASS);
#endif

    /* VP8 does not support a lower bound on the keyframe interval in
     * automatic keyframe placement mode.
     */
    if (cfg->kf_mode != VPX_KF_DISABLED && cfg->kf_min_dist != cfg->kf_max_dist
        && cfg->kf_min_dist > 0)
        ERROR("kf_min_dist not supported in auto mode, use 0 "
              "or kf_max_dist instead.");

    RANGE_CHECK_BOOL(vp8_cfg,               enable_auto_alt_ref);
    RANGE_CHECK(vp8_cfg, cpu_used,           -16, 16);

#if CONFIG_REALTIME_ONLY && !CONFIG_TEMPORAL_DENOISING
    RANGE_CHECK(vp8_cfg, noise_sensitivity,  0, 0);
#else
    RANGE_CHECK_HI(vp8_cfg, noise_sensitivity,  6);
#endif

    RANGE_CHECK(vp8_cfg, token_partitions,   VP8_ONE_TOKENPARTITION,
                VP8_EIGHT_TOKENPARTITION);
    RANGE_CHECK_HI(vp8_cfg, Sharpness,       7);
    RANGE_CHECK(vp8_cfg, arnr_max_frames, 0, 15);
    RANGE_CHECK_HI(vp8_cfg, arnr_strength,   6);
    RANGE_CHECK(vp8_cfg, cq_level, 0, 63);
    if (finalize && (cfg->rc_end_usage == VPX_CQ || cfg->rc_end_usage == VPX_Q))
        RANGE_CHECK(vp8_cfg, cq_level,
                    cfg->rc_min_quantizer, cfg->rc_max_quantizer);

#if !(CONFIG_REALTIME_ONLY)
    if (cfg->g_pass == VPX_RC_LAST_PASS)
    {
        size_t           packet_sz = sizeof(FIRSTPASS_STATS);
        int              n_packets = (int)(cfg->rc_twopass_stats_in.sz /
                                          packet_sz);
        FIRSTPASS_STATS *stats;

        if (!cfg->rc_twopass_stats_in.buf)
            ERROR("rc_twopass_stats_in.buf not set.");

        if (cfg->rc_twopass_stats_in.sz % packet_sz)
            ERROR("rc_twopass_stats_in.sz indicates truncated packet.");

        if (cfg->rc_twopass_stats_in.sz < 2 * packet_sz)
            ERROR("rc_twopass_stats_in requires at least two packets.");

        stats = (void*)((char *)cfg->rc_twopass_stats_in.buf
                + (n_packets - 1) * packet_sz);

        if ((int)(stats->count + 0.5) != n_packets - 1)
            ERROR("rc_twopass_stats_in missing EOS stats packet");
    }
#endif

    RANGE_CHECK(cfg, ts_number_layers, 1, 5);

    if (cfg->ts_number_layers > 1)
    {
        unsigned int i;
        RANGE_CHECK_HI(cfg, ts_periodicity, 16);

        for (i=1; i<cfg->ts_number_layers; i++)
            if (cfg->ts_target_bitrate[i] <= cfg->ts_target_bitrate[i-1])
                ERROR("ts_target_bitrate entries are not strictly increasing");

        RANGE_CHECK(cfg, ts_rate_decimator[cfg->ts_number_layers-1], 1, 1);
        for (i=cfg->ts_number_layers-2; i>0; i--)
            if (cfg->ts_rate_decimator[i-1] != 2*cfg->ts_rate_decimator[i])
                ERROR("ts_rate_decimator factors are not powers of 2");

        RANGE_CHECK_HI(cfg, ts_layer_id[i], cfg->ts_number_layers-1);
    }

#if (CONFIG_REALTIME_ONLY & CONFIG_ONTHEFLY_BITPACKING)
    if(cfg->g_threads > (1 << vp8_cfg->token_partitions))
        ERROR("g_threads cannot be bigger than number of token partitions");
#endif

    return VPX_CODEC_OK;
}


static vpx_codec_err_t validate_img(vpx_codec_alg_priv_t *ctx,
                                    const vpx_image_t    *img)
{
    switch (img->fmt)
    {
    case VPX_IMG_FMT_YV12:
    case VPX_IMG_FMT_I420:
    case VPX_IMG_FMT_VPXI420:
    case VPX_IMG_FMT_VPXYV12:
        break;
    default:
        ERROR("Invalid image format. Only YV12 and I420 images are supported");
    }

    if ((img->d_w != ctx->cfg.g_w) || (img->d_h != ctx->cfg.g_h))
        ERROR("Image size must match encoder init configuration size");

    return VPX_CODEC_OK;
}


static vpx_codec_err_t set_vp8e_config(VP8_CONFIG *oxcf,
                                       vpx_codec_enc_cfg_t cfg,
                                       struct vp8_extracfg vp8_cfg,
                                       vpx_codec_priv_enc_mr_cfg_t *mr_cfg)
{
    oxcf->multi_threaded         = cfg.g_threads;
    oxcf->Version               = cfg.g_profile;

    oxcf->Width                 = cfg.g_w;
    oxcf->Height                = cfg.g_h;
    oxcf->timebase              = cfg.g_timebase;

    oxcf->error_resilient_mode = cfg.g_error_resilient;

    switch (cfg.g_pass)
    {
    case VPX_RC_ONE_PASS:
        oxcf->Mode = MODE_BESTQUALITY;
        break;
    case VPX_RC_FIRST_PASS:
        oxcf->Mode = MODE_FIRSTPASS;
        break;
    case VPX_RC_LAST_PASS:
        oxcf->Mode = MODE_SECONDPASS_BEST;
        break;
    }

    if (cfg.g_pass == VPX_RC_FIRST_PASS || cfg.g_pass == VPX_RC_ONE_PASS)
    {
        oxcf->allow_lag     = 0;
        oxcf->lag_in_frames = 0;
    }
    else
    {
        oxcf->allow_lag     = (cfg.g_lag_in_frames) > 0;
        oxcf->lag_in_frames = cfg.g_lag_in_frames;
    }

    oxcf->allow_df               = (cfg.rc_dropframe_thresh > 0);
    oxcf->drop_frames_water_mark   = cfg.rc_dropframe_thresh;

    oxcf->allow_spatial_resampling = cfg.rc_resize_allowed;
    oxcf->resample_up_water_mark   = cfg.rc_resize_up_thresh;
    oxcf->resample_down_water_mark = cfg.rc_resize_down_thresh;

    if (cfg.rc_end_usage == VPX_VBR) {
      oxcf->end_usage = USAGE_LOCAL_FILE_PLAYBACK;
    } else if (cfg.rc_end_usage == VPX_CBR) {
      oxcf->end_usage = USAGE_STREAM_FROM_SERVER;
    } else if (cfg.rc_end_usage == VPX_CQ) {
      oxcf->end_usage = USAGE_CONSTRAINED_QUALITY;
    } else if (cfg.rc_end_usage == VPX_Q) {
      oxcf->end_usage = USAGE_CONSTANT_QUALITY;
    }

    oxcf->target_bandwidth         = cfg.rc_target_bitrate;
    oxcf->rc_max_intra_bitrate_pct = vp8_cfg.rc_max_intra_bitrate_pct;

    oxcf->best_allowed_q           = cfg.rc_min_quantizer;
    oxcf->worst_allowed_q          = cfg.rc_max_quantizer;
    oxcf->cq_level                 = vp8_cfg.cq_level;
    oxcf->fixed_q = -1;

    oxcf->under_shoot_pct          = cfg.rc_undershoot_pct;
    oxcf->over_shoot_pct           = cfg.rc_overshoot_pct;

    oxcf->maximum_buffer_size_in_ms   = cfg.rc_buf_sz;
    oxcf->starting_buffer_level_in_ms = cfg.rc_buf_initial_sz;
    oxcf->optimal_buffer_level_in_ms  = cfg.rc_buf_optimal_sz;

    oxcf->maximum_buffer_size      = cfg.rc_buf_sz;
    oxcf->starting_buffer_level    = cfg.rc_buf_initial_sz;
    oxcf->optimal_buffer_level     = cfg.rc_buf_optimal_sz;

    oxcf->two_pass_vbrbias         = cfg.rc_2pass_vbr_bias_pct;
    oxcf->two_pass_vbrmin_section  = cfg.rc_2pass_vbr_minsection_pct;
    oxcf->two_pass_vbrmax_section  = cfg.rc_2pass_vbr_maxsection_pct;

    oxcf->auto_key                 = cfg.kf_mode == VPX_KF_AUTO
                                       && cfg.kf_min_dist != cfg.kf_max_dist;
    oxcf->key_freq                 = cfg.kf_max_dist;

    oxcf->number_of_layers         = cfg.ts_number_layers;
    oxcf->periodicity              = cfg.ts_periodicity;

    if (oxcf->number_of_layers > 1)
    {
        memcpy (oxcf->target_bitrate, cfg.ts_target_bitrate,
                          sizeof(cfg.ts_target_bitrate));
        memcpy (oxcf->rate_decimator, cfg.ts_rate_decimator,
                          sizeof(cfg.ts_rate_decimator));
        memcpy (oxcf->layer_id, cfg.ts_layer_id, sizeof(cfg.ts_layer_id));
    }

#if CONFIG_MULTI_RES_ENCODING
    /* When mr_cfg is NULL, oxcf->mr_total_resolutions and oxcf->mr_encoder_id
     * are both memset to 0, which ensures the correct logic under this
     * situation.
     */
    if(mr_cfg)
    {
        oxcf->mr_total_resolutions        = mr_cfg->mr_total_resolutions;
        oxcf->mr_encoder_id               = mr_cfg->mr_encoder_id;
        oxcf->mr_down_sampling_factor.num = mr_cfg->mr_down_sampling_factor.num;
        oxcf->mr_down_sampling_factor.den = mr_cfg->mr_down_sampling_factor.den;
        oxcf->mr_low_res_mode_info        = mr_cfg->mr_low_res_mode_info;
    }
#endif

    oxcf->cpu_used               = vp8_cfg.cpu_used;
    oxcf->encode_breakout        = vp8_cfg.static_thresh;
    oxcf->play_alternate         = vp8_cfg.enable_auto_alt_ref;
    oxcf->noise_sensitivity      = vp8_cfg.noise_sensitivity;
    oxcf->Sharpness              = vp8_cfg.Sharpness;
    oxcf->token_partitions       = vp8_cfg.token_partitions;

    oxcf->two_pass_stats_in      = cfg.rc_twopass_stats_in;
    oxcf->output_pkt_list        = vp8_cfg.pkt_list;

    oxcf->arnr_max_frames        = vp8_cfg.arnr_max_frames;
    oxcf->arnr_strength          = vp8_cfg.arnr_strength;

    oxcf->tuning                 = vp8_cfg.tuning;

    /*
        printf("Current VP8 Settings: \n");
        printf("target_bandwidth: %d\n", oxcf->target_bandwidth);
        printf("noise_sensitivity: %d\n", oxcf->noise_sensitivity);
        printf("Sharpness: %d\n",    oxcf->Sharpness);
        printf("cpu_used: %d\n",  oxcf->cpu_used);
        printf("Mode: %d\n",     oxcf->Mode);
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
        printf("allow_spatial_resampling: %d\n",  oxcf->allow_spatial_resampling);
        printf("resample_down_water_mark: %d\n", oxcf->resample_down_water_mark);
        printf("resample_up_water_mark: %d\n", oxcf->resample_up_water_mark);
        printf("allow_df: %d\n", oxcf->allow_df);
        printf("drop_frames_water_mark: %d\n", oxcf->drop_frames_water_mark);
        printf("two_pass_vbrbias: %d\n",  oxcf->two_pass_vbrbias);
        printf("two_pass_vbrmin_section: %d\n", oxcf->two_pass_vbrmin_section);
        printf("two_pass_vbrmax_section: %d\n", oxcf->two_pass_vbrmax_section);
        printf("allow_lag: %d\n", oxcf->allow_lag);
        printf("lag_in_frames: %d\n", oxcf->lag_in_frames);
        printf("play_alternate: %d\n", oxcf->play_alternate);
        printf("Version: %d\n", oxcf->Version);
        printf("multi_threaded: %d\n",   oxcf->multi_threaded);
        printf("encode_breakout: %d\n", oxcf->encode_breakout);
    */
    return VPX_CODEC_OK;
}

static vpx_codec_err_t vp8e_set_config(vpx_codec_alg_priv_t       *ctx,
                                       const vpx_codec_enc_cfg_t  *cfg)
{
    vpx_codec_err_t res;

    if (((cfg->g_w != ctx->cfg.g_w) || (cfg->g_h != ctx->cfg.g_h))
        && (cfg->g_lag_in_frames > 1 || cfg->g_pass != VPX_RC_ONE_PASS))
        ERROR("Cannot change width or height after initialization");

    /* Prevent increasing lag_in_frames. This check is stricter than it needs
     * to be -- the limit is not increasing past the first lag_in_frames
     * value, but we don't track the initial config, only the last successful
     * config.
     */
    if ((cfg->g_lag_in_frames > ctx->cfg.g_lag_in_frames))
        ERROR("Cannot increase lag_in_frames");

    res = validate_config(ctx, cfg, &ctx->vp8_cfg, 0);

    if (!res)
    {
        ctx->cfg = *cfg;
        set_vp8e_config(&ctx->oxcf, ctx->cfg, ctx->vp8_cfg, NULL);
        vp8_change_config(ctx->cpi, &ctx->oxcf);
    }

    return res;
}


int vp8_reverse_trans(int);


static vpx_codec_err_t get_param(vpx_codec_alg_priv_t *ctx,
                                 int                   ctrl_id,
                                 va_list               args)
{
    void *arg = va_arg(args, void *);

#define MAP(id, var) case id: *(RECAST(id, arg)) = var; break

    if (!arg)
        return VPX_CODEC_INVALID_PARAM;

    switch (ctrl_id)
    {
        MAP(VP8E_GET_LAST_QUANTIZER, vp8_get_quantizer(ctx->cpi));
        MAP(VP8E_GET_LAST_QUANTIZER_64, vp8_reverse_trans(vp8_get_quantizer(ctx->cpi)));
    }

    return VPX_CODEC_OK;
#undef MAP
}


static vpx_codec_err_t set_param(vpx_codec_alg_priv_t *ctx,
                                 int                   ctrl_id,
                                 va_list               args)
{
    vpx_codec_err_t     res  = VPX_CODEC_OK;
    struct vp8_extracfg xcfg = ctx->vp8_cfg;

#define MAP(id, var) case id: var = CAST(id, args); break;

    switch (ctrl_id)
    {
        MAP(VP8E_SET_CPUUSED,               xcfg.cpu_used);
        MAP(VP8E_SET_ENABLEAUTOALTREF,      xcfg.enable_auto_alt_ref);
        MAP(VP8E_SET_NOISE_SENSITIVITY,     xcfg.noise_sensitivity);
        MAP(VP8E_SET_SHARPNESS,             xcfg.Sharpness);
        MAP(VP8E_SET_STATIC_THRESHOLD,      xcfg.static_thresh);
        MAP(VP8E_SET_TOKEN_PARTITIONS,      xcfg.token_partitions);

        MAP(VP8E_SET_ARNR_MAXFRAMES,        xcfg.arnr_max_frames);
        MAP(VP8E_SET_ARNR_STRENGTH ,        xcfg.arnr_strength);
        MAP(VP8E_SET_TUNING,                xcfg.tuning);
        MAP(VP8E_SET_CQ_LEVEL,              xcfg.cq_level);
        MAP(VP8E_SET_MAX_INTRA_BITRATE_PCT, xcfg.rc_max_intra_bitrate_pct);

    }

    res = validate_config(ctx, &ctx->cfg, &xcfg, 0);

    if (!res)
    {
        ctx->vp8_cfg = xcfg;
        set_vp8e_config(&ctx->oxcf, ctx->cfg, ctx->vp8_cfg, NULL);
        vp8_change_config(ctx->cpi, &ctx->oxcf);
    }

    return res;
#undef MAP
}

static vpx_codec_err_t vp8e_mr_alloc_mem(const vpx_codec_enc_cfg_t *cfg,
                                        void **mem_loc)
{
    vpx_codec_err_t res = 0;

#if CONFIG_MULTI_RES_ENCODING
    LOWER_RES_FRAME_INFO *shared_mem_loc;
    int mb_rows = ((cfg->g_w + 15) >>4);
    int mb_cols = ((cfg->g_h + 15) >>4);

    shared_mem_loc = calloc(1, sizeof(LOWER_RES_FRAME_INFO));
    if(!shared_mem_loc)
    {
        res = VPX_CODEC_MEM_ERROR;
    }

    shared_mem_loc->mb_info = calloc(mb_rows*mb_cols, sizeof(LOWER_RES_MB_INFO));
    if(!(shared_mem_loc->mb_info))
    {
        res = VPX_CODEC_MEM_ERROR;
    }
    else
    {
        *mem_loc = (void *)shared_mem_loc;
        res = VPX_CODEC_OK;
    }
#endif
    return res;
}

static vpx_codec_err_t vp8e_init(vpx_codec_ctx_t *ctx,
                                 vpx_codec_priv_enc_mr_cfg_t *mr_cfg)
{
    vpx_codec_err_t        res = VPX_CODEC_OK;
    struct vpx_codec_alg_priv *priv;
    vpx_codec_enc_cfg_t       *cfg;
    unsigned int               i;

    struct VP8_COMP *optr;

    vp8_rtcd();

    if (!ctx->priv)
    {
        priv = calloc(1, sizeof(struct vpx_codec_alg_priv));

        if (!priv)
        {
            return VPX_CODEC_MEM_ERROR;
        }

        ctx->priv = &priv->base;
        ctx->priv->sz = sizeof(*ctx->priv);
        ctx->priv->iface = ctx->iface;
        ctx->priv->alg_priv = priv;
        ctx->priv->init_flags = ctx->init_flags;

        if (ctx->config.enc)
        {
            /* Update the reference to the config structure to an
             * internal copy.
             */
            ctx->priv->alg_priv->cfg = *ctx->config.enc;
            ctx->config.enc = &ctx->priv->alg_priv->cfg;
        }

        cfg =  &ctx->priv->alg_priv->cfg;

        /* Select the extra vp8 configuration table based on the current
         * usage value. If the current usage value isn't found, use the
         * values for usage case 0.
         */
        for (i = 0;
             extracfg_map[i].usage && extracfg_map[i].usage != cfg->g_usage;
             i++);

        priv->vp8_cfg = extracfg_map[i].cfg;
        priv->vp8_cfg.pkt_list = &priv->pkt_list.head;

        priv->cx_data_sz = priv->cfg.g_w * priv->cfg.g_h * 3 / 2 * 2;

        if (priv->cx_data_sz < 32768) priv->cx_data_sz = 32768;

        priv->cx_data = malloc(priv->cx_data_sz);

        if (!priv->cx_data)
        {
            return VPX_CODEC_MEM_ERROR;
        }

        if(mr_cfg)
            ctx->priv->enc.total_encoders   = mr_cfg->mr_total_resolutions;
        else
            ctx->priv->enc.total_encoders   = 1;

        res = validate_config(priv, &priv->cfg, &priv->vp8_cfg, 0);

        if (!res)
        {
            set_vp8e_config(&ctx->priv->alg_priv->oxcf,
                             ctx->priv->alg_priv->cfg,
                             ctx->priv->alg_priv->vp8_cfg,
                             mr_cfg);

            optr = vp8_create_compressor(&ctx->priv->alg_priv->oxcf);

            if (!optr)
                res = VPX_CODEC_MEM_ERROR;
            else
                ctx->priv->alg_priv->cpi = optr;
        }
    }

    return res;
}

static vpx_codec_err_t vp8e_destroy(vpx_codec_alg_priv_t *ctx)
{
#if CONFIG_MULTI_RES_ENCODING
    /* Free multi-encoder shared memory */
    if (ctx->oxcf.mr_total_resolutions > 0 && (ctx->oxcf.mr_encoder_id == ctx->oxcf.mr_total_resolutions-1))
    {
        LOWER_RES_FRAME_INFO *shared_mem_loc = (LOWER_RES_FRAME_INFO *)ctx->oxcf.mr_low_res_mode_info;
        free(shared_mem_loc->mb_info);
        free(ctx->oxcf.mr_low_res_mode_info);
    }
#endif

    free(ctx->cx_data);
    vp8_remove_compressor(&ctx->cpi);
    free(ctx);
    return VPX_CODEC_OK;
}

static vpx_codec_err_t image2yuvconfig(const vpx_image_t   *img,
                                       YV12_BUFFER_CONFIG  *yv12)
{
    vpx_codec_err_t        res = VPX_CODEC_OK;
    yv12->y_buffer = img->planes[VPX_PLANE_Y];
    yv12->u_buffer = img->planes[VPX_PLANE_U];
    yv12->v_buffer = img->planes[VPX_PLANE_V];

    yv12->y_crop_width  = img->d_w;
    yv12->y_crop_height = img->d_h;
    yv12->y_width  = img->d_w;
    yv12->y_height = img->d_h;
    yv12->uv_width = (1 + yv12->y_width) / 2;
    yv12->uv_height = (1 + yv12->y_height) / 2;

    yv12->y_stride = img->stride[VPX_PLANE_Y];
    yv12->uv_stride = img->stride[VPX_PLANE_U];

    yv12->border  = (img->stride[VPX_PLANE_Y] - img->w) / 2;
    return res;
}

static void pick_quickcompress_mode(vpx_codec_alg_priv_t  *ctx,
                                    unsigned long          duration,
                                    unsigned long          deadline)
{
    unsigned int new_qc;

#if !(CONFIG_REALTIME_ONLY)
    /* Use best quality mode if no deadline is given. */
    new_qc = MODE_BESTQUALITY;

    if (deadline)
    {
        uint64_t     duration_us;

        /* Convert duration parameter from stream timebase to microseconds */
        duration_us = (uint64_t)duration * 1000000
                      * (uint64_t)ctx->cfg.g_timebase.num
                      / (uint64_t)ctx->cfg.g_timebase.den;

        /* If the deadline is more that the duration this frame is to be shown,
         * use good quality mode. Otherwise use realtime mode.
         */
        new_qc = (deadline > duration_us) ? MODE_GOODQUALITY : MODE_REALTIME;
    }

#else
    new_qc = MODE_REALTIME;
#endif

    if (ctx->cfg.g_pass == VPX_RC_FIRST_PASS)
        new_qc = MODE_FIRSTPASS;
    else if (ctx->cfg.g_pass == VPX_RC_LAST_PASS)
        new_qc = (new_qc == MODE_BESTQUALITY)
                 ? MODE_SECONDPASS_BEST
                 : MODE_SECONDPASS;

    if (ctx->oxcf.Mode != new_qc)
    {
        ctx->oxcf.Mode = new_qc;
        vp8_change_config(ctx->cpi, &ctx->oxcf);
    }
}


static vpx_codec_err_t vp8e_encode(vpx_codec_alg_priv_t  *ctx,
                                   const vpx_image_t     *img,
                                   vpx_codec_pts_t        pts,
                                   unsigned long          duration,
                                   vpx_enc_frame_flags_t  flags,
                                   unsigned long          deadline)
{
    vpx_codec_err_t res = VPX_CODEC_OK;

    if (!ctx->cfg.rc_target_bitrate)
        return res;

    if (img)
        res = validate_img(ctx, img);

    if (!res)
        res = validate_config(ctx, &ctx->cfg, &ctx->vp8_cfg, 1);

    pick_quickcompress_mode(ctx, duration, deadline);
    vpx_codec_pkt_list_init(&ctx->pkt_list);

    /* Handle Flags */
    if (((flags & VP8_EFLAG_NO_UPD_GF) && (flags & VP8_EFLAG_FORCE_GF))
        || ((flags & VP8_EFLAG_NO_UPD_ARF) && (flags & VP8_EFLAG_FORCE_ARF)))
    {
        ctx->base.err_detail = "Conflicting flags.";
        return VPX_CODEC_INVALID_PARAM;
    }

    if (flags & (VP8_EFLAG_NO_REF_LAST | VP8_EFLAG_NO_REF_GF
                 | VP8_EFLAG_NO_REF_ARF))
    {
        int ref = 7;

        if (flags & VP8_EFLAG_NO_REF_LAST)
            ref ^= VP8_LAST_FRAME;

        if (flags & VP8_EFLAG_NO_REF_GF)
            ref ^= VP8_GOLD_FRAME;

        if (flags & VP8_EFLAG_NO_REF_ARF)
            ref ^= VP8_ALTR_FRAME;

        vp8_use_as_reference(ctx->cpi, ref);
    }

    if (flags & (VP8_EFLAG_NO_UPD_LAST | VP8_EFLAG_NO_UPD_GF
                 | VP8_EFLAG_NO_UPD_ARF | VP8_EFLAG_FORCE_GF
                 | VP8_EFLAG_FORCE_ARF))
    {
        int upd = 7;

        if (flags & VP8_EFLAG_NO_UPD_LAST)
            upd ^= VP8_LAST_FRAME;

        if (flags & VP8_EFLAG_NO_UPD_GF)
            upd ^= VP8_GOLD_FRAME;

        if (flags & VP8_EFLAG_NO_UPD_ARF)
            upd ^= VP8_ALTR_FRAME;

        vp8_update_reference(ctx->cpi, upd);
    }

    if (flags & VP8_EFLAG_NO_UPD_ENTROPY)
    {
        vp8_update_entropy(ctx->cpi, 0);
    }

    /* Handle fixed keyframe intervals */
    if (ctx->cfg.kf_mode == VPX_KF_AUTO
        && ctx->cfg.kf_min_dist == ctx->cfg.kf_max_dist)
    {
        if (++ctx->fixed_kf_cntr > ctx->cfg.kf_min_dist)
        {
            flags |= VPX_EFLAG_FORCE_KF;
            ctx->fixed_kf_cntr = 1;
        }
    }

    /* Initialize the encoder instance on the first frame*/
    if (!res && ctx->cpi)
    {
        unsigned int lib_flags;
        YV12_BUFFER_CONFIG sd;
        int64_t dst_time_stamp, dst_end_time_stamp;
        unsigned long size, cx_data_sz;
        unsigned char *cx_data;
        unsigned char *cx_data_end;
        int comp_data_state = 0;

        /* Set up internal flags */
        if (ctx->base.init_flags & VPX_CODEC_USE_PSNR)
            ((VP8_COMP *)ctx->cpi)->b_calculate_psnr = 1;

        if (ctx->base.init_flags & VPX_CODEC_USE_OUTPUT_PARTITION)
            ((VP8_COMP *)ctx->cpi)->output_partition = 1;

        /* Convert API flags to internal codec lib flags */
        lib_flags = (flags & VPX_EFLAG_FORCE_KF) ? FRAMEFLAGS_KEY : 0;

        /* vp8 use 10,000,000 ticks/second as time stamp */
        dst_time_stamp    = pts * 10000000 * ctx->cfg.g_timebase.num / ctx->cfg.g_timebase.den;
        dst_end_time_stamp = (pts + duration) * 10000000 * ctx->cfg.g_timebase.num / ctx->cfg.g_timebase.den;

        if (img != NULL)
        {
            res = image2yuvconfig(img, &sd);

            if (vp8_receive_raw_frame(ctx->cpi, ctx->next_frame_flag | lib_flags,
                                      &sd, dst_time_stamp, dst_end_time_stamp))
            {
                VP8_COMP *cpi = (VP8_COMP *)ctx->cpi;
                res = update_error_state(ctx, &cpi->common.error);
            }

            /* reset for next frame */
            ctx->next_frame_flag = 0;
        }

        cx_data = ctx->cx_data;
        cx_data_sz = ctx->cx_data_sz;
        cx_data_end = ctx->cx_data + cx_data_sz;
        lib_flags = 0;

        while (cx_data_sz >= ctx->cx_data_sz / 2)
        {
            comp_data_state = vp8_get_compressed_data(ctx->cpi,
                                                  &lib_flags,
                                                  &size,
                                                  cx_data,
                                                  cx_data_end,
                                                  &dst_time_stamp,
                                                  &dst_end_time_stamp,
                                                  !img);

            if(comp_data_state == VPX_CODEC_CORRUPT_FRAME)
                return VPX_CODEC_CORRUPT_FRAME;
            else if(comp_data_state == -1)
                break;

            if (size)
            {
                vpx_codec_pts_t    round, delta;
                vpx_codec_cx_pkt_t pkt;
                VP8_COMP *cpi = (VP8_COMP *)ctx->cpi;

                /* Add the frame packet to the list of returned packets. */
                round = (vpx_codec_pts_t)1000000
                        * ctx->cfg.g_timebase.num / 2 - 1;
                delta = (dst_end_time_stamp - dst_time_stamp);
                pkt.kind = VPX_CODEC_CX_FRAME_PKT;
                pkt.data.frame.pts =
                    (dst_time_stamp * ctx->cfg.g_timebase.den + round)
                    / ctx->cfg.g_timebase.num / 10000000;
                pkt.data.frame.duration = (unsigned long)
                    ((delta * ctx->cfg.g_timebase.den + round)
                    / ctx->cfg.g_timebase.num / 10000000);
                pkt.data.frame.flags = lib_flags << 16;

                if (lib_flags & FRAMEFLAGS_KEY)
                    pkt.data.frame.flags |= VPX_FRAME_IS_KEY;

                if (!cpi->common.show_frame)
                {
                    pkt.data.frame.flags |= VPX_FRAME_IS_INVISIBLE;

                    /* This timestamp should be as close as possible to the
                     * prior PTS so that if a decoder uses pts to schedule when
                     * to do this, we start right after last frame was decoded.
                     * Invisible frames have no duration.
                     */
                    pkt.data.frame.pts = ((cpi->last_time_stamp_seen
                        * ctx->cfg.g_timebase.den + round)
                        / ctx->cfg.g_timebase.num / 10000000) + 1;
                    pkt.data.frame.duration = 0;
                }

                if (cpi->droppable)
                    pkt.data.frame.flags |= VPX_FRAME_IS_DROPPABLE;

                if (cpi->output_partition)
                {
                    int i;
                    const int num_partitions =
                            (1 << cpi->common.multi_token_partition) + 1;

                    pkt.data.frame.flags |= VPX_FRAME_IS_FRAGMENT;

                    for (i = 0; i < num_partitions; ++i)
                    {
#if CONFIG_REALTIME_ONLY & CONFIG_ONTHEFLY_BITPACKING
                        pkt.data.frame.buf = cpi->partition_d[i];
#else
                        pkt.data.frame.buf = cx_data;
                        cx_data += cpi->partition_sz[i];
                        cx_data_sz -= cpi->partition_sz[i];
#endif
                        pkt.data.frame.sz = cpi->partition_sz[i];
                        pkt.data.frame.partition_id = i;
                        /* don't set the fragment bit for the last partition */
                        if (i == (num_partitions - 1))
                            pkt.data.frame.flags &= ~VPX_FRAME_IS_FRAGMENT;
                        vpx_codec_pkt_list_add(&ctx->pkt_list.head, &pkt);
                    }
#if CONFIG_REALTIME_ONLY & CONFIG_ONTHEFLY_BITPACKING
                    /* In lagged mode the encoder can buffer multiple frames.
                     * We don't want this in partitioned output because
                     * partitions are spread all over the output buffer.
                     * So, force an exit!
                     */
                    cx_data_sz -= ctx->cx_data_sz / 2;
#endif
                }
                else
                {
                    pkt.data.frame.buf = cx_data;
                    pkt.data.frame.sz  = size;
                    pkt.data.frame.partition_id = -1;
                    vpx_codec_pkt_list_add(&ctx->pkt_list.head, &pkt);
                    cx_data += size;
                    cx_data_sz -= size;
                }
            }
        }
    }

    return res;
}


static const vpx_codec_cx_pkt_t *vp8e_get_cxdata(vpx_codec_alg_priv_t  *ctx,
        vpx_codec_iter_t      *iter)
{
    return vpx_codec_pkt_list_get(&ctx->pkt_list.head, iter);
}

static vpx_codec_err_t vp8e_set_reference(vpx_codec_alg_priv_t *ctx,
        int ctr_id,
        va_list args)
{
    vpx_ref_frame_t *data = va_arg(args, vpx_ref_frame_t *);

    if (data)
    {
        vpx_ref_frame_t *frame = (vpx_ref_frame_t *)data;
        YV12_BUFFER_CONFIG sd;

        image2yuvconfig(&frame->img, &sd);
        vp8_set_reference(ctx->cpi, frame->frame_type, &sd);
        return VPX_CODEC_OK;
    }
    else
        return VPX_CODEC_INVALID_PARAM;

}

static vpx_codec_err_t vp8e_get_reference(vpx_codec_alg_priv_t *ctx,
        int ctr_id,
        va_list args)
{

    vpx_ref_frame_t *data = va_arg(args, vpx_ref_frame_t *);

    if (data)
    {
        vpx_ref_frame_t *frame = (vpx_ref_frame_t *)data;
        YV12_BUFFER_CONFIG sd;

        image2yuvconfig(&frame->img, &sd);
        vp8_get_reference(ctx->cpi, frame->frame_type, &sd);
        return VPX_CODEC_OK;
    }
    else
        return VPX_CODEC_INVALID_PARAM;
}

static vpx_codec_err_t vp8e_set_previewpp(vpx_codec_alg_priv_t *ctx,
        int ctr_id,
        va_list args)
{
#if CONFIG_POSTPROC
    vp8_postproc_cfg_t *data = va_arg(args, vp8_postproc_cfg_t *);
    (void)ctr_id;

    if (data)
    {
        ctx->preview_ppcfg = *((vp8_postproc_cfg_t *)data);
        return VPX_CODEC_OK;
    }
    else
        return VPX_CODEC_INVALID_PARAM;
#else
    (void)ctx;
    (void)ctr_id;
    (void)args;
    return VPX_CODEC_INCAPABLE;
#endif
}


static vpx_image_t *vp8e_get_preview(vpx_codec_alg_priv_t *ctx)
{

    YV12_BUFFER_CONFIG sd;
    vp8_ppflags_t flags = {0};

    if (ctx->preview_ppcfg.post_proc_flag)
    {
        flags.post_proc_flag        = ctx->preview_ppcfg.post_proc_flag;
        flags.deblocking_level      = ctx->preview_ppcfg.deblocking_level;
        flags.noise_level           = ctx->preview_ppcfg.noise_level;
    }

    if (0 == vp8_get_preview_raw_frame(ctx->cpi, &sd, &flags))
    {

        /*
        vpx_img_wrap(&ctx->preview_img, VPX_IMG_FMT_YV12,
            sd.y_width + 2*VP8BORDERINPIXELS,
            sd.y_height + 2*VP8BORDERINPIXELS,
            1,
            sd.buffer_alloc);
        vpx_img_set_rect(&ctx->preview_img,
            VP8BORDERINPIXELS, VP8BORDERINPIXELS,
            sd.y_width, sd.y_height);
            */

        ctx->preview_img.bps = 12;
        ctx->preview_img.planes[VPX_PLANE_Y] = sd.y_buffer;
        ctx->preview_img.planes[VPX_PLANE_U] = sd.u_buffer;
        ctx->preview_img.planes[VPX_PLANE_V] = sd.v_buffer;

        ctx->preview_img.fmt = VPX_IMG_FMT_I420;
        ctx->preview_img.x_chroma_shift = 1;
        ctx->preview_img.y_chroma_shift = 1;

        ctx->preview_img.d_w = sd.y_width;
        ctx->preview_img.d_h = sd.y_height;
        ctx->preview_img.stride[VPX_PLANE_Y] = sd.y_stride;
        ctx->preview_img.stride[VPX_PLANE_U] = sd.uv_stride;
        ctx->preview_img.stride[VPX_PLANE_V] = sd.uv_stride;
        ctx->preview_img.w   = sd.y_width;
        ctx->preview_img.h   = sd.y_height;

        return &ctx->preview_img;
    }
    else
        return NULL;
}

static vpx_codec_err_t vp8e_update_entropy(vpx_codec_alg_priv_t *ctx,
        int ctr_id,
        va_list args)
{
    int update = va_arg(args, int);
    vp8_update_entropy(ctx->cpi, update);
    return VPX_CODEC_OK;

}

static vpx_codec_err_t vp8e_update_reference(vpx_codec_alg_priv_t *ctx,
        int ctr_id,
        va_list args)
{
    int update = va_arg(args, int);
    vp8_update_reference(ctx->cpi, update);
    return VPX_CODEC_OK;
}

static vpx_codec_err_t vp8e_use_reference(vpx_codec_alg_priv_t *ctx,
        int ctr_id,
        va_list args)
{
    int reference_flag = va_arg(args, int);
    vp8_use_as_reference(ctx->cpi, reference_flag);
    return VPX_CODEC_OK;
}

static vpx_codec_err_t vp8e_set_roi_map(vpx_codec_alg_priv_t *ctx,
                                        int ctr_id,
                                        va_list args)
{
    vpx_roi_map_t *data = va_arg(args, vpx_roi_map_t *);

    if (data)
    {
        vpx_roi_map_t *roi = (vpx_roi_map_t *)data;

        if (!vp8_set_roimap(ctx->cpi, roi->roi_map, roi->rows, roi->cols, roi->delta_q, roi->delta_lf, roi->static_threshold))
            return VPX_CODEC_OK;
        else
            return VPX_CODEC_INVALID_PARAM;
    }
    else
        return VPX_CODEC_INVALID_PARAM;
}


static vpx_codec_err_t vp8e_set_activemap(vpx_codec_alg_priv_t *ctx,
        int ctr_id,
        va_list args)
{
    vpx_active_map_t *data = va_arg(args, vpx_active_map_t *);

    if (data)
    {

        vpx_active_map_t *map = (vpx_active_map_t *)data;

        if (!vp8_set_active_map(ctx->cpi, map->active_map, map->rows, map->cols))
            return VPX_CODEC_OK;
        else
            return VPX_CODEC_INVALID_PARAM;
    }
    else
        return VPX_CODEC_INVALID_PARAM;
}

static vpx_codec_err_t vp8e_set_scalemode(vpx_codec_alg_priv_t *ctx,
        int ctr_id,
        va_list args)
{

    vpx_scaling_mode_t *data =  va_arg(args, vpx_scaling_mode_t *);

    if (data)
    {
        int res;
        vpx_scaling_mode_t scalemode = *(vpx_scaling_mode_t *)data ;
        res = vp8_set_internal_size(ctx->cpi,
                                    (VPX_SCALING)scalemode.h_scaling_mode,
                                    (VPX_SCALING)scalemode.v_scaling_mode);

        if (!res)
        {
            /*force next frame a key frame to effect scaling mode */
            ctx->next_frame_flag |= FRAMEFLAGS_KEY;
            return VPX_CODEC_OK;
        }
        else
            return VPX_CODEC_INVALID_PARAM;
    }
    else
        return VPX_CODEC_INVALID_PARAM;
}


static vpx_codec_ctrl_fn_map_t vp8e_ctf_maps[] =
{
    {VP8_SET_REFERENCE,                 vp8e_set_reference},
    {VP8_COPY_REFERENCE,                vp8e_get_reference},
    {VP8_SET_POSTPROC,                  vp8e_set_previewpp},
    {VP8E_UPD_ENTROPY,                  vp8e_update_entropy},
    {VP8E_UPD_REFERENCE,                vp8e_update_reference},
    {VP8E_USE_REFERENCE,                vp8e_use_reference},
    {VP8E_SET_ROI_MAP,                  vp8e_set_roi_map},
    {VP8E_SET_ACTIVEMAP,                vp8e_set_activemap},
    {VP8E_SET_SCALEMODE,                vp8e_set_scalemode},
    {VP8E_SET_CPUUSED,                  set_param},
    {VP8E_SET_NOISE_SENSITIVITY,        set_param},
    {VP8E_SET_ENABLEAUTOALTREF,         set_param},
    {VP8E_SET_SHARPNESS,                set_param},
    {VP8E_SET_STATIC_THRESHOLD,         set_param},
    {VP8E_SET_TOKEN_PARTITIONS,         set_param},
    {VP8E_GET_LAST_QUANTIZER,           get_param},
    {VP8E_GET_LAST_QUANTIZER_64,        get_param},
    {VP8E_SET_ARNR_MAXFRAMES,           set_param},
    {VP8E_SET_ARNR_STRENGTH ,           set_param},
    {VP8E_SET_TUNING,                   set_param},
    {VP8E_SET_CQ_LEVEL,                 set_param},
    {VP8E_SET_MAX_INTRA_BITRATE_PCT,    set_param},
    { -1, NULL},
};

static vpx_codec_enc_cfg_map_t vp8e_usage_cfg_map[] =
{
    {
    0,
    {
        0,                  /* g_usage */
        0,                  /* g_threads */
        0,                  /* g_profile */

        320,                /* g_width */
        240,                /* g_height */
        {1, 30},            /* g_timebase */

        0,                  /* g_error_resilient */

        VPX_RC_ONE_PASS,    /* g_pass */

        0,                  /* g_lag_in_frames */

        0,                  /* rc_dropframe_thresh */
        0,                  /* rc_resize_allowed */
        1,                  /* rc_scaled_width */
        1,                  /* rc_scaled_height */
        60,                 /* rc_resize_down_thresold */
        30,                 /* rc_resize_up_thresold */

        VPX_VBR,            /* rc_end_usage */
#if VPX_ENCODER_ABI_VERSION > (1 + VPX_CODEC_ABI_VERSION)
        {0},                /* rc_twopass_stats_in */
#endif
        256,                /* rc_target_bandwidth */
        4,                  /* rc_min_quantizer */
        63,                 /* rc_max_quantizer */
        100,                /* rc_undershoot_pct */
        100,                /* rc_overshoot_pct */

        6000,               /* rc_max_buffer_size */
        4000,               /* rc_buffer_initial_size; */
        5000,               /* rc_buffer_optimal_size; */

        50,                 /* rc_two_pass_vbrbias  */
        0,                  /* rc_two_pass_vbrmin_section */
        400,                /* rc_two_pass_vbrmax_section */

        /* keyframing settings (kf) */
        VPX_KF_AUTO,        /* g_kfmode*/
        0,                  /* kf_min_dist */
        128,                /* kf_max_dist */

#if VPX_ENCODER_ABI_VERSION == (1 + VPX_CODEC_ABI_VERSION)
        "vp8.fpf"           /* first pass filename */
#endif
        VPX_SS_DEFAULT_LAYERS, /* ss_number_layers */
        {0},                /* ss_target_bitrate */
        1,                  /* ts_number_layers */
        {0},                /* ts_target_bitrate */
        {0},                /* ts_rate_decimator */
        0,                  /* ts_periodicity */
        {0},                /* ts_layer_id */
    }},
    { -1, {NOT_IMPLEMENTED}}
};


#ifndef VERSION_STRING
#define VERSION_STRING
#endif
CODEC_INTERFACE(vpx_codec_vp8_cx) =
{
    "WebM Project VP8 Encoder" VERSION_STRING,
    VPX_CODEC_INTERNAL_ABI_VERSION,
    VPX_CODEC_CAP_ENCODER | VPX_CODEC_CAP_PSNR |
    VPX_CODEC_CAP_OUTPUT_PARTITION,
    /* vpx_codec_caps_t          caps; */
    vp8e_init,          /* vpx_codec_init_fn_t       init; */
    vp8e_destroy,       /* vpx_codec_destroy_fn_t    destroy; */
    vp8e_ctf_maps,      /* vpx_codec_ctrl_fn_map_t  *ctrl_maps; */
    NOT_IMPLEMENTED,    /* vpx_codec_get_mmap_fn_t   get_mmap; */
    NOT_IMPLEMENTED,    /* vpx_codec_set_mmap_fn_t   set_mmap; */
    {
        NOT_IMPLEMENTED,    /* vpx_codec_peek_si_fn_t    peek_si; */
        NOT_IMPLEMENTED,    /* vpx_codec_get_si_fn_t     get_si; */
        NOT_IMPLEMENTED,    /* vpx_codec_decode_fn_t     decode; */
        NOT_IMPLEMENTED,    /* vpx_codec_frame_get_fn_t  frame_get; */
    },
    {
        vp8e_usage_cfg_map, /* vpx_codec_enc_cfg_map_t    peek_si; */
        vp8e_encode,        /* vpx_codec_encode_fn_t      encode; */
        vp8e_get_cxdata,    /* vpx_codec_get_cx_data_fn_t   frame_get; */
        vp8e_set_config,
        NOT_IMPLEMENTED,
        vp8e_get_preview,
        vp8e_mr_alloc_mem,
    } /* encoder functions */
};
