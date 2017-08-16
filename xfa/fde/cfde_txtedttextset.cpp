// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_txtedttextset.h"

#include "xfa/fde/cfde_txtedtengine.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/cfx_txtbreak.h"

CFDE_TxtEdtTextSet::CFDE_TxtEdtTextSet(CFDE_TxtEdtPage* pPage)
    : m_pPage(pPage) {}

CFDE_TxtEdtTextSet::~CFDE_TxtEdtTextSet() {}

int32_t CFDE_TxtEdtTextSet::GetDisplayPos(const FDE_TEXTEDITPIECE& piece,
                                          FXTEXT_CHARPOS* pCharPos) const {
  int32_t nLength = piece.nCount;
  if (nLength < 1)
    return 0;

  CFDE_TxtEdtEngine* pEngine =
      static_cast<CFDE_TxtEdtEngine*>(m_pPage->GetEngine());
  const FDE_TXTEDTPARAMS* pTextParams = pEngine->GetEditParams();
  CFX_TxtBreak* pBreak = pEngine->GetTextBreak();
  uint32_t dwLayoutStyle = pBreak->GetLayoutStyles();
  FX_TXTRUN tr;
  tr.pAccess = m_pPage.Get();
  tr.pIdentity = &piece;
  tr.iLength = nLength;
  tr.pFont = pTextParams->pFont;
  tr.fFontSize = pTextParams->fFontSize;
  tr.dwStyles = dwLayoutStyle;
  tr.dwCharStyles = piece.dwCharStyles;
  tr.pRect = &piece.rtPiece;
  return pBreak->GetDisplayPos(&tr, pCharPos);
}

std::vector<CFX_RectF> CFDE_TxtEdtTextSet::GetCharRects(
    const FDE_TEXTEDITPIECE* pPiece,
    bool bBBox) const {
  if (!pPiece || pPiece->nCount < 1)
    return std::vector<CFX_RectF>();

  auto* pEngine = static_cast<CFDE_TxtEdtEngine*>(m_pPage->GetEngine());
  const FDE_TXTEDTPARAMS* pTextParams = pEngine->GetEditParams();
  uint32_t dwLayoutStyle = pEngine->GetTextBreak()->GetLayoutStyles();
  FX_TXTRUN tr;
  tr.pAccess = m_pPage.Get();
  tr.pIdentity = pPiece;
  tr.iLength = pPiece->nCount;
  tr.pFont = pTextParams->pFont;
  tr.fFontSize = pTextParams->fFontSize;
  tr.dwStyles = dwLayoutStyle;
  tr.dwCharStyles = pPiece->dwCharStyles;
  tr.pRect = &pPiece->rtPiece;
  return pEngine->GetTextBreak()->GetCharRects(&tr, bBBox);
}
