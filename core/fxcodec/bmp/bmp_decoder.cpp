// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/bmp/bmp_decoder.h"

#include <utility>

#include "core/fxcodec/bmp/cfx_bmpcontext.h"
#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/fx_codec_def.h"
#include "core/fxcrt/check.h"

namespace fxcodec {

// static
std::unique_ptr<ProgressiveDecoderIface::Context> BmpDecoder::StartDecode(
    Delegate* pDelegate) {
  return std::make_unique<CFX_BmpContext>(pDelegate);
}

// static
BmpDecoder::Status BmpDecoder::ReadHeader(
    ProgressiveDecoderIface::Context* pContext,
    int32_t* width,
    int32_t* height,
    bool* tb_flag,
    int32_t* components,
    pdfium::span<const FX_ARGB>* palette,
    CFX_DIBAttribute* pAttribute) {
  DCHECK(pAttribute);

  auto* ctx = static_cast<CFX_BmpContext*>(pContext);
  Status status = ctx->m_Bmp.ReadHeader();
  if (status != Status::kSuccess)
    return status;

  *width = ctx->m_Bmp.width();
  *height = ctx->m_Bmp.height();
  *tb_flag = ctx->m_Bmp.img_tb_flag();
  *components = ctx->m_Bmp.components();
  *palette = ctx->m_Bmp.palette();
  pAttribute->m_wDPIUnit = CFX_DIBAttribute::kResUnitMeter;
  pAttribute->m_nXDPI = ctx->m_Bmp.dpi_x();
  pAttribute->m_nYDPI = ctx->m_Bmp.dpi_y();
  return Status::kSuccess;
}

// static
BmpDecoder::Status BmpDecoder::LoadImage(
    ProgressiveDecoderIface::Context* pContext) {
  return static_cast<CFX_BmpContext*>(pContext)->m_Bmp.DecodeImage();
}

// static
FX_FILESIZE BmpDecoder::GetAvailInput(
    ProgressiveDecoderIface::Context* pContext) {
  return static_cast<CFX_BmpContext*>(pContext)->m_Bmp.GetAvailInput();
}

// static
bool BmpDecoder::Input(ProgressiveDecoderIface::Context* pContext,
                       RetainPtr<CFX_CodecMemory> codec_memory) {
  auto* ctx = static_cast<CFX_BmpContext*>(pContext);
  ctx->m_Bmp.SetInputBuffer(std::move(codec_memory));
  return true;
}

}  // namespace fxcodec
