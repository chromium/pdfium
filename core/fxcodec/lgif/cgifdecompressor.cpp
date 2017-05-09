// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/lgif/cgifdecompressor.h"

#include <utility>

#include "core/fxcodec/codec/ccodec_gifmodule.h"
#include "core/fxcodec/lgif/fx_gif.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

FXGIF_Context::FXGIF_Context() {}

FXGIF_Context::~FXGIF_Context() {}

CGifDecompressor::CGifDecompressor(FXGIF_Context* p, char* error_string)
    : decode_status(GIF_D_STATUS_SIG),
      err_ptr(error_string),
      context_ptr(p),
      cmt_data_ptr(new CFX_ByteString),
      pt_ptr_arr_ptr(new std::vector<GifPlainText*>),
      img_ptr_arr_ptr(new std::vector<GifImage*>) {}

CGifDecompressor::~CGifDecompressor() {
  // TODO(npm): fix ownership in CGifDecompressor to avoid all of the frees and
  // deletes in here.
  GifPalette* pGlobalPal = m_GlobalPalette.data();
  if (img_ptr_arr_ptr) {
    size_t size_img_arr = img_ptr_arr_ptr->size();
    for (size_t i = 0; i < size_img_arr; i++) {
      GifImage* p = (*img_ptr_arr_ptr)[i];
      FX_Free(p->image_info_ptr);
      FX_Free(p->image_gce_ptr);
      FX_Free(p->image_row_buf);
      if (p->local_pal_ptr && p->local_pal_ptr != pGlobalPal)
        FX_Free(p->local_pal_ptr);
      FX_Free(p);
    }
    img_ptr_arr_ptr->clear();
    delete img_ptr_arr_ptr;
  }
  delete cmt_data_ptr;
  FX_Free(gce_ptr);
  if (pt_ptr_arr_ptr) {
    size_t size_pt_arr = pt_ptr_arr_ptr->size();
    for (size_t i = 0; i < size_pt_arr; i++) {
      GifPlainText* p = (*pt_ptr_arr_ptr)[i];
      FX_Free(p->gce_ptr);
      FX_Free(p->pte_ptr);
      delete p->string_ptr;
      FX_Free(p);
    }
    pt_ptr_arr_ptr->clear();
    delete pt_ptr_arr_ptr;
  }
}

void CGifDecompressor::ErrorData(const char* err_msg) {
  strncpy(err_ptr, err_msg, GIF_MAX_ERROR_SIZE - 1);
  longjmp(jmpbuf, 1);
}

uint8_t* CGifDecompressor::AskBufForPal(int32_t pal_size) {
  return context_ptr->parent_ptr->GetDelegate()->GifAskLocalPaletteBuf(
      gif_get_frame_num(this), pal_size);
}

void CGifDecompressor::RecordCurrentPosition(uint32_t* cur_pos_ptr) {
  context_ptr->parent_ptr->GetDelegate()->GifRecordCurrentPosition(
      *cur_pos_ptr);
}

void CGifDecompressor::ReadScanline(int32_t row_num, uint8_t* row_buf) {
  context_ptr->parent_ptr->GetDelegate()->GifReadScanline(row_num, row_buf);
}

bool CGifDecompressor::GetRecordPosition(uint32_t cur_pos,
                                         int32_t left,
                                         int32_t top,
                                         int32_t width,
                                         int32_t height,
                                         int32_t pal_num,
                                         void* pal_ptr,
                                         int32_t delay_time,
                                         bool user_input,
                                         int32_t trans_index,
                                         int32_t disposal_method,
                                         bool interlace) {
  return context_ptr->parent_ptr->GetDelegate()->GifInputRecordPositionBuf(
      cur_pos, FX_RECT(left, top, left + width, top + height), pal_num, pal_ptr,
      delay_time, user_input, trans_index, disposal_method, interlace);
}
