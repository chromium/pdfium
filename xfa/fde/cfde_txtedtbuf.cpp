// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_txtedtbuf.h"

#include <algorithm>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

const int kDefaultChunkSize = 1024;

}  // namespace

CFDE_TxtEdtBuf::CFDE_TxtEdtBuf() : m_chunkSize(kDefaultChunkSize), m_nTotal(0) {
  m_chunks.push_back(NewChunk());
}

CFDE_TxtEdtBuf::~CFDE_TxtEdtBuf() {}

int32_t CFDE_TxtEdtBuf::GetChunkSize() const {
  return m_chunkSize;
}

int32_t CFDE_TxtEdtBuf::GetTextLength() const {
  return m_nTotal;
}

void CFDE_TxtEdtBuf::SetText(const CFX_WideString& wsText) {
  ASSERT(!wsText.IsEmpty());

  Clear(false);
  int32_t nTextLength = wsText.GetLength();
  int32_t nNeedCount =
      ((nTextLength - 1) / GetChunkSize() + 1) - m_chunks.size();
  int32_t i = 0;
  for (i = 0; i < nNeedCount; i++)
    m_chunks.push_back(NewChunk());

  int32_t nTotalCount = m_chunks.size();
  const FX_WCHAR* lpSrcBuf = wsText.c_str();
  int32_t nLeave = nTextLength;
  int32_t nCopyedLength = GetChunkSize();
  for (i = 0; i < nTotalCount && nLeave > 0; i++) {
    if (nLeave < nCopyedLength) {
      nCopyedLength = nLeave;
    }

    ChunkHeader* chunk = m_chunks[i].get();
    FXSYS_memcpy(chunk->wChars.get(), lpSrcBuf,
                 nCopyedLength * sizeof(FX_WCHAR));
    nLeave -= nCopyedLength;
    lpSrcBuf += nCopyedLength;
    chunk->nUsed = nCopyedLength;
  }
  m_nTotal = nTextLength;
}

CFX_WideString CFDE_TxtEdtBuf::GetText() const {
  return GetRange(0, m_nTotal);
}

FX_WCHAR CFDE_TxtEdtBuf::GetCharByIndex(int32_t nIndex) const {
  ASSERT(nIndex >= 0 && nIndex < GetTextLength());

  ChunkHeader* pChunkHeader = nullptr;
  int32_t nTotal = 0;
  for (const auto& chunk : m_chunks) {
    pChunkHeader = chunk.get();
    nTotal += pChunkHeader->nUsed;
    if (nTotal > nIndex)
      break;
  }
  ASSERT(pChunkHeader);

  FX_WCHAR* buf = pChunkHeader->wChars.get();
  return buf[pChunkHeader->nUsed - (nTotal - nIndex)];
}

CFX_WideString CFDE_TxtEdtBuf::GetRange(int32_t nBegin, int32_t nLength) const {
  if (nLength == 0 || GetTextLength() == 0)
    return CFX_WideString();

  ASSERT(nBegin >= 0 && nLength > 0 && nBegin < GetTextLength() &&
         nBegin + nLength <= GetTextLength());

  int32_t chunkIndex = 0;
  int32_t charIndex = 0;
  std::tie(chunkIndex, charIndex) = Index2CP(nBegin);

  int32_t nLeave = nLength;
  int32_t nCount = m_chunks.size();

  CFX_WideString wsText;
  FX_WCHAR* lpDstBuf = wsText.GetBuffer(nLength);
  int32_t nChunkIndex = chunkIndex;

  ChunkHeader* chunkHeader = m_chunks[nChunkIndex].get();
  int32_t nCopyLength = chunkHeader->nUsed - charIndex;
  FX_WCHAR* lpSrcBuf = chunkHeader->wChars.get() + charIndex;
  while (nLeave > 0) {
    if (nLeave <= nCopyLength) {
      nCopyLength = nLeave;
    }
    FXSYS_memcpy(lpDstBuf, lpSrcBuf, nCopyLength * sizeof(FX_WCHAR));
    nChunkIndex++;
    if (nChunkIndex >= nCount) {
      break;
    }
    chunkHeader = m_chunks[nChunkIndex].get();
    lpSrcBuf = chunkHeader->wChars.get();
    nLeave -= nCopyLength;
    lpDstBuf += nCopyLength;
    nCopyLength = chunkHeader->nUsed;
  }
  wsText.ReleaseBuffer();

  return wsText;
}

void CFDE_TxtEdtBuf::Insert(int32_t nPos,
                            const FX_WCHAR* lpText,
                            int32_t nLength) {
  ASSERT(nPos >= 0 && nPos <= m_nTotal);
  ASSERT(nLength > 0);

  int32_t chunkIndex = 0;
  int32_t charIndex = 0;
  std::tie(chunkIndex, charIndex) = Index2CP(nPos);

  int32_t nLengthTemp = nLength;
  if (charIndex != 0) {
    auto newChunk = NewChunk();

    ChunkHeader* chunk = m_chunks[chunkIndex].get();
    int32_t nCopy = chunk->nUsed - charIndex;

    FXSYS_memcpy(newChunk->wChars.get(), chunk->wChars.get() + charIndex,
                 nCopy * sizeof(FX_WCHAR));
    chunk->nUsed -= nCopy;
    chunkIndex++;

    newChunk->nUsed = nCopy;
    m_chunks.insert(m_chunks.begin() + chunkIndex, std::move(newChunk));
    charIndex = 0;
  }

  if (chunkIndex != 0) {
    ChunkHeader* chunk = m_chunks[chunkIndex - 1].get();
    if (chunk->nUsed != GetChunkSize()) {
      chunkIndex--;
      int32_t nFree = GetChunkSize() - chunk->nUsed;
      int32_t nCopy = std::min(nLengthTemp, nFree);
      FXSYS_memcpy(chunk->wChars.get() + chunk->nUsed, lpText,
                   nCopy * sizeof(FX_WCHAR));
      lpText += nCopy;
      nLengthTemp -= nCopy;
      chunk->nUsed += nCopy;
      chunkIndex++;
    }
  }

  while (nLengthTemp > 0) {
    auto chunk = NewChunk();

    int32_t nCopy = std::min(nLengthTemp, GetChunkSize());
    FXSYS_memcpy(chunk->wChars.get(), lpText, nCopy * sizeof(FX_WCHAR));
    lpText += nCopy;
    nLengthTemp -= nCopy;
    chunk->nUsed = nCopy;
    m_chunks.insert(m_chunks.begin() + chunkIndex, std::move(chunk));
    chunkIndex++;
  }
  m_nTotal += nLength;
}

void CFDE_TxtEdtBuf::Delete(int32_t nIndex, int32_t nLength) {
  ASSERT(nLength > 0 && nIndex >= 0 && nIndex + nLength <= m_nTotal);

  int32_t endChunkIndex = 0;
  int32_t endCharIndex = 0;
  std::tie(endChunkIndex, endCharIndex) = Index2CP(nIndex + nLength - 1);
  m_nTotal -= nLength;

  ChunkHeader* chunk = m_chunks[endChunkIndex].get();
  int32_t nFirstPart = endCharIndex + 1;
  int32_t nMovePart = chunk->nUsed - nFirstPart;
  if (nMovePart != 0) {
    int32_t nDelete = std::min(nFirstPart, nLength);
    FXSYS_memmove(chunk->wChars.get() + nFirstPart - nDelete,
                  chunk->wChars.get() + nFirstPart,
                  nMovePart * sizeof(FX_WCHAR));
    chunk->nUsed -= nDelete;
    nLength -= nDelete;
    endChunkIndex--;
  }

  while (nLength > 0) {
    ChunkHeader* curChunk = m_chunks[endChunkIndex].get();
    int32_t nDeleted = std::min(curChunk->nUsed, nLength);
    curChunk->nUsed -= nDeleted;
    if (curChunk->nUsed == 0)
      m_chunks.erase(m_chunks.begin() + endChunkIndex);

    nLength -= nDeleted;
    endChunkIndex--;
  }
}

void CFDE_TxtEdtBuf::Clear(bool bRelease) {
  if (bRelease) {
    m_chunks.clear();
  } else {
    size_t i = 0;
    while (i < m_chunks.size())
      m_chunks[i++]->nUsed = 0;
  }
  m_nTotal = 0;
}

void CFDE_TxtEdtBuf::SetChunkSizeForTesting(size_t size) {
  ASSERT(size > 0);

  m_chunkSize = size;
  m_chunks.clear();
  m_chunks.push_back(NewChunk());
}

std::tuple<int32_t, int32_t> CFDE_TxtEdtBuf::Index2CP(int32_t nIndex) const {
  ASSERT(nIndex <= GetTextLength());

  if (nIndex == m_nTotal) {
    return std::tuple<int32_t, int32_t>(m_chunks.size() - 1,
                                        m_chunks.back()->nUsed);
  }

  int32_t chunkIndex = 0;
  int32_t nTotal = 0;
  for (auto& chunk : m_chunks) {
    nTotal += chunk->nUsed;
    if (nTotal > nIndex)
      break;
    chunkIndex++;
  }

  int32_t charIndex = m_chunks[chunkIndex]->nUsed - (nTotal - nIndex);
  return std::tuple<int32_t, int32_t>(chunkIndex, charIndex);
}

std::unique_ptr<CFDE_TxtEdtBuf::ChunkHeader> CFDE_TxtEdtBuf::NewChunk() {
  auto chunk = pdfium::MakeUnique<ChunkHeader>();
  chunk->wChars.reset(FX_Alloc(FX_WCHAR, GetChunkSize()));
  chunk->nUsed = 0;
  return chunk;
}

CFDE_TxtEdtBuf::ChunkHeader::ChunkHeader() {}

CFDE_TxtEdtBuf::ChunkHeader::~ChunkHeader() {}

CFDE_TxtEdtBuf::Iterator::Iterator(CFDE_TxtEdtBuf* pBuf, FX_WCHAR wcAlias)
    : m_pBuf(pBuf),
      m_nCurChunk(0),
      m_nCurIndex(0),
      m_nIndex(0),
      m_Alias(wcAlias) {
  ASSERT(m_pBuf);
}

CFDE_TxtEdtBuf::Iterator::~Iterator() {}

bool CFDE_TxtEdtBuf::Iterator::Next(bool bPrev) {
  if (bPrev) {
    if (m_nIndex == 0)
      return false;

    ASSERT(m_nCurChunk < pdfium::CollectionSize<int32_t>(m_pBuf->m_chunks));

    ChunkHeader* chunk = nullptr;
    if (m_nCurIndex > 0) {
      m_nCurIndex--;
    } else {
      while (m_nCurChunk > 0) {
        --m_nCurChunk;
        chunk = m_pBuf->m_chunks[m_nCurChunk].get();
        if (chunk->nUsed > 0) {
          m_nCurIndex = chunk->nUsed - 1;
          break;
        }
      }
    }
    ASSERT(m_nCurChunk >= 0);
    m_nIndex--;
    return true;
  } else {
    if (m_nIndex >= (m_pBuf->m_nTotal - 1))
      return false;

    ASSERT(m_nCurChunk < pdfium::CollectionSize<int32_t>(m_pBuf->m_chunks));
    if (m_pBuf->m_chunks[m_nCurChunk]->nUsed != (m_nCurIndex + 1)) {
      m_nCurIndex++;
    } else {
      int32_t nEnd = m_pBuf->m_chunks.size() - 1;
      while (m_nCurChunk < nEnd) {
        m_nCurChunk++;
        ChunkHeader* chunkTemp = m_pBuf->m_chunks[m_nCurChunk].get();
        if (chunkTemp->nUsed > 0) {
          m_nCurIndex = 0;
          break;
        }
      }
    }
    m_nIndex++;
    return true;
  }
}

void CFDE_TxtEdtBuf::Iterator::SetAt(int32_t nIndex) {
  ASSERT(nIndex >= 0 && nIndex < m_pBuf->m_nTotal);

  std::tie(m_nCurChunk, m_nCurIndex) = m_pBuf->Index2CP(nIndex);
  m_nIndex = nIndex;
}

int32_t CFDE_TxtEdtBuf::Iterator::GetAt() const {
  return m_nIndex;
}

FX_WCHAR CFDE_TxtEdtBuf::Iterator::GetChar() {
  ASSERT(m_nIndex >= 0 && m_nIndex < m_pBuf->m_nTotal);
  if (m_Alias == 0 || m_nIndex == (m_pBuf->m_nTotal - 1)) {
    FX_WCHAR* buf = m_pBuf->m_chunks[m_nCurChunk]->wChars.get();
    return buf[m_nCurIndex];
  }
  return m_Alias;
}

bool CFDE_TxtEdtBuf::Iterator::IsEOF(bool bTail) const {
  return bTail ? m_nIndex == (m_pBuf->GetTextLength() - 2) : m_nIndex == 0;
}

IFX_CharIter* CFDE_TxtEdtBuf::Iterator::Clone() {
  CFDE_TxtEdtBuf::Iterator* pIter = new CFDE_TxtEdtBuf::Iterator(m_pBuf);
  pIter->m_nCurChunk = m_nCurChunk;
  pIter->m_nCurIndex = m_nCurIndex;
  pIter->m_nIndex = m_nIndex;
  pIter->m_Alias = m_Alias;
  return pIter;
}
