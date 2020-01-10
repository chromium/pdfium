// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_imagestretcher.h"

#include <climits>
#include <tuple>

#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cstretchengine.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

const int kMaxProgressiveStretchPixels = 1000000;

bool SourceSizeWithinLimit(int width, int height) {
  return !height || width < kMaxProgressiveStretchPixels / height;
}

FXDIB_Format GetStretchedFormat(const CFX_DIBBase& src) {
  FXDIB_Format format = src.GetFormat();
  if (format == FXDIB_1bppMask)
    return FXDIB_8bppMask;
  if (format == FXDIB_1bppRgb)
    return FXDIB_8bppRgb;
  if (format == FXDIB_8bppRgb && src.GetPalette())
    return FXDIB_Rgb;
  return format;
}

// Returns tuple c, m, y, k
std::tuple<int, int, int, int> CmykDecode(const uint32_t cmyk) {
  return std::make_tuple(FXSYS_GetCValue(cmyk), FXSYS_GetMValue(cmyk),
                         FXSYS_GetYValue(cmyk), FXSYS_GetKValue(cmyk));
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
      m_DestFormat(GetStretchedFormat(*pSource)),
      m_DestBPP(GetBppFromFormat(m_DestFormat)) {
  ASSERT(m_ClipRect.Valid());
}

CFX_ImageStretcher::~CFX_ImageStretcher() {}

bool CFX_ImageStretcher::Start() {
  if (m_DestWidth == 0 || m_DestHeight == 0)
    return false;

  if (m_pSource->GetFormat() == FXDIB_1bppRgb && m_pSource->GetPalette()) {
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
  } else if (m_pSource->GetFormat() == FXDIB_1bppCmyk &&
             m_pSource->GetPalette()) {
    FX_CMYK pal[256];
    int c0;
    int m0;
    int y0;
    int k0;
    std::tie(c0, m0, y0, k0) = CmykDecode(m_pSource->GetPaletteArgb(0));
    int c1;
    int m1;
    int y1;
    int k1;
    std::tie(c1, m1, y1, k1) = CmykDecode(m_pSource->GetPaletteArgb(1));
    for (int i = 0; i < 256; ++i) {
      int c = c0 + (c1 - c0) * i / 255;
      int m = m0 + (m1 - m0) * i / 255;
      int y = y0 + (y1 - y0) * i / 255;
      int k = k0 + (k1 - k0) * i / 255;
      pal[i] = CmykEncode(c, m, y, k);
    }
    if (!m_pDest->SetInfo(m_ClipRect.Width(), m_ClipRect.Height(), m_DestFormat,
                          pal)) {
      return false;
    }
  } else if (!m_pDest->SetInfo(m_ClipRect.Width(), m_ClipRect.Height(),
                               m_DestFormat, nullptr)) {
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
  m_pStretchEngine = pdfium::MakeUnique<CStretchEngine>(
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
