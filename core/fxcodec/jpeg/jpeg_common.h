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
#elif defined(USE_LIBJPEG_TURBO)
#include "third_party/libjpeg_turbo/jerror.h"
#include "third_party/libjpeg_turbo/jpeglib.h"
#else
#include "third_party/libjpeg/jerror.h"
#include "third_party/libjpeg/jpeglib.h"
#endif

struct JpegCommon {
  jmp_buf jmpbuf;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr error_mgr;
  struct jpeg_source_mgr source_mgr;
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
void jpeg_common_error_do_nothing(j_common_ptr cinfo);
void jpeg_common_error_do_nothing_int(j_common_ptr cinfo, int arg);
void jpeg_common_error_do_nothing_char(j_common_ptr cinfo, char* arg);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // CORE_FXCODEC_JPEG_JPEG_COMMON_H_
