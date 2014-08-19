sub vp9_common_forward_decls() {
print <<EOF
/*
 * VP9
 */

#include "vpx/vpx_integer.h"
#include "vp9/common/vp9_enums.h"
#include "vp9/common/vp9_idct.h"

struct macroblockd;

/* Encoder forward decls */
struct macroblock;
struct vp9_variance_vtable;
struct search_site_config;
struct mv;
union int_mv;
struct yv12_buffer_config;
EOF
}
forward_decls qw/vp9_common_forward_decls/;

# x86inc.asm doesn't work if pic is enabled on 32 bit platforms so no assembly.
if (vpx_config("CONFIG_USE_X86INC") eq "yes") {
  $mmx_x86inc = 'mmx';
  $sse_x86inc = 'sse';
  $sse2_x86inc = 'sse2';
  $ssse3_x86inc = 'ssse3';
  $avx_x86inc = 'avx';
  $avx2_x86inc = 'avx2';
} else {
  $mmx_x86inc = $sse_x86inc = $sse2_x86inc = $ssse3_x86inc =
  $avx_x86inc = $avx2_x86inc = '';
}

# this variable is for functions that are 64 bit only.
if ($opts{arch} eq "x86_64") {
  $mmx_x86_64 = 'mmx';
  $sse2_x86_64 = 'sse2';
  $ssse3_x86_64 = 'ssse3';
  $avx_x86_64 = 'avx';
  $avx2_x86_64 = 'avx2';
} else {
  $mmx_x86_64 = $sse2_x86_64 = $ssse3_x86_64 =
  $avx_x86_64 = $avx2_x86_64 = '';
}

#
# RECON
#
add_proto qw/void vp9_d207_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d207_predictor_4x4/, "$ssse3_x86inc";

add_proto qw/void vp9_d45_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d45_predictor_4x4/, "$ssse3_x86inc";

add_proto qw/void vp9_d63_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d63_predictor_4x4/, "$ssse3_x86inc";

add_proto qw/void vp9_h_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_h_predictor_4x4 neon_asm dspr2/, "$ssse3_x86inc";
$vp9_h_predictor_4x4_neon_asm=vp9_h_predictor_4x4_neon;

add_proto qw/void vp9_d117_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d117_predictor_4x4/;

add_proto qw/void vp9_d135_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d135_predictor_4x4/;

add_proto qw/void vp9_d153_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d153_predictor_4x4/, "$ssse3_x86inc";

add_proto qw/void vp9_v_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_v_predictor_4x4 neon_asm/, "$sse_x86inc";
$vp9_v_predictor_4x4_neon_asm=vp9_v_predictor_4x4_neon;

add_proto qw/void vp9_tm_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_tm_predictor_4x4 neon_asm dspr2/, "$sse_x86inc";
$vp9_tm_predictor_4x4_neon_asm=vp9_tm_predictor_4x4_neon;

add_proto qw/void vp9_dc_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_predictor_4x4 dspr2/, "$sse_x86inc";

add_proto qw/void vp9_dc_top_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_top_predictor_4x4/;

add_proto qw/void vp9_dc_left_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_left_predictor_4x4/;

add_proto qw/void vp9_dc_128_predictor_4x4/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_128_predictor_4x4/;

add_proto qw/void vp9_d207_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d207_predictor_8x8/, "$ssse3_x86inc";

add_proto qw/void vp9_d45_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d45_predictor_8x8/, "$ssse3_x86inc";

add_proto qw/void vp9_d63_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d63_predictor_8x8/, "$ssse3_x86inc";

add_proto qw/void vp9_h_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_h_predictor_8x8 neon_asm dspr2/, "$ssse3_x86inc";
$vp9_h_predictor_8x8_neon_asm=vp9_h_predictor_8x8_neon;

add_proto qw/void vp9_d117_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d117_predictor_8x8/;

add_proto qw/void vp9_d135_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d135_predictor_8x8/;

add_proto qw/void vp9_d153_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d153_predictor_8x8/, "$ssse3_x86inc";

add_proto qw/void vp9_v_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_v_predictor_8x8 neon_asm/, "$sse_x86inc";
$vp9_v_predictor_8x8_neon_asm=vp9_v_predictor_8x8_neon;

add_proto qw/void vp9_tm_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_tm_predictor_8x8 neon_asm dspr2/, "$sse2_x86inc";
$vp9_tm_predictor_8x8_neon_asm=vp9_tm_predictor_8x8_neon;

add_proto qw/void vp9_dc_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_predictor_8x8 dspr2/, "$sse_x86inc";

add_proto qw/void vp9_dc_top_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_top_predictor_8x8/;

add_proto qw/void vp9_dc_left_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_left_predictor_8x8/;

add_proto qw/void vp9_dc_128_predictor_8x8/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_128_predictor_8x8/;

add_proto qw/void vp9_d207_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d207_predictor_16x16/, "$ssse3_x86inc";

add_proto qw/void vp9_d45_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d45_predictor_16x16/, "$ssse3_x86inc";

add_proto qw/void vp9_d63_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d63_predictor_16x16/, "$ssse3_x86inc";

add_proto qw/void vp9_h_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_h_predictor_16x16 neon_asm dspr2/, "$ssse3_x86inc";
$vp9_h_predictor_16x16_neon_asm=vp9_h_predictor_16x16_neon;

add_proto qw/void vp9_d117_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d117_predictor_16x16/;

add_proto qw/void vp9_d135_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d135_predictor_16x16/;

add_proto qw/void vp9_d153_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d153_predictor_16x16/, "$ssse3_x86inc";

add_proto qw/void vp9_v_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_v_predictor_16x16 neon_asm/, "$sse2_x86inc";
$vp9_v_predictor_16x16_neon_asm=vp9_v_predictor_16x16_neon;

add_proto qw/void vp9_tm_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_tm_predictor_16x16 neon_asm/, "$sse2_x86inc";
$vp9_tm_predictor_16x16_neon_asm=vp9_tm_predictor_16x16_neon;

add_proto qw/void vp9_dc_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_predictor_16x16 dspr2/, "$sse2_x86inc";

add_proto qw/void vp9_dc_top_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_top_predictor_16x16/;

add_proto qw/void vp9_dc_left_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_left_predictor_16x16/;

add_proto qw/void vp9_dc_128_predictor_16x16/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_128_predictor_16x16/;

add_proto qw/void vp9_d207_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d207_predictor_32x32/, "$ssse3_x86inc";

add_proto qw/void vp9_d45_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d45_predictor_32x32/, "$ssse3_x86inc";

add_proto qw/void vp9_d63_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d63_predictor_32x32/, "$ssse3_x86inc";

add_proto qw/void vp9_h_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_h_predictor_32x32 neon_asm/, "$ssse3_x86inc";
$vp9_h_predictor_32x32_neon_asm=vp9_h_predictor_32x32_neon;

add_proto qw/void vp9_d117_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d117_predictor_32x32/;

add_proto qw/void vp9_d135_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d135_predictor_32x32/;

add_proto qw/void vp9_d153_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_d153_predictor_32x32/;

add_proto qw/void vp9_v_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_v_predictor_32x32 neon_asm/, "$sse2_x86inc";
$vp9_v_predictor_32x32_neon_asm=vp9_v_predictor_32x32_neon;

add_proto qw/void vp9_tm_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_tm_predictor_32x32 neon_asm/, "$sse2_x86_64";
$vp9_tm_predictor_32x32_neon_asm=vp9_tm_predictor_32x32_neon;

add_proto qw/void vp9_dc_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_predictor_32x32/, "$sse2_x86inc";

add_proto qw/void vp9_dc_top_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_top_predictor_32x32/;

add_proto qw/void vp9_dc_left_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_left_predictor_32x32/;

add_proto qw/void vp9_dc_128_predictor_32x32/, "uint8_t *dst, ptrdiff_t y_stride, const uint8_t *above, const uint8_t *left";
specialize qw/vp9_dc_128_predictor_32x32/;

#
# Loopfilter
#
add_proto qw/void vp9_lpf_vertical_16/, "uint8_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh";
specialize qw/vp9_lpf_vertical_16 sse2 neon_asm dspr2/;
$vp9_lpf_vertical_16_neon_asm=vp9_lpf_vertical_16_neon;

add_proto qw/void vp9_lpf_vertical_16_dual/, "uint8_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh";
specialize qw/vp9_lpf_vertical_16_dual sse2 neon_asm dspr2/;
$vp9_lpf_vertical_16_dual_neon_asm=vp9_lpf_vertical_16_dual_neon;

add_proto qw/void vp9_lpf_vertical_8/, "uint8_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int count";
specialize qw/vp9_lpf_vertical_8 sse2 neon_asm dspr2/;
$vp9_lpf_vertical_8_neon_asm=vp9_lpf_vertical_8_neon;

add_proto qw/void vp9_lpf_vertical_8_dual/, "uint8_t *s, int pitch, const uint8_t *blimit0, const uint8_t *limit0, const uint8_t *thresh0, const uint8_t *blimit1, const uint8_t *limit1, const uint8_t *thresh1";
specialize qw/vp9_lpf_vertical_8_dual sse2 neon_asm dspr2/;
$vp9_lpf_vertical_8_dual_neon_asm=vp9_lpf_vertical_8_dual_neon;

add_proto qw/void vp9_lpf_vertical_4/, "uint8_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int count";
specialize qw/vp9_lpf_vertical_4 mmx neon_asm dspr2/;
$vp9_lpf_vertical_4_neon_asm=vp9_lpf_vertical_4_neon;

add_proto qw/void vp9_lpf_vertical_4_dual/, "uint8_t *s, int pitch, const uint8_t *blimit0, const uint8_t *limit0, const uint8_t *thresh0, const uint8_t *blimit1, const uint8_t *limit1, const uint8_t *thresh1";
specialize qw/vp9_lpf_vertical_4_dual sse2 neon_asm dspr2/;
$vp9_lpf_vertical_4_dual_neon_asm=vp9_lpf_vertical_4_dual_neon;

add_proto qw/void vp9_lpf_horizontal_16/, "uint8_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int count";
specialize qw/vp9_lpf_horizontal_16 sse2 avx2 neon_asm dspr2/;
$vp9_lpf_horizontal_16_neon_asm=vp9_lpf_horizontal_16_neon;

add_proto qw/void vp9_lpf_horizontal_8/, "uint8_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int count";
specialize qw/vp9_lpf_horizontal_8 sse2 neon_asm dspr2/;
$vp9_lpf_horizontal_8_neon_asm=vp9_lpf_horizontal_8_neon;

add_proto qw/void vp9_lpf_horizontal_8_dual/, "uint8_t *s, int pitch, const uint8_t *blimit0, const uint8_t *limit0, const uint8_t *thresh0, const uint8_t *blimit1, const uint8_t *limit1, const uint8_t *thresh1";
specialize qw/vp9_lpf_horizontal_8_dual sse2 neon_asm dspr2/;
$vp9_lpf_horizontal_8_dual_neon_asm=vp9_lpf_horizontal_8_dual_neon;

add_proto qw/void vp9_lpf_horizontal_4/, "uint8_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int count";
specialize qw/vp9_lpf_horizontal_4 mmx neon_asm dspr2/;
$vp9_lpf_horizontal_4_neon_asm=vp9_lpf_horizontal_4_neon;

add_proto qw/void vp9_lpf_horizontal_4_dual/, "uint8_t *s, int pitch, const uint8_t *blimit0, const uint8_t *limit0, const uint8_t *thresh0, const uint8_t *blimit1, const uint8_t *limit1, const uint8_t *thresh1";
specialize qw/vp9_lpf_horizontal_4_dual sse2 neon_asm dspr2/;
$vp9_lpf_horizontal_4_dual_neon_asm=vp9_lpf_horizontal_4_dual_neon;

#
# post proc
#
if (vpx_config("CONFIG_VP9_POSTPROC") eq "yes") {
add_proto qw/void vp9_mbpost_proc_down/, "uint8_t *dst, int pitch, int rows, int cols, int flimit";
specialize qw/vp9_mbpost_proc_down mmx sse2/;
$vp9_mbpost_proc_down_sse2=vp9_mbpost_proc_down_xmm;

add_proto qw/void vp9_mbpost_proc_across_ip/, "uint8_t *src, int pitch, int rows, int cols, int flimit";
specialize qw/vp9_mbpost_proc_across_ip sse2/;
$vp9_mbpost_proc_across_ip_sse2=vp9_mbpost_proc_across_ip_xmm;

add_proto qw/void vp9_post_proc_down_and_across/, "const uint8_t *src_ptr, uint8_t *dst_ptr, int src_pixels_per_line, int dst_pixels_per_line, int rows, int cols, int flimit";
specialize qw/vp9_post_proc_down_and_across mmx sse2/;
$vp9_post_proc_down_and_across_sse2=vp9_post_proc_down_and_across_xmm;

add_proto qw/void vp9_plane_add_noise/, "uint8_t *Start, char *noise, char blackclamp[16], char whiteclamp[16], char bothclamp[16], unsigned int Width, unsigned int Height, int Pitch";
specialize qw/vp9_plane_add_noise mmx sse2/;
$vp9_plane_add_noise_sse2=vp9_plane_add_noise_wmt;
}

add_proto qw/void vp9_blend_mb_inner/, "uint8_t *y, uint8_t *u, uint8_t *v, int y1, int u1, int v1, int alpha, int stride";
specialize qw/vp9_blend_mb_inner/;

add_proto qw/void vp9_blend_mb_outer/, "uint8_t *y, uint8_t *u, uint8_t *v, int y1, int u1, int v1, int alpha, int stride";
specialize qw/vp9_blend_mb_outer/;

add_proto qw/void vp9_blend_b/, "uint8_t *y, uint8_t *u, uint8_t *v, int y1, int u1, int v1, int alpha, int stride";
specialize qw/vp9_blend_b/;

#
# Sub Pixel Filters
#
add_proto qw/void vp9_convolve_copy/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h";
specialize qw/vp9_convolve_copy neon_asm dspr2/, "$sse2_x86inc";
$vp9_convolve_copy_neon_asm=vp9_convolve_copy_neon;

add_proto qw/void vp9_convolve_avg/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h";
specialize qw/vp9_convolve_avg neon_asm dspr2/, "$sse2_x86inc";
$vp9_convolve_avg_neon_asm=vp9_convolve_avg_neon;

add_proto qw/void vp9_convolve8/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h";
specialize qw/vp9_convolve8 sse2 ssse3 neon_asm dspr2/;
$vp9_convolve8_neon_asm=vp9_convolve8_neon;

add_proto qw/void vp9_convolve8_horiz/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h";
specialize qw/vp9_convolve8_horiz sse2 ssse3 neon_asm dspr2/;
$vp9_convolve8_horiz_neon_asm=vp9_convolve8_horiz_neon;

add_proto qw/void vp9_convolve8_vert/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h";
specialize qw/vp9_convolve8_vert sse2 ssse3 neon_asm dspr2/;
$vp9_convolve8_vert_neon_asm=vp9_convolve8_vert_neon;

add_proto qw/void vp9_convolve8_avg/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h";
specialize qw/vp9_convolve8_avg sse2 ssse3 neon_asm dspr2/;
$vp9_convolve8_avg_neon_asm=vp9_convolve8_avg_neon;

add_proto qw/void vp9_convolve8_avg_horiz/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h";
specialize qw/vp9_convolve8_avg_horiz sse2 ssse3 neon_asm dspr2/;
$vp9_convolve8_avg_horiz_neon_asm=vp9_convolve8_avg_horiz_neon;

add_proto qw/void vp9_convolve8_avg_vert/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h";
specialize qw/vp9_convolve8_avg_vert sse2 ssse3 neon_asm dspr2/;
$vp9_convolve8_avg_vert_neon_asm=vp9_convolve8_avg_vert_neon;

#
# dct
#
if (vpx_config("CONFIG_VP9_HIGH") eq "yes") {
  add_proto qw/void vp9_idct4x4_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct4x4_1_add/;

  add_proto qw/void vp9_idct4x4_16_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct4x4_16_add/;

  add_proto qw/void vp9_idct8x8_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct8x8_1_add/;

  add_proto qw/void vp9_idct8x8_64_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct8x8_64_add/;

  add_proto qw/void vp9_idct8x8_12_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct8x8_12_add/;

  add_proto qw/void vp9_idct16x16_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct16x16_1_add/;

  add_proto qw/void vp9_idct16x16_256_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct16x16_256_add/;

  add_proto qw/void vp9_idct16x16_10_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct16x16_10_add/;

  add_proto qw/void vp9_idct32x32_1024_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct32x32_1024_add/;

  add_proto qw/void vp9_idct32x32_34_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct32x32_34_add/;

  add_proto qw/void vp9_idct32x32_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct32x32_1_add/;

  add_proto qw/void vp9_iht4x4_16_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type";
  specialize qw/vp9_iht4x4_16_add/;

  add_proto qw/void vp9_iht8x8_64_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type";
  specialize qw/vp9_iht8x8_64_add/;

  add_proto qw/void vp9_iht16x16_256_add/, "const tran_low_t *input, uint8_t *output, int pitch, int tx_type";
  specialize qw/vp9_iht16x16_256_add/;

  # dct and add

  add_proto qw/void vp9_iwht4x4_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_iwht4x4_1_add/;

  add_proto qw/void vp9_iwht4x4_16_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_iwht4x4_16_add/;
} else {
  add_proto qw/void vp9_idct4x4_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct4x4_1_add sse2 neon_asm dspr2/;
  $vp9_idct4x4_1_add_neon_asm=vp9_idct4x4_1_add_neon;

  add_proto qw/void vp9_idct4x4_16_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct4x4_16_add sse2 neon_asm dspr2/;
  $vp9_idct4x4_16_add_neon_asm=vp9_idct4x4_16_add_neon;

  add_proto qw/void vp9_idct8x8_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct8x8_1_add sse2 neon_asm dspr2/;
  $vp9_idct8x8_1_add_neon_asm=vp9_idct8x8_1_add_neon;

  add_proto qw/void vp9_idct8x8_64_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct8x8_64_add sse2 neon_asm dspr2/, "$ssse3_x86_64";
  $vp9_idct8x8_64_add_neon_asm=vp9_idct8x8_64_add_neon;

  add_proto qw/void vp9_idct8x8_12_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct8x8_12_add sse2 neon_asm dspr2/, "$ssse3_x86_64";
  $vp9_idct8x8_12_add_neon_asm=vp9_idct8x8_12_add_neon;

  add_proto qw/void vp9_idct16x16_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct16x16_1_add sse2 neon_asm dspr2/;
  $vp9_idct16x16_1_add_neon_asm=vp9_idct16x16_1_add_neon;

  add_proto qw/void vp9_idct16x16_256_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct16x16_256_add sse2 ssse3 neon_asm dspr2/;
  $vp9_idct16x16_256_add_neon_asm=vp9_idct16x16_256_add_neon;

  add_proto qw/void vp9_idct16x16_10_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct16x16_10_add sse2 ssse3 neon_asm dspr2/;
  $vp9_idct16x16_10_add_neon_asm=vp9_idct16x16_10_add_neon;

  add_proto qw/void vp9_idct32x32_1024_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct32x32_1024_add sse2 neon_asm dspr2/;
  $vp9_idct32x32_1024_add_neon_asm=vp9_idct32x32_1024_add_neon;

  add_proto qw/void vp9_idct32x32_34_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct32x32_34_add sse2 neon_asm dspr2/;
  $vp9_idct32x32_34_add_neon_asm=vp9_idct32x32_1024_add_neon;

  add_proto qw/void vp9_idct32x32_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_idct32x32_1_add sse2 neon_asm dspr2/;
  $vp9_idct32x32_1_add_neon_asm=vp9_idct32x32_1_add_neon;

  add_proto qw/void vp9_iht4x4_16_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type";
  specialize qw/vp9_iht4x4_16_add sse2 neon_asm dspr2/;
  $vp9_iht4x4_16_add_neon_asm=vp9_iht4x4_16_add_neon;

  add_proto qw/void vp9_iht8x8_64_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type";
  specialize qw/vp9_iht8x8_64_add sse2 neon_asm dspr2/;
  $vp9_iht8x8_64_add_neon_asm=vp9_iht8x8_64_add_neon;

  add_proto qw/void vp9_iht16x16_256_add/, "const tran_low_t *input, uint8_t *output, int pitch, int tx_type";
  specialize qw/vp9_iht16x16_256_add sse2 dspr2/;

  # dct and add

  add_proto qw/void vp9_iwht4x4_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_iwht4x4_1_add/;

  add_proto qw/void vp9_iwht4x4_16_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride";
  specialize qw/vp9_iwht4x4_16_add/;
}


# High bitdepth functions
if (vpx_config("CONFIG_VP9_HIGH") eq "yes") {
add_proto qw/void vp9_high_d207_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d207_predictor_4x4/;

add_proto qw/void vp9_high_d45_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d45_predictor_4x4/;

add_proto qw/void vp9_high_d63_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d63_predictor_4x4/;

add_proto qw/void vp9_high_h_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_h_predictor_4x4/;

add_proto qw/void vp9_high_d117_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d117_predictor_4x4/;

add_proto qw/void vp9_high_d135_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d135_predictor_4x4/;

add_proto qw/void vp9_high_d153_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d153_predictor_4x4/;

add_proto qw/void vp9_high_v_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_v_predictor_4x4 neon/, "$sse_x86inc";

add_proto qw/void vp9_high_tm_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_tm_predictor_4x4/, "$sse_x86inc";

add_proto qw/void vp9_high_dc_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_predictor_4x4/, "$sse_x86inc";

add_proto qw/void vp9_high_dc_top_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_top_predictor_4x4/;

add_proto qw/void vp9_high_dc_left_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_left_predictor_4x4/;

add_proto qw/void vp9_high_dc_128_predictor_4x4/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_128_predictor_4x4/;

add_proto qw/void vp9_high_d207_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d207_predictor_8x8/;

add_proto qw/void vp9_high_d45_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d45_predictor_8x8/;

add_proto qw/void vp9_high_d63_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d63_predictor_8x8/;

add_proto qw/void vp9_high_h_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_h_predictor_8x8/;

add_proto qw/void vp9_high_d117_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d117_predictor_8x8/;

add_proto qw/void vp9_high_d135_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d135_predictor_8x8/;

add_proto qw/void vp9_high_d153_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d153_predictor_8x8/;

add_proto qw/void vp9_high_v_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_v_predictor_8x8/, "$sse2_x86inc";

add_proto qw/void vp9_high_tm_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_tm_predictor_8x8/, "$sse2_x86inc";

add_proto qw/void vp9_high_dc_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_predictor_8x8/, "$sse2_x86inc";;

add_proto qw/void vp9_high_dc_top_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_top_predictor_8x8/;

add_proto qw/void vp9_high_dc_left_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_left_predictor_8x8/;

add_proto qw/void vp9_high_dc_128_predictor_8x8/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_128_predictor_8x8/;

add_proto qw/void vp9_high_d207_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d207_predictor_16x16/;

add_proto qw/void vp9_high_d45_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d45_predictor_16x16/;

add_proto qw/void vp9_high_d63_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d63_predictor_16x16/;

add_proto qw/void vp9_high_h_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_h_predictor_16x16/;

add_proto qw/void vp9_high_d117_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d117_predictor_16x16/;

add_proto qw/void vp9_high_d135_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d135_predictor_16x16/;

add_proto qw/void vp9_high_d153_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d153_predictor_16x16/;

add_proto qw/void vp9_high_v_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_v_predictor_16x16 neon/, "$sse2_x86inc";

add_proto qw/void vp9_high_tm_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_tm_predictor_16x16/, "$sse2_x86_64";

add_proto qw/void vp9_high_dc_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_predictor_16x16/, "$sse2_x86inc";

add_proto qw/void vp9_high_dc_top_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_top_predictor_16x16/;

add_proto qw/void vp9_high_dc_left_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_left_predictor_16x16/;

add_proto qw/void vp9_high_dc_128_predictor_16x16/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_128_predictor_16x16/;

add_proto qw/void vp9_high_d207_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d207_predictor_32x32/;

add_proto qw/void vp9_high_d45_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d45_predictor_32x32/;

add_proto qw/void vp9_high_d63_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d63_predictor_32x32/;

add_proto qw/void vp9_high_h_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_h_predictor_32x32/;

add_proto qw/void vp9_high_d117_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d117_predictor_32x32/;

add_proto qw/void vp9_high_d135_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d135_predictor_32x32/;

add_proto qw/void vp9_high_d153_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_d153_predictor_32x32/;

add_proto qw/void vp9_high_v_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_v_predictor_32x32/, "$sse2_x86inc";

add_proto qw/void vp9_high_tm_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_tm_predictor_32x32/, "$sse2_x86_64";

add_proto qw/void vp9_high_dc_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_predictor_32x32/, "$sse2_x86_64";

add_proto qw/void vp9_high_dc_top_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_top_predictor_32x32/;

add_proto qw/void vp9_high_dc_left_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_left_predictor_32x32/;

add_proto qw/void vp9_high_dc_128_predictor_32x32/, "uint16_t *dst, ptrdiff_t y_stride, const uint16_t *above, const uint16_t *left, int bps";
specialize qw/vp9_high_dc_128_predictor_32x32/;

#
# Loopfilter
#
add_proto qw/void vp9_high_lpf_vertical_16/, "uint16_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int bps";
specialize qw/vp9_high_lpf_vertical_16 sse2/;

add_proto qw/void vp9_high_lpf_vertical_16_dual/, "uint16_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int bps";
specialize qw/vp9_high_lpf_vertical_16_dual sse2/;

add_proto qw/void vp9_high_lpf_vertical_8/, "uint16_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int count, int bps";
specialize qw/vp9_high_lpf_vertical_8 sse2/;

add_proto qw/void vp9_high_lpf_vertical_8_dual/, "uint16_t *s, int pitch, const uint8_t *blimit0, const uint8_t *limit0, const uint8_t *thresh0, const uint8_t *blimit1, const uint8_t *limit1, const uint8_t *thresh1, int bps";
specialize qw/vp9_high_lpf_vertical_8_dual sse2/;

add_proto qw/void vp9_high_lpf_vertical_4/, "uint16_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int count, int bps";
specialize qw/vp9_high_lpf_vertical_4 sse2/;

add_proto qw/void vp9_high_lpf_vertical_4_dual/, "uint16_t *s, int pitch, const uint8_t *blimit0, const uint8_t *limit0, const uint8_t *thresh0, const uint8_t *blimit1, const uint8_t *limit1, const uint8_t *thresh1, int bps";
specialize qw/vp9_high_lpf_vertical_4_dual sse2/;

add_proto qw/void vp9_high_lpf_horizontal_16/, "uint16_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int count, int bps";
specialize qw/vp9_high_lpf_horizontal_16 sse2/;

add_proto qw/void vp9_high_lpf_horizontal_8/, "uint16_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int count, int bps";
specialize qw/vp9_high_lpf_horizontal_8 sse2/;

add_proto qw/void vp9_high_lpf_horizontal_8_dual/, "uint16_t *s, int pitch, const uint8_t *blimit0, const uint8_t *limit0, const uint8_t *thresh0, const uint8_t *blimit1, const uint8_t *limit1, const uint8_t *thresh1, int bps";
specialize qw/vp9_high_lpf_horizontal_8_dual sse2/;

add_proto qw/void vp9_high_lpf_horizontal_4/, "uint16_t *s, int pitch, const uint8_t *blimit, const uint8_t *limit, const uint8_t *thresh, int count, int bps";
specialize qw/vp9_high_lpf_horizontal_4 sse2/;

add_proto qw/void vp9_high_lpf_horizontal_4_dual/, "uint16_t *s, int pitch, const uint8_t *blimit0, const uint8_t *limit0, const uint8_t *thresh0, const uint8_t *blimit1, const uint8_t *limit1, const uint8_t *thresh1, int bps";
specialize qw/vp9_high_lpf_horizontal_4_dual sse2/;

#
# post proc
#
if (vpx_config("CONFIG_VP9_POSTPROC") eq "yes") {
add_proto qw/void vp9_high_mbpost_proc_down/, "uint16_t *dst, int pitch, int rows, int cols, int flimit";
specialize qw/vp9_high_mbpost_proc_down/;

add_proto qw/void vp9_high_mbpost_proc_across_ip/, "uint16_t *src, int pitch, int rows, int cols, int flimit";
specialize qw/vp9_high_mbpost_proc_across_ip/;

add_proto qw/void vp9_high_post_proc_down_and_across/, "const uint16_t *src_ptr, uint16_t *dst_ptr, int src_pixels_per_line, int dst_pixels_per_line, int rows, int cols, int flimit";
specialize qw/vp9_high_post_proc_down_and_across/;

add_proto qw/void vp9_high_plane_add_noise/, "uint8_t *Start, char *noise, char blackclamp[16], char whiteclamp[16], char bothclamp[16], unsigned int Width, unsigned int Height, int Pitch";
specialize qw/vp9_high_plane_add_noise/;
}

add_proto qw/void vp9_high_blend_mb_inner/, "uint8_t *y, uint8_t *u, uint8_t *v, int y1, int u1, int v1, int alpha, int stride";
specialize qw/vp9_high_blend_mb_inner/;

add_proto qw/void vp9_high_blend_mb_outer/, "uint8_t *y, uint8_t *u, uint8_t *v, int y1, int u1, int v1, int alpha, int stride";
specialize qw/vp9_high_blend_mb_outer/;

add_proto qw/void vp9_high_blend_b/, "uint8_t *y, uint8_t *u, uint8_t *v, int y1, int u1, int v1, int alpha, int stride";
specialize qw/vp9_high_blend_b/;

#
# Sub Pixel Filters
#
add_proto qw/void vp9_high_convolve_copy/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h, int bps";
specialize qw/vp9_high_convolve_copy/;

add_proto qw/void vp9_high_convolve_avg/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h, int bps";
specialize qw/vp9_high_convolve_avg/;

add_proto qw/void vp9_high_convolve8/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h, int bps";
specialize qw/vp9_high_convolve8/, "$sse2_x86_64";

add_proto qw/void vp9_high_convolve8_horiz/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h, int bps";
specialize qw/vp9_high_convolve8_horiz/, "$sse2_x86_64";

add_proto qw/void vp9_high_convolve8_vert/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h, int bps";
specialize qw/vp9_high_convolve8_vert/, "$sse2_x86_64";

add_proto qw/void vp9_high_convolve8_avg/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h, int bps";
specialize qw/vp9_high_convolve8_avg/, "$sse2_x86_64";

add_proto qw/void vp9_high_convolve8_avg_horiz/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h, int bps";
specialize qw/vp9_high_convolve8_avg_horiz/, "$sse2_x86_64";

add_proto qw/void vp9_high_convolve8_avg_vert/, "const uint8_t *src, ptrdiff_t src_stride, uint8_t *dst, ptrdiff_t dst_stride, const int16_t *filter_x, int x_step_q4, const int16_t *filter_y, int y_step_q4, int w, int h, int bps";
specialize qw/vp9_high_convolve8_avg_vert/, "$sse2_x86_64";

#
# dct
#
add_proto qw/void vp9_high_idct4x4_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_idct4x4_1_add/;

add_proto qw/void vp9_high_idct4x4_16_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_idct4x4_16_add/;

add_proto qw/void vp9_high_idct8x8_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_idct8x8_1_add/;

add_proto qw/void vp9_high_idct8x8_64_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_idct8x8_64_add/;

add_proto qw/void vp9_high_idct8x8_10_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_idct8x8_10_add/;

add_proto qw/void vp9_high_idct16x16_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_idct16x16_1_add/;

add_proto qw/void vp9_high_idct16x16_256_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_idct16x16_256_add/;

add_proto qw/void vp9_high_idct16x16_10_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_idct16x16_10_add/;

add_proto qw/void vp9_high_idct32x32_1024_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_idct32x32_1024_add/;

add_proto qw/void vp9_high_idct32x32_34_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_idct32x32_34_add/;

add_proto qw/void vp9_high_idct32x32_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_idct32x32_1_add/;

add_proto qw/void vp9_high_iht4x4_16_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type, int bps";
specialize qw/vp9_high_iht4x4_16_add/;

add_proto qw/void vp9_high_iht8x8_64_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type, int bps";
specialize qw/vp9_high_iht8x8_64_add/;

add_proto qw/void vp9_high_iht16x16_256_add/, "const tran_low_t *input, uint8_t *output, int pitch, int tx_type, int bps";
specialize qw/vp9_high_iht16x16_256_add/;

# dct and add

add_proto qw/void vp9_high_iwht4x4_1_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_iwht4x4_1_add/;

add_proto qw/void vp9_high_iwht4x4_16_add/, "const tran_low_t *input, uint8_t *dest, int dest_stride, int bps";
specialize qw/vp9_high_iwht4x4_16_add/;
}

#
# Encoder functions below this point.
#
if (vpx_config("CONFIG_VP9_ENCODER") eq "yes") {


# variance
add_proto qw/unsigned int vp9_variance32x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance32x16 avx2/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance16x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance16x32/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance64x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance64x32 avx2/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance32x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance32x64/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance32x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance32x32 avx2 neon/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance64x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance64x64 avx2/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance16x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance16x16 mmx avx2 neon/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance16x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance16x8 mmx/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance8x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance8x16 mmx/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance8x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance8x8 mmx neon/, "$sse2_x86inc";

add_proto qw/void vp9_get8x8var/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, int *sum";
specialize qw/vp9_get8x8var mmx neon/, "$sse2_x86inc";

add_proto qw/void vp9_get16x16var/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, int *sum";
specialize qw/vp9_get16x16var avx2 neon/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance8x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance8x4/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance4x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance4x8/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_variance4x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_variance4x4 mmx/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance64x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance64x64 avx2/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance64x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance64x64 avx2/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance32x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance32x64/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance32x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance32x64/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance64x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance64x32/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance64x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance64x32/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance32x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance32x16/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance32x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance32x16/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance16x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance16x32/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance16x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance16x32/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance32x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance32x32 avx2 neon/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance32x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance32x32 avx2/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance16x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance16x16 neon/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance16x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance16x16/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance8x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance8x16/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance8x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance8x16/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance16x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance16x8/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance16x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance16x8/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance8x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance8x8 neon/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance8x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance8x8/, "$sse2_x86inc", "$ssse3_x86inc";

# TODO(jingning): need to convert 8x4/4x8 functions into mmx/sse form
add_proto qw/unsigned int vp9_sub_pixel_variance8x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance8x4/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance8x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance8x4/, "$sse2_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance4x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance4x8/, "$sse_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_avg_variance4x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance4x8/, "$sse_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sub_pixel_variance4x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_sub_pixel_variance4x4/, "$sse_x86inc", "$ssse3_x86inc";
#vp9_sub_pixel_variance4x4_sse2=vp9_sub_pixel_variance4x4_wmt

add_proto qw/unsigned int vp9_sub_pixel_avg_variance4x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_sub_pixel_avg_variance4x4/, "$sse_x86inc", "$ssse3_x86inc";

add_proto qw/unsigned int vp9_sad64x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_sad64x64 neon/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad32x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_sad32x64/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad64x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_sad64x32/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad32x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_sad32x16/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad16x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_sad16x32/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad32x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_sad32x32 neon/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad16x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_sad16x16 mmx neon/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad16x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_sad16x8 mmx/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad8x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_sad8x16 mmx/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad8x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_sad8x8 mmx neon/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad8x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_sad8x4/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad4x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_sad4x8/, "$sse_x86inc";

add_proto qw/unsigned int vp9_sad4x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_sad4x4 mmx/, "$sse_x86inc";

add_proto qw/unsigned int vp9_sad64x64_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad64x64_avg/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad32x64_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad32x64_avg/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad64x32_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad64x32_avg/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad32x16_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad32x16_avg/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad16x32_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad16x32_avg/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad32x32_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad32x32_avg/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad16x16_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad16x16_avg/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad16x8_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad16x8_avg/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad8x16_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad8x16_avg/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad8x8_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad8x8_avg/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad8x4_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad8x4_avg/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_sad4x8_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad4x8_avg/, "$sse_x86inc";

add_proto qw/unsigned int vp9_sad4x4_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_sad4x4_avg/, "$sse_x86inc";

add_proto qw/void vp9_sad64x64x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad64x64x3/;

add_proto qw/void vp9_sad32x32x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad32x32x3/;

add_proto qw/void vp9_sad16x16x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad16x16x3 sse3 ssse3/;

add_proto qw/void vp9_sad16x8x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad16x8x3 sse3 ssse3/;

add_proto qw/void vp9_sad8x16x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad8x16x3 sse3/;

add_proto qw/void vp9_sad8x8x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad8x8x3 sse3/;

add_proto qw/void vp9_sad4x4x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad4x4x3 sse3/;

add_proto qw/void vp9_sad64x64x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_sad64x64x8/;

add_proto qw/void vp9_sad32x32x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_sad32x32x8/;

add_proto qw/void vp9_sad16x16x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_sad16x16x8 sse4/;

add_proto qw/void vp9_sad16x8x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_sad16x8x8 sse4/;

add_proto qw/void vp9_sad8x16x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_sad8x16x8 sse4/;

add_proto qw/void vp9_sad8x8x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_sad8x8x8 sse4/;

add_proto qw/void vp9_sad8x4x8/, "const uint8_t *src_ptr, int src_stride, const uint8_t *ref_ptr, int ref_stride, uint32_t *sad_array";
specialize qw/vp9_sad8x4x8/;

add_proto qw/void vp9_sad4x8x8/, "const uint8_t *src_ptr, int src_stride, const uint8_t *ref_ptr, int ref_stride, uint32_t *sad_array";
specialize qw/vp9_sad4x8x8/;

add_proto qw/void vp9_sad4x4x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_sad4x4x8 sse4/;

add_proto qw/void vp9_sad64x64x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad64x64x4d sse2 avx2/;

add_proto qw/void vp9_sad32x64x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad32x64x4d sse2/;

add_proto qw/void vp9_sad64x32x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad64x32x4d sse2/;

add_proto qw/void vp9_sad32x16x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad32x16x4d sse2/;

add_proto qw/void vp9_sad16x32x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad16x32x4d sse2/;

add_proto qw/void vp9_sad32x32x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad32x32x4d sse2 avx2/;

add_proto qw/void vp9_sad16x16x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad16x16x4d sse2/;

add_proto qw/void vp9_sad16x8x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad16x8x4d sse2/;

add_proto qw/void vp9_sad8x16x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad8x16x4d sse2/;

add_proto qw/void vp9_sad8x8x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad8x8x4d sse2/;

# TODO(jingning): need to convert these 4x8/8x4 functions into sse2 form
add_proto qw/void vp9_sad8x4x4d/, "const uint8_t *src_ptr, int src_stride, const uint8_t* const ref_ptr[], int ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad8x4x4d sse2/;

add_proto qw/void vp9_sad4x8x4d/, "const uint8_t *src_ptr, int src_stride, const uint8_t* const ref_ptr[], int ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad4x8x4d sse/;

add_proto qw/void vp9_sad4x4x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_sad4x4x4d sse/;

add_proto qw/unsigned int vp9_mse16x16/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_mse16x16 mmx avx2/, "$sse2_x86inc";

add_proto qw/unsigned int vp9_mse8x16/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_mse8x16/;

add_proto qw/unsigned int vp9_mse16x8/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_mse16x8/;

add_proto qw/unsigned int vp9_mse8x8/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_mse8x8/;

add_proto qw/unsigned int vp9_get_mb_ss/, "const int16_t *";
specialize qw/vp9_get_mb_ss mmx sse2/;
# ENCODEMB INVOKE

add_proto qw/void vp9_subtract_block/, "int rows, int cols, int16_t *diff_ptr, ptrdiff_t diff_stride, const uint8_t *src_ptr, ptrdiff_t src_stride, const uint8_t *pred_ptr, ptrdiff_t pred_stride";
specialize qw/vp9_subtract_block neon/, "$sse2_x86inc";

<<<<<<< HEAD   (959563 Merge "Hdr change for profiles > 1 for intra-only frames" in)
if (vpx_config("CONFIG_VP9_HIGH") eq "yes") {
# the transform coefficients are held in 32-bit
# values, so the assembler code for  vp9_block_error can no longer be used.
  add_proto qw/int64_t vp9_block_error/, "const tran_low_t *coeff, const tran_low_t *dqcoeff, intptr_t block_size, int64_t *ssz";
  specialize qw/vp9_block_error/;
=======
add_proto qw/void vp9_quantize_fp/, "const int16_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, int16_t *qcoeff_ptr, int16_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
specialize qw/vp9_quantize_fp neon/, "$ssse3_x86_64";
>>>>>>> BRANCH (2bfbe9 Merge "vpxenc.sh: use --test-decode=fatal for vp9")

  add_proto qw/void vp9_quantize_fp/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
  specialize qw/vp9_quantize_fp/;

  add_proto qw/void vp9_quantize_fp_32x32/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
  specialize qw/vp9_quantize_fp_32x32/;

  add_proto qw/void vp9_quantize_b/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
  specialize qw/vp9_quantize_b/;

  add_proto qw/void vp9_quantize_b_32x32/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
  specialize qw/vp9_quantize_b_32x32/;
} else {
  add_proto qw/int64_t vp9_block_error/, "const tran_low_t *coeff, const tran_low_t *dqcoeff, intptr_t block_size, int64_t *ssz";
  specialize qw/vp9_block_error avx2/;

  add_proto qw/void vp9_quantize_fp/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
  specialize qw/vp9_quantize_fp/, "$ssse3_x86_64";

  add_proto qw/void vp9_quantize_fp_32x32/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
  specialize qw/vp9_quantize_fp_32x32/, "$ssse3_x86_64";

  add_proto qw/void vp9_quantize_b/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
  specialize qw/vp9_quantize_b/, "$ssse3_x86_64";

  add_proto qw/void vp9_quantize_b_32x32/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
  specialize qw/vp9_quantize_b_32x32/;
}

#
# Structured Similarity (SSIM)
#
if (vpx_config("CONFIG_INTERNAL_STATS") eq "yes") {
    add_proto qw/void vp9_ssim_parms_8x8/, "uint8_t *s, int sp, uint8_t *r, int rp, unsigned long *sum_s, unsigned long *sum_r, unsigned long *sum_sq_s, unsigned long *sum_sq_r, unsigned long *sum_sxr";
    specialize qw/vp9_ssim_parms_8x8/, "$sse2_x86_64";

    add_proto qw/void vp9_ssim_parms_16x16/, "uint8_t *s, int sp, uint8_t *r, int rp, unsigned long *sum_s, unsigned long *sum_r, unsigned long *sum_sq_s, unsigned long *sum_sq_r, unsigned long *sum_sxr";
    specialize qw/vp9_ssim_parms_16x16/, "$sse2_x86_64";
}

# fdct functions
<<<<<<< HEAD   (959563 Merge "Hdr change for profiles > 1 for intra-only frames" in)
if (vpx_config("CONFIG_VP9_HIGH") eq "yes") {
  add_proto qw/void vp9_fht4x4/, "const int16_t *input, tran_low_t *output, int stride, int tx_type";
  specialize qw/vp9_fht4x4/;
=======
add_proto qw/void vp9_fht4x4/, "const int16_t *input, int16_t *output, int stride, int tx_type";
specialize qw/vp9_fht4x4 sse2/;
>>>>>>> BRANCH (2bfbe9 Merge "vpxenc.sh: use --test-decode=fatal for vp9")

<<<<<<< HEAD   (959563 Merge "Hdr change for profiles > 1 for intra-only frames" in)
  add_proto qw/void vp9_fht8x8/, "const int16_t *input, tran_low_t *output, int stride, int tx_type";
  specialize qw/vp9_fht8x8/;
=======
add_proto qw/void vp9_fht8x8/, "const int16_t *input, int16_t *output, int stride, int tx_type";
specialize qw/vp9_fht8x8 sse2/;
>>>>>>> BRANCH (2bfbe9 Merge "vpxenc.sh: use --test-decode=fatal for vp9")

  add_proto qw/void vp9_fht16x16/, "const int16_t *input, tran_low_t *output, int stride, int tx_type";
  specialize qw/vp9_fht16x16/;

  add_proto qw/void vp9_fwht4x4/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fwht4x4/;

  add_proto qw/void vp9_fdct4x4_1/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct4x4_1/;

<<<<<<< HEAD   (959563 Merge "Hdr change for profiles > 1 for intra-only frames" in)
  add_proto qw/void vp9_fdct4x4/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct4x4/;
=======
add_proto qw/void vp9_fdct4x4/, "const int16_t *input, int16_t *output, int stride";
specialize qw/vp9_fdct4x4 sse2/;
>>>>>>> BRANCH (2bfbe9 Merge "vpxenc.sh: use --test-decode=fatal for vp9")

<<<<<<< HEAD   (959563 Merge "Hdr change for profiles > 1 for intra-only frames" in)
  add_proto qw/void vp9_fdct8x8_1/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct8x8_1/;
=======
add_proto qw/void vp9_fdct8x8_1/, "const int16_t *input, int16_t *output, int stride";
specialize qw/vp9_fdct8x8_1 sse2 neon/;
>>>>>>> BRANCH (2bfbe9 Merge "vpxenc.sh: use --test-decode=fatal for vp9")

<<<<<<< HEAD   (959563 Merge "Hdr change for profiles > 1 for intra-only frames" in)
  add_proto qw/void vp9_fdct8x8/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct8x8/;
=======
add_proto qw/void vp9_fdct8x8/, "const int16_t *input, int16_t *output, int stride";
specialize qw/vp9_fdct8x8 sse2 neon/, "$ssse3_x86_64";
>>>>>>> BRANCH (2bfbe9 Merge "vpxenc.sh: use --test-decode=fatal for vp9")

  add_proto qw/void vp9_fdct16x16_1/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct16x16_1/;

  add_proto qw/void vp9_fdct16x16/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct16x16/;

  add_proto qw/void vp9_fdct32x32_1/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct32x32_1/;

  add_proto qw/void vp9_fdct32x32/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct32x32/;

  add_proto qw/void vp9_fdct32x32_rd/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct32x32_rd/;
} else {
  add_proto qw/void vp9_fht4x4/, "const int16_t *input, tran_low_t *output, int stride, int tx_type";
  specialize qw/vp9_fht4x4 sse2 avx2/;

  add_proto qw/void vp9_fht8x8/, "const int16_t *input, tran_low_t *output, int stride, int tx_type";
  specialize qw/vp9_fht8x8 sse2 avx2/;

  add_proto qw/void vp9_fht16x16/, "const int16_t *input, tran_low_t *output, int stride, int tx_type";
  specialize qw/vp9_fht16x16 sse2/;

  add_proto qw/void vp9_fwht4x4/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fwht4x4/, "$mmx_x86inc";

  add_proto qw/void vp9_fdct4x4_1/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct4x4_1 sse2/;

  add_proto qw/void vp9_fdct4x4/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct4x4 sse2 avx2/;

  add_proto qw/void vp9_fdct8x8_1/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct8x8_1 sse2/;

  add_proto qw/void vp9_fdct8x8/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct8x8 sse2 avx2/, "$ssse3_x86_64";

  add_proto qw/void vp9_fdct16x16_1/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct16x16_1 sse2/;

  add_proto qw/void vp9_fdct16x16/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct16x16 sse2/;

  add_proto qw/void vp9_fdct32x32_1/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct32x32_1 sse2/;

  add_proto qw/void vp9_fdct32x32/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct32x32 sse2 avx2/;

  add_proto qw/void vp9_fdct32x32_rd/, "const int16_t *input, tran_low_t *output, int stride";
  specialize qw/vp9_fdct32x32_rd sse2 avx2/;
}

#
# Motion search
#
add_proto qw/int vp9_full_search_sad/, "const struct macroblock *x, const struct mv *ref_mv, int sad_per_bit, int distance, const struct vp9_variance_vtable *fn_ptr, const struct mv *center_mv, struct mv *best_mv";
specialize qw/vp9_full_search_sad sse3 sse4_1/;
$vp9_full_search_sad_sse3=vp9_full_search_sadx3;
$vp9_full_search_sad_sse4_1=vp9_full_search_sadx8;

add_proto qw/int vp9_refining_search_sad/, "const struct macroblock *x, struct mv *ref_mv, int sad_per_bit, int distance, const struct vp9_variance_vtable *fn_ptr, const struct mv *center_mv";
specialize qw/vp9_refining_search_sad/;

add_proto qw/int vp9_diamond_search_sad/, "const struct macroblock *x, const struct search_site_config *cfg,  struct mv *ref_mv, struct mv *best_mv, int search_param, int sad_per_bit, int *num00, const struct vp9_variance_vtable *fn_ptr, const struct mv *center_mv";
specialize qw/vp9_diamond_search_sad/;

add_proto qw/int vp9_full_range_search/, "const struct macroblock *x, const struct search_site_config *cfg, struct mv *ref_mv, struct mv *best_mv, int search_param, int sad_per_bit, int *num00, const struct vp9_variance_vtable *fn_ptr, const struct mv *center_mv";
specialize qw/vp9_full_range_search/;

add_proto qw/void vp9_temporal_filter_apply/, "uint8_t *frame1, unsigned int stride, uint8_t *frame2, unsigned int block_width, unsigned int block_height, int strength, int filter_weight, unsigned int *accumulator, uint16_t *count";
specialize qw/vp9_temporal_filter_apply sse2/;

if (vpx_config("CONFIG_VP9_HIGH") eq "yes") {

# variance
add_proto qw/unsigned int vp9_high_variance32x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance32x16/;

add_proto qw/unsigned int vp9_high_variance16x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance16x32/;

add_proto qw/unsigned int vp9_high_variance64x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance64x32/;

add_proto qw/unsigned int vp9_high_variance32x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance32x64/;

add_proto qw/unsigned int vp9_high_variance32x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance32x32/;

add_proto qw/unsigned int vp9_high_variance64x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance64x64/;

add_proto qw/unsigned int vp9_high_variance16x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance16x16/;

add_proto qw/unsigned int vp9_high_variance16x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance16x8/;

add_proto qw/unsigned int vp9_high_variance8x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance8x16/;

add_proto qw/unsigned int vp9_high_variance8x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance8x8/;

add_proto qw/unsigned int vp9_high_variance8x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance8x4/;

add_proto qw/unsigned int vp9_high_variance4x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance4x8/;

add_proto qw/unsigned int vp9_high_variance4x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_variance4x4/;

add_proto qw/void vp9_high_get8x8var/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, int *sum";
specialize qw/vp9_high_get8x8var/;

add_proto qw/void vp9_high_get16x16var/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, int *sum";
specialize qw/vp9_high_get16x16var/;

add_proto qw/unsigned int vp9_high_10_variance32x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance32x16/;

add_proto qw/unsigned int vp9_high_10_variance16x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance16x32/;

add_proto qw/unsigned int vp9_high_10_variance64x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance64x32/;

add_proto qw/unsigned int vp9_high_10_variance32x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance32x64/;

add_proto qw/unsigned int vp9_high_10_variance32x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance32x32/;

add_proto qw/unsigned int vp9_high_10_variance64x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance64x64/;

add_proto qw/unsigned int vp9_high_10_variance16x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance16x16/;

add_proto qw/unsigned int vp9_high_10_variance16x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance16x8/;

add_proto qw/unsigned int vp9_high_10_variance8x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance8x16/;

add_proto qw/unsigned int vp9_high_10_variance8x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance8x8/;

add_proto qw/unsigned int vp9_high_10_variance8x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance8x4/;

add_proto qw/unsigned int vp9_high_10_variance4x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance4x8/;

add_proto qw/unsigned int vp9_high_10_variance4x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_variance4x4/;

add_proto qw/void vp9_high_10_get8x8var/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, int *sum";
specialize qw/vp9_high_10_get8x8var/;

add_proto qw/void vp9_high_10_get16x16var/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, int *sum";
specialize qw/vp9_high_10_get16x16var/;

add_proto qw/unsigned int vp9_high_12_variance32x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance32x16/;

add_proto qw/unsigned int vp9_high_12_variance16x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance16x32/;

add_proto qw/unsigned int vp9_high_12_variance64x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance64x32/;

add_proto qw/unsigned int vp9_high_12_variance32x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance32x64/;

add_proto qw/unsigned int vp9_high_12_variance32x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance32x32/;

add_proto qw/unsigned int vp9_high_12_variance64x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance64x64/;

add_proto qw/unsigned int vp9_high_12_variance16x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance16x16/;

add_proto qw/unsigned int vp9_high_12_variance16x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance16x8/;

add_proto qw/unsigned int vp9_high_12_variance8x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance8x16/;

add_proto qw/unsigned int vp9_high_12_variance8x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance8x8/;

add_proto qw/unsigned int vp9_high_12_variance8x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance8x4/;

add_proto qw/unsigned int vp9_high_12_variance4x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance4x8/;

add_proto qw/unsigned int vp9_high_12_variance4x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_variance4x4/;

add_proto qw/void vp9_high_12_get8x8var/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, int *sum";
specialize qw/vp9_high_12_get8x8var/;

add_proto qw/void vp9_high_12_get16x16var/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, int *sum";
specialize qw/vp9_high_12_get16x16var/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance64x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance64x64/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance64x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance64x64/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance32x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance32x64/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance32x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance32x64/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance64x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance64x32/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance64x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance64x32/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance32x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance32x16/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance32x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance32x16/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance16x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance16x32/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance16x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance16x32/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance32x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance32x32/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance32x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance32x32/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance16x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance16x16/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance16x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance16x16/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance8x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance8x16/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance8x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance8x16/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance16x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance16x8/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance16x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance16x8/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance8x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance8x8/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance8x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance8x8/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance8x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance8x4/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance8x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance8x4/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance4x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance4x8/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance4x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance4x8/;

add_proto qw/unsigned int vp9_high_sub_pixel_variance4x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_sub_pixel_variance4x4/;

add_proto qw/unsigned int vp9_high_sub_pixel_avg_variance4x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_sub_pixel_avg_variance4x4/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance64x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance64x64/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance64x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance64x64/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance32x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance32x64/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance32x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance32x64/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance64x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance64x32/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance64x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance64x32/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance32x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance32x16/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance32x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance32x16/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance16x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance16x32/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance16x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance16x32/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance32x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance32x32/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance32x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance32x32/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance16x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance16x16/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance16x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance16x16/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance8x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance8x16/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance8x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance8x16/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance16x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance16x8/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance16x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance16x8/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance8x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance8x8/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance8x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance8x8/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance8x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance8x4/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance8x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance8x4/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance4x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance4x8/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance4x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance4x8/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_variance4x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_10_sub_pixel_variance4x4/;

add_proto qw/unsigned int vp9_high_10_sub_pixel_avg_variance4x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_10_sub_pixel_avg_variance4x4/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance64x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance64x64/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance64x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance64x64/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance32x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance32x64/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance32x64/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance32x64/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance64x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance64x32/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance64x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance64x32/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance32x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance32x16/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance32x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance32x16/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance16x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance16x32/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance16x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance16x32/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance32x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance32x32/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance32x32/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance32x32/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance16x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance16x16/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance16x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance16x16/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance8x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance8x16/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance8x16/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance8x16/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance16x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance16x8/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance16x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance16x8/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance8x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance8x8/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance8x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance8x8/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance8x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance8x4/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance8x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance8x4/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance4x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance4x8/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance4x8/, "const uint8_t *src_ptr, int source_stride, int xoffset, int yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance4x8/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_variance4x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse";
specialize qw/vp9_high_12_sub_pixel_variance4x4/;

add_proto qw/unsigned int vp9_high_12_sub_pixel_avg_variance4x4/, "const uint8_t *src_ptr, int source_stride, int xoffset, int  yoffset, const uint8_t *ref_ptr, int ref_stride, unsigned int *sse, const uint8_t *second_pred";
specialize qw/vp9_high_12_sub_pixel_avg_variance4x4/;

add_proto qw/unsigned int vp9_high_sad64x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_high_sad64x64/;

add_proto qw/unsigned int vp9_high_sad32x64/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_high_sad32x64/;

add_proto qw/unsigned int vp9_high_sad64x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_high_sad64x32/;

add_proto qw/unsigned int vp9_high_sad32x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_high_sad32x16/;

add_proto qw/unsigned int vp9_high_sad16x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_high_sad16x32/;

add_proto qw/unsigned int vp9_high_sad32x32/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_high_sad32x32/;

add_proto qw/unsigned int vp9_high_sad16x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_high_sad16x16/;

add_proto qw/unsigned int vp9_high_sad16x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_high_sad16x8/;

add_proto qw/unsigned int vp9_high_sad8x16/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_high_sad8x16/;

add_proto qw/unsigned int vp9_high_sad8x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_high_sad8x8/;

add_proto qw/unsigned int vp9_high_sad8x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_high_sad8x4/;

add_proto qw/unsigned int vp9_high_sad4x8/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride";
specialize qw/vp9_high_sad4x8/;

add_proto qw/unsigned int vp9_high_sad4x4/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride";
specialize qw/vp9_high_sad4x4/;

add_proto qw/unsigned int vp9_high_sad64x64_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad64x64_avg/;

add_proto qw/unsigned int vp9_high_sad32x64_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad32x64_avg/;

add_proto qw/unsigned int vp9_high_sad64x32_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad64x32_avg/;

add_proto qw/unsigned int vp9_high_sad32x16_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad32x16_avg/;

add_proto qw/unsigned int vp9_high_sad16x32_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad16x32_avg/;

add_proto qw/unsigned int vp9_high_sad32x32_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad32x32_avg/;

add_proto qw/unsigned int vp9_high_sad16x16_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad16x16_avg/;

add_proto qw/unsigned int vp9_high_sad16x8_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad16x8_avg/;

add_proto qw/unsigned int vp9_high_sad8x16_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad8x16_avg/;

add_proto qw/unsigned int vp9_high_sad8x8_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad8x8_avg/;

add_proto qw/unsigned int vp9_high_sad8x4_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad8x4_avg/;

add_proto qw/unsigned int vp9_high_sad4x8_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad4x8_avg/;

add_proto qw/unsigned int vp9_high_sad4x4_avg/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, const uint8_t *second_pred";
specialize qw/vp9_high_sad4x4_avg/;

add_proto qw/void vp9_high_sad64x64x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad64x64x3/;

add_proto qw/void vp9_high_sad32x32x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad32x32x3/;

add_proto qw/void vp9_high_sad16x16x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad16x16x3/;

add_proto qw/void vp9_high_sad16x8x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad16x8x3/;

add_proto qw/void vp9_high_sad8x16x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad8x16x3/;

add_proto qw/void vp9_high_sad8x8x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad8x8x3/;

add_proto qw/void vp9_high_sad4x4x3/, "const uint8_t *src_ptr, int source_stride, const uint8_t *ref_ptr, int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad4x4x3/;

add_proto qw/void vp9_high_sad64x64x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_high_sad64x64x8/;

add_proto qw/void vp9_high_sad32x32x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_high_sad32x32x8/;

add_proto qw/void vp9_high_sad16x16x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_high_sad16x16x8/;

add_proto qw/void vp9_high_sad16x8x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_high_sad16x8x8/;

add_proto qw/void vp9_high_sad8x16x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_high_sad8x16x8/;

add_proto qw/void vp9_high_sad8x8x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_high_sad8x8x8/;

add_proto qw/void vp9_high_sad8x4x8/, "const uint8_t *src_ptr, int src_stride, const uint8_t *ref_ptr, int ref_stride, uint32_t *sad_array";
specialize qw/vp9_high_sad8x4x8/;

add_proto qw/void vp9_high_sad4x8x8/, "const uint8_t *src_ptr, int src_stride, const uint8_t *ref_ptr, int ref_stride, uint32_t *sad_array";
specialize qw/vp9_high_sad4x8x8/;

add_proto qw/void vp9_high_sad4x4x8/, "const uint8_t *src_ptr, int  src_stride, const uint8_t *ref_ptr, int  ref_stride, uint32_t *sad_array";
specialize qw/vp9_high_sad4x4x8/;

add_proto qw/void vp9_high_sad64x64x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad64x64x4d/;

add_proto qw/void vp9_high_sad32x64x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad32x64x4d/;

add_proto qw/void vp9_high_sad64x32x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad64x32x4d/;

add_proto qw/void vp9_high_sad32x16x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad32x16x4d/;

add_proto qw/void vp9_high_sad16x32x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad16x32x4d/;

add_proto qw/void vp9_high_sad32x32x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad32x32x4d/;

add_proto qw/void vp9_high_sad16x16x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad16x16x4d/;

add_proto qw/void vp9_high_sad16x8x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad16x8x4d/;

add_proto qw/void vp9_high_sad8x16x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad8x16x4d/;

add_proto qw/void vp9_high_sad8x8x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad8x8x4d/;

# TODO(jingning): need to convert these 4x8/8x4 functions into sse2 form
add_proto qw/void vp9_high_sad8x4x4d/, "const uint8_t *src_ptr, int src_stride, const uint8_t* const ref_ptr[], int ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad8x4x4d/;

add_proto qw/void vp9_high_sad4x8x4d/, "const uint8_t *src_ptr, int src_stride, const uint8_t* const ref_ptr[], int ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad4x8x4d/;

add_proto qw/void vp9_high_sad4x4x4d/, "const uint8_t *src_ptr, int  src_stride, const uint8_t* const ref_ptr[], int  ref_stride, unsigned int *sad_array";
specialize qw/vp9_high_sad4x4x4d/;

add_proto qw/unsigned int vp9_high_mse16x16/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_mse16x16/;

add_proto qw/unsigned int vp9_high_mse8x16/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_mse8x16/;

add_proto qw/unsigned int vp9_high_mse16x8/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_mse16x8/;

add_proto qw/unsigned int vp9_high_mse8x8/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_mse8x8/;

add_proto qw/unsigned int vp9_high_10_mse16x16/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_10_mse16x16/;

add_proto qw/unsigned int vp9_high_10_mse8x16/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_10_mse8x16/;

add_proto qw/unsigned int vp9_high_10_mse16x8/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_10_mse16x8/;

add_proto qw/unsigned int vp9_high_10_mse8x8/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_10_mse8x8/;

add_proto qw/unsigned int vp9_high_12_mse16x16/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_12_mse16x16/;

add_proto qw/unsigned int vp9_high_12_mse8x16/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_12_mse8x16/;

add_proto qw/unsigned int vp9_high_12_mse16x8/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_12_mse16x8/;

add_proto qw/unsigned int vp9_high_12_mse8x8/, "const uint8_t *src_ptr, int  source_stride, const uint8_t *ref_ptr, int  recon_stride, unsigned int *sse";
specialize qw/vp9_high_12_mse8x8/;

add_proto qw/unsigned int vp9_high_get_mb_ss/, "const int16_t *";
specialize qw/vp9_high_get_mb_ss/;
# ENCODEMB INVOKE

add_proto qw/int64_t vp9_high_block_error/, "const tran_low_t *coeff, const tran_low_t *dqcoeff, intptr_t block_size, int64_t *ssz, int bps";
specialize qw/vp9_high_block_error/;

add_proto qw/void vp9_high_subtract_block/, "int rows, int cols, int16_t *diff_ptr, ptrdiff_t diff_stride, const uint8_t *src_ptr, ptrdiff_t src_stride, const uint8_t *pred_ptr, ptrdiff_t pred_stride, int bps";
specialize qw/vp9_high_subtract_block/;

add_proto qw/void vp9_high_quantize_fp/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
specialize qw/vp9_high_quantize_fp/;

add_proto qw/void vp9_high_quantize_fp_32x32/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
specialize qw/vp9_high_quantize_fp_32x32/;

add_proto qw/void vp9_high_quantize_b/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
specialize qw/vp9_high_quantize_b/;

add_proto qw/void vp9_high_quantize_b_32x32/, "const tran_low_t *coeff_ptr, intptr_t n_coeffs, int skip_block, const int16_t *zbin_ptr, const int16_t *round_ptr, const int16_t *quant_ptr, const int16_t *quant_shift_ptr, tran_low_t *qcoeff_ptr, tran_low_t *dqcoeff_ptr, const int16_t *dequant_ptr, int zbin_oq_value, uint16_t *eob_ptr, const int16_t *scan, const int16_t *iscan";
specialize qw/vp9_high_quantize_b_32x32/;

#
# Structured Similarity (SSIM)
#
if (vpx_config("CONFIG_INTERNAL_STATS") eq "yes") {
    add_proto qw/void vp9_high_ssim_parms_8x8/, "uint16_t *s, int sp, uint16_t *r, int rp, uint32_t *sum_s, uint32_t *sum_r, uint32_t *sum_sq_s, uint32_t *sum_sq_r, uint32_t *sum_sxr";
    specialize qw/vp9_high_ssim_parms_8x8/;

    add_proto qw/void vp9_high_ssim_parms_8x8_shift/, "uint16_t *s, int sp, uint16_t *r, int rp, uint32_t *sum_s, uint32_t *sum_r, uint32_t *sum_sq_s, uint32_t *sum_sq_r, uint32_t *sum_sxr, unsigned int bps, unsigned int shift";
    specialize qw/vp9_high_ssim_parms_8x8_shift/;
}

# fdct functions
add_proto qw/void vp9_high_fht4x4/, "const int16_t *input, tran_low_t *output, int stride, int tx_type";
specialize qw/vp9_high_fht4x4/;

add_proto qw/void vp9_high_fht8x8/, "const int16_t *input, tran_low_t *output, int stride, int tx_type";
specialize qw/vp9_high_fht8x8/;

add_proto qw/void vp9_high_fht16x16/, "const int16_t *input, tran_low_t *output, int stride, int tx_type";
specialize qw/vp9_high_fht16x16/;

add_proto qw/void vp9_high_fwht4x4/, "const int16_t *input, tran_low_t *output, int stride";
specialize qw/vp9_high_fwht4x4/;

add_proto qw/void vp9_high_fdct4x4/, "const int16_t *input, tran_low_t *output, int stride";
specialize qw/vp9_high_fdct4x4/;

add_proto qw/void vp9_high_fdct8x8_1/, "const int16_t *input, tran_low_t *output, int stride";
specialize qw/vp9_high_fdct8x8_1/;

add_proto qw/void vp9_high_fdct8x8/, "const int16_t *input, tran_low_t *output, int stride";
specialize qw/vp9_high_fdct8x8/;

add_proto qw/void vp9_high_fdct16x16_1/, "const int16_t *input, tran_low_t *output, int stride";
specialize qw/vp9_high_fdct16x16_1/;

add_proto qw/void vp9_high_fdct16x16/, "const int16_t *input, tran_low_t *output, int stride";
specialize qw/vp9_high_fdct16x16/;

add_proto qw/void vp9_high_fdct32x32_1/, "const int16_t *input, tran_low_t *output, int stride";
specialize qw/vp9_high_fdct32x32_1/;

add_proto qw/void vp9_high_fdct32x32/, "const int16_t *input, tran_low_t *output, int stride";
specialize qw/vp9_high_fdct32x32/;

add_proto qw/void vp9_high_fdct32x32_rd/, "const int16_t *input, tran_low_t *output, int stride";
specialize qw/vp9_high_fdct32x32_rd/;

add_proto qw/void vp9_high_temporal_filter_apply/, "uint8_t *frame1, unsigned int stride, uint8_t *frame2, unsigned int block_width, unsigned int block_height, int strength, int filter_weight, unsigned int *accumulator, uint16_t *count";
specialize qw/vp9_high_temporal_filter_apply/;

}
# End vp9_high encoder functions

}
# end encoder functions
1;
