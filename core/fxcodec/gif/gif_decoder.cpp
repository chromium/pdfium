// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/gif/gif_decoder.h"

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/gif/cfx_gifcontext.h"
#include "core/fxge/fx_dib.h"

namespace fxcodec {

// static
std::unique_ptr<ProgressiveDecoderIface::Context> GifDecoder::StartDecode(
    Delegate* pDelegate) {
  return std::make_unique<CFX_GifContext>(pDelegate);
}

// static
CFX_GifDecodeStatus GifDecoder::ReadHeader(
    ProgressiveDecoderIface::Context* pContext,
    int* width,
    int* height,
    int* pal_num,
    CFX_GifPalette** pal_pp,
    int* bg_index) {
  auto* context = static_cast<CFX_GifContext*>(pContext);
  CFX_GifDecodeStatus ret = context->ReadHeader();
  if (ret != CFX_GifDecodeStatus::Success)
    return ret;

  *width = context->width_;
  *height = context->height_;
  *pal_num = (2 << context->global_palette_exp_);
  *pal_pp = context->global_palette_.empty() ? nullptr
                                             : context->global_palette_.data();
  *bg_index = context->bc_index_;
  return CFX_GifDecodeStatus::Success;
}

// static
std::pair<CFX_GifDecodeStatus, size_t> GifDecoder::LoadFrameInfo(
    ProgressiveDecoderIface::Context* pContext) {
  auto* context = static_cast<CFX_GifContext*>(pContext);
  CFX_GifDecodeStatus ret = context->GetFrame();
  if (ret != CFX_GifDecodeStatus::Success)
    return {ret, 0};
  return {CFX_GifDecodeStatus::Success, context->GetFrameNum()};
}

// static
CFX_GifDecodeStatus GifDecoder::LoadFrame(
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
                       RetainPtr<CFX_CodecMemory> codec_memory,
                       CFX_DIBAttribute*) {
  auto* ctx = static_cast<CFX_GifContext*>(pContext);
  ctx->SetInputBuffer(std::move(codec_memory));
  return true;
}

}  // namespace fxcodec
