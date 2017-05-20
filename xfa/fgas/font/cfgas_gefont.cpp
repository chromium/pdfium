// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/font/cfgas_gefont.h"

#include <memory>
#include <utility>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/cfx_unicodeencoding.h"
#include "core/fxge/cfx_unicodeencodingex.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fgas/font/fgas_fontutils.h"
#include "xfa/fxfa/cxfa_fontmgr.h"

// static
CFX_RetainPtr<CFGAS_GEFont> CFGAS_GEFont::LoadFont(const wchar_t* pszFontFamily,
                                                   uint32_t dwFontStyles,
                                                   uint16_t wCodePage,
                                                   CFGAS_FontMgr* pFontMgr) {
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  if (!pFontMgr)
    return nullptr;

  return pFontMgr->GetFontByCodePage(wCodePage, dwFontStyles, pszFontFamily);
#else
  auto pFont = pdfium::MakeRetain<CFGAS_GEFont>(pFontMgr);
  if (!pFont->LoadFontInternal(pszFontFamily, dwFontStyles, wCodePage))
    return nullptr;
  return pFont;
#endif
}

// static
CFX_RetainPtr<CFGAS_GEFont> CFGAS_GEFont::LoadFont(CFX_Font* pExternalFont,
                                                   CFGAS_FontMgr* pFontMgr) {
  auto pFont = pdfium::MakeRetain<CFGAS_GEFont>(pFontMgr);
  if (!pFont->LoadFontInternal(pExternalFont))
    return nullptr;
  return pFont;
}

// static
CFX_RetainPtr<CFGAS_GEFont> CFGAS_GEFont::LoadFont(
    std::unique_ptr<CFX_Font> pInternalFont,
    CFGAS_FontMgr* pFontMgr) {
  auto pFont = pdfium::MakeRetain<CFGAS_GEFont>(pFontMgr);
  if (!pFont->LoadFontInternal(std::move(pInternalFont)))
    return nullptr;
  return pFont;
}

CFGAS_GEFont::CFGAS_GEFont(CFGAS_FontMgr* pFontMgr)
    :
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
      m_bUseLogFontStyle(false),
      m_dwLogFontStyle(0),
#endif
      m_pFont(nullptr),
      m_bExternalFont(false),
      m_pFontMgr(pFontMgr) {
}

CFGAS_GEFont::CFGAS_GEFont(const CFX_RetainPtr<CFGAS_GEFont>& src,
                           uint32_t dwFontStyles)
    :
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
      m_bUseLogFontStyle(false),
      m_dwLogFontStyle(0),
#endif
      m_pFont(nullptr),
      m_bExternalFont(false),
      m_pSrcFont(src),
      m_pFontMgr(src->m_pFontMgr) {
  ASSERT(m_pSrcFont->m_pFont);
  m_pFont = new CFX_Font;
  m_pFont->LoadClone(m_pSrcFont->m_pFont);
  CFX_SubstFont* pSubst = m_pFont->GetSubstFont();
  if (!pSubst) {
    pSubst = new CFX_SubstFont;
    m_pFont->SetSubstFont(std::unique_ptr<CFX_SubstFont>(pSubst));
  }
  pSubst->m_Weight =
      (dwFontStyles & FX_FONTSTYLE_Bold) ? FXFONT_FW_BOLD : FXFONT_FW_NORMAL;
  if (dwFontStyles & FX_FONTSTYLE_Italic)
    pSubst->m_SubstFlags |= FXFONT_SUBST_ITALIC;
  InitFont();
}

CFGAS_GEFont::~CFGAS_GEFont() {
  if (!m_bExternalFont)
    delete m_pFont;
}

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
bool CFGAS_GEFont::LoadFontInternal(const wchar_t* pszFontFamily,
                                    uint32_t dwFontStyles,
                                    uint16_t wCodePage) {
  if (m_pFont)
    return false;
  CFX_ByteString csFontFamily;
  if (pszFontFamily)
    csFontFamily = CFX_ByteString::FromUnicode(pszFontFamily);
  uint32_t dwFlags = 0;
  if (dwFontStyles & FX_FONTSTYLE_FixedPitch)
    dwFlags |= FXFONT_FIXED_PITCH;
  if (dwFontStyles & FX_FONTSTYLE_Serif)
    dwFlags |= FXFONT_SERIF;
  if (dwFontStyles & FX_FONTSTYLE_Symbolic)
    dwFlags |= FXFONT_SYMBOLIC;
  if (dwFontStyles & FX_FONTSTYLE_Script)
    dwFlags |= FXFONT_SCRIPT;
  if (dwFontStyles & FX_FONTSTYLE_Italic)
    dwFlags |= FXFONT_ITALIC;
  if (dwFontStyles & FX_FONTSTYLE_Bold)
    dwFlags |= FXFONT_BOLD;
  if (dwFontStyles & FX_FONTSTYLE_ExactMatch)
    dwFlags |= FXFONT_EXACTMATCH;
  int32_t iWeight =
      (dwFontStyles & FX_FONTSTYLE_Bold) ? FXFONT_FW_BOLD : FXFONT_FW_NORMAL;
  m_pFont = new CFX_Font;
  if ((dwFlags & FXFONT_ITALIC) && (dwFlags & FXFONT_BOLD))
    csFontFamily += ",BoldItalic";
  else if (dwFlags & FXFONT_BOLD)
    csFontFamily += ",Bold";
  else if (dwFlags & FXFONT_ITALIC)
    csFontFamily += ",Italic";
  m_pFont->LoadSubst(csFontFamily, true, dwFlags, iWeight, 0, wCodePage, false);
  if (!m_pFont->GetFace())
    return false;
  return InitFont();
}
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

bool CFGAS_GEFont::LoadFontInternal(CFX_Font* pExternalFont) {
  if (m_pFont || !pExternalFont)
    return false;

  m_pFont = pExternalFont;
  m_bExternalFont = true;
  return InitFont();
}

bool CFGAS_GEFont::LoadFontInternal(std::unique_ptr<CFX_Font> pInternalFont) {
  if (m_pFont || !pInternalFont)
    return false;

  m_pFont = pInternalFont.release();
  m_bExternalFont = false;
  return InitFont();
}

bool CFGAS_GEFont::InitFont() {
  if (!m_pFont)
    return false;

  if (m_pFontEncoding)
    return true;

  m_pFontEncoding = FX_CreateFontEncodingEx(m_pFont, FXFM_ENCODING_NONE);
  return !!m_pFontEncoding;
}

CFX_RetainPtr<CFGAS_GEFont> CFGAS_GEFont::Derive(uint32_t dwFontStyles,
                                                 uint16_t wCodePage) {
  CFX_RetainPtr<CFGAS_GEFont> pFont(this);
  if (GetFontStyles() == dwFontStyles)
    return pFont;
  return pdfium::MakeRetain<CFGAS_GEFont>(pFont, dwFontStyles);
}

CFX_WideString CFGAS_GEFont::GetFamilyName() const {
  if (!m_pFont->GetSubstFont() ||
      m_pFont->GetSubstFont()->m_Family.GetLength() == 0) {
    return CFX_WideString::FromLocal(m_pFont->GetFamilyName().AsStringC());
  }
  return CFX_WideString::FromLocal(
      m_pFont->GetSubstFont()->m_Family.AsStringC());
}

uint32_t CFGAS_GEFont::GetFontStyles() const {
  ASSERT(m_pFont);
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  if (m_bUseLogFontStyle)
    return m_dwLogFontStyle;
#endif

  uint32_t dwStyles = 0;
  auto* pSubstFont = m_pFont->GetSubstFont();
  if (pSubstFont) {
    if (pSubstFont->m_Weight == FXFONT_FW_BOLD)
      dwStyles |= FX_FONTSTYLE_Bold;
    if (pSubstFont->m_SubstFlags & FXFONT_SUBST_ITALIC)
      dwStyles |= FX_FONTSTYLE_Italic;
  } else {
    if (m_pFont->IsBold())
      dwStyles |= FX_FONTSTYLE_Bold;
    if (m_pFont->IsItalic())
      dwStyles |= FX_FONTSTYLE_Italic;
  }
  return dwStyles;
}

bool CFGAS_GEFont::GetCharWidth(wchar_t wUnicode,
                                int32_t& iWidth,
                                bool bCharCode) {
  return GetCharWidthInternal(wUnicode, iWidth, true, bCharCode);
}

bool CFGAS_GEFont::GetCharWidthInternal(wchar_t wUnicode,
                                        int32_t& iWidth,
                                        bool bRecursive,
                                        bool bCharCode) {
  auto it = m_CharWidthMap.find(wUnicode);
  iWidth = it != m_CharWidthMap.end() ? it->second : 0;
  if (iWidth == 65535)
    return false;

  if (iWidth > 0)
    return true;

  if (!m_pProvider ||
      !m_pProvider->GetCharWidth(CFX_RetainPtr<CFGAS_GEFont>(this), wUnicode,
                                 bCharCode, &iWidth)) {
    CFX_RetainPtr<CFGAS_GEFont> pFont;
    int32_t iGlyph = GetGlyphIndex(wUnicode, true, &pFont, bCharCode);
    if (iGlyph != 0xFFFF && pFont) {
      if (pFont.Get() == this) {
        iWidth = m_pFont->GetGlyphWidth(iGlyph);
        if (iWidth < 0)
          iWidth = -1;
      } else if (pFont->GetCharWidthInternal(wUnicode, iWidth, false,
                                             bCharCode)) {
        return true;
      }
    } else {
      iWidth = -1;
    }
  }
  m_CharWidthMap[wUnicode] = iWidth;
  return iWidth > 0;
}

bool CFGAS_GEFont::GetCharBBox(wchar_t wUnicode,
                               CFX_Rect* bbox,
                               bool bCharCode) {
  return GetCharBBoxInternal(wUnicode, bbox, true, bCharCode);
}

bool CFGAS_GEFont::GetCharBBoxInternal(wchar_t wUnicode,
                                       CFX_Rect* bbox,
                                       bool bRecursive,
                                       bool bCharCode) {
  auto it = m_BBoxMap.find(wUnicode);
  if (it != m_BBoxMap.end()) {
    *bbox = it->second;
    return true;
  }

  CFX_RetainPtr<CFGAS_GEFont> pFont;
  int32_t iGlyph = GetGlyphIndex(wUnicode, true, &pFont, bCharCode);
  if (!pFont || iGlyph == 0xFFFF)
    return false;

  if (pFont.Get() != this)
    return pFont->GetCharBBoxInternal(wUnicode, bbox, false, bCharCode);

  FX_RECT rtBBox;
  if (!m_pFont->GetGlyphBBox(iGlyph, rtBBox))
    return false;

  CFX_Rect rt(rtBBox.left, rtBBox.top, rtBBox.Width(), rtBBox.Height());
  m_BBoxMap[wUnicode] = rt;
  *bbox = rt;
  return true;
}

bool CFGAS_GEFont::GetBBox(CFX_Rect* bbox) {
  FX_RECT rt(0, 0, 0, 0);
  if (!m_pFont->GetBBox(rt))
    return false;

  bbox->left = rt.left;
  bbox->width = rt.Width();
  bbox->top = rt.bottom;
  bbox->height = -rt.Height();
  return true;
}

int32_t CFGAS_GEFont::GetGlyphIndex(wchar_t wUnicode, bool bCharCode) {
  return GetGlyphIndex(wUnicode, true, nullptr, bCharCode);
}

int32_t CFGAS_GEFont::GetGlyphIndex(wchar_t wUnicode,
                                    bool bRecursive,
                                    CFX_RetainPtr<CFGAS_GEFont>* ppFont,
                                    bool bCharCode) {
  int32_t iGlyphIndex = m_pFontEncoding->GlyphFromCharCode(wUnicode);
  if (iGlyphIndex > 0) {
    if (ppFont)
      ppFont->Reset(this);
    return iGlyphIndex;
  }
  const FGAS_FONTUSB* pFontUSB = FGAS_GetUnicodeBitField(wUnicode);
  if (!pFontUSB)
    return 0xFFFF;

  uint16_t wBitField = pFontUSB->wBitField;
  if (wBitField >= 128)
    return 0xFFFF;

  auto it = m_FontMapper.find(wUnicode);
  if (it != m_FontMapper.end() && it->second && it->second.Get() != this) {
    iGlyphIndex =
        it->second->GetGlyphIndex(wUnicode, false, nullptr, bCharCode);
    if (iGlyphIndex != 0xFFFF) {
      for (size_t i = 0; i < m_SubstFonts.size(); ++i) {
        if (m_SubstFonts[i] == it->second) {
          if (ppFont)
            *ppFont = it->second;
          return (iGlyphIndex | ((i + 1) << 24));
        }
      }
    }
  }
  if (!m_pFontMgr || !bRecursive)
    return 0xFFFF;

  CFX_WideString wsFamily = GetFamilyName();
  CFX_RetainPtr<CFGAS_GEFont> pFont =
      m_pFontMgr->GetFontByUnicode(wUnicode, GetFontStyles(), wsFamily.c_str());
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  if (!pFont)
    pFont = m_pFontMgr->GetFontByUnicode(wUnicode, GetFontStyles(), nullptr);
#endif
  if (!pFont || pFont.Get() == this)  // Avoids direct cycles below.
    return 0xFFFF;

  m_FontMapper[wUnicode] = pFont;
  m_SubstFonts.push_back(pFont);
  iGlyphIndex = pFont->GetGlyphIndex(wUnicode, false, nullptr, bCharCode);
  if (iGlyphIndex == 0xFFFF)
    return 0xFFFF;

  if (ppFont)
    *ppFont = pFont;
  return (iGlyphIndex | (m_SubstFonts.size() << 24));
}

int32_t CFGAS_GEFont::GetAscent() const {
  return m_pFont->GetAscent();
}

int32_t CFGAS_GEFont::GetDescent() const {
  return m_pFont->GetDescent();
}

CFX_RetainPtr<CFGAS_GEFont> CFGAS_GEFont::GetSubstFont(int32_t iGlyphIndex) {
  iGlyphIndex = static_cast<uint32_t>(iGlyphIndex) >> 24;
  if (iGlyphIndex == 0)
    return CFX_RetainPtr<CFGAS_GEFont>(this);
  return m_SubstFonts[iGlyphIndex - 1];
}
