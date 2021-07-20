// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/win32/cgdi_printer_driver.h"

#include <windows.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_windowsrenderdevice.h"
#include "core/fxge/dib/cfx_dibextractor.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/render_defines.h"
#include "core/fxge/text_char_pos.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"

#if defined(PDFIUM_PRINT_TEXT_WITH_GDI)
#include "core/fxcrt/widestring.h"
#endif

#if defined(PDFIUM_PRINT_TEXT_WITH_GDI)
namespace {

class ScopedState {
 public:
  FX_STACK_ALLOCATED();

  ScopedState(HDC hDC, HFONT hFont)
      : m_hDC(hDC),
        m_iState(SaveDC(m_hDC)),
        m_hFont(SelectObject(m_hDC, hFont)) {}

  ScopedState(const ScopedState&) = delete;
  ScopedState& operator=(const ScopedState&) = delete;

  ~ScopedState() {
    HGDIOBJ hFont = SelectObject(m_hDC, m_hFont);
    DeleteObject(hFont);
    RestoreDC(m_hDC, m_iState);
  }

 private:
  const HDC m_hDC;
  const int m_iState;
  const HGDIOBJ m_hFont;
};

}  // namespace

bool g_pdfium_print_text_with_gdi = false;

PDFiumEnsureTypefaceCharactersAccessible g_pdfium_typeface_accessible_func =
    nullptr;
#endif

CGdiPrinterDriver::CGdiPrinterDriver(HDC hDC)
    : CGdiDeviceDriver(hDC, DeviceType::kPrinter),
      m_HorzSize(::GetDeviceCaps(m_hDC, HORZSIZE)),
      m_VertSize(::GetDeviceCaps(m_hDC, VERTSIZE)) {}

CGdiPrinterDriver::~CGdiPrinterDriver() = default;

int CGdiPrinterDriver::GetDeviceCaps(int caps_id) const {
  if (caps_id == FXDC_HORZ_SIZE)
    return m_HorzSize;
  if (caps_id == FXDC_VERT_SIZE)
    return m_VertSize;
  return CGdiDeviceDriver::GetDeviceCaps(caps_id);
}

bool CGdiPrinterDriver::SetDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                  uint32_t color,
                                  const FX_RECT& src_rect,
                                  int left,
                                  int top,
                                  BlendMode blend_type) {
  if (pSource->IsMaskFormat()) {
    FX_RECT clip_rect(left, top, left + src_rect.Width(),
                      top + src_rect.Height());
    return StretchDIBits(pSource, color, left - src_rect.left,
                         top - src_rect.top, pSource->GetWidth(),
                         pSource->GetHeight(), &clip_rect,
                         FXDIB_ResampleOptions(), BlendMode::kNormal);
  }
  DCHECK(pSource);
  DCHECK(!pSource->IsMaskFormat());
  DCHECK_EQ(blend_type, BlendMode::kNormal);
  if (pSource->IsAlphaFormat())
    return false;

  CFX_DIBExtractor temp(pSource);
  RetainPtr<CFX_DIBitmap> pBitmap = temp.GetBitmap();
  if (!pBitmap)
    return false;

  return GDI_SetDIBits(pBitmap, src_rect, left, top);
}

bool CGdiPrinterDriver::StretchDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                      uint32_t color,
                                      int dest_left,
                                      int dest_top,
                                      int dest_width,
                                      int dest_height,
                                      const FX_RECT* pClipRect,
                                      const FXDIB_ResampleOptions& options,
                                      BlendMode blend_type) {
  if (pSource->IsMaskFormat()) {
    int alpha = FXARGB_A(color);
    if (pSource->GetBPP() != 1 || alpha != 255)
      return false;

    if (dest_width < 0 || dest_height < 0) {
      RetainPtr<CFX_DIBitmap> pFlipped =
          pSource->FlipImage(dest_width < 0, dest_height < 0);
      if (!pFlipped)
        return false;

      if (dest_width < 0)
        dest_left += dest_width;
      if (dest_height < 0)
        dest_top += dest_height;

      return GDI_StretchBitMask(pFlipped, dest_left, dest_top, abs(dest_width),
                                abs(dest_height), color);
    }

    CFX_DIBExtractor temp(pSource);
    RetainPtr<CFX_DIBitmap> pBitmap = temp.GetBitmap();
    if (!pBitmap)
      return false;
    return GDI_StretchBitMask(pBitmap, dest_left, dest_top, dest_width,
                              dest_height, color);
  }

  if (pSource->IsAlphaFormat())
    return false;

  if (dest_width < 0 || dest_height < 0) {
    RetainPtr<CFX_DIBitmap> pFlipped =
        pSource->FlipImage(dest_width < 0, dest_height < 0);
    if (!pFlipped)
      return false;

    if (dest_width < 0)
      dest_left += dest_width;
    if (dest_height < 0)
      dest_top += dest_height;

    return GDI_StretchDIBits(pFlipped, dest_left, dest_top, abs(dest_width),
                             abs(dest_height), options);
  }

  CFX_DIBExtractor temp(pSource);
  RetainPtr<CFX_DIBitmap> pBitmap = temp.GetBitmap();
  if (!pBitmap)
    return false;
  return GDI_StretchDIBits(pBitmap, dest_left, dest_top, dest_width,
                           dest_height, options);
}

bool CGdiPrinterDriver::StartDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                    int bitmap_alpha,
                                    uint32_t color,
                                    const CFX_Matrix& matrix,
                                    const FXDIB_ResampleOptions& options,
                                    std::unique_ptr<CFX_ImageRenderer>* handle,
                                    BlendMode blend_type) {
  if (bitmap_alpha < 255 || pSource->IsAlphaFormat() ||
      (pSource->IsMaskFormat() && (pSource->GetBPP() != 1))) {
    return false;
  }
  CFX_FloatRect unit_rect = matrix.GetUnitRect();
  FX_RECT full_rect = unit_rect.GetOuterRect();
  if (fabs(matrix.b) < 0.5f && matrix.a != 0 && fabs(matrix.c) < 0.5f &&
      matrix.d != 0) {
    bool bFlipX = matrix.a < 0;
    bool bFlipY = matrix.d > 0;
    return StretchDIBits(pSource, color,
                         bFlipX ? full_rect.right : full_rect.left,
                         bFlipY ? full_rect.bottom : full_rect.top,
                         bFlipX ? -full_rect.Width() : full_rect.Width(),
                         bFlipY ? -full_rect.Height() : full_rect.Height(),
                         nullptr, FXDIB_ResampleOptions(), blend_type);
  }
  if (fabs(matrix.a) >= 0.5f || fabs(matrix.d) >= 0.5f)
    return false;

  RetainPtr<CFX_DIBitmap> pTransformed =
      pSource->SwapXY(matrix.c > 0, matrix.b < 0);
  if (!pTransformed)
    return false;

  return StretchDIBits(pTransformed, color, full_rect.left, full_rect.top,
                       full_rect.Width(), full_rect.Height(), nullptr,
                       FXDIB_ResampleOptions(), blend_type);
}

bool CGdiPrinterDriver::DrawDeviceText(
    int nChars,
    const TextCharPos* pCharPos,
    CFX_Font* pFont,
    const CFX_Matrix& mtObject2Device,
    float font_size,
    uint32_t color,
    const CFX_TextRenderOptions& /*options*/) {
#if defined(PDFIUM_PRINT_TEXT_WITH_GDI)
  if (!g_pdfium_print_text_with_gdi)
    return false;

  if (nChars < 1 || !pFont || !pFont->IsEmbedded() || !pFont->IsTTFont())
    return false;

  // Scale factor used to minimize the kerning problems caused by rounding
  // errors below. Value chosen based on the title of https://crbug.com/18383
  const double kScaleFactor = 10;

  // Font
  //
  // Note that |pFont| has the actual font to render with embedded within, but
  // but unfortunately AddFontMemResourceEx() does not seem to cooperate.
  // Loading font data to memory seems to work, but then enumerating the fonts
  // fails to find it. This requires more investigation. In the meanwhile,
  // assume the printing is happening on the machine that generated the PDF, so
  // the embedded font, if not a web font, is available through GDI anyway.
  // TODO(thestig): Figure out why AddFontMemResourceEx() does not work.
  // Generalize this method to work for all PDFs with embedded fonts.
  // In sandboxed environments, font loading may not work at all, so this may be
  // the best possible effort.
  LOGFONT lf = {};
  lf.lfHeight = -font_size * kScaleFactor;
  lf.lfWeight = pFont->IsBold() ? FW_BOLD : FW_NORMAL;
  lf.lfItalic = pFont->IsItalic();
  lf.lfCharSet = DEFAULT_CHARSET;

  const WideString wsName =
      WideString::FromUTF8(pFont->GetFaceName().AsStringView());
  size_t iNameLen =
      std::min(wsName.GetLength(), static_cast<size_t>(LF_FACESIZE - 1));
  memcpy(lf.lfFaceName, wsName.c_str(), sizeof(lf.lfFaceName[0]) * iNameLen);
  lf.lfFaceName[iNameLen] = 0;

  HFONT hFont = CreateFontIndirect(&lf);
  if (!hFont)
    return false;

  ScopedState state(m_hDC, hFont);
  size_t nTextMetricSize = GetOutlineTextMetrics(m_hDC, 0, nullptr);
  if (nTextMetricSize == 0) {
    // Give up and fail if there is no way to get the font to try again.
    if (!g_pdfium_typeface_accessible_func)
      return false;

    // Try to get the font. Any letter will do.
    g_pdfium_typeface_accessible_func(&lf, L"A", 1);
    nTextMetricSize = GetOutlineTextMetrics(m_hDC, 0, nullptr);
    if (nTextMetricSize == 0)
      return false;
  }

  std::vector<BYTE, FxAllocAllocator<BYTE>> buf(nTextMetricSize);
  OUTLINETEXTMETRIC* pTextMetric =
      reinterpret_cast<OUTLINETEXTMETRIC*>(buf.data());
  if (GetOutlineTextMetrics(m_hDC, nTextMetricSize, pTextMetric) == 0)
    return false;

  // If the selected font is not the requested font, then bail out. This can
  // happen with web fonts, for example.
  wchar_t* wsSelectedName = reinterpret_cast<wchar_t*>(
      buf.data() + reinterpret_cast<size_t>(pTextMetric->otmpFaceName));
  if (wsName != wsSelectedName)
    return false;

  // Transforms
  SetGraphicsMode(m_hDC, GM_ADVANCED);
  XFORM xform;
  xform.eM11 = mtObject2Device.a / kScaleFactor;
  xform.eM12 = mtObject2Device.b / kScaleFactor;
  xform.eM21 = -mtObject2Device.c / kScaleFactor;
  xform.eM22 = -mtObject2Device.d / kScaleFactor;
  xform.eDx = mtObject2Device.e;
  xform.eDy = mtObject2Device.f;
  ModifyWorldTransform(m_hDC, &xform, MWT_LEFTMULTIPLY);

  // Color
  FX_COLORREF colorref = ArgbToColorRef(color);
  SetTextColor(m_hDC, colorref);
  SetBkMode(m_hDC, TRANSPARENT);

  // Text
  WideString wsText;
  std::vector<INT, FxAllocAllocator<INT>> spacing(nChars);
  float fPreviousOriginX = 0;
  for (int i = 0; i < nChars; ++i) {
    // Only works with PDFs from Skia's PDF generator. Cannot handle arbitrary
    // values from PDFs.
    const TextCharPos& charpos = pCharPos[i];
    DCHECK_EQ(charpos.m_AdjustMatrix[0], 0);
    DCHECK_EQ(charpos.m_AdjustMatrix[1], 0);
    DCHECK_EQ(charpos.m_AdjustMatrix[2], 0);
    DCHECK_EQ(charpos.m_AdjustMatrix[3], 0);
    DCHECK_EQ(charpos.m_Origin.y, 0);

    // Round the spacing to the nearest integer, but keep track of the rounding
    // error for calculating the next spacing value.
    float fOriginX = charpos.m_Origin.x * kScaleFactor;
    float fPixelSpacing = fOriginX - fPreviousOriginX;
    spacing[i] = FXSYS_roundf(fPixelSpacing);
    fPreviousOriginX = fOriginX - (fPixelSpacing - spacing[i]);

    wsText += charpos.m_GlyphIndex;
  }

  // Draw
  SetTextAlign(m_hDC, TA_LEFT | TA_BASELINE);
  if (ExtTextOutW(m_hDC, 0, 0, ETO_GLYPH_INDEX, nullptr, wsText.c_str(), nChars,
                  nChars > 1 ? &spacing[1] : nullptr)) {
    return true;
  }

  // Give up and fail if there is no way to get the font to try again.
  if (!g_pdfium_typeface_accessible_func)
    return false;

  // Try to get the font and draw again.
  g_pdfium_typeface_accessible_func(&lf, wsText.c_str(), nChars);
  return !!ExtTextOutW(m_hDC, 0, 0, ETO_GLYPH_INDEX, nullptr, wsText.c_str(),
                       nChars, nChars > 1 ? &spacing[1] : nullptr);
#else
  return false;
#endif
}
