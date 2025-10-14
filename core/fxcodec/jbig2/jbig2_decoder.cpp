// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jbig2/jbig2_decoder.h"

#include <algorithm>

#include "core/fxcodec/jbig2/JBig2_Context.h"
#include "core/fxcodec/jbig2/JBig2_DocumentContext.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/span_util.h"

namespace fxcodec {

namespace {

FXCODEC_STATUS Decode(Jbig2Context* pJbig2Context, bool decode_success) {
  FXCODEC_STATUS status = pJbig2Context->context_->GetProcessingStatus();
  if (status != FXCODEC_STATUS::kDecodeFinished) {
    return status;
  }
  pJbig2Context->context_.reset();
  if (!decode_success) {
    return FXCODEC_STATUS::kError;
  }
  uint32_t byte_size = pJbig2Context->height_ * pJbig2Context->dest_pitch_;
  pdfium::span<uint8_t> nonraw_span(pJbig2Context->dest_buf_.first(byte_size));
  auto dword_span = fxcrt::reinterpret_span<uint32_t>(nonraw_span);
  for (auto& pix : dword_span) {
    pix = ~pix;
  }
  return FXCODEC_STATUS::kDecodeFinished;
}

}  // namespace

Jbig2Context::Jbig2Context() = default;

Jbig2Context::~Jbig2Context() = default;

// static
FXCODEC_STATUS Jbig2Decoder::StartDecode(
    Jbig2Context* pJbig2Context,
    JBig2_DocumentContext* pJBig2DocumentContext,
    uint32_t width,
    uint32_t height,
    pdfium::span<const uint8_t> src_span,
    uint64_t src_key,
    pdfium::span<const uint8_t> global_span,
    uint64_t global_key,
    pdfium::span<uint8_t> dest_buf,
    uint32_t dest_pitch,
    PauseIndicatorIface* pPause,
    bool reject_large_regions_when_fuzzing) {
  pJbig2Context->width_ = width;
  pJbig2Context->height_ = height;
  pJbig2Context->src_span_ = src_span;
  pJbig2Context->src_key_ = src_key;
  pJbig2Context->global_span_ = global_span;
  pJbig2Context->global_key_ = global_key;
  pJbig2Context->dest_buf_ = dest_buf;
  pJbig2Context->dest_pitch_ = dest_pitch;
  std::ranges::fill(dest_buf.first(Fx2DSizeOrDie(height, dest_pitch)), 0);
  pJbig2Context->context_ =
      CJBig2_Context::Create(global_span, global_key, src_span, src_key,
                             pJBig2DocumentContext->GetSymbolDictCache());

  if (reject_large_regions_when_fuzzing) {
    pJbig2Context->context_->RejectLargeRegionsWhenFuzzing();
  }

  bool succeeded = pJbig2Context->context_->GetFirstPage(
      dest_buf, width, height, dest_pitch, pPause);
  return Decode(pJbig2Context, succeeded);
}

// static
FXCODEC_STATUS Jbig2Decoder::ContinueDecode(Jbig2Context* pJbig2Context,
                                            PauseIndicatorIface* pPause) {
  bool succeeded = pJbig2Context->context_->Continue(pPause);
  return Decode(pJbig2Context, succeeded);
}

}  // namespace fxcodec
