// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_txtedttextset.h"

#include "xfa/fde/cfde_txtedtengine.h"
#include "xfa/fde/cfde_txtedtpage.h"

CFDE_TxtEdtTextSet::CFDE_TxtEdtTextSet(CFDE_TxtEdtPage* pPage)
    : m_pPage(pPage) {}

CFDE_TxtEdtTextSet::~CFDE_TxtEdtTextSet() {}

FDE_VISUALOBJTYPE CFDE_TxtEdtTextSet::GetType() {
  return FDE_VISUALOBJ_Text;
}

FX_BOOL CFDE_TxtEdtTextSet::GetBBox(FDE_HVISUALOBJ hVisualObj,
                                    CFX_RectF& bbox) {
  return FALSE;
}

FX_BOOL CFDE_TxtEdtTextSet::GetMatrix(FDE_HVISUALOBJ hVisualObj,
                                      CFX_Matrix& matrix) {
  return FALSE;
}

FX_BOOL CFDE_TxtEdtTextSet::GetRect(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) {
  rt = reinterpret_cast<const FDE_TEXTEDITPIECE*>(hVisualObj)->rtPiece;
  return TRUE;
}

FX_BOOL CFDE_TxtEdtTextSet::GetClip(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) {
  return FALSE;
}

int32_t CFDE_TxtEdtTextSet::GetString(FDE_HVISUALOBJ hText,
                                      CFX_WideString& wsText) {
  const FDE_TEXTEDITPIECE* pPiece =
      reinterpret_cast<const FDE_TEXTEDITPIECE*>(hText);
  FX_WCHAR* pBuffer = wsText.GetBuffer(pPiece->nCount);
  for (int32_t i = 0; i < pPiece->nCount; i++) {
    pBuffer[i] = m_pPage->GetChar((void*)hText, i);
  }
  wsText.ReleaseBuffer(pPiece->nCount);
  return pPiece->nCount;
}

IFX_Font* CFDE_TxtEdtTextSet::GetFont(FDE_HVISUALOBJ hText) {
  return m_pPage->GetEngine()->GetEditParams()->pFont;
}

FX_FLOAT CFDE_TxtEdtTextSet::GetFontSize(FDE_HVISUALOBJ hText) {
  return m_pPage->GetEngine()->GetEditParams()->fFontSize;
}

FX_ARGB CFDE_TxtEdtTextSet::GetFontColor(FDE_HVISUALOBJ hText) {
  return m_pPage->GetEngine()->GetEditParams()->dwFontColor;
}

int32_t CFDE_TxtEdtTextSet::GetDisplayPos(FDE_HVISUALOBJ hText,
                                          FXTEXT_CHARPOS* pCharPos,
                                          FX_BOOL bCharCode,
                                          CFX_WideString* pWSForms) {
  if (!hText)
    return 0;

  const FDE_TEXTEDITPIECE* pPiece =
      reinterpret_cast<const FDE_TEXTEDITPIECE*>(hText);
  int32_t nLength = pPiece->nCount;
  if (nLength < 1)
    return 0;

  CFDE_TxtEdtEngine* pEngine =
      static_cast<CFDE_TxtEdtEngine*>(m_pPage->GetEngine());
  const FDE_TXTEDTPARAMS* pTextParams = pEngine->GetEditParams();
  CFX_TxtBreak* pBreak = pEngine->GetTextBreak();
  uint32_t dwLayoutStyle = pBreak->GetLayoutStyles();
  FX_TXTRUN tr;
  tr.pAccess = m_pPage;
  tr.pIdentity = (void*)hText;
  tr.iLength = nLength;
  tr.pFont = pTextParams->pFont;
  tr.fFontSize = pTextParams->fFontSize;
  tr.dwStyles = dwLayoutStyle;
  tr.iCharRotation = pTextParams->nCharRotation;
  tr.dwCharStyles = pPiece->dwCharStyles;
  tr.pRect = &(pPiece->rtPiece);
  tr.wLineBreakChar = pTextParams->wLineBreakChar;
  return pBreak->GetDisplayPos(&tr, pCharPos, bCharCode, pWSForms);
}

int32_t CFDE_TxtEdtTextSet::GetCharRects(FDE_HVISUALOBJ hText,
                                         CFX_RectFArray& rtArray) {
  return GetCharRects_Impl(hText, rtArray);
}

int32_t CFDE_TxtEdtTextSet::GetCharRects_Impl(FDE_HVISUALOBJ hText,
                                              CFX_RectFArray& rtArray,
                                              FX_BOOL bBBox) {
  if (!hText)
    return 0;

  const FDE_TEXTEDITPIECE* pPiece =
      reinterpret_cast<const FDE_TEXTEDITPIECE*>(hText);
  CFDE_TxtEdtEngine* pEngine =
      static_cast<CFDE_TxtEdtEngine*>(m_pPage->GetEngine());
  int32_t nLength = pPiece->nCount;
  if (nLength < 1)
    return 0;

  const FDE_TXTEDTPARAMS* pTextParams = pEngine->GetEditParams();
  uint32_t dwLayoutStyle = pEngine->GetTextBreak()->GetLayoutStyles();
  FX_TXTRUN tr;
  tr.pAccess = m_pPage;
  tr.pIdentity = (void*)hText;
  tr.iLength = nLength;
  tr.pFont = pTextParams->pFont;
  tr.fFontSize = pTextParams->fFontSize;
  tr.dwStyles = dwLayoutStyle;
  tr.iCharRotation = pTextParams->nCharRotation;
  tr.dwCharStyles = pPiece->dwCharStyles;
  tr.pRect = &(pPiece->rtPiece);
  tr.wLineBreakChar = pTextParams->wLineBreakChar;
  return pEngine->GetTextBreak()->GetCharRects(&tr, rtArray, bBBox);
}
