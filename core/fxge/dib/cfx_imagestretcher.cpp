// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_imagestretcher.h"

#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/span.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cstretchengine.h"
#include "core/fxge/dib/fx_dib.h"

namespace {

const int kMaxProgressiveStretchPixels = 1000000;

bool SourceSizeWithinLimit(int width, int height) {
  return !height || width < kMaxProgressiveStretchPixels / height;
}

FXDIB_Format GetStretchedFormat(const CFX_DIBBase& src) {
  FXDIB_Format format = src.GetFormat();
  if (format == FXDIB_Format::k1bppMask) {
    return FXDIB_Format::k8bppMask;
  }
  if (format == FXDIB_Format::k1bppRgb) {
    return FXDIB_Format::k8bppRgb;
  }
  if (format == FXDIB_Format::k8bppRgb && src.HasPalette()) {
    return FXDIB_Format::kBgr;
  }
  return format;
}

// Builds a new palette with a size of `CFX_DIBBase::kPaletteSize` from the
// existing palette in `source`.
DataVector<uint32_t> BuildPaletteFrom1BppSource(
    const RetainPtr<const CFX_DIBBase>& source) {
  DCHECK_EQ(FXDIB_Format::k1bppRgb, source->GetFormat());
  DCHECK(source->HasPalette());

  const FX_BGRA_STRUCT<uint8_t> bgra0 =
      ArgbToBGRAStruct(source->GetPaletteArgb(0));
  const FX_BGRA_STRUCT<uint8_t> bgra1 =
      ArgbToBGRAStruct(source->GetPaletteArgb(1));
  CHECK_EQ(255, bgra0.alpha);
  CHECK_EQ(255, bgra1.alpha);

  DataVector<uint32_t> palette(CFX_DIBBase::kPaletteSize);
  for (int i = 0; i < static_cast<int>(CFX_DIBBase::kPaletteSize); ++i) {
    int r = bgra0.red + (bgra1.red - bgra0.red) * i / 255;
    int g = bgra0.green + (bgra1.green - bgra0.green) * i / 255;
    int b = bgra0.blue + (bgra1.blue - bgra0.blue) * i / 255;
    palette[i] = ArgbEncode(255, r, g, b);
  }
  return palette;
}

}  // namespace

CFX_ImageStretcher::CFX_ImageStretcher(ScanlineComposerIface* pDest,
                                       RetainPtr<const CFX_DIBBase> source,
                                       int dest_width,
                                       int dest_height,
                                       const FX_RECT& bitmap_rect,
                                       const FXDIB_ResampleOptions& options)
    : dest_(pDest),
      source_(std::move(source)),
      resample_options_(options),
      dest_width_(dest_width),
      dest_height_(dest_height),
      clip_rect_(bitmap_rect),
      dest_format_(GetStretchedFormat(*source_)) {
  DCHECK(clip_rect_.Valid());
}

CFX_ImageStretcher::~CFX_ImageStretcher() = default;

bool CFX_ImageStretcher::Start() {
  if (dest_width_ == 0 || dest_height_ == 0) {
    return false;
  }

  if (source_->GetFormat() == FXDIB_Format::k1bppRgb && source_->HasPalette()) {
    if (!dest_->SetInfo(clip_rect_.Width(), clip_rect_.Height(), dest_format_,
                        BuildPaletteFrom1BppSource(source_))) {
      return false;
    }
  } else if (!dest_->SetInfo(clip_rect_.Width(), clip_rect_.Height(),
                             dest_format_, {})) {
    return false;
  }
  return StartStretch();
}

bool CFX_ImageStretcher::Continue(PauseIndicatorIface* pPause) {
  return ContinueStretch(pPause);
}

RetainPtr<const CFX_DIBBase> CFX_ImageStretcher::source() {
  return source_;
}

bool CFX_ImageStretcher::StartStretch() {
  stretch_engine_ = std::make_unique<CStretchEngine>(
      dest_, dest_format_, dest_width_, dest_height_, clip_rect_, source_,
      resample_options_);
  stretch_engine_->StartStretchHorz();
  if (SourceSizeWithinLimit(source_->GetWidth(), source_->GetHeight())) {
    stretch_engine_->Continue(nullptr);
    return false;
  }
  return true;
}

bool CFX_ImageStretcher::ContinueStretch(PauseIndicatorIface* pPause) {
  return stretch_engine_ && stretch_engine_->Continue(pPause);
}
