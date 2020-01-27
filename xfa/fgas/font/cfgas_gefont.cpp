// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/font/cfgas_gefont.h"

#include <memory>
#include <utility>

#include "build/build_config.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/cfx_unicodeencodingex.h"
#include "core/fxge/fx_font.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fgas/font/fgas_fontutils.h"

// static
RetainPtr<CFGAS_GEFont> CFGAS_GEFont::LoadFont(const wchar_t* pszFontFamily,
                                               uint32_t dwFontStyles,
                                               uint16_t wCodePage,
                                               CFGAS_FontMgr* pFontMgr) {
#if defined(OS_WIN)
  auto pFont = pdfium::MakeRetain<CFGAS_GEFont>(pFontMgr);
  if (!pFont->LoadFontInternal(pszFontFamily, dwFontStyles, wCodePage))
    return nullptr;
  return pFont;
#else
  if (!pFontMgr)
    return nullptr;
  return pFontMgr->GetFontByCodePage(wCodePage, dwFontStyles, pszFontFamily);
#endif
}

// static
RetainPtr<CFGAS_GEFont> CFGAS_GEFont::LoadFont(
    const RetainPtr<CPDF_Font>& pPDFFont,
    CFGAS_FontMgr* pFontMgr) {
  auto pFont = pdfium::MakeRetain<CFGAS_GEFont>(pFontMgr);
  if (!pFont->LoadFontInternal(pPDFFont))
    return nullptr;

  return pFont;
}

// static
RetainPtr<CFGAS_GEFont> CFGAS_GEFont::LoadFont(
    std::unique_ptr<CFX_Font> pInternalFont,
    CFGAS_FontMgr* pFontMgr) {
  auto pFont = pdfium::MakeRetain<CFGAS_GEFont>(pFontMgr);
  if (!pFont->LoadFontInternal(std::move(pInternalFont)))
    return nullptr;
  return pFont;
}

// static
RetainPtr<CFGAS_GEFont> CFGAS_GEFont::LoadStockFont(
    CPDF_Document* pDoc,
    CFGAS_FontMgr* pMgr,
    const ByteString& font_family) {
  RetainPtr<CPDF_Font> stock_font =
      CPDF_Font::GetStockFont(pDoc, font_family.AsStringView());
  return stock_font ? CFGAS_GEFont::LoadFont(stock_font, pMgr) : nullptr;
}

CFGAS_GEFont::CFGAS_GEFont(CFGAS_FontMgr* pFontMgr) : m_pFontMgr(pFontMgr) {}

CFGAS_GEFont::~CFGAS_GEFont() = default;

#if defined(OS_WIN)
bool CFGAS_GEFont::LoadFontInternal(const wchar_t* pszFontFamily,
                                    uint32_t dwFontStyles,
                                    uint16_t wCodePage) {
  if (m_pFont)
    return false;
  ByteString csFontFamily;
  if (pszFontFamily)
    csFontFamily = WideString(pszFontFamily).ToDefANSI();

  int32_t iWeight =
      FontStyleIsForceBold(dwFontStyles) ? FXFONT_FW_BOLD : FXFONT_FW_NORMAL;
  m_pFont = pdfium::MakeUnique<CFX_Font>();
  if (FontStyleIsItalic(dwFontStyles) && FontStyleIsForceBold(dwFontStyles))
    csFontFamily += ",BoldItalic";
  else if (FontStyleIsForceBold(dwFontStyles))
    csFontFamily += ",Bold";
  else if (FontStyleIsItalic(dwFontStyles))
    csFontFamily += ",Italic";

  m_pFont->LoadSubst(csFontFamily, true, dwFontStyles, iWeight, 0, wCodePage,
                     false);
  return m_pFont->GetFaceRec() && InitFont();
}
#endif  // defined(OS_WIN)

bool CFGAS_GEFont::LoadFontInternal(const RetainPtr<CPDF_Font>& pPDFFont) {
  CFX_Font* pExternalFont = pPDFFont->GetFont();
  if (m_pFont || !pExternalFont)
    return false;

  m_pFont = pExternalFont;
  if (!InitFont())
    return false;

  m_pPDFFont = pPDFFont;  // Keep pPDFFont alive for the duration.
  return true;
}

bool CFGAS_GEFont::LoadFontInternal(std::unique_ptr<CFX_Font> pInternalFont) {
  if (m_pFont || !pInternalFont)
    return false;

  m_pFont = std::move(pInternalFont);
  return InitFont();
}

bool CFGAS_GEFont::InitFont() {
  if (!m_pFont)
    return false;

  if (m_pFontEncoding)
    return true;

  m_pFontEncoding = FX_CreateFontEncodingEx(m_pFont.Get());
  return !!m_pFontEncoding;
}

WideString CFGAS_GEFont::GetFamilyName() const {
  CFX_SubstFont* subst_font = m_pFont->GetSubstFont();
  ByteString family_name = subst_font && !subst_font->m_Family.IsEmpty()
                               ? subst_font->m_Family
                               : m_pFont->GetFamilyName();
  return WideString::FromDefANSI(family_name.AsStringView());
}

uint32_t CFGAS_GEFont::GetFontStyles() const {
  ASSERT(m_pFont);
  if (m_dwLogFontStyle.has_value())
    return m_dwLogFontStyle.value();

  uint32_t dwStyles = 0;
  auto* pSubstFont = m_pFont->GetSubstFont();
  if (pSubstFont) {
    if (pSubstFont->m_Weight == FXFONT_FW_BOLD)
      dwStyles |= FXFONT_FORCE_BOLD;
  } else {
    if (m_pFont->IsBold())
      dwStyles |= FXFONT_FORCE_BOLD;
    if (m_pFont->IsItalic())
      dwStyles |= FXFONT_ITALIC;
  }
  return dwStyles;
}

bool CFGAS_GEFont::GetCharWidth(wchar_t wUnicode, int32_t* pWidth) {
  auto it = m_CharWidthMap.find(wUnicode);
  *pWidth = it != m_CharWidthMap.end() ? it->second : 0;
  if (*pWidth == 65535)
    return false;

  if (*pWidth > 0)
    return true;

  RetainPtr<CFGAS_GEFont> pFont;
  int32_t iGlyph;
  std::tie(iGlyph, pFont) = GetGlyphIndexAndFont(wUnicode, true);
  if (iGlyph != 0xFFFF && pFont) {
    if (pFont == this) {
      *pWidth = m_pFont->GetGlyphWidth(iGlyph);
      if (*pWidth < 0)
        *pWidth = -1;
    } else if (pFont->GetCharWidth(wUnicode, pWidth)) {
      return true;
    }
  } else {
    *pWidth = -1;
  }

  m_CharWidthMap[wUnicode] = *pWidth;
  return *pWidth > 0;
}

bool CFGAS_GEFont::GetCharBBox(wchar_t wUnicode, FX_RECT* bbox) {
  auto it = m_BBoxMap.find(wUnicode);
  if (it != m_BBoxMap.end()) {
    *bbox = it->second;
    return true;
  }

  RetainPtr<CFGAS_GEFont> pFont;
  int32_t iGlyph;
  std::tie(iGlyph, pFont) = GetGlyphIndexAndFont(wUnicode, true);
  if (!pFont || iGlyph == 0xFFFF)
    return false;

  if (pFont.Get() != this)
    return pFont->GetCharBBox(wUnicode, bbox);

  FX_RECT rtBBox;
  if (!m_pFont->GetGlyphBBox(iGlyph, &rtBBox))
    return false;

  m_BBoxMap[wUnicode] = rtBBox;
  *bbox = rtBBox;
  return true;
}

bool CFGAS_GEFont::GetBBox(FX_RECT* bbox) {
  return m_pFont->GetBBox(bbox);
}

int32_t CFGAS_GEFont::GetGlyphIndex(wchar_t wUnicode) {
  int32_t glyph;
  RetainPtr<CFGAS_GEFont> font;
  std::tie(glyph, font) = GetGlyphIndexAndFont(wUnicode, true);
  return glyph;
}

std::pair<int32_t, RetainPtr<CFGAS_GEFont>> CFGAS_GEFont::GetGlyphIndexAndFont(
    wchar_t wUnicode,
    bool bRecursive) {
  int32_t iGlyphIndex = m_pFontEncoding->GlyphFromCharCode(wUnicode);
  if (iGlyphIndex > 0)
    return {iGlyphIndex, pdfium::WrapRetain(this)};

  const FGAS_FONTUSB* pFontUSB = FGAS_GetUnicodeBitField(wUnicode);
  if (!pFontUSB)
    return {0xFFFF, nullptr};

  uint16_t wBitField = pFontUSB->wBitField;
  if (wBitField >= 128)
    return {0xFFFF, nullptr};

  auto it = m_FontMapper.find(wUnicode);
  if (it != m_FontMapper.end() && it->second && it->second.Get() != this) {
    RetainPtr<CFGAS_GEFont> font;
    std::tie(iGlyphIndex, font) =
        it->second->GetGlyphIndexAndFont(wUnicode, false);
    if (iGlyphIndex != 0xFFFF) {
      for (size_t i = 0; i < m_SubstFonts.size(); ++i) {
        if (m_SubstFonts[i] == it->second)
          return {(iGlyphIndex | ((i + 1) << 24)), it->second};
      }
    }
  }
  if (!m_pFontMgr || !bRecursive)
    return {0xFFFF, nullptr};

  WideString wsFamily = GetFamilyName();
  RetainPtr<CFGAS_GEFont> pFont =
      m_pFontMgr->GetFontByUnicode(wUnicode, GetFontStyles(), wsFamily.c_str());
#if !defined(OS_WIN)
  if (!pFont)
    pFont = m_pFontMgr->GetFontByUnicode(wUnicode, GetFontStyles(), nullptr);
#endif
  if (!pFont || pFont == this)  // Avoids direct cycles below.
    return {0xFFFF, nullptr};

  m_FontMapper[wUnicode] = pFont;
  m_SubstFonts.push_back(pFont);

  RetainPtr<CFGAS_GEFont> font;
  std::tie(iGlyphIndex, font) = pFont->GetGlyphIndexAndFont(wUnicode, false);
  if (iGlyphIndex == 0xFFFF)
    return {0xFFFF, nullptr};

  return {(iGlyphIndex | (m_SubstFonts.size() << 24)), pFont};
}

int32_t CFGAS_GEFont::GetAscent() const {
  return m_pFont->GetAscent();
}

int32_t CFGAS_GEFont::GetDescent() const {
  return m_pFont->GetDescent();
}

RetainPtr<CFGAS_GEFont> CFGAS_GEFont::GetSubstFont(int32_t iGlyphIndex) {
  iGlyphIndex = static_cast<uint32_t>(iGlyphIndex) >> 24;
  if (iGlyphIndex == 0)
    return pdfium::WrapRetain(this);
  return m_SubstFonts[iGlyphIndex - 1];
}
