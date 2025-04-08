// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/font/cfgas_gefont.h"

#include <memory>
#include <utility>

#include "build/build_config.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fxcrt/check.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/cfx_unicodeencodingex.h"
#include "core/fxge/fx_font.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gemodule.h"
#include "xfa/fgas/font/fgas_fontutils.h"

// static
RetainPtr<CFGAS_GEFont> CFGAS_GEFont::LoadFont(const wchar_t* pszFontFamily,
                                               uint32_t dwFontStyles,
                                               FX_CodePage wCodePage) {
#if BUILDFLAG(IS_WIN)
  auto pFont = pdfium::MakeRetain<CFGAS_GEFont>();
  if (!pFont->LoadFontInternal(pszFontFamily, dwFontStyles, wCodePage)) {
    return nullptr;
  }
  return pFont;
#else
  return CFGAS_GEModule::Get()->GetFontMgr()->GetFontByCodePage(
      wCodePage, dwFontStyles, pszFontFamily);
#endif
}

// static
RetainPtr<CFGAS_GEFont> CFGAS_GEFont::LoadFont(RetainPtr<CPDF_Font> pPDFFont) {
  auto pFont = pdfium::MakeRetain<CFGAS_GEFont>();
  if (!pFont->LoadFontInternal(std::move(pPDFFont))) {
    return nullptr;
  }

  return pFont;
}

// static
RetainPtr<CFGAS_GEFont> CFGAS_GEFont::LoadFont(
    std::unique_ptr<CFX_Font> pInternalFont) {
  auto pFont = pdfium::MakeRetain<CFGAS_GEFont>();
  if (!pFont->LoadFontInternal(std::move(pInternalFont))) {
    return nullptr;
  }

  return pFont;
}

// static
RetainPtr<CFGAS_GEFont> CFGAS_GEFont::LoadStockFont(
    CPDF_Document* pDoc,
    const ByteString& font_family) {
  RetainPtr<CPDF_Font> stock_font =
      CPDF_Font::GetStockFont(pDoc, font_family.AsStringView());
  return stock_font ? CFGAS_GEFont::LoadFont(std::move(stock_font)) : nullptr;
}

CFGAS_GEFont::CFGAS_GEFont() = default;

CFGAS_GEFont::~CFGAS_GEFont() = default;

#if BUILDFLAG(IS_WIN)
bool CFGAS_GEFont::LoadFontInternal(const wchar_t* pszFontFamily,
                                    uint32_t dwFontStyles,
                                    FX_CodePage wCodePage) {
  if (font_) {
    return false;
  }
  ByteString csFontFamily;
  if (pszFontFamily) {
    csFontFamily = WideString(pszFontFamily).ToDefANSI();
  }

  int32_t iWeight = FontStyleIsForceBold(dwFontStyles)
                        ? pdfium::kFontWeightBold
                        : pdfium::kFontWeightNormal;
  font_ = std::make_unique<CFX_Font>();
  if (FontStyleIsItalic(dwFontStyles) && FontStyleIsForceBold(dwFontStyles)) {
    csFontFamily += ",BoldItalic";
  } else if (FontStyleIsForceBold(dwFontStyles)) {
    csFontFamily += ",Bold";
  } else if (FontStyleIsItalic(dwFontStyles)) {
    csFontFamily += ",Italic";
  }

  font_->LoadSubst(csFontFamily, true, dwFontStyles, iWeight, 0, wCodePage,
                   false);
  return font_->GetFace() && InitFont();
}
#endif  // BUILDFLAG(IS_WIN)

bool CFGAS_GEFont::LoadFontInternal(RetainPtr<CPDF_Font> pPDFFont) {
  DCHECK(pPDFFont);

  if (font_) {
    return false;
  }

  pdffont_ = std::move(pPDFFont);  // Keep `pPDFFont` alive for the duration.
  font_ = pdffont_->GetFont();
  return InitFont();
}

bool CFGAS_GEFont::LoadFontInternal(std::unique_ptr<CFX_Font> pInternalFont) {
  if (font_ || !pInternalFont) {
    return false;
  }

  font_ = std::move(pInternalFont);
  return InitFont();
}

bool CFGAS_GEFont::InitFont() {
  if (!font_) {
    return false;
  }

  if (font_encoding_) {
    return true;
  }

  font_encoding_ = FX_CreateFontEncodingEx(font_.Get());
  return !!font_encoding_;
}

WideString CFGAS_GEFont::GetFamilyName() const {
  CFX_SubstFont* subst_font = font_->GetSubstFont();
  ByteString family_name = subst_font && !subst_font->m_Family.IsEmpty()
                               ? subst_font->m_Family
                               : font_->GetFamilyName();
  return WideString::FromDefANSI(family_name.AsStringView());
}

uint32_t CFGAS_GEFont::GetFontStyles() const {
  DCHECK(font_);
  if (log_font_style_.has_value()) {
    return log_font_style_.value();
  }

  uint32_t dwStyles = 0;
  auto* pSubstFont = font_->GetSubstFont();
  if (pSubstFont) {
    if (pSubstFont->m_Weight == pdfium::kFontWeightBold) {
      dwStyles |= pdfium::kFontStyleForceBold;
    }
  } else {
    if (font_->IsBold()) {
      dwStyles |= pdfium::kFontStyleForceBold;
    }
    if (font_->IsItalic()) {
      dwStyles |= pdfium::kFontStyleItalic;
    }
  }
  return dwStyles;
}

std::optional<uint16_t> CFGAS_GEFont::GetCharWidth(wchar_t wUnicode) {
  auto it = char_width_map_.find(wUnicode);
  if (it != char_width_map_.end()) {
    return it->second;
  }

  auto [glyph, pFont] = GetGlyphIndexAndFont(wUnicode, true);
  if (!pFont || glyph == 0xffff) {
    char_width_map_[wUnicode] = std::nullopt;
    return std::nullopt;
  }
  if (pFont != this) {
    return pFont->GetCharWidth(wUnicode);
  }

  int32_t width_from_cfx_font = font_->GetGlyphWidth(glyph);
  if (width_from_cfx_font < 0) {
    char_width_map_[wUnicode] = std::nullopt;
    return std::nullopt;
  }
  uint16_t width = static_cast<uint16_t>(width_from_cfx_font);
  char_width_map_[wUnicode] = width;
  return width;
}

std::optional<FX_RECT> CFGAS_GEFont::GetCharBBox(wchar_t wUnicode) {
  auto it = bbox_map_.find(wUnicode);
  if (it != bbox_map_.end()) {
    return it->second;
  }

  auto [iGlyph, pFont] = GetGlyphIndexAndFont(wUnicode, true);
  if (!pFont || iGlyph == 0xFFFF) {
    return std::nullopt;
  }

  if (pFont.Get() != this) {
    return pFont->GetCharBBox(wUnicode);
  }

  std::optional<FX_RECT> rtBBox = font_->GetGlyphBBox(iGlyph);
  if (rtBBox.has_value()) {
    bbox_map_[wUnicode] = rtBBox.value();
  }

  return rtBBox;
}

int32_t CFGAS_GEFont::GetGlyphIndex(wchar_t wUnicode) {
  return GetGlyphIndexAndFont(wUnicode, true).first;
}

std::pair<int32_t, RetainPtr<CFGAS_GEFont>> CFGAS_GEFont::GetGlyphIndexAndFont(
    wchar_t wUnicode,
    bool bRecursive) {
  int32_t iGlyphIndex = font_encoding_->GlyphFromCharCode(wUnicode);
  if (iGlyphIndex > 0) {
    return {iGlyphIndex, pdfium::WrapRetain(this)};
  }

  const FGAS_FONTUSB* pFontUSB = FGAS_GetUnicodeBitField(wUnicode);
  if (!pFontUSB) {
    return {0xFFFF, nullptr};
  }

  uint16_t wBitField = pFontUSB->wBitField;
  if (wBitField >= 128) {
    return {0xFFFF, nullptr};
  }

  auto it = font_mapper_.find(wUnicode);
  if (it != font_mapper_.end() && it->second && it->second.Get() != this) {
    RetainPtr<CFGAS_GEFont> font;
    std::tie(iGlyphIndex, font) =
        it->second->GetGlyphIndexAndFont(wUnicode, false);
    if (iGlyphIndex != 0xFFFF) {
      for (size_t i = 0; i < subst_fonts_.size(); ++i) {
        if (subst_fonts_[i] == it->second) {
          return {(iGlyphIndex | ((i + 1) << 24)), it->second};
        }
      }
    }
  }
  if (!bRecursive) {
    return {0xFFFF, nullptr};
  }

  CFGAS_FontMgr* pFontMgr = CFGAS_GEModule::Get()->GetFontMgr();
  WideString wsFamily = GetFamilyName();
  RetainPtr<CFGAS_GEFont> pFont =
      pFontMgr->GetFontByUnicode(wUnicode, GetFontStyles(), wsFamily.c_str());
#if !BUILDFLAG(IS_WIN)
  if (!pFont) {
    pFont = pFontMgr->GetFontByUnicode(wUnicode, GetFontStyles(), nullptr);
  }
#endif
  if (!pFont || pFont == this) {  // Avoids direct cycles below.
    return {0xFFFF, nullptr};
  }

  font_mapper_[wUnicode] = pFont;
  subst_fonts_.push_back(pFont);

  RetainPtr<CFGAS_GEFont> font;
  std::tie(iGlyphIndex, font) = pFont->GetGlyphIndexAndFont(wUnicode, false);
  if (iGlyphIndex == 0xFFFF) {
    return {0xFFFF, nullptr};
  }

  return {(iGlyphIndex | (subst_fonts_.size() << 24)), pFont};
}

int32_t CFGAS_GEFont::GetAscent() const {
  return font_->GetAscent();
}

int32_t CFGAS_GEFont::GetDescent() const {
  return font_->GetDescent();
}

RetainPtr<CFGAS_GEFont> CFGAS_GEFont::GetSubstFont(int32_t iGlyphIndex) {
  iGlyphIndex = static_cast<uint32_t>(iGlyphIndex) >> 24;
  if (iGlyphIndex == 0) {
    return pdfium::WrapRetain(this);
  }
  return subst_fonts_[iGlyphIndex - 1];
}
