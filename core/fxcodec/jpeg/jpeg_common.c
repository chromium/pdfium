// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jpeg/jpeg_common.h"

// This is a thin C wrapper around the JPEG API to avoid calling setjmp() from
// C++ code.

boolean jpeg_common_create_decompress(JpegCommon* jpeg_common) {
  if (setjmp(jpeg_common->jmpbuf) == -1) {
    return FALSE;
  }
  jpeg_create_decompress(&jpeg_common->cinfo);
  return TRUE;
}

void jpeg_common_destroy_decompress(JpegCommon* jpeg_common) {
  jpeg_destroy_decompress(&jpeg_common->cinfo);
}

boolean jpeg_common_start_decompress(JpegCommon* jpeg_common) {
  if (setjmp(jpeg_common->jmpbuf) == -1) {
    return FALSE;
  }
  return jpeg_start_decompress(&jpeg_common->cinfo);
}

int jpeg_common_read_header(JpegCommon* jpeg_common, boolean flag) {
  if (setjmp(jpeg_common->jmpbuf) == -1) {
    return -1;
  }
  return jpeg_read_header(&jpeg_common->cinfo, flag);
}

int jpeg_common_read_scanlines(JpegCommon* jpeg_common,
                               void* buf,
                               unsigned int count) {
  if (setjmp(jpeg_common->jmpbuf) == -1) {
    return -1;
  }
  return jpeg_read_scanlines(&jpeg_common->cinfo, buf, count);
}

void jpeg_common_src_do_nothing(j_decompress_ptr cinfo) {}

boolean jpeg_common_src_fill_buffer(j_decompress_ptr cinfo) {
  return FALSE;
}

boolean jpeg_common_src_resync(j_decompress_ptr cinfo, int desired) {
  return FALSE;
}

void jpeg_common_error_do_nothing(j_common_ptr cinfo) {}

void jpeg_common_error_do_nothing_int(j_common_ptr cinfo, int arg) {}

void jpeg_common_error_do_nothing_char(j_common_ptr cinfo, char* arg) {}
