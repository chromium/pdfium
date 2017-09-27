// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/codec/ccodec_bmpmodule.h"

#include "core/fxcodec/codec/codec_int.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/lbmp/fx_bmp.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/ptr_util.h"

CBmpContext::CBmpContext(CCodec_BmpModule* pModule,
                         CCodec_BmpModule::Delegate* pDelegate)
    : m_pModule(pModule), m_pDelegate(pDelegate) {
}

CBmpContext::~CBmpContext() {}

CCodec_BmpModule::CCodec_BmpModule() {}

CCodec_BmpModule::~CCodec_BmpModule() {}

std::unique_ptr<CCodec_BmpModule::Context> CCodec_BmpModule::Start(
    Delegate* pDelegate) {
  auto p = pdfium::MakeUnique<CBmpContext>(this, pDelegate);
  p->m_Bmp.context_ptr = p.get();
  return p;
}

int32_t CCodec_BmpModule::ReadHeader(Context* pContext,
                                     int32_t* width,
                                     int32_t* height,
                                     bool* tb_flag,
                                     int32_t* components,
                                     int32_t* pal_num,
                                     std::vector<uint32_t>* palette,
                                     CFX_DIBAttribute* pAttribute) {
  auto* ctx = static_cast<CBmpContext*>(pContext);
  if (setjmp(ctx->m_Bmp.jmpbuf))
    return 0;

  int32_t ret = ctx->m_Bmp.ReadHeader();
  if (ret != 1)
    return ret;

  *width = ctx->m_Bmp.width;
  *height = ctx->m_Bmp.height;
  *tb_flag = ctx->m_Bmp.imgTB_flag;
  *components = ctx->m_Bmp.components;
  *pal_num = ctx->m_Bmp.pal_num;
  *palette = ctx->m_Bmp.palette;
  if (pAttribute) {
    pAttribute->m_wDPIUnit = FXCODEC_RESUNIT_METER;
    pAttribute->m_nXDPI = ctx->m_Bmp.dpi_x;
    pAttribute->m_nYDPI = ctx->m_Bmp.dpi_y;
    pAttribute->m_nBmpCompressType = ctx->m_Bmp.compress_flag;
  }
  return 1;
}

int32_t CCodec_BmpModule::LoadImage(Context* pContext) {
  auto* ctx = static_cast<CBmpContext*>(pContext);
  if (setjmp(ctx->m_Bmp.jmpbuf))
    return 0;

  return ctx->m_Bmp.DecodeImage();
}

uint32_t CCodec_BmpModule::GetAvailInput(Context* pContext,
                                         uint8_t** avail_buf_ptr) {
  auto* ctx = static_cast<CBmpContext*>(pContext);
  return ctx->m_Bmp.GetAvailInput(avail_buf_ptr);
}

void CCodec_BmpModule::Input(Context* pContext,
                             const uint8_t* src_buf,
                             uint32_t src_size) {
  auto* ctx = static_cast<CBmpContext*>(pContext);
  ctx->m_Bmp.SetInputBuffer(const_cast<uint8_t*>(src_buf), src_size);
}
