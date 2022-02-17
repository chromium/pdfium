// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/font/cfgas_defaultfontmanager.h"

#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/fx_font.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/font/cfgas_gemodule.h"
#include "xfa/fgas/font/fgas_fontutils.h"

// static
RetainPtr<CFGAS_GEFont> CFGAS_DefaultFontManager::GetFont(
    WideStringView wsFontFamily,
    uint32_t dwFontStyles) {
  WideString wsFontName(wsFontFamily);
  CFGAS_FontMgr* pFontMgr = CFGAS_GEModule::Get()->GetFontMgr();
  RetainPtr<CFGAS_GEFont> pFont = pFontMgr->LoadFont(
      wsFontName.c_str(), dwFontStyles, FX_CodePage::kFailure);
  if (pFont)
    return pFont;

  const FGAS_FontInfo* pCurFont =
      FGAS_FontInfoByFontName(wsFontName.AsStringView());
  if (!pCurFont || !pCurFont->pReplaceFont)
    return pFont;

  uint32_t dwStyle = 0;
  // TODO(dsinclair): Why doesn't this check the other flags?
  if (FontStyleIsForceBold(dwFontStyles))
    dwStyle |= FXFONT_FORCE_BOLD;
  if (FontStyleIsItalic(dwFontStyles))
    dwStyle |= FXFONT_ITALIC;

  const char* pReplace = pCurFont->pReplaceFont;
  int32_t iLength = pdfium::base::checked_cast<int32_t>(strlen(pReplace));
  while (iLength > 0) {
    const char* pNameText = pReplace;
    while (*pNameText != ',' && iLength > 0) {
      pNameText++;
      iLength--;
    }
    WideString wsReplace =
        WideString::FromASCII(ByteStringView(pReplace, pNameText - pReplace));
    pFont =
        pFontMgr->LoadFont(wsReplace.c_str(), dwStyle, FX_CodePage::kFailure);
    if (pFont)
      break;

    iLength--;
    pNameText++;
    pReplace = pNameText;
  }
  return pFont;
}

// static
RetainPtr<CFGAS_GEFont> CFGAS_DefaultFontManager::GetDefaultFont(
    uint32_t dwFontStyles) {
  CFGAS_FontMgr* pFontMgr = CFGAS_GEModule::Get()->GetFontMgr();
  RetainPtr<CFGAS_GEFont> pFont =
      pFontMgr->LoadFont(L"Arial Narrow", dwFontStyles, FX_CodePage::kFailure);
  if (pFont)
    return pFont;

  return pFontMgr->LoadFont(nullptr, dwFontStyles, FX_CodePage::kFailure);
}
