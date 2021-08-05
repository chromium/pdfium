// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_imagestretcher.h"

#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cstretchengine.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/check.h"

namespace {

const int kMaxProgressiveStretchPixels = 1000000;

bool SourceSizeWithinLimit(int width, int height) {
  return !height || width < kMaxProgressiveStretchPixels / height;
}

FXDIB_Format GetStretchedFormat(const CFX_DIBBase& src) {
  FXDIB_Format format = src.GetFormat();
  if (format == FXDIB_Format::k1bppMask)
    return FXDIB_Format::k8bppMask;
  if (format == FXDIB_Format::k1bppRgb)
    return FXDIB_Format::k8bppRgb;
  if (format == FXDIB_Format::k8bppRgb && src.HasPalette())
    return FXDIB_Format::kRgb;
  return format;
}

}  // namespace

CFX_ImageStretcher::CFX_ImageStretcher(ScanlineComposerIface* pDest,
                                       const RetainPtr<CFX_DIBBase>& pSource,
                                       int dest_width,
                                       int dest_height,
                                       const FX_RECT& bitmap_rect,
                                       const FXDIB_ResampleOptions& options)
    : m_pDest(pDest),
      m_pSource(pSource),
      m_ResampleOptions(options),
      m_DestWidth(dest_width),
      m_DestHeight(dest_height),
      m_ClipRect(bitmap_rect),
      m_DestFormat(GetStretchedFormat(*pSource)) {
  DCHECK(m_ClipRect.Valid());
}

CFX_ImageStretcher::~CFX_ImageStretcher() = default;

bool CFX_ImageStretcher::Start() {
  if (m_DestWidth == 0 || m_DestHeight == 0)
    return false;

  if (m_pSource->GetFormat() == FXDIB_Format::k1bppRgb &&
      m_pSource->HasPalette()) {
    FX_ARGB pal[256];
    int a0;
    int r0;
    int g0;
    int b0;
    std::tie(a0, r0, g0, b0) = ArgbDecode(m_pSource->GetPaletteArgb(0));
    int a1;
    int r1;
    int g1;
    int b1;
    std::tie(a1, r1, g1, b1) = ArgbDecode(m_pSource->GetPaletteArgb(1));
    for (int i = 0; i < 256; ++i) {
      int a = a0 + (a1 - a0) * i / 255;
      int r = r0 + (r1 - r0) * i / 255;
      int g = g0 + (g1 - g0) * i / 255;
      int b = b0 + (b1 - b0) * i / 255;
      pal[i] = ArgbEncode(a, r, g, b);
    }
    if (!m_pDest->SetInfo(m_ClipRect.Width(), m_ClipRect.Height(), m_DestFormat,
                          pal)) {
      return false;
    }
  } else if (!m_pDest->SetInfo(m_ClipRect.Width(), m_ClipRect.Height(),
                               m_DestFormat, {})) {
    return false;
  }
  return StartStretch();
}

bool CFX_ImageStretcher::Continue(PauseIndicatorIface* pPause) {
  return ContinueStretch(pPause);
}

RetainPtr<CFX_DIBBase> CFX_ImageStretcher::source() {
  return m_pSource;
}

bool CFX_ImageStretcher::StartStretch() {
  m_pStretchEngine = std::make_unique<CStretchEngine>(
      m_pDest.Get(), m_DestFormat, m_DestWidth, m_DestHeight, m_ClipRect,
      m_pSource, m_ResampleOptions);
  m_pStretchEngine->StartStretchHorz();
  if (SourceSizeWithinLimit(m_pSource->GetWidth(), m_pSource->GetHeight())) {
    m_pStretchEngine->Continue(nullptr);
    return false;
  }
  return true;
}

bool CFX_ImageStretcher::ContinueStretch(PauseIndicatorIface* pPause) {
  return m_pStretchEngine && m_pStretchEngine->Continue(pPause);
}
