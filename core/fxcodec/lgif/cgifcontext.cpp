// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/lgif/cgifcontext.h"

#include <utility>

#include "core/fxcodec/codec/ccodec_gifmodule.h"
#include "core/fxcodec/lgif/fx_gif.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CGifContext::CGifContext(CCodec_GifModule* gif_module,
                         CCodec_GifModule::Delegate* pDelegate)
    : m_pModule(gif_module),
      m_pDelegate(pDelegate),
      global_pal_num(0),
      img_row_offset(0),
      img_row_avail_size(0),
      avail_in(0),
      decode_status(GIF_D_STATUS_SIG),
      skip_size(0),
      next_in(nullptr),
      width(0),
      height(0),
      bc_index(0),
      pixel_aspect(0),
      global_sort_flag(0),
      global_color_resolution(0),
      img_pass_num(0) {
  memset(m_szLastError, 0, sizeof(m_szLastError));
}

CGifContext::~CGifContext() {}

void CGifContext::AddError(const char* err_msg) {
  strncpy(m_szLastError, err_msg, GIF_MAX_ERROR_SIZE - 1);
}

void CGifContext::RecordCurrentPosition(uint32_t* cur_pos_ptr) {
  m_pDelegate->GifRecordCurrentPosition(*cur_pos_ptr);
}

void CGifContext::ReadScanline(int32_t row_num, uint8_t* row_buf) {
  m_pDelegate->GifReadScanline(row_num, row_buf);
}

bool CGifContext::GetRecordPosition(uint32_t cur_pos,
                                    int32_t left,
                                    int32_t top,
                                    int32_t width,
                                    int32_t height,
                                    int32_t pal_num,
                                    GifPalette* pal_ptr,
                                    int32_t delay_time,
                                    bool user_input,
                                    int32_t trans_index,
                                    int32_t disposal_method,
                                    bool interlace) {
  return m_pDelegate->GifInputRecordPositionBuf(
      cur_pos, FX_RECT(left, top, left + width, top + height), pal_num, pal_ptr,
      delay_time, user_input, trans_index, disposal_method, interlace);
}
