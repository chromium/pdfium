// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/codec/ccodec_gifmodule.h"

#include "core/fxcodec/codec/codec_int.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/lgif/cgifdecompressor.h"
#include "core/fxcodec/lgif/fx_gif.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/ptr_util.h"

CCodec_GifModule::CCodec_GifModule() {
  memset(m_szLastError, 0, sizeof(m_szLastError));
}

CCodec_GifModule::~CCodec_GifModule() {}

FXGIF_Context* CCodec_GifModule::Start() {
  FXGIF_Context* p = FX_Alloc(FXGIF_Context, 1);
  if (!p)
    return nullptr;

  memset(p, 0, sizeof(FXGIF_Context));
  p->parent_ptr = this;
  p->m_Gif = pdfium::MakeUnique<CGifDecompressor>(p, m_szLastError);
  return p;
}

void CCodec_GifModule::Finish(FXGIF_Context* ctx) {
  if (ctx) {
    ctx->m_Gif = nullptr;
    FX_Free(ctx);
  }
}

GifDecodeStatus CCodec_GifModule::ReadHeader(FXGIF_Context* ctx,
                                             int* width,
                                             int* height,
                                             int* pal_num,
                                             void** pal_pp,
                                             int* bg_index,
                                             CFX_DIBAttribute* pAttribute) {
  if (setjmp(ctx->m_Gif->jmpbuf))
    return GifDecodeStatus::Error;

  GifDecodeStatus ret = gif_read_header(ctx->m_Gif.get());
  if (ret != GifDecodeStatus::Success)
    return ret;

  *width = ctx->m_Gif->width;
  *height = ctx->m_Gif->height;
  *pal_num = ctx->m_Gif->global_pal_num;
  *pal_pp = ctx->m_Gif->m_GlobalPalette.data();
  *bg_index = ctx->m_Gif->bc_index;
  return GifDecodeStatus::Success;
}

GifDecodeStatus CCodec_GifModule::LoadFrameInfo(FXGIF_Context* ctx,
                                                int* frame_num) {
  if (setjmp(ctx->m_Gif->jmpbuf))
    return GifDecodeStatus::Error;

  GifDecodeStatus ret = gif_get_frame(ctx->m_Gif.get());
  if (ret != GifDecodeStatus::Success)
    return ret;

  *frame_num = gif_get_frame_num(ctx->m_Gif.get());
  return GifDecodeStatus::Success;
}

GifDecodeStatus CCodec_GifModule::LoadFrame(FXGIF_Context* ctx,
                                            int frame_num,
                                            CFX_DIBAttribute* pAttribute) {
  if (setjmp(ctx->m_Gif->jmpbuf))
    return GifDecodeStatus::Error;

  GifDecodeStatus ret = gif_load_frame(ctx->m_Gif.get(), frame_num);
  if (ret == GifDecodeStatus::Success) {
    if (pAttribute) {
      pAttribute->m_nGifLeft =
          ctx->m_Gif->m_Images[frame_num]->m_ImageInfo.left;
      pAttribute->m_nGifTop = ctx->m_Gif->m_Images[frame_num]->m_ImageInfo.top;
      pAttribute->m_fAspectRatio = ctx->m_Gif->pixel_aspect;
      const uint8_t* buf =
          reinterpret_cast<const uint8_t*>(ctx->m_Gif->cmt_data.GetBuffer(0));
      uint32_t len = ctx->m_Gif->cmt_data.GetLength();
      if (len > 21) {
        uint8_t size = *buf++;
        if (size != 0)
          pAttribute->m_strAuthor = CFX_ByteString(buf, size);
        else
          pAttribute->m_strAuthor.clear();
      }
    }
  }
  return ret;
}

uint32_t CCodec_GifModule::GetAvailInput(FXGIF_Context* ctx,
                                         uint8_t** avail_buf_ptr) {
  return gif_get_avail_input(ctx->m_Gif.get(), avail_buf_ptr);
}

void CCodec_GifModule::Input(FXGIF_Context* ctx,
                             const uint8_t* src_buf,
                             uint32_t src_size) {
  gif_input_buffer(ctx->m_Gif.get(), (uint8_t*)src_buf, src_size);
}
