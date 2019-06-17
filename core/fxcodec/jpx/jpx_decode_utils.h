// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JPX_JPX_DECODE_UTILS_H_
#define CORE_FXCODEC_JPX_JPX_DECODE_UTILS_H_

#include <stdint.h>

#if defined(USE_SYSTEM_LIBOPENJPEG2)
#include <openjpeg.h>
#else
#include "third_party/libopenjpeg20/openjpeg.h"
#endif

namespace fxcodec {

struct DecodeData {
  DecodeData(const uint8_t* data, OPJ_SIZE_T size)
      : src_data(data), src_size(size), offset(0) {}

  const uint8_t* src_data;
  OPJ_SIZE_T src_size;
  OPJ_SIZE_T offset;
};

/* Wrappers for C-style callbacks. */
OPJ_SIZE_T opj_read_from_memory(void* p_buffer,
                                OPJ_SIZE_T nb_bytes,
                                void* p_user_data);
OPJ_OFF_T opj_skip_from_memory(OPJ_OFF_T nb_bytes, void* p_user_data);
OPJ_BOOL opj_seek_from_memory(OPJ_OFF_T nb_bytes, void* p_user_data);

}  // namespace fxcodec

#endif  // CORE_FXCODEC_JPX_JPX_DECODE_UTILS_H_
