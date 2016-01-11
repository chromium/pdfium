// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "xfa/src/foxitlib.h"
#include "xfa/src/fee/include/ifde_txtedtbuf.h"
#include "xfa/src/fee/include/ifde_txtedtengine.h"
#include "xfa/src/fee/include/ifde_txtedtpage.h"
#include "xfa/src/fee/include/fx_wordbreak.h"
#include "fde_txtedtpage.h"
#include "fde_txtedtengine.h"
#include "fde_txtedtparag.h"
#include "fde_txtedtbuf.h"
#define FDE_TXTEDT_TOLERANCE 0.1f
IFDE_TxtEdtPage* IFDE_TxtEdtPage::Create(IFDE_TxtEdtEngine* pEngine,
                                         int32_t nIndex) {
  return (IFDE_TxtEdtPage*)new CFDE_TxtEdtPage(pEngine, nIndex);
}
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
  rt = ((FDE_LPTEXTEDITPIECE)(hVisualObj))->rtPiece;
  return TRUE;
}
FX_BOOL CFDE_TxtEdtTextSet::GetClip(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) {
  return FALSE;
}
int32_t CFDE_TxtEdtTextSet::GetString(FDE_HVISUALOBJ hText,
                                      CFX_WideString& wsText) {
  FDE_LPTEXTEDITPIECE pPiece = (FDE_LPTEXTEDITPIECE)hText;
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
  if (hText == NULL) {
    return 0;
  }
  FDE_LPTEXTEDITPIECE pPiece = (FDE_LPTEXTEDITPIECE)hText;
  int32_t nLength = pPiece->nCount;
  if (nLength < 1) {
    return 0;
  }
  CFDE_TxtEdtEngine* pEngine = (CFDE_TxtEdtEngine*)(m_pPage->GetEngine());
  const FDE_TXTEDTPARAMS* pTextParams = pEngine->GetEditParams();
  IFX_TxtBreak* pBreak = pEngine->GetTextBreak();
  FX_DWORD dwLayoutStyle = pBreak->GetLayoutStyles();
  FX_TXTRUN tr;
  tr.pAccess = m_pPage;
  tr.pIdentity = (void*)hText;
  tr.pStr = NULL;
  tr.pWidths = NULL;
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
  if (hText == NULL) {
    return 0;
  }
  FDE_LPTEXTEDITPIECE pPiece = (FDE_LPTEXTEDITPIECE)hText;
  CFDE_TxtEdtEngine* pEngine = (CFDE_TxtEdtEngine*)(m_pPage->GetEngine());
  int32_t nLength = pPiece->nCount;
  if (nLength < 1) {
    return 0;
  }
  const FDE_TXTEDTPARAMS* pTextParams = pEngine->GetEditParams();
  FX_DWORD dwLayoutStyle = pEngine->GetTextBreak()->GetLayoutStyles();
  FX_TXTRUN tr;
  tr.pAccess = m_pPage;
  tr.pIdentity = (void*)hText;
  tr.pStr = NULL;
  tr.pWidths = NULL;
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
CFDE_TxtEdtPage::CFDE_TxtEdtPage(IFDE_TxtEdtEngine* pEngine, int32_t nPageIndex)
    : m_pIter(nullptr),
      m_pTextSet(nullptr),
      m_pBgnParag(nullptr),
      m_pEndParag(nullptr),
      m_nRefCount(0),
      m_nPageStart(-1),
      m_nCharCount(0),
      m_nPageIndex(nPageIndex),
      m_bLoaded(FALSE),
      m_pCharWidth(nullptr) {
  FXSYS_memset(&m_rtPage, 0, sizeof(CFX_RectF));
  FXSYS_memset(&m_rtPageMargin, 0, sizeof(CFX_RectF));
  FXSYS_memset(&m_rtPageContents, 0, sizeof(CFX_RectF));
  FXSYS_memset(&m_rtPageCanvas, 0, sizeof(CFX_RectF));
  m_pEditEngine = (CFDE_TxtEdtEngine*)pEngine;
}
CFDE_TxtEdtPage::~CFDE_TxtEdtPage() {
  m_PieceMassArr.RemoveAll(TRUE);
  if (m_pTextSet) {
    delete m_pTextSet;
    m_pTextSet = NULL;
  }
  if (m_pCharWidth) {
    delete[] m_pCharWidth;
    m_pCharWidth = NULL;
  }
  if (m_pIter != NULL) {
    m_pIter->Release();
    m_pIter = NULL;
  }
}
void CFDE_TxtEdtPage::Release() {
  delete this;
}
IFDE_TxtEdtEngine* CFDE_TxtEdtPage::GetEngine() const {
  return (IFDE_TxtEdtEngine*)m_pEditEngine;
}
FDE_VISUALOBJTYPE CFDE_TxtEdtPage::GetType() {
  return FDE_VISUALOBJ_Text;
}
FX_BOOL CFDE_TxtEdtPage::GetBBox(FDE_HVISUALOBJ hVisualObj, CFX_RectF& bbox) {
  return FALSE;
}
FX_BOOL CFDE_TxtEdtPage::GetMatrix(FDE_HVISUALOBJ hVisualObj,
                                   CFX_Matrix& matrix) {
  return FALSE;
}
FX_BOOL CFDE_TxtEdtPage::GetRect(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) {
  return FALSE;
}
FX_BOOL CFDE_TxtEdtPage::GetClip(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) {
  return FALSE;
}
int32_t CFDE_TxtEdtPage::GetCharRect(int32_t nIndex,
                                     CFX_RectF& rect,
                                     FX_BOOL bBBox) const {
  FXSYS_assert(m_nRefCount > 0);
  FXSYS_assert(nIndex >= 0 && nIndex < m_nCharCount);
  if (m_nRefCount < 1) {
    return 0;
  }
  int32_t nCount = m_PieceMassArr.GetSize();
  for (int32_t i = 0; i < nCount; i++) {
    const FDE_LPTEXTEDITPIECE pPiece = m_PieceMassArr.GetPtrAt(i);
    if (nIndex >= pPiece->nStart &&
        nIndex < (pPiece->nStart + pPiece->nCount)) {
      CFX_RectFArray rectArr;
      if (bBBox) {
        m_pTextSet->GetCharRects_Impl((FDE_HVISUALOBJ)pPiece, rectArr, bBBox);
      } else {
        m_pTextSet->GetCharRects((FDE_HVISUALOBJ)pPiece, rectArr);
      }
      rect = rectArr[nIndex - pPiece->nStart];
      return pPiece->nBidiLevel;
    }
  }
  FXSYS_assert(0);
  return 0;
}
int32_t CFDE_TxtEdtPage::GetCharIndex(const CFX_PointF& fPoint,
                                      FX_BOOL& bBefore) {
  FX_BOOL bVertical = m_pEditEngine->GetEditParams()->dwLayoutStyles &
                      FDE_TEXTEDITLAYOUT_DocVertical;
  CFX_PointF ptF = fPoint;
  NormalizePt2Rect(ptF, m_rtPageContents, FDE_TXTEDT_TOLERANCE);
  int32_t nCount = m_PieceMassArr.GetSize();
  CFX_RectF rtLine;
  int32_t nBgn = 0;
  int32_t nEnd = 0;
  FX_BOOL bInLine = FALSE;
  int32_t i = 0;
  for (i = 0; i < nCount; i++) {
    const FDE_LPTEXTEDITPIECE pPiece = m_PieceMassArr.GetPtrAt(i);
    if (!bInLine && (bVertical ? (pPiece->rtPiece.left <= ptF.x &&
                                  pPiece->rtPiece.right() > ptF.x)
                               : (pPiece->rtPiece.top <= ptF.y &&
                                  pPiece->rtPiece.bottom() > ptF.y))) {
      nBgn = nEnd = i;
      rtLine = pPiece->rtPiece;
      bInLine = TRUE;
    } else if (bInLine) {
      if (bVertical ? (!(pPiece->rtPiece.left <= ptF.x &&
                         pPiece->rtPiece.right() > ptF.x))
                    : (pPiece->rtPiece.bottom() <= ptF.y ||
                       pPiece->rtPiece.top > ptF.y)) {
        nEnd = i - 1;
        break;
      } else {
        rtLine.Union(pPiece->rtPiece);
      }
    }
  }
  NormalizePt2Rect(ptF, rtLine, FDE_TXTEDT_TOLERANCE);
  int32_t nCaret = 0;
  FDE_LPTEXTEDITPIECE pPiece = NULL;
  for (i = nBgn; i <= nEnd; i++) {
    pPiece = m_PieceMassArr.GetPtrAt(i);
    nCaret = m_nPageStart + pPiece->nStart;
    if (pPiece->rtPiece.Contains(ptF)) {
      CFX_RectFArray rectArr;
      m_pTextSet->GetCharRects((FDE_HVISUALOBJ)pPiece, rectArr);
      int32_t nRtCount = rectArr.GetSize();
      for (int32_t j = 0; j < nRtCount; j++) {
        if (rectArr[j].Contains(ptF)) {
          nCaret = m_nPageStart + pPiece->nStart + j;
          if (nCaret >= m_pEditEngine->GetTextBufLength()) {
            bBefore = TRUE;
            return m_pEditEngine->GetTextBufLength();
          }
          FX_WCHAR wChar = m_pEditEngine->GetTextBuf()->GetCharByIndex(nCaret);
          if (wChar == L'\n' || wChar == L'\r') {
            if (wChar == L'\n') {
              if (m_pEditEngine->GetTextBuf()->GetCharByIndex(nCaret - 1) ==
                  L'\r') {
                nCaret--;
              }
            }
            bBefore = TRUE;
            return nCaret;
          }
          if (bVertical
                  ? (ptF.y > ((rectArr[j].top + rectArr[j].bottom()) / 2))
                  : (ptF.x > ((rectArr[j].left + rectArr[j].right()) / 2))) {
            bBefore = FX_IsOdd(pPiece->nBidiLevel);
          } else {
            bBefore = !FX_IsOdd(pPiece->nBidiLevel);
          }
          return nCaret;
        }
      }
    }
  }
  bBefore = TRUE;
  return nCaret;
}
int32_t CFDE_TxtEdtPage::GetCharStart() const {
  return m_nPageStart;
}
int32_t CFDE_TxtEdtPage::GetCharCount() const {
  return m_nCharCount;
}
int32_t CFDE_TxtEdtPage::GetDisplayPos(const CFX_RectF& rtClip,
                                       FXTEXT_CHARPOS*& pCharPos,
                                       FX_LPRECTF pBBox) const {
  pCharPos = FX_Alloc(FXTEXT_CHARPOS, m_nCharCount);
  int32_t nCharPosCount = 0;
  FDE_HVISUALOBJ hVisualObj = NULL;
  int32_t nVisualObjCount = m_PieceMassArr.GetSize();
  FXTEXT_CHARPOS* pos = pCharPos;
  CFX_RectF rtObj;
  for (int32_t i = 0; i < nVisualObjCount; i++) {
    hVisualObj = (FDE_HVISUALOBJ)m_PieceMassArr.GetPtrAt(i);
    m_pTextSet->GetRect(hVisualObj, rtObj);
    if (!rtClip.IntersectWith(rtObj)) {
      continue;
    }
    int32_t nCount = m_pTextSet->GetDisplayPos(hVisualObj, pos, FALSE);
    nCharPosCount += nCount;
    pos += nCount;
  }
  if ((nCharPosCount * 5) < (m_nCharCount << 2)) {
    FXTEXT_CHARPOS* pTemp = FX_Alloc(FXTEXT_CHARPOS, nCharPosCount);
    FXSYS_memcpy(pTemp, pCharPos, sizeof(FXTEXT_CHARPOS) * nCharPosCount);
    FX_Free(pCharPos);
    pCharPos = pTemp;
  }
  return nCharPosCount;
}
void CFDE_TxtEdtPage::CalcRangeRectArray(int32_t nStart,
                                         int32_t nCount,
                                         CFX_RectFArray& RectFArr) const {
  int32_t nPieceCount = m_PieceMassArr.GetSize();
  int32_t nEnd = nStart + nCount - 1;
  FX_BOOL bInRange = FALSE;
  for (int32_t i = 0; i < nPieceCount; i++) {
    FDE_LPTEXTEDITPIECE piece = m_PieceMassArr.GetPtrAt(i);
    if (!bInRange) {
      if (nStart >= piece->nStart && nStart < (piece->nStart + piece->nCount)) {
        int32_t nRangeEnd = piece->nCount - 1;
        FX_BOOL bEnd = FALSE;
        if (nEnd >= piece->nStart && nEnd < (piece->nStart + piece->nCount)) {
          nRangeEnd = nEnd - piece->nStart;
          bEnd = TRUE;
        }
        CFX_RectFArray rcArr;
        m_pTextSet->GetCharRects((FDE_HVISUALOBJ)piece, rcArr);
        CFX_RectF rectPiece = rcArr[nStart - piece->nStart];
        rectPiece.Union(rcArr[nRangeEnd]);
        RectFArr.Add(rectPiece);
        if (bEnd) {
          return;
        }
        bInRange = TRUE;
      }
    } else {
      if (nEnd >= piece->nStart && nEnd < (piece->nStart + piece->nCount)) {
        CFX_RectFArray rcArr;
        m_pTextSet->GetCharRects((FDE_HVISUALOBJ)piece, rcArr);
        CFX_RectF rectPiece = rcArr[0];
        rectPiece.Union(rcArr[nEnd - piece->nStart]);
        RectFArr.Add(rectPiece);
        return;
      }
      RectFArr.Add(piece->rtPiece);
    }
  }
  return;
}
int32_t CFDE_TxtEdtPage::SelectWord(const CFX_PointF& fPoint, int32_t& nCount) {
  if (m_nRefCount < 0) {
    return -1;
  }
  IFDE_TxtEdtBuf* pBuf = m_pEditEngine->GetTextBuf();
  FX_BOOL bBefore;
  int32_t nIndex = GetCharIndex(fPoint, bBefore);
  if (nIndex == m_pEditEngine->GetTextBufLength()) {
    nIndex = m_pEditEngine->GetTextBufLength() - 1;
  }
  if (nIndex < 0) {
    return -1;
  }
  IFX_WordBreak* pIter = FX_WordBreak_Create();
  pIter->Attach(new CFDE_TxtEdtBufIter((CFDE_TxtEdtBuf*)pBuf));
  pIter->SetAt(nIndex);
  nCount = pIter->GetWordLength();
  int32_t nRet = pIter->GetWordPos();
  pIter->Release();
  return nRet;
}
FX_BOOL CFDE_TxtEdtPage::IsLoaded(FX_LPCRECTF pClipBox) {
  return m_bLoaded;
}
int32_t CFDE_TxtEdtPage::LoadPage(FX_LPCRECTF pClipBox, IFX_Pause* pPause) {
  if (m_nRefCount > 0) {
    m_nRefCount++;
    return m_nRefCount;
  }
  IFDE_TxtEdtBuf* pBuf = m_pEditEngine->GetTextBuf();
  const FDE_TXTEDTPARAMS* pParams = m_pEditEngine->GetEditParams();
  if (m_pIter != NULL) {
    m_pIter->Release();
  }
  FX_WCHAR wcAlias = 0;
  if (pParams->dwMode & FDE_TEXTEDITMODE_Password) {
    wcAlias = m_pEditEngine->GetAliasChar();
  }
  m_pIter = new CFDE_TxtEdtBufIter((CFDE_TxtEdtBuf*)pBuf, wcAlias);
  IFX_TxtBreak* pBreak = m_pEditEngine->GetTextBreak();
  pBreak->EndBreak(FX_TXTBREAK_ParagraphBreak);
  pBreak->ClearBreakPieces();
  int32_t nPageLineCount = m_pEditEngine->GetPageLineCount();
  int32_t nStartLine = nPageLineCount * m_nPageIndex;
  int32_t nEndLine = std::min((nStartLine + nPageLineCount - 1),
                              (m_pEditEngine->GetLineCount() - 1));
  int32_t nPageStart, nPageEnd, nTemp, nBgnParag, nStartLineInParag, nEndParag,
      nEndLineInParag;
  nBgnParag = m_pEditEngine->Line2Parag(0, 0, nStartLine, nStartLineInParag);
  m_pBgnParag = (CFDE_TxtEdtParag*)m_pEditEngine->GetParag(nBgnParag);
  m_pBgnParag->LoadParag();
  m_pBgnParag->GetLineRange(nStartLine - nStartLineInParag, nPageStart, nTemp);
  nEndParag = m_pEditEngine->Line2Parag(nBgnParag, nStartLineInParag, nEndLine,
                                        nEndLineInParag);
  m_pEndParag = (CFDE_TxtEdtParag*)m_pEditEngine->GetParag(nEndParag);
  m_pEndParag->LoadParag();
  m_pEndParag->GetLineRange(nEndLine - nEndLineInParag, nPageEnd, nTemp);
  nPageEnd += (nTemp - 1);
  FX_BOOL bVertial = pParams->dwLayoutStyles & FDE_TEXTEDITLAYOUT_DocVertical;
  FX_BOOL bLineReserve =
      pParams->dwLayoutStyles & FDE_TEXTEDITLAYOUT_LineReserve;
  FX_FLOAT fLineStart =
      bVertial
          ? (bLineReserve ? (pParams->fPlateWidth - pParams->fLineSpace) : 0.0f)
          : 0.0f;
  FX_FLOAT fLineStep =
      (bVertial && bLineReserve) ? (-pParams->fLineSpace) : pParams->fLineSpace;
  FX_FLOAT fLinePos = fLineStart;
  if (m_pTextSet == NULL) {
    m_pTextSet = new CFDE_TxtEdtTextSet(this);
  }
  m_PieceMassArr.RemoveAll(TRUE);
  FX_DWORD dwBreakStatus = FX_TXTBREAK_None;
  int32_t nPieceStart = 0;
  if (m_pCharWidth != NULL) {
    delete[] m_pCharWidth;
  }
  m_pCharWidth = new int32_t[nPageEnd - nPageStart + 1];
  pBreak->EndBreak(FX_TXTBREAK_ParagraphBreak);
  pBreak->ClearBreakPieces();
  m_nPageStart = nPageStart;
  m_nCharCount = nPageEnd - nPageStart + 1;
  FX_BOOL bReload = FALSE;
  FX_FLOAT fDefCharWidth = 0;
  IFX_CharIter* pIter = m_pIter->Clone();
  pIter->SetAt(nPageStart);
  m_pIter->SetAt(nPageStart);
  FX_BOOL bFirstPiece = TRUE;
  do {
    if (bReload) {
      dwBreakStatus = pBreak->EndBreak(FX_TXTBREAK_ParagraphBreak);
    } else {
      FX_WCHAR wAppend = pIter->GetChar();
      dwBreakStatus = pBreak->AppendChar(wAppend);
    }
    if (pIter->GetAt() == nPageEnd && dwBreakStatus < FX_TXTBREAK_LineBreak) {
      dwBreakStatus = pBreak->EndBreak(FX_TXTBREAK_ParagraphBreak);
    }
    if (dwBreakStatus > FX_TXTBREAK_PieceBreak) {
      int32_t nPieceCount = pBreak->CountBreakPieces();
      for (int32_t j = 0; j < nPieceCount; j++) {
        const CFX_TxtPiece* pPiece = pBreak->GetBreakPiece(j);
        FDE_TEXTEDITPIECE TxtEdtPiece;
        FXSYS_memset(&TxtEdtPiece, 0, sizeof(FDE_TEXTEDITPIECE));
        TxtEdtPiece.nBidiLevel = pPiece->m_iBidiLevel;
        TxtEdtPiece.nCount = pPiece->GetLength();
        TxtEdtPiece.nStart = nPieceStart;
        TxtEdtPiece.dwCharStyles = pPiece->m_dwCharStyles;
        if (FX_IsOdd(pPiece->m_iBidiLevel)) {
          TxtEdtPiece.dwCharStyles |= FX_TXTCHARSTYLE_OddBidiLevel;
        }
        FX_FLOAT fParaBreakWidth = 0.0f;
        if (pPiece->m_dwStatus > FX_TXTBREAK_PieceBreak) {
          FX_WCHAR wRtChar = pParams->wLineBreakChar;
          if (TxtEdtPiece.nCount >= 2) {
            FX_WCHAR wChar = pBuf->GetCharByIndex(
                m_nPageStart + TxtEdtPiece.nStart + TxtEdtPiece.nCount - 1);
            FX_WCHAR wCharPre = pBuf->GetCharByIndex(
                m_nPageStart + TxtEdtPiece.nStart + TxtEdtPiece.nCount - 2);
            if (wChar == wRtChar) {
              fParaBreakWidth += fDefCharWidth;
            }
            if (wCharPre == wRtChar) {
              fParaBreakWidth += fDefCharWidth;
            }
          } else if (TxtEdtPiece.nCount >= 1) {
            FX_WCHAR wChar = pBuf->GetCharByIndex(
                m_nPageStart + TxtEdtPiece.nStart + TxtEdtPiece.nCount - 1);
            if (wChar == wRtChar) {
              fParaBreakWidth += fDefCharWidth;
            }
          }
        }
        if (pParams->dwLayoutStyles & FDE_TEXTEDITLAYOUT_DocVertical) {
          TxtEdtPiece.rtPiece.left = fLinePos;
          TxtEdtPiece.rtPiece.top = (FX_FLOAT)pPiece->m_iStartPos / 20000.0f;
          TxtEdtPiece.rtPiece.width = pParams->fLineSpace;
          TxtEdtPiece.rtPiece.height =
              (FX_FLOAT)pPiece->m_iWidth / 20000.0f + fParaBreakWidth;
        } else {
          TxtEdtPiece.rtPiece.left = (FX_FLOAT)pPiece->m_iStartPos / 20000.0f;
          TxtEdtPiece.rtPiece.top = fLinePos;
          TxtEdtPiece.rtPiece.width =
              (FX_FLOAT)pPiece->m_iWidth / 20000.0f + fParaBreakWidth;
          TxtEdtPiece.rtPiece.height = pParams->fLineSpace;
        }
        if (bFirstPiece) {
          m_rtPageContents = TxtEdtPiece.rtPiece;
          bFirstPiece = FALSE;
        } else {
          m_rtPageContents.Union(TxtEdtPiece.rtPiece);
        }
        nPieceStart += TxtEdtPiece.nCount;
        m_PieceMassArr.Add(TxtEdtPiece);
        for (int32_t k = 0; k < TxtEdtPiece.nCount; k++) {
          CFX_Char* ptc = pPiece->GetCharPtr(k);
          m_pCharWidth[TxtEdtPiece.nStart + k] = ptc->m_iCharWidth;
        }
      }
      fLinePos += fLineStep;
      pBreak->ClearBreakPieces();
    }
    if (pIter->GetAt() == nPageEnd && dwBreakStatus == FX_TXTBREAK_LineBreak) {
      bReload = TRUE;
      pIter->Next(TRUE);
    }
  } while (pIter->Next(FALSE) && (pIter->GetAt() <= nPageEnd));
  if (m_rtPageContents.left != 0) {
    FX_FLOAT fDelta = 0.0f;
    if (m_rtPageContents.width < pParams->fPlateWidth) {
      if (pParams->dwAlignment & FDE_TEXTEDITALIGN_Right) {
        fDelta = pParams->fPlateWidth - m_rtPageContents.width;
      } else if (pParams->dwAlignment & FDE_TEXTEDITALIGN_Center) {
        if ((pParams->dwLayoutStyles & FDE_TEXTEDITLAYOUT_CombText) &&
            m_nCharCount > 1) {
          int32_t nCount = m_nCharCount - 1;
          int32_t n = (m_pEditEngine->m_nLimit - nCount) / 2;
          fDelta = (m_rtPageContents.width / nCount) * n;
        } else {
          fDelta = (pParams->fPlateWidth - m_rtPageContents.width) / 2;
        }
      }
    }
    FX_FLOAT fOffset = m_rtPageContents.left - fDelta;
    int32_t nCount = m_PieceMassArr.GetSize();
    for (int32_t i = 0; i < nCount; i++) {
      FDE_LPTEXTEDITPIECE pPiece = m_PieceMassArr.GetPtrAt(i);
      pPiece->rtPiece.Offset(-fOffset, 0.0f);
    }
    m_rtPageContents.Offset(-fOffset, 0.0f);
  }
  if (m_pEditEngine->GetEditParams()->dwLayoutStyles &
      FDE_TEXTEDITLAYOUT_LastLineHeight) {
    m_rtPageContents.height -= pParams->fLineSpace - pParams->fFontSize;
    int32_t nCount = m_PieceMassArr.GetSize();
    FDE_LPTEXTEDITPIECE pPiece = m_PieceMassArr.GetPtrAt(nCount - 1);
    pPiece->rtPiece.height = pParams->fFontSize;
  }
  pIter->Release();
  m_nRefCount = 1;
  m_bLoaded = TRUE;
  return 0;
}
void CFDE_TxtEdtPage::UnloadPage(FX_LPCRECTF pClipBox) {
  FXSYS_assert(m_nRefCount > 0);
  m_nRefCount--;
  if (m_nRefCount == 0) {
    m_PieceMassArr.RemoveAll();
    if (m_pTextSet) {
      delete m_pTextSet;
      m_pTextSet = NULL;
    }
    if (m_pCharWidth) {
      delete[] m_pCharWidth;
      m_pCharWidth = NULL;
    }
    if (m_pBgnParag) {
      m_pBgnParag->UnloadParag();
    }
    if (m_pEndParag) {
      m_pEndParag->UnloadParag();
    }
    if (m_pIter) {
      m_pIter->Release();
      m_pIter = NULL;
    }
    m_pBgnParag = NULL;
    m_pEndParag = NULL;
  }
  return;
}
const CFX_RectF& CFDE_TxtEdtPage::GetContentsBox() {
  return m_rtPageContents;
}
FX_POSITION CFDE_TxtEdtPage::GetFirstPosition(FDE_HVISUALOBJ hCanvas) {
  if (m_PieceMassArr.GetSize() < 1) {
    return NULL;
  }
  return (FX_POSITION)1;
}
FDE_HVISUALOBJ CFDE_TxtEdtPage::GetNext(FDE_HVISUALOBJ hCanvas,
                                        FX_POSITION& pos,
                                        IFDE_VisualSet*& pVisualSet) {
  if (m_pTextSet == NULL) {
    pos = NULL;
    return NULL;
  }
  int32_t nPos = (int32_t)(uintptr_t)pos;
  pVisualSet = m_pTextSet;
  if (nPos + 1 > m_PieceMassArr.GetSize()) {
    pos = NULL;
  } else {
    pos = (FX_POSITION)(uintptr_t)(nPos + 1);
  }
  return (FDE_HVISUALOBJ)(m_PieceMassArr.GetPtrAt(nPos - 1));
}
FDE_HVISUALOBJ CFDE_TxtEdtPage::GetParentCanvas(FDE_HVISUALOBJ hCanvas,
                                                IFDE_VisualSet*& pVisualSet) {
  return NULL;
}
FX_WCHAR CFDE_TxtEdtPage::GetChar(void* pIdentity, int32_t index) const {
  int32_t nIndex =
      m_nPageStart + ((FDE_LPTEXTEDITPIECE)pIdentity)->nStart + index;
  if (nIndex != m_pIter->GetAt()) {
    m_pIter->SetAt(nIndex);
  }
  FX_WCHAR wChar = m_pIter->GetChar();
  m_pIter->Next();
  return wChar;
}
int32_t CFDE_TxtEdtPage::GetWidth(void* pIdentity, int32_t index) const {
  int32_t nWidth =
      m_pCharWidth[((FDE_LPTEXTEDITPIECE)pIdentity)->nStart + index];
  return nWidth;
}
void CFDE_TxtEdtPage::NormalizePt2Rect(CFX_PointF& ptF,
                                       const CFX_RectF& rtF,
                                       FX_FLOAT fTolerance) const {
  if (rtF.Contains(ptF.x, ptF.y)) {
    return;
  }
  if (ptF.x < rtF.left) {
    ptF.x = rtF.left;
  } else if (ptF.x >= rtF.right()) {
    ptF.x = rtF.right() - fTolerance;
  }
  if (ptF.y < rtF.top) {
    ptF.y = rtF.top;
  } else if (ptF.y >= rtF.bottom()) {
    ptF.y = rtF.bottom() - fTolerance;
  }
}
