// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_deffontmgr.h"

#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_fontmgr.h"

CXFA_DefFontMgr::CXFA_DefFontMgr() {}

CXFA_DefFontMgr::~CXFA_DefFontMgr() {}

CFX_RetainPtr<CFGAS_GEFont> CXFA_DefFontMgr::GetFont(
    CXFA_FFDoc* hDoc,
    const CFX_WideStringC& wsFontFamily,
    uint32_t dwFontStyles,
    uint16_t wCodePage) {
  CFX_WideString wsFontName(wsFontFamily);
  CFGAS_FontMgr* pFDEFontMgr = hDoc->GetApp()->GetFDEFontMgr();
  CFX_RetainPtr<CFGAS_GEFont> pFont =
      pFDEFontMgr->LoadFont(wsFontName.c_str(), dwFontStyles, wCodePage);
  if (!pFont) {
    const XFA_FONTINFO* pCurFont =
        XFA_GetFontINFOByFontName(wsFontName.AsStringC());
    if (pCurFont && pCurFont->pReplaceFont) {
      uint32_t dwStyle = 0;
      if (dwFontStyles & FX_FONTSTYLE_Bold) {
        dwStyle |= FX_FONTSTYLE_Bold;
      }
      if (dwFontStyles & FX_FONTSTYLE_Italic) {
        dwStyle |= FX_FONTSTYLE_Italic;
      }
      const wchar_t* pReplace = pCurFont->pReplaceFont;
      int32_t iLength = FXSYS_wcslen(pReplace);
      while (iLength > 0) {
        const wchar_t* pNameText = pReplace;
        while (*pNameText != L',' && iLength > 0) {
          pNameText++;
          iLength--;
        }
        CFX_WideString wsReplace =
            CFX_WideString(pReplace, pNameText - pReplace);
        pFont = pFDEFontMgr->LoadFont(wsReplace.c_str(), dwStyle, wCodePage);
        if (pFont)
          break;

        iLength--;
        pNameText++;
        pReplace = pNameText;
      }
    }
  }
  if (pFont)
    m_CacheFonts.push_back(pFont);
  return pFont;
}

CFX_RetainPtr<CFGAS_GEFont> CXFA_DefFontMgr::GetDefaultFont(
    CXFA_FFDoc* hDoc,
    const CFX_WideStringC& wsFontFamily,
    uint32_t dwFontStyles,
    uint16_t wCodePage) {
  CFGAS_FontMgr* pFDEFontMgr = hDoc->GetApp()->GetFDEFontMgr();
  CFX_RetainPtr<CFGAS_GEFont> pFont =
      pFDEFontMgr->LoadFont(L"Arial Narrow", dwFontStyles, wCodePage);
  if (!pFont) {
    pFont = pFDEFontMgr->LoadFont(static_cast<const wchar_t*>(nullptr),
                                  dwFontStyles, wCodePage);
  }
  if (pFont)
    m_CacheFonts.push_back(pFont);
  return pFont;
}
