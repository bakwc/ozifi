
include_directories(${PROJECT_SOURCE_DIR}/contrib/libvpx)
include_directories(${PROJECT_SOURCE_DIR}/contrib/libvpx/third_party)

add_library(libvpx
md5_utils.c
warnings.c
#vpx_scale/win32/scaleopt.c
vpx_scale/generic
vpx_scale/generic/yv12config.c
vpx_scale/generic/vpx_scale.c
vpx_scale/generic/gen_scalers.c
vpx_scale/generic/yv12extend.c
vpx_scale/vpx_scale_rtcd.c
vpx_scale/vpx_scale_asm_offsets.c
vpx_scale/mips/dspr2/yv12extend_dspr2.c
third_party/libyuv/source/scale_argb.cc
third_party/libyuv/source/scale_mips.cc
third_party/libyuv/source/rotate_mips.cc
third_party/libyuv/source/convert_jpeg.cc
third_party/libyuv/source/compare_win.cc
third_party/libyuv/source/convert_from.cc
third_party/libyuv/source/scale.cc
third_party/libyuv/source/scale_neon.cc
third_party/libyuv/source/compare_common.cc
third_party/libyuv/source/scale_win.cc
third_party/libyuv/source/row_any.cc
third_party/libyuv/source/video_common.cc
third_party/libyuv/source/row_common.cc
third_party/libyuv/source/rotate.cc
third_party/libyuv/source/scale_neon64.cc
third_party/libyuv/source/row_neon64.cc
third_party/libyuv/source/format_conversion.cc
third_party/libyuv/source/convert_from_argb.cc
third_party/libyuv/source/row_win.cc
third_party/libyuv/source/rotate_neon.cc
third_party/libyuv/source/compare_posix.cc
third_party/libyuv/source/mjpeg_validate.cc
third_party/libyuv/source/convert.cc
third_party/libyuv/source/convert_to_i420.cc
third_party/libyuv/source/compare.cc
third_party/libyuv/source/mjpeg_decoder.cc
third_party/libyuv/source/convert_to_argb.cc
third_party/libyuv/source/compare_neon.cc
third_party/libyuv/source/rotate_argb.cc
third_party/libyuv/source/row_mips.cc
third_party/libyuv/source/cpu_id.cc
third_party/libyuv/source/convert_argb.cc
third_party/libyuv/source/row_posix.cc
third_party/libyuv/source/scale_common.cc
third_party/libyuv/source/planar_functions.cc
third_party/libyuv/source/scale_posix.cc
third_party/libyuv/source/row_neon.cc
third_party/x86inc
third_party/googletest/src
third_party/googletest/src/src
third_party/googletest/src/src/gtest_main.cc
third_party/googletest/src/src/gtest-all.cc
vpx_ports/arm_cpudetect.c
vp9/vp9_dx_iface.c
vp9/encoder/vp9_quantize.c
vp9/encoder/vp9_firstpass.c
vp9/encoder/vp9_cost.c
vp9/encoder/vp9_mcomp.c
vp9/encoder/vp9_variance.c
vp9/encoder/vp9_writer.c
vp9/encoder/vp9_pickmode.c
vp9/encoder/vp9_picklpf.c
vp9/encoder/vp9_resize.c
vp9/encoder/vp9_encoder.c
vp9/encoder/vp9_rd.c
vp9/encoder/vp9_context_tree.c
vp9/encoder/vp9_sad.c
#vp9/encoder/arm/neon/vp9_quantize_neon.c
#vp9/encoder/arm/neon/vp9_variance_neon.c
#vp9/encoder/arm/neon/vp9_dct_neon.c
#vp9/encoder/arm/neon/vp9_sad_neon.c
#vp9/encoder/arm/neon/vp9_subtract_neon.c
vp9/encoder/vp9_aq_complexity.c
vp9/encoder/vp9_speed_features.c
vp9/encoder/vp9_rdopt.c
vp9/encoder/vp9_encodemv.c
vp9/encoder/vp9_encodeframe.c
vp9/encoder/vp9_encodemb.c
vp9/encoder/vp9_segmentation.c
vp9/encoder/vp9_extend.c
vp9/encoder/vp9_denoiser.c
vp9/encoder/vp9_subexp.c
vp9/encoder/vp9_dct.c
#vp9/encoder/x86/vp9_variance_avx2.c
vp9/encoder/x86/vp9_dct_sse2.c
#vp9/encoder/x86/vp9_subpel_variance_impl_intrin_avx2.c
vp9/encoder/x86/vp9_dct32x32_sse2.c
#vp9/encoder/x86/vp9_dct_avx2.c
#vp9/encoder/x86/vp9_variance_impl_intrin_avx2.c
vp9/encoder/x86/vp9_variance_sse2.c
#vp9/encoder/x86/vp9_error_intrin_avx2.c
#vp9/encoder/x86/vp9_dct32x32_avx2.c
#vp9/encoder/x86/vp9_sad4d_intrin_avx2.c
vp9/encoder/vp9_treewriter.c
vp9/encoder/vp9_svc_layercontext.c
vp9/encoder/vp9_aq_cyclicrefresh.c
vp9/encoder/vp9_ratectrl.c
vp9/encoder/vp9_mbgraph.c
vp9/encoder/vp9_tokenize.c
vp9/encoder/vp9_lookahead.c
vp9/encoder/vp9_aq_variance.c
vp9/encoder/vp9_write_bit_buffer.c
vp9/encoder/vp9_temporal_filter.c
vp9/encoder/vp9_ssim.c
vp9/encoder/vp9_bitstream.c
vp9/decoder/vp9_detokenize.c
vp9/decoder/vp9_read_bit_buffer.c
vp9/decoder/vp9_decodeframe.c
vp9/decoder/vp9_decodemv.c
vp9/decoder/vp9_reader.c
vp9/decoder/vp9_dsubexp.c
vp9/decoder/vp9_dthread.c
vp9/decoder/vp9_decoder.c
vp9/exports_enc
vp9/exports_dec
vp9/vp9_cx_iface.c
vp9/common/vp9_loopfilter_filters.c
vp9/common/vp9_postproc.c
vp9/common/vp9_prob.c
vp9/common/vp9_debugmodes.c
vp9/common/vp9_reconintra.c
vp9/common/vp9_entropy.c
vp9/common/vp9_scan.c
vp9/common/vp9_tile_common.c
vp9/common/vp9_entropymode.c
vp9/common/vp9_blockd.c
vp9/common/arm/neon/vp9_idct16x16_neon.c
vp9/common/arm/neon/vp9_convolve_neon.c
vp9/common/arm/neon/vp9_loopfilter_16_neon.c
vp9/common/vp9_seg_common.c
vp9/common/vp9_thread.c
vp9/common/vp9_filter.c
vp9/common/vp9_rtcd.c
vp9/common/vp9_frame_buffers.c
vp9/common/vp9_alloccommon.c
#vp9/common/x86/vp9_idct_intrin_ssse3.c
#vp9/common/x86/vp9_loopfilter_intrin_avx2.c
vp9/common/x86/vp9_idct_intrin_sse2.c
#vp9/common/x86/vp9_subpixel_8t_intrin_avx2.c
vp9/common/x86/vp9_asm_stubs.c
#vp9/common/x86/vp9_subpixel_8t_intrin_ssse3.c
vp9/common/x86/vp9_loopfilter_intrin_sse2.c
vp9/common/vp9_quant_common.c
vp9/common/vp9_loopfilter.c
vp9/common/vp9_common_data.c
vp9/common/vp9_textblit.c
vp9/common/vp9_mvref_common.c
vp9/common/vp9_scale.c
vp9/common/vp9_idct.c
vp9/common/vp9_reconinter.c
vp9/common/vp9_entropymv.c
vp9/common/vp9_convolve.c
vp9/common/mips/dspr2/vp9_itrans32_cols_dspr2.c
vp9/common/mips/dspr2/vp9_convolve2_avg_horiz_dspr2.c
vp9/common/mips/dspr2/vp9_convolve2_horiz_dspr2.c
vp9/common/mips/dspr2/vp9_convolve8_avg_dspr2.c
vp9/common/mips/dspr2/vp9_convolve2_vert_dspr2.c
vp9/common/mips/dspr2/vp9_itrans8_dspr2.c
vp9/common/mips/dspr2/vp9_convolve8_avg_horiz_dspr2.c
vp9/common/mips/dspr2/vp9_mblpf_horiz_loopfilter_dspr2.c
vp9/common/mips/dspr2/vp9_convolve8_horiz_dspr2.c
vp9/common/mips/dspr2/vp9_convolve8_vert_dspr2.c
vp9/common/mips/dspr2/vp9_loopfilter_filters_dspr2.c
vp9/common/mips/dspr2/vp9_mblpf_vert_loopfilter_dspr2.c
vp9/common/mips/dspr2/vp9_itrans4_dspr2.c
vp9/common/mips/dspr2/vp9_intrapred16_dspr2.c
vp9/common/mips/dspr2/vp9_convolve2_dspr2.c
vp9/common/mips/dspr2/vp9_intrapred8_dspr2.c
vp9/common/mips/dspr2/vp9_itrans32_dspr2.c
vp9/common/mips/dspr2/vp9_convolve2_avg_dspr2.c
vp9/common/mips/dspr2/vp9_itrans16_dspr2.c
vp9/common/mips/dspr2/vp9_intrapred4_dspr2.c
vp9/common/mips/dspr2/vp9_convolve8_dspr2.c
vp9/common/mips/dspr2/vp9_mbloop_loopfilter_dspr2.c
vp9/common/vp9_pred_common.c
tools_common.c
webmenc.cc
vpx_config.c
video_writer.c
examples/resize_util.c
examples/decode_with_drops.c
examples/postproc.c
examples/set_maps.c
examples/simple_decoder.c
examples/simple_encoder.c
examples/vp8_multi_resolution_encoder.c
examples/decode_to_md5.c
examples/twopass_encoder.c
examples/vp8cx_set_ref.c
examples/vp9_spatial_svc_encoder.c
examples/vpx_temporal_svc_encoder.c
vpxdec.c
y4minput.c
ivfenc.c
vpx/src
vpx/src/vpx_encoder.c
vpx/src/vpx_codec.c
vpx/src/svc_encodeframe.c
vpx/src/vpx_decoder.c
vpx/src/vpx_image.c
vpx/src/vpx_psnr.c
vpx/exports_enc
vpx/exports_dec
ivfdec.c
build/make/obj_int_extract.c
webmdec.cc
vpxstats.c
vpx_mem/vpx_mem.c
vpx_mem/vpx_mem_tracker.c
vpx_mem/memory_manager/hmm_largest.c
vpx_mem/memory_manager/hmm_shrink.c
vpx_mem/memory_manager/hmm_base.c
vpx_mem/memory_manager/hmm_grow.c
vpx_mem/memory_manager/hmm_dflt_abort.c
vpx_mem/memory_manager/hmm_resize.c
vpx_mem/memory_manager/hmm_true.c
vpx_mem/memory_manager/hmm_alloc.c
rate_hist.c
video_reader.c
y4menc.c
args.c
vpxenc.c
vp8/vp8_cx_iface.c
vp8/encoder/boolhuff.c
vp8/encoder/onyx_if.c
vp8/encoder/tokenize.c
vp8/encoder/lookahead.c
vp8/encoder/mcomp.c
vp8/encoder/temporal_filter.c
vp8/encoder/modecosts.c
#vp8/encoder/arm/neon/shortfdct_neon.c
#vp8/encoder/arm/neon/vp8_shortwalsh4x4_neon.c
#vp8/encoder/arm/neon/subtract_neon.c
#vp8/encoder/arm/neon/picklpf_arm.c
#vp8/encoder/arm/neon/denoising_neon.c
#vp8/encoder/arm/quantize_arm.c
#vp8/encoder/arm/boolhuff_arm.c
#vp8/encoder/arm/dct_arm.c
vp8/encoder/ppc
#vp8/encoder/ppc/csystemdependent.c
vp8/encoder/firstpass.c
vp8/encoder/pickinter.c
vp8/encoder/rdopt.c
vp8/encoder/ssim.c
vp8/encoder/segmentation.c
vp8/encoder/x86/denoising_sse2.c
vp8/encoder/x86/vp8_enc_stubs_mmx.c
vp8/encoder/x86/vp8_enc_stubs_sse2.c
#vp8/encoder/x86/quantize_sse4.c
vp8/encoder/x86/quantize_sse2.c
#vp8/encoder/x86/quantize_ssse3.c
vp8/encoder/ethreading.c
vp8/encoder/encodemb.c
vp8/encoder/dct.c
vp8/encoder/quantize.c
vp8/encoder/vp8_asm_enc_offsets.c
vp8/encoder/ratectrl.c
vp8/encoder/encodeframe.c
vp8/encoder/encodemv.c
vp8/encoder/bitstream.c
vp8/encoder/encodeintra.c
vp8/encoder/denoising.c
vp8/encoder/treewriter.c
#vp8/encoder/mr_dissim.c
vp8/encoder/picklpf.c
vp8/decoder/decodeframe.c
vp8/decoder/dboolhuff.c
vp8/decoder/threading.c
#vp8/decoder/error_concealment.c
vp8/decoder/decodemv.c
vp8/decoder/detokenize.c
vp8/decoder/onyxd_if.c
vp8/vp8_dx_iface.c
vp8/exports_enc
vp8/exports_dec
vp8/common/entropy.c
vp8/common/filter.c
vp8/common/reconinter.c
vp8/common/extend.c
vp8/common/idct_blk.c
vp8/common/reconintra.c
vp8/common/textblit.c
vp8/common/generic
#vp8/common/generic/systemdependent.c
vp8/common/loopfilter.c
vp8/common/postproc.c
vp8/common/setupintrarecon.c
vp8/common/context.c
vp8/common/entropymv.c
vp8/common/mbpitch.c
#vp8/common/arm/variance_arm.c
#vp8/common/arm/loopfilter_arm.c
#vp8/common/arm/neon/vp8_subpixelvariance_neon.c
#vp8/common/arm/neon/bilinearpredict_neon.c
#vp8/common/arm/neon/iwalsh_neon.c
#vp8/common/arm/neon/loopfiltersimpleverticaledge_neon.c
#vp8/common/arm/neon/copymem_neon.c
#vp8/common/arm/neon/loopfiltersimplehorizontaledge_neon.c
#vp8/common/arm/neon/sad_neon.c
#vp8/common/arm/neon/loopfilter_neon.c
#vp8/common/arm/neon/reconintra_neon.c
#vp8/common/arm/neon/dc_only_idct_add_neon.c
#vp8/common/arm/neon/idct_dequant_full_2x_neon.c
#vp8/common/arm/neon/shortidct4x4llm_neon.c
#vp8/common/arm/neon/idct_dequant_0_2x_neon.c
#vp8/common/arm/neon/dequantizeb_neon.c
#vp8/common/arm/neon/sixtappredict_neon.c
#vp8/common/arm/neon/mbloopfilter_neon.c
#vp8/common/arm/neon/idct_blk_neon.c
#vp8/common/arm/neon/dequant_idct_neon.c
#vp8/common/arm/neon/variance_neon.c
#vp8/common/arm/bilinearfilter_arm.c
#vp8/common/arm/armv6/idct_blk_v6.c
#vp8/common/arm/dequantize_arm.c
#vp8/common/arm/filter_arm.c
vp8/common/dequantize.c
vp8/common/entropymode.c
vp8/common/rtcd.c
vp8/common/ppc
#vp8/common/ppc/systemdependent.c
#vp8/common/ppc/loopfilter_altivec.c
vp8/common/modecont.c
vp8/common/treecoder.c
vp8/common/quant_common.c
vp8/common/mfqe.c
vp8/common/idctllm.c
vp8/common/x86/idct_blk_sse2.c
vp8/common/x86/vp8_asm_stubs.c
vp8/common/x86/loopfilter_x86.c
vp8/common/x86/idct_blk_mmx.c
vp8/common/x86/variance_sse2.c
vp8/common/x86/recon_wrapper_sse2.c
vp8/common/x86/filter_x86.c
#vp8/common/x86/variance_ssse3.c
vp8/common/x86/variance_mmx.c
vp8/common/loopfilter_filters.c
vp8/common/variance_c.c
vp8/common/blockd.c
vp8/common/findnearmv.c
vp8/common/alloccommon.c
vp8/common/swapyv12buffer.c
vp8/common/sad_c.c
vp8/common/reconintra4x4.c
vp8/common/debugmodes.c
vp8/common/mips/dspr2/idct_blk_dspr2.c
vp8/common/mips/dspr2/idctllm_dspr2.c
vp8/common/mips/dspr2/reconinter_dspr2.c
vp8/common/mips/dspr2/dequantize_dspr2.c
vp8/common/mips/dspr2/loopfilter_filters_dspr2.c
vp8/common/mips/dspr2/filter_dspr2.c
)
