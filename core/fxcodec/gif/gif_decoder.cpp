// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/gif/gif_decoder.h"

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/gif/cfx_gifcontext.h"
#include "core/fxge/dib/fx_dib.h"

namespace fxcodec {

// static
std::unique_ptr<ProgressiveDecoderIface::Context> GifDecoder::StartDecode(
    Delegate* pDelegate) {
  return std::make_unique<CFX_GifContext>(pDelegate);
}

// static
GifDecoder::Status GifDecoder::ReadHeader(
    ProgressiveDecoderIface::Context* pContext,
    int* width,
    int* height,
    int* pal_num,
    CFX_GifPalette** pal_pp,
    int* bg_index) {
  auto* context = static_cast<CFX_GifContext*>(pContext);
  Status ret = context->ReadHeader();
  if (ret != Status::kSuccess)
    return ret;

  *width = context->width_;
  *height = context->height_;
  *pal_num = (2 << context->global_palette_exp_);
  *pal_pp = context->global_palette_.empty() ? nullptr
                                             : context->global_palette_.data();
  *bg_index = context->bc_index_;
  return Status::kSuccess;
}

// static
std::pair<GifDecoder::Status, size_t> GifDecoder::LoadFrameInfo(
    ProgressiveDecoderIface::Context* pContext) {
  auto* context = static_cast<CFX_GifContext*>(pContext);
  Status ret = context->GetFrame();
  if (ret != Status::kSuccess)
    return {ret, 0};
  return {Status::kSuccess, context->GetFrameNum()};
}

// static
GifDecoder::Status GifDecoder::LoadFrame(
    ProgressiveDecoderIface::Context* pContext,
    size_t frame_num) {
  return static_cast<CFX_GifContext*>(pContext)->LoadFrame(frame_num);
}

// static
FX_FILESIZE GifDecoder::GetAvailInput(
    ProgressiveDecoderIface::Context* pContext) {
  return static_cast<CFX_GifContext*>(pContext)->GetAvailInput();
}

// static
bool GifDecoder::Input(ProgressiveDecoderIface::Context* pContext,
                       RetainPtr<CFX_CodecMemory> codec_memory) {
  auto* ctx = static_cast<CFX_GifContext*>(pContext);
  ctx->SetInputBuffer(std::move(codec_memory));
  return true;
}

}  // namespace fxcodec
