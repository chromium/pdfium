// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JPEG_JPEG_COMMON_H_
#define CORE_FXCODEC_JPEG_JPEG_COMMON_H_

// Common code for interacting with libjpeg shared by other files in
// core/fxcodec/jpeg/. Not intended to be included in headers.

#include <setjmp.h>
#include <stdio.h>

#include "build/build_config.h"

#if BUILDFLAG(IS_WIN)
// windows.h must come before the third_party/libjpeg_turbo includes.
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#undef FAR
#if defined(USE_SYSTEM_LIBJPEG)
#include <jerror.h>
#include <jpeglib.h>
#else
#include "jerror.h"   // NOLINT(build/include_directory)
#include "jpeglib.h"  // NOLINT(build/include_directory)
#endif

struct JpegCommon {
  jmp_buf jmpbuf;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr error_mgr;
  struct jpeg_source_mgr source_mgr;
  unsigned int skip_size;
};
typedef struct JpegCommon JpegCommon;

boolean jpeg_common_create_decompress(JpegCommon* jpeg_common);
void jpeg_common_destroy_decompress(JpegCommon* jpeg_common);
boolean jpeg_common_start_decompress(JpegCommon* jpeg_common);
int jpeg_common_read_header(JpegCommon* jpeg_common, boolean flag);
int jpeg_common_read_scanlines(JpegCommon* jpeg_common,
                               void* buf,
                               unsigned int count);

//  Callbacks.
void jpeg_common_src_do_nothing(j_decompress_ptr cinfo);
boolean jpeg_common_src_fill_buffer(j_decompress_ptr cinfo);
boolean jpeg_common_src_resync(j_decompress_ptr cinfo, int desired);
void jpeg_common_src_skip_data_or_record(j_decompress_ptr cinfo, long num);
void jpeg_common_src_skip_data_or_trap(j_decompress_ptr cinfo, long num);
void jpeg_common_error_do_nothing(j_common_ptr cinfo);
void jpeg_common_error_do_nothing_int(j_common_ptr cinfo, int arg);
void jpeg_common_error_do_nothing_char(j_common_ptr cinfo, char* arg);
void jpeg_common_error_fatal(j_common_ptr cinfo);

#if BUILDFLAG(IS_WIN)
void jpeg_common_dest_do_nothing(j_compress_ptr cinfo);
boolean jpeg_common_dest_empty(j_compress_ptr cinfo);
#endif  // BUILDFLAG(IS_WIN)

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // CORE_FXCODEC_JPEG_JPEG_COMMON_H_
