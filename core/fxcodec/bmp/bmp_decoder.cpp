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
std::unique_ptr<ProgressiveDecoderContext> BmpDecoder::StartDecode(
    Delegate* pDelegate) {
  return std::make_unique<CFX_BmpContext>(pDelegate);
}

// static
BmpDecoder::Status BmpDecoder::ReadHeader(ProgressiveDecoderContext* context,
                                          int32_t* width,
                                          int32_t* height,
                                          bool* tb_flag,
                                          int32_t* components,
                                          pdfium::span<const FX_ARGB>* palette,
                                          CFX_DIBAttribute* pAttribute) {
  DCHECK(pAttribute);

  auto* ctx = static_cast<CFX_BmpContext*>(context);
  Status status = ctx->bmp_.ReadHeader();
  if (status != Status::kSuccess) {
    return status;
  }

  *width = ctx->bmp_.width();
  *height = ctx->bmp_.height();
  *tb_flag = ctx->bmp_.img_tb_flag();
  *components = ctx->bmp_.components();
  *palette = ctx->bmp_.palette();
  pAttribute->dpi_unit_ = CFX_DIBAttribute::kResUnitMeter;
  pAttribute->x_dpi_ = ctx->bmp_.dpi_x();
  pAttribute->y_dpi_ = ctx->bmp_.dpi_y();
  return Status::kSuccess;
}

// static
BmpDecoder::Status BmpDecoder::LoadImage(ProgressiveDecoderContext* context) {
  return static_cast<CFX_BmpContext*>(context)->bmp_.DecodeImage();
}

// static
FX_FILESIZE BmpDecoder::GetAvailInput(ProgressiveDecoderContext* context) {
  return static_cast<CFX_BmpContext*>(context)->bmp_.GetAvailInput();
}

// static
bool BmpDecoder::Input(ProgressiveDecoderContext* context,
                       RetainPtr<CFX_CodecMemory> codec_memory) {
  auto* ctx = static_cast<CFX_BmpContext*>(context);
  ctx->bmp_.SetInputBuffer(std::move(codec_memory));
  return true;
}

}  // namespace fxcodec
