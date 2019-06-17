// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/gif/gifmodule.h"

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/gif/cfx_gif.h"
#include "core/fxcodec/gif/cfx_gifcontext.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/ptr_util.h"

namespace fxcodec {

GifModule::GifModule() = default;

GifModule::~GifModule() = default;

std::unique_ptr<ModuleIface::Context> GifModule::Start(Delegate* pDelegate) {
  return pdfium::MakeUnique<CFX_GifContext>(this, pDelegate);
}

CFX_GifDecodeStatus GifModule::ReadHeader(Context* pContext,
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
  *pal_num = (2 << context->global_pal_exp_);
  *pal_pp = context->global_palette_.empty() ? nullptr
                                             : context->global_palette_.data();
  *bg_index = context->bc_index_;
  return CFX_GifDecodeStatus::Success;
}

std::pair<CFX_GifDecodeStatus, size_t> GifModule::LoadFrameInfo(
    Context* pContext) {
  auto* context = static_cast<CFX_GifContext*>(pContext);
  CFX_GifDecodeStatus ret = context->GetFrame();
  if (ret != CFX_GifDecodeStatus::Success)
    return {ret, 0};
  return {CFX_GifDecodeStatus::Success, context->GetFrameNum()};
}

CFX_GifDecodeStatus GifModule::LoadFrame(Context* pContext, size_t frame_num) {
  return static_cast<CFX_GifContext*>(pContext)->LoadFrame(frame_num);
}

FX_FILESIZE GifModule::GetAvailInput(Context* pContext) const {
  return static_cast<CFX_GifContext*>(pContext)->GetAvailInput();
}

bool GifModule::Input(Context* pContext,
                      RetainPtr<CFX_CodecMemory> codec_memory,
                      CFX_DIBAttribute*) {
  auto* ctx = static_cast<CFX_GifContext*>(pContext);
  ctx->SetInputBuffer(std::move(codec_memory));
  return true;
}

}  // namespace fxcodec
