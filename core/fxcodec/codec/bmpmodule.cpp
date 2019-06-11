// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/codec/bmpmodule.h"

#include <utility>

#include "core/fxcodec/bmp/cfx_bmpcontext.h"
#include "core/fxcodec/codec/cfx_codec_memory.h"
#include "core/fxcodec/codec/codec_int.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/ptr_util.h"

namespace fxcodec {

BmpModule::BmpModule() = default;

BmpModule::~BmpModule() = default;

std::unique_ptr<CodecModuleIface::Context> BmpModule::Start(
    Delegate* pDelegate) {
  auto p = pdfium::MakeUnique<CFX_BmpContext>(this, pDelegate);
  p->m_Bmp.context_ptr_ = p.get();
  return p;
}

int32_t BmpModule::ReadHeader(Context* pContext,
                              int32_t* width,
                              int32_t* height,
                              bool* tb_flag,
                              int32_t* components,
                              int32_t* pal_num,
                              std::vector<uint32_t>* palette,
                              CFX_DIBAttribute* pAttribute) {
  ASSERT(pAttribute);

  auto* ctx = static_cast<CFX_BmpContext*>(pContext);
  if (setjmp(ctx->m_Bmp.jmpbuf_))
    return 0;

  int32_t ret = ctx->m_Bmp.ReadHeader();
  if (ret != 1)
    return ret;

  *width = ctx->m_Bmp.width_;
  *height = ctx->m_Bmp.height_;
  *tb_flag = ctx->m_Bmp.imgTB_flag_;
  *components = ctx->m_Bmp.components_;
  *pal_num = ctx->m_Bmp.pal_num_;
  *palette = ctx->m_Bmp.palette_;
  pAttribute->m_wDPIUnit = FXCODEC_RESUNIT_METER;
  pAttribute->m_nXDPI = ctx->m_Bmp.dpi_x_;
  pAttribute->m_nYDPI = ctx->m_Bmp.dpi_y_;
  return 1;
}

int32_t BmpModule::LoadImage(Context* pContext) {
  auto* ctx = static_cast<CFX_BmpContext*>(pContext);
  if (setjmp(ctx->m_Bmp.jmpbuf_))
    return 0;

  return ctx->m_Bmp.DecodeImage();
}

FX_FILESIZE BmpModule::GetAvailInput(Context* pContext) const {
  return static_cast<CFX_BmpContext*>(pContext)->m_Bmp.GetAvailInput();
}

bool BmpModule::Input(Context* pContext,
                      RetainPtr<CFX_CodecMemory> codec_memory,
                      CFX_DIBAttribute*) {
  auto* ctx = static_cast<CFX_BmpContext*>(pContext);
  ctx->m_Bmp.SetInputBuffer(std::move(codec_memory));
  return true;
}

}  // namespace fxcodec
