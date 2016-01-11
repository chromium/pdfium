// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "xfa/src/foxitlib.h"
#include "xfa/src/fee/include/ifde_txtedtbuf.h"
#include "xfa/src/fee/include/ifde_txtedtengine.h"
#include "fde_txtedtbuf.h"
#define FDE_DEFCHUNKCOUNT 2
#define FDE_TXTEDT_FORMATBLOCK_BGN 0xFFF9
#define FDE_TXTEDT_FORMATBLOCK_END 0xFFFB
#define FDE_TXTEDT_ZEROWIDTHSPACE 0x200B
#ifdef FDE_USEFORMATBLOCK
CFDE_TxtEdtBufIter::CFDE_TxtEdtBufIter(CFDE_TxtEdtBuf* pBuf,
                                       FX_BOOL bForDisplay)
#else
CFDE_TxtEdtBufIter::CFDE_TxtEdtBufIter(CFDE_TxtEdtBuf* pBuf, FX_WCHAR wcAlias)
#endif
    : m_pBuf(pBuf),
      m_nCurChunk(0),
      m_nCurIndex(0),
      m_nIndex(0),
#ifdef FDE_USEFORMATBLOCK
      m_bForDisplay(bForDisplay),
      m_nAliasCount(0),
#endif
      m_Alias(wcAlias) {
  FXSYS_assert(m_pBuf);
}
CFDE_TxtEdtBufIter::~CFDE_TxtEdtBufIter() {}
void CFDE_TxtEdtBufIter::Release() {
  delete this;
}
FX_BOOL CFDE_TxtEdtBufIter::Next(FX_BOOL bPrev) {
  if (bPrev) {
    if (m_nIndex == 0) {
      return FALSE;
    }
    FXSYS_assert(m_nCurChunk < m_pBuf->m_Chunks.GetSize());
    CFDE_TxtEdtBuf::FDE_LPCHUNKHEADER lpChunk = NULL;
    if (m_nCurIndex > 0) {
      m_nCurIndex--;
    } else {
      while (m_nCurChunk > 0) {
        --m_nCurChunk;
        lpChunk =
            (CFDE_TxtEdtBuf::FDE_LPCHUNKHEADER)m_pBuf->m_Chunks[m_nCurChunk];
        if (lpChunk->nUsed > 0) {
          m_nCurIndex = lpChunk->nUsed - 1;
          break;
        }
      }
    }
    FXSYS_assert(m_nCurChunk >= 0);
    m_nIndex--;
    return TRUE;
  } else {
    if (m_nIndex >= (m_pBuf->m_nTotal - 1)) {
      return FALSE;
    }
    FXSYS_assert(m_nCurChunk < m_pBuf->m_Chunks.GetSize());
    CFDE_TxtEdtBuf::FDE_LPCHUNKHEADER lpChunk =
        (CFDE_TxtEdtBuf::FDE_LPCHUNKHEADER)m_pBuf->m_Chunks[m_nCurChunk];
    if (lpChunk->nUsed != (m_nCurIndex + 1)) {
      m_nCurIndex++;
    } else {
      int32_t nEnd = m_pBuf->m_Chunks.GetSize() - 1;
      while (m_nCurChunk < nEnd) {
        m_nCurChunk++;
        CFDE_TxtEdtBuf::FDE_LPCHUNKHEADER lpChunkTemp =
            (CFDE_TxtEdtBuf::FDE_LPCHUNKHEADER)m_pBuf->m_Chunks[m_nCurChunk];
        if (lpChunkTemp->nUsed > 0) {
          m_nCurIndex = 0;
          break;
        }
      }
    }
    m_nIndex++;
    return TRUE;
  }
}
void CFDE_TxtEdtBufIter::SetAt(int32_t nIndex) {
  FXSYS_assert(nIndex >= 0 && nIndex < m_pBuf->m_nTotal);
  CFDE_TxtEdtBuf::FDE_CHUNKPLACE cp;
  m_pBuf->Index2CP(nIndex, cp);
  m_nIndex = nIndex;
  m_nCurChunk = cp.nChunkIndex;
  m_nCurIndex = cp.nCharIndex;
}
int32_t CFDE_TxtEdtBufIter::GetAt() const {
  return m_nIndex;
}
FX_WCHAR CFDE_TxtEdtBufIter::GetChar() {
  FXSYS_assert(m_nIndex >= 0 && m_nIndex < m_pBuf->m_nTotal);
#ifdef FDE_USEFORMATBLOCK
  if (m_bForDisplay) {
    if (m_bInField) {
      FXSYS_assert(m_nAliasCount >= 0 && m_nAliasCount <= 2);
      if (m_nAliasCount > 0) {
        m_nAliasCount--;
        return FDE_TXTEDT_ZEROWIDTHSPACE;
      }
      FX_WCHAR wc =
          ((CFDE_TxtEdtBuf::FDE_LPCHUNKHEADER)m_pBuf->m_Chunks[m_nCurChunk])
              ->wChars[m_nCurIndex];
      if (wc == FDE_TXTEDT_FORMATBLOCK_END) {
        m_nAliasCount = 0;
        m_bInField = FALSE;
      }
      return wc;
    } else {
      FX_WCHAR wc =
          ((CFDE_TxtEdtBuf::FDE_LPCHUNKHEADER)m_pBuf->m_Chunks[m_nCurChunk])
              ->wChars[m_nCurIndex];
      if (wc == FDE_TXTEDT_FORMATBLOCK_BGN) {
        m_nAliasCount = 2;
        m_bInField = TRUE;
      }
      return wc;
    }
  }
#endif
  if (m_Alias == 0 || m_nIndex == (m_pBuf->m_nTotal - 1)) {
    return ((CFDE_TxtEdtBuf::FDE_LPCHUNKHEADER)m_pBuf->m_Chunks[m_nCurChunk])
        ->wChars[m_nCurIndex];
  }
  return m_Alias;
}
FX_BOOL CFDE_TxtEdtBufIter::IsEOF(FX_BOOL bTail) const {
  return bTail ? m_nIndex == (m_pBuf->GetTextLength() - 2) : m_nIndex == 0;
}
IFX_CharIter* CFDE_TxtEdtBufIter::Clone() {
  CFDE_TxtEdtBufIter* pIter = new CFDE_TxtEdtBufIter(m_pBuf);
  pIter->m_nCurChunk = m_nCurChunk;
  pIter->m_nCurIndex = m_nCurIndex;
  pIter->m_nIndex = m_nIndex;
  pIter->m_Alias = m_Alias;
  return pIter;
}
CFDE_TxtEdtBuf::CFDE_TxtEdtBuf(int32_t nDefChunkSize)
    : m_nChunkSize(nDefChunkSize),
      m_nTotal(0),
      m_bChanged(FALSE),
      m_pAllocator(NULL) {
  FXSYS_assert(m_nChunkSize);
  ResetChunkBuffer(FDE_DEFCHUNKCOUNT, m_nChunkSize);
}
void CFDE_TxtEdtBuf::Release() {
  delete this;
}
CFDE_TxtEdtBuf::~CFDE_TxtEdtBuf() {
  Clear(TRUE);
  m_pAllocator->Release();
  m_Chunks.RemoveAll();
}
FX_BOOL CFDE_TxtEdtBuf::SetChunkSize(int32_t nChunkSize) {
  FXSYS_assert(nChunkSize);
  ResetChunkBuffer(FDE_DEFCHUNKCOUNT, nChunkSize);
  return TRUE;
}
int32_t CFDE_TxtEdtBuf::GetChunkSize() const {
  return m_nChunkSize;
}
int32_t CFDE_TxtEdtBuf::GetTextLength() const {
  return m_nTotal;
}
void CFDE_TxtEdtBuf::SetText(const CFX_WideString& wsText) {
  FXSYS_assert(!wsText.IsEmpty());
  Clear(FALSE);
  int32_t nTextLength = wsText.GetLength();
  int32_t nNeedCount =
      ((nTextLength - 1) / m_nChunkSize + 1) - m_Chunks.GetSize();
  int32_t i = 0;
  for (i = 0; i < nNeedCount; i++) {
    FDE_LPCHUNKHEADER lpChunk = (FDE_LPCHUNKHEADER)m_pAllocator->Alloc(
        sizeof(FDE_CHUNKHEADER) + (m_nChunkSize - 1) * sizeof(FX_WCHAR));
    lpChunk->nUsed = 0;
    m_Chunks.Add(lpChunk);
  }
  int32_t nTotalCount = m_Chunks.GetSize();
  const FX_WCHAR* lpSrcBuf = wsText.c_str();
  int32_t nLeave = nTextLength;
  int32_t nCopyedLength = m_nChunkSize;
  for (i = 0; i < nTotalCount && nLeave > 0; i++) {
    if (nLeave < nCopyedLength) {
      nCopyedLength = nLeave;
    }
    FDE_LPCHUNKHEADER lpChunk = (FDE_LPCHUNKHEADER)m_Chunks[i];
    FXSYS_memcpy(lpChunk->wChars, lpSrcBuf, nCopyedLength * sizeof(FX_WCHAR));
    nLeave -= nCopyedLength;
    lpSrcBuf += nCopyedLength;
    lpChunk->nUsed = nCopyedLength;
  }
  m_nTotal = nTextLength;
  m_bChanged = TRUE;
}
void CFDE_TxtEdtBuf::GetText(CFX_WideString& wsText) const {
  GetRange(wsText, 0, m_nTotal);
}
FX_WCHAR CFDE_TxtEdtBuf::GetCharByIndex(int32_t nIndex) const {
  FXSYS_assert(nIndex >= 0 && nIndex < GetTextLength());
  FDE_LPCHUNKHEADER pChunkHeader = NULL;
  int32_t nTotal = 0;
  int32_t nCount = m_Chunks.GetSize();
  int32_t i = 0;
  for (i = 0; i < nCount; i++) {
    pChunkHeader = (FDE_LPCHUNKHEADER)m_Chunks[i];
    nTotal += pChunkHeader->nUsed;
    if (nTotal > nIndex) {
      break;
    }
  }
  FXSYS_assert(pChunkHeader);
  return pChunkHeader->wChars[pChunkHeader->nUsed - (nTotal - nIndex)];
}
void CFDE_TxtEdtBuf::GetRange(CFX_WideString& wsText,
                              int32_t nBegin,
                              int32_t nLength) const {
  FDE_CHUNKPLACE cp;
  Index2CP(nBegin, cp);
  int32_t nLeave = nLength;
  int32_t nCount = m_Chunks.GetSize();
  FX_WCHAR* lpDstBuf = wsText.GetBuffer(nLength);
  int32_t nChunkIndex = cp.nChunkIndex;
  FDE_LPCHUNKHEADER lpChunkHeader = (FDE_LPCHUNKHEADER)m_Chunks[nChunkIndex];
  int32_t nCopyLength = lpChunkHeader->nUsed - cp.nCharIndex;
  FX_WCHAR* lpSrcBuf = lpChunkHeader->wChars + cp.nCharIndex;
  while (nLeave > 0) {
    if (nLeave <= nCopyLength) {
      nCopyLength = nLeave;
    }
    FXSYS_memcpy(lpDstBuf, lpSrcBuf, nCopyLength * sizeof(FX_WCHAR));
    nChunkIndex++;
    if (nChunkIndex >= nCount) {
      break;
    }
    lpChunkHeader = (FDE_LPCHUNKHEADER)m_Chunks[nChunkIndex];
    lpSrcBuf = lpChunkHeader->wChars;
    nLeave -= nCopyLength;
    lpDstBuf += nCopyLength;
    nCopyLength = lpChunkHeader->nUsed;
  }
  wsText.ReleaseBuffer();
}
void CFDE_TxtEdtBuf::Insert(int32_t nPos,
                            const FX_WCHAR* lpText,
                            int32_t nLength) {
  FXSYS_assert(nPos >= 0 && nPos <= m_nTotal);
  FDE_CHUNKPLACE cp;
  Index2CP(nPos, cp);
  int32_t nLengthTemp = nLength;
  if (cp.nCharIndex != 0) {
    FDE_LPCHUNKHEADER lpNewChunk = (FDE_LPCHUNKHEADER)m_pAllocator->Alloc(
        sizeof(FDE_CHUNKHEADER) + (m_nChunkSize - 1) * sizeof(FX_WCHAR));
    FDE_LPCHUNKHEADER lpChunk = (FDE_LPCHUNKHEADER)m_Chunks[cp.nChunkIndex];
    int32_t nCopy = lpChunk->nUsed - cp.nCharIndex;
    FXSYS_memcpy(lpNewChunk->wChars, lpChunk->wChars + cp.nCharIndex,
                 nCopy * sizeof(FX_WCHAR));
    lpChunk->nUsed -= nCopy;
    cp.nChunkIndex++;
    m_Chunks.InsertAt(cp.nChunkIndex, lpNewChunk);
    lpNewChunk->nUsed = nCopy;
    cp.nCharIndex = 0;
  }
  if (cp.nChunkIndex != 0) {
    FDE_LPCHUNKHEADER lpChunk = (FDE_LPCHUNKHEADER)m_Chunks[cp.nChunkIndex - 1];
    if (lpChunk->nUsed != m_nChunkSize) {
      cp.nChunkIndex--;
      int32_t nFree = m_nChunkSize - lpChunk->nUsed;
      int32_t nCopy = std::min(nLengthTemp, nFree);
      FXSYS_memcpy(lpChunk->wChars + lpChunk->nUsed, lpText,
                   nCopy * sizeof(FX_WCHAR));
      lpText += nCopy;
      nLengthTemp -= nCopy;
      lpChunk->nUsed += nCopy;
      cp.nChunkIndex++;
    }
  }
  while (nLengthTemp > 0) {
    FDE_LPCHUNKHEADER lpChunk = (FDE_LPCHUNKHEADER)m_pAllocator->Alloc(
        sizeof(FDE_CHUNKHEADER) + (m_nChunkSize - 1) * sizeof(FX_WCHAR));
    FXSYS_assert(lpChunk);
    int32_t nCopy = std::min(nLengthTemp, m_nChunkSize);
    FXSYS_memcpy(lpChunk->wChars, lpText, nCopy * sizeof(FX_WCHAR));
    lpText += nCopy;
    nLengthTemp -= nCopy;
    lpChunk->nUsed = nCopy;
    m_Chunks.InsertAt(cp.nChunkIndex, lpChunk);
    cp.nChunkIndex++;
  }
  m_nTotal += nLength;
  m_bChanged = TRUE;
}
void CFDE_TxtEdtBuf::Delete(int32_t nIndex, int32_t nLength) {
  FXSYS_assert(nLength > 0 && nIndex >= 0 && nIndex + nLength <= m_nTotal);
  FDE_CHUNKPLACE cpEnd;
  Index2CP(nIndex + nLength - 1, cpEnd);
  m_nTotal -= nLength;
  FDE_LPCHUNKHEADER lpChunk = (FDE_LPCHUNKHEADER)m_Chunks[cpEnd.nChunkIndex];
  int32_t nFirstPart = cpEnd.nCharIndex + 1;
  int32_t nMovePart = lpChunk->nUsed - nFirstPart;
  if (nMovePart != 0) {
    int32_t nDelete = std::min(nFirstPart, nLength);
    FXSYS_memmove(lpChunk->wChars + nFirstPart - nDelete,
                  lpChunk->wChars + nFirstPart, nMovePart * sizeof(FX_WCHAR));
    lpChunk->nUsed -= nDelete;
    nLength -= nDelete;
    cpEnd.nChunkIndex--;
  }
  while (nLength > 0) {
    lpChunk = (FDE_LPCHUNKHEADER)m_Chunks[cpEnd.nChunkIndex];
    int32_t nDeleted = std::min(lpChunk->nUsed, nLength);
    lpChunk->nUsed -= nDeleted;
    if (lpChunk->nUsed == 0) {
      m_pAllocator->Free(lpChunk);
      m_Chunks.RemoveAt(cpEnd.nChunkIndex);
      lpChunk = NULL;
    }
    nLength -= nDeleted;
    cpEnd.nChunkIndex--;
  }
  m_bChanged = TRUE;
}
void CFDE_TxtEdtBuf::Clear(FX_BOOL bRelease) {
  int32_t i = 0;
  int32_t nCount = m_Chunks.GetSize();
  if (bRelease) {
    while (i < nCount) {
      m_pAllocator->Free(m_Chunks[i++]);
    }
    m_Chunks.RemoveAll();
  } else {
    while (i < nCount) {
      ((FDE_LPCHUNKHEADER)m_Chunks[i++])->nUsed = 0;
    }
  }
  m_nTotal = 0;
  m_bChanged = TRUE;
}
FX_BOOL CFDE_TxtEdtBuf::Optimize(IFX_Pause* pPause) {
  if (m_bChanged == FALSE) {
    return TRUE;
  }
  if (m_nTotal == 0) {
    return TRUE;
  }
  int32_t nCount = m_Chunks.GetSize();
  if (nCount == 0) {
    return TRUE;
  }
  int32_t i = 0;
  for (; i < nCount; i++) {
    FDE_LPCHUNKHEADER lpChunk = (FDE_LPCHUNKHEADER)m_Chunks[i];
    if (lpChunk->nUsed == 0) {
      m_pAllocator->Free(lpChunk);
      m_Chunks.RemoveAt(i);
      --i;
      --nCount;
    }
  }
  if (pPause != NULL && pPause->NeedToPauseNow()) {
    return FALSE;
  }
  FDE_LPCHUNKHEADER lpPreChunk = (FDE_LPCHUNKHEADER)m_Chunks[0];
  FDE_LPCHUNKHEADER lpCurChunk = NULL;
  for (i = 1; i < nCount; i++) {
    lpCurChunk = (FDE_LPCHUNKHEADER)m_Chunks[i];
    if (lpPreChunk->nUsed + lpCurChunk->nUsed <= m_nChunkSize) {
      FXSYS_memcpy(lpPreChunk->wChars + lpPreChunk->nUsed, lpCurChunk->wChars,
                   lpCurChunk->nUsed * sizeof(FX_WCHAR));
      lpPreChunk->nUsed += lpCurChunk->nUsed;
      m_pAllocator->Free(lpCurChunk);
      m_Chunks.RemoveAt(i);
      --i;
      --nCount;
    } else {
      lpPreChunk = lpCurChunk;
    }
    if (pPause != NULL && pPause->NeedToPauseNow()) {
      return FALSE;
    }
  }
  m_bChanged = FALSE;
  return TRUE;
}
void CFDE_TxtEdtBuf::ResetChunkBuffer(int32_t nDefChunkCount,
                                      int32_t nChunkSize) {
  FXSYS_assert(nChunkSize);
  FXSYS_assert(nDefChunkCount);
  if (m_pAllocator) {
    m_pAllocator->Release();
    m_pAllocator = NULL;
  }
  m_Chunks.RemoveAll();
  m_nChunkSize = nChunkSize;
  int32_t nChunkLength =
      sizeof(FDE_CHUNKHEADER) + (m_nChunkSize - 1) * sizeof(FX_WCHAR);
  m_pAllocator =
      FX_CreateAllocator(FX_ALLOCTYPE_Fixed, nDefChunkCount, nChunkLength);
  FXSYS_assert(m_pAllocator);
  FDE_LPCHUNKHEADER lpChunkHeader =
      (FDE_LPCHUNKHEADER)m_pAllocator->Alloc(nChunkLength);
  FXSYS_assert(lpChunkHeader);
  lpChunkHeader->nUsed = 0;
  m_Chunks.Add(lpChunkHeader);
  m_nTotal = 0;
}
int32_t CFDE_TxtEdtBuf::CP2Index(const FDE_CHUNKPLACE& cp) const {
  int32_t nTotal = cp.nCharIndex;
  int32_t i = 0;
  for (i = 0; i < cp.nChunkIndex; i++) {
    nTotal += ((FDE_LPCHUNKHEADER)m_Chunks[i])->nUsed;
  }
  return nTotal;
}
void CFDE_TxtEdtBuf::Index2CP(int32_t nIndex, FDE_CHUNKPLACE& cp) const {
  FXSYS_assert(nIndex <= GetTextLength());
  if (nIndex == m_nTotal) {
    cp.nChunkIndex = m_Chunks.GetSize() - 1;
    cp.nCharIndex = ((FDE_LPCHUNKHEADER)m_Chunks[cp.nChunkIndex])->nUsed;
    return;
  }
  int32_t i = 0;
  int32_t nTotal = 0;
  int32_t nCount = m_Chunks.GetSize();
  for (; i < nCount; i++) {
    nTotal += ((FDE_LPCHUNKHEADER)m_Chunks[i])->nUsed;
    if (nTotal > nIndex) {
      break;
    }
  }
  cp.nChunkIndex = i;
  cp.nCharIndex = ((FDE_LPCHUNKHEADER)m_Chunks[i])->nUsed - (nTotal - nIndex);
}
