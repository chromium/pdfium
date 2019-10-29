// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/bmp/bmpmodule.h"

#include <utility>

#include "core/fxcodec/bmp/cfx_bmpcontext.h"
#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/ptr_util.h"

namespace fxcodec {

BmpModule::BmpModule() = default;

BmpModule::~BmpModule() = default;

std::unique_ptr<ModuleIface::Context> BmpModule::Start(Delegate* pDelegate) {
  return pdfium::MakeUnique<CFX_BmpContext>(this, pDelegate);
}

BmpModule::Status BmpModule::ReadHeader(Context* pContext,
                                        int32_t* width,
                                        int32_t* height,
                                        bool* tb_flag,
                                        int32_t* components,
                                        int32_t* pal_num,
                                        const std::vector<uint32_t>** palette,
                                        CFX_DIBAttribute* pAttribute) {
  ASSERT(pAttribute);

  auto* ctx = static_cast<CFX_BmpContext*>(pContext);
  Status status = ctx->m_Bmp.ReadHeader();
  if (status != Status::kSuccess)
    return status;

  *width = ctx->m_Bmp.width();
  *height = ctx->m_Bmp.height();
  *tb_flag = ctx->m_Bmp.img_tb_flag();
  *components = ctx->m_Bmp.components();
  *pal_num = ctx->m_Bmp.pal_num();
  *palette = ctx->m_Bmp.palette();
  pAttribute->m_wDPIUnit = FXCODEC_RESUNIT_METER;
  pAttribute->m_nXDPI = ctx->m_Bmp.dpi_x();
  pAttribute->m_nYDPI = ctx->m_Bmp.dpi_y();
  return Status::kSuccess;
}

BmpModule::Status BmpModule::LoadImage(Context* pContext) {
  return static_cast<CFX_BmpContext*>(pContext)->m_Bmp.DecodeImage();
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
