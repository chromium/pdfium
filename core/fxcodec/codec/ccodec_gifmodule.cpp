// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/codec/ccodec_gifmodule.h"

#include "core/fxcodec/codec/codec_int.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/lgif/cgifcontext.h"
#include "core/fxcodec/lgif/fx_gif.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/ptr_util.h"

CCodec_GifModule::CCodec_GifModule() {
  memset(m_szLastError, 0, sizeof(m_szLastError));
}

CCodec_GifModule::~CCodec_GifModule() {}

std::unique_ptr<CGifContext> CCodec_GifModule::Start() {
  return pdfium::MakeUnique<CGifContext>(this, m_szLastError);
}

GifDecodeStatus CCodec_GifModule::ReadHeader(CGifContext* context,
                                             int* width,
                                             int* height,
                                             int* pal_num,
                                             void** pal_pp,
                                             int* bg_index,
                                             CFX_DIBAttribute* pAttribute) {
  GifDecodeStatus ret = gif_read_header(context);
  if (ret != GifDecodeStatus::Success)
    return ret;

  *width = context->width;
  *height = context->height;
  *pal_num = context->global_pal_num;
  *pal_pp = context->m_GlobalPalette.empty() ? nullptr
                                             : context->m_GlobalPalette.data();
  *bg_index = context->bc_index;
  return GifDecodeStatus::Success;
}

GifDecodeStatus CCodec_GifModule::LoadFrameInfo(CGifContext* context,
                                                int* frame_num) {
  GifDecodeStatus ret = gif_get_frame(context);
  if (ret != GifDecodeStatus::Success)
    return ret;

  *frame_num = gif_get_frame_num(context);
  return GifDecodeStatus::Success;
}

GifDecodeStatus CCodec_GifModule::LoadFrame(CGifContext* context,
                                            int frame_num,
                                            CFX_DIBAttribute* pAttribute) {
  GifDecodeStatus ret = gif_load_frame(context, frame_num);
  if (ret != GifDecodeStatus::Success || !pAttribute)
    return ret;

  pAttribute->m_nGifLeft = context->m_Images[frame_num]->m_ImageInfo.left;
  pAttribute->m_nGifTop = context->m_Images[frame_num]->m_ImageInfo.top;
  pAttribute->m_fAspectRatio = context->pixel_aspect;
  const uint8_t* buf =
      reinterpret_cast<const uint8_t*>(context->cmt_data.GetBuffer(0));
  uint32_t len = context->cmt_data.GetLength();
  if (len > 21) {
    uint8_t size = *buf++;
    if (size != 0)
      pAttribute->m_strAuthor = CFX_ByteString(buf, size);
    else
      pAttribute->m_strAuthor.clear();
  }
  return GifDecodeStatus::Success;
}

uint32_t CCodec_GifModule::GetAvailInput(CGifContext* context,
                                         uint8_t** avail_buf_ptr) {
  return gif_get_avail_input(context, avail_buf_ptr);
}

void CCodec_GifModule::Input(CGifContext* context,
                             const uint8_t* src_buf,
                             uint32_t src_size) {
  gif_input_buffer(context, (uint8_t*)src_buf, src_size);
}
