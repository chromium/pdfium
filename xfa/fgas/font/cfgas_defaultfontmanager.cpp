// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/font/cfgas_defaultfontmanager.h"

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxge/fx_font.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/font/cfgas_gemodule.h"
#include "xfa/fgas/font/fgas_fontutils.h"

// static
RetainPtr<CFGAS_GEFont> CFGAS_DefaultFontManager::GetFont(
    WideString wsFontName,
    uint32_t dwFontStyles) {
  CFGAS_FontMgr* font_mgr = CFGAS_GEModule::Get()->GetFontMgr();
  RetainPtr<CFGAS_GEFont> font = font_mgr->LoadFont(
      wsFontName.c_str(), dwFontStyles, FX_CodePage::kFailure);
  if (font) {
    return font;
  }
  const FGAS_FontInfo* pCurFont =
      FGAS_FontInfoByFontName(wsFontName.AsStringView());
  if (!pCurFont || !pCurFont->pReplaceFont) {
    return nullptr;
  }
  uint32_t dwStyle = 0;
  // TODO(dsinclair): Why doesn't this check the other flags?
  if (FontStyleIsForceBold(dwFontStyles)) {
    dwStyle |= pdfium::kFontStyleForceBold;
  }
  if (FontStyleIsItalic(dwFontStyles)) {
    dwStyle |= pdfium::kFontStyleItalic;
  }
  ByteStringView replace_view(pCurFont->pReplaceFont);
  while (!replace_view.IsEmpty()) {
    ByteStringView segment;
    auto found = replace_view.Find(',');
    if (found.has_value()) {
      segment = replace_view.First(found.value());
      replace_view = replace_view.Substr(found.value() + 1);
    } else {
      segment = replace_view;
      replace_view = ByteStringView();
    }
    font = font_mgr->LoadFont(WideString::FromASCII(segment).c_str(), dwStyle,
                              FX_CodePage::kFailure);
    if (font) {
      return font;
    }
  }
  return nullptr;
}

// static
RetainPtr<CFGAS_GEFont> CFGAS_DefaultFontManager::GetDefaultFont(
    uint32_t dwFontStyles) {
  CFGAS_FontMgr* font_mgr = CFGAS_GEModule::Get()->GetFontMgr();
  RetainPtr<CFGAS_GEFont> font =
      font_mgr->LoadFont(L"Arial Narrow", dwFontStyles, FX_CodePage::kFailure);
  if (font) {
    return font;
  }

  return font_mgr->LoadFont(nullptr, dwFontStyles, FX_CodePage::kFailure);
}
