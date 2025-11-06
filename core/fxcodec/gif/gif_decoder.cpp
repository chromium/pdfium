// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/gif/gif_decoder.h"

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/gif/cfx_gifcontext.h"
#include "core/fxge/dib/fx_dib.h"

namespace fxcodec {

// static
std::unique_ptr<ProgressiveDecoderContext> GifDecoder::StartDecode(
    Delegate* pDelegate) {
  return std::make_unique<CFX_GifContext>(pDelegate);
}

// static
GifDecoder::Status GifDecoder::ReadHeader(ProgressiveDecoderContext* context,
                                          int* width,
                                          int* height,
                                          pdfium::span<CFX_GifPalette>* palette,
                                          int* bg_index) {
  auto* ctx = static_cast<CFX_GifContext*>(context);
  Status ret = ctx->ReadHeader();
  if (ret != Status::kSuccess) {
    return ret;
  }

  *width = ctx->width_;
  *height = ctx->height_;
  *palette = ctx->global_palette_;
  *bg_index = ctx->bc_index_;
  return Status::kSuccess;
}

// static
std::pair<GifDecoder::Status, size_t> GifDecoder::LoadFrameInfo(
    ProgressiveDecoderContext* context) {
  auto* ctx = static_cast<CFX_GifContext*>(context);
  Status ret = ctx->GetFrame();
  if (ret != Status::kSuccess) {
    return {ret, 0};
  }
  return {Status::kSuccess, ctx->GetFrameNum()};
}

// static
GifDecoder::Status GifDecoder::LoadFrame(ProgressiveDecoderContext* context,
                                         size_t frame_num) {
  return static_cast<CFX_GifContext*>(context)->LoadFrame(frame_num);
}

// static
FX_FILESIZE GifDecoder::GetAvailInput(ProgressiveDecoderContext* context) {
  return static_cast<CFX_GifContext*>(context)->GetAvailInput();
}

// static
bool GifDecoder::Input(ProgressiveDecoderContext* context,
                       RetainPtr<CFX_CodecMemory> codec_memory) {
  auto* ctx = static_cast<CFX_GifContext*>(context);
  ctx->SetInputBuffer(std::move(codec_memory));
  return true;
}

}  // namespace fxcodec
