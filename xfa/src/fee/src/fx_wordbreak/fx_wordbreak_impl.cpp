// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "fx_wordbreak_impl.h"
#define FX_IsOdd(a) ((a)&1)
FX_WordBreakProp FX_GetWordBreakProperty(FX_WCHAR wcCodePoint) {
  FX_DWORD dwProperty =
      (FX_DWORD)gs_FX_WordBreak_CodePointProperties[wcCodePoint >> 1];
  return (FX_WordBreakProp)(FX_IsOdd(wcCodePoint) ? (dwProperty & 0x0F)
                                                  : (dwProperty >> 4));
}
CFX_CharIter::CFX_CharIter(const CFX_WideString& wsText)
    : m_wsText(wsText), m_nIndex(0) {
  FXSYS_assert(!wsText.IsEmpty());
}
CFX_CharIter::~CFX_CharIter() {}
void CFX_CharIter::Release() {
  delete this;
}
FX_BOOL CFX_CharIter::Next(FX_BOOL bPrev) {
  if (bPrev) {
    if (m_nIndex <= 0) {
      return FALSE;
    }
    m_nIndex--;
  } else {
    if (m_nIndex + 1 >= m_wsText.GetLength()) {
      return FALSE;
    }
    m_nIndex++;
  }
  return TRUE;
}
FX_WCHAR CFX_CharIter::GetChar() {
  return m_wsText.GetAt(m_nIndex);
}
void CFX_CharIter::SetAt(int32_t nIndex) {
  if (nIndex < 0 || nIndex >= m_wsText.GetLength()) {
    return;
  }
  m_nIndex = nIndex;
}
int32_t CFX_CharIter::GetAt() const {
  return m_nIndex;
}
FX_BOOL CFX_CharIter::IsEOF(FX_BOOL bTail) const {
  return bTail ? (m_nIndex + 1 == m_wsText.GetLength()) : (m_nIndex == 0);
}
IFX_CharIter* CFX_CharIter::Clone() {
  CFX_CharIter* pIter = new CFX_CharIter(m_wsText);
  pIter->m_nIndex = m_nIndex;
  return pIter;
}
CFX_WordBreak::CFX_WordBreak() : m_pPreIter(NULL), m_pCurIter(NULL) {}
CFX_WordBreak::~CFX_WordBreak() {
  if (m_pPreIter) {
    m_pPreIter->Release();
    m_pPreIter = NULL;
  }
  if (m_pCurIter) {
    m_pCurIter->Release();
    m_pCurIter = NULL;
  }
}
void CFX_WordBreak::Release() {
  delete this;
}
void CFX_WordBreak::Attach(IFX_CharIter* pIter) {
  FXSYS_assert(pIter);
  m_pCurIter = pIter;
}
void CFX_WordBreak::Attach(const CFX_WideString& wsText) {
  m_pCurIter = new CFX_CharIter(wsText);
}
FX_BOOL CFX_WordBreak::Next(FX_BOOL bPrev) {
  IFX_CharIter* pIter = bPrev ? m_pPreIter->Clone() : m_pCurIter->Clone();
  if (pIter->IsEOF(!bPrev)) {
    return FALSE;
  }
  pIter->Next(bPrev);
  if (!FindNextBreakPos(pIter, bPrev, TRUE)) {
    pIter->Release();
    return FALSE;
  }
  if (bPrev) {
    m_pCurIter->Release();
    m_pCurIter = m_pPreIter;
    m_pCurIter->Next(TRUE);
    m_pPreIter = pIter;
  } else {
    m_pPreIter->Release();
    m_pPreIter = m_pCurIter;
    m_pPreIter->Next();
    m_pCurIter = pIter;
  }
  return TRUE;
}
void CFX_WordBreak::SetAt(int32_t nIndex) {
  if (m_pPreIter) {
    m_pPreIter->Release();
    m_pPreIter = NULL;
  }
  m_pCurIter->SetAt(nIndex);
  FindNextBreakPos(m_pCurIter, TRUE, FALSE);
  m_pPreIter = m_pCurIter;
  m_pCurIter = m_pPreIter->Clone();
  FindNextBreakPos(m_pCurIter, FALSE, FALSE);
}
int32_t CFX_WordBreak::GetWordPos() const {
  return m_pPreIter->GetAt();
}
int32_t CFX_WordBreak::GetWordLength() const {
  return m_pCurIter->GetAt() - m_pPreIter->GetAt() + 1;
}
void CFX_WordBreak::GetWord(CFX_WideString& wsWord) const {
  int32_t nWordLength = GetWordLength();
  if (nWordLength <= 0) {
    return;
  }
  FX_WCHAR* lpBuf = wsWord.GetBuffer(nWordLength);
  IFX_CharIter* pTempIter = m_pPreIter->Clone();
  int32_t i = 0;
  while (pTempIter->GetAt() <= m_pCurIter->GetAt()) {
    lpBuf[i++] = pTempIter->GetChar();
    FX_BOOL bEnd = pTempIter->Next();
    if (!bEnd) {
      break;
    }
  }
  pTempIter->Release();
  wsWord.ReleaseBuffer(nWordLength);
}
FX_BOOL CFX_WordBreak::IsEOF(FX_BOOL bTail) const {
  return m_pCurIter->IsEOF(bTail);
}
FX_BOOL CFX_WordBreak::FindNextBreakPos(IFX_CharIter* pIter,
                                        FX_BOOL bPrev,
                                        FX_BOOL bFromNext) {
  FX_WordBreakProp ePreType = FX_WordBreakProp_None;
  FX_WordBreakProp eCurType = FX_WordBreakProp_None;
  FX_WordBreakProp eNextType = FX_WordBreakProp_None;
  if (pIter->IsEOF(!bPrev)) {
    return TRUE;
  }
  if (!(bFromNext || pIter->IsEOF(bPrev))) {
    pIter->Next(!bPrev);
    FX_WCHAR wcTemp = pIter->GetChar();
    ePreType = FX_GetWordBreakProperty(wcTemp);
    pIter->Next(bPrev);
  }
  FX_WCHAR wcTemp = pIter->GetChar();
  eCurType = FX_GetWordBreakProperty(wcTemp);
  FX_BOOL bFirst = TRUE;
  do {
    pIter->Next(bPrev);
    FX_WCHAR wcTemp = pIter->GetChar();
    eNextType = FX_GetWordBreakProperty(wcTemp);
    FX_WORD wBreak =
        gs_FX_WordBreak_Table[eCurType] & ((FX_WORD)(1 << eNextType));
    if (wBreak) {
      if (pIter->IsEOF(!bPrev)) {
        pIter->Next(!bPrev);
        return TRUE;
      }
      if (bFirst) {
        int32_t nFlags = 0;
        if (eCurType == FX_WordBreakProp_MidLetter) {
          if (eNextType == FX_WordBreakProp_ALetter) {
            nFlags = 1;
          }
        } else if (eCurType == FX_WordBreakProp_MidNum) {
          if (eNextType == FX_WordBreakProp_Numberic) {
            nFlags = 2;
          }
        } else if (eCurType == FX_WordBreakProp_MidNumLet) {
          if (eNextType == FX_WordBreakProp_ALetter) {
            nFlags = 1;
          } else if (eNextType == FX_WordBreakProp_Numberic) {
            nFlags = 2;
          }
        }
        if (nFlags > 0) {
          FXSYS_assert(nFlags <= 2);
          if (!((nFlags == 1 && ePreType == FX_WordBreakProp_ALetter) ||
                (nFlags == 2 && ePreType == FX_WordBreakProp_Numberic))) {
            pIter->Next(!bPrev);
            return TRUE;
          }
          pIter->Next(bPrev);
          wBreak = FALSE;
        }
        bFirst = FALSE;
      }
      if (wBreak) {
        int32_t nFlags = 0;
        if (eNextType == FX_WordBreakProp_MidLetter) {
          if (eCurType == FX_WordBreakProp_ALetter) {
            nFlags = 1;
          }
        } else if (eNextType == FX_WordBreakProp_MidNum) {
          if (eCurType == FX_WordBreakProp_Numberic) {
            nFlags = 2;
          }
        } else if (eNextType == FX_WordBreakProp_MidNumLet) {
          if (eCurType == FX_WordBreakProp_ALetter) {
            nFlags = 1;
          } else if (eCurType == FX_WordBreakProp_Numberic) {
            nFlags = 2;
          }
        }
        if (nFlags <= 0) {
          pIter->Next(!bPrev);
          return TRUE;
        }
        FXSYS_assert(nFlags <= 2);
        pIter->Next(bPrev);
        wcTemp = pIter->GetChar();
        eNextType = (FX_WordBreakProp)FX_GetWordBreakProperty(wcTemp);
        if (!((nFlags == 1 && eNextType == FX_WordBreakProp_ALetter) ||
              (nFlags == 2 && eNextType == FX_WordBreakProp_Numberic))) {
          pIter->Next(!bPrev);
          pIter->Next(!bPrev);
          return TRUE;
        }
      }
    }
    ePreType = eCurType;
    eCurType = eNextType;
    bFirst = FALSE;
  } while (!pIter->IsEOF(!bPrev));
  return TRUE;
}
IFX_WordBreak* FX_WordBreak_Create() {
  return new CFX_WordBreak;
}
