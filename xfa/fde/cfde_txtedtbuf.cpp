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

const int32_t kDefaultChunkSize = 1024;

}  // namespace

CFDE_TxtEdtBuf::CFDE_TxtEdtBuf() : m_chunkSize(kDefaultChunkSize), m_nTotal(0) {
  m_chunks.push_back(pdfium::MakeUnique<ChunkHeader>(m_chunkSize));
}

CFDE_TxtEdtBuf::~CFDE_TxtEdtBuf() {}

void CFDE_TxtEdtBuf::SetText(const CFX_WideString& wsText) {
  ASSERT(!wsText.IsEmpty());

  Clear();

  int32_t nTextLength = wsText.GetLength();
  int32_t nNeedCount = ((nTextLength - 1) / m_chunkSize + 1) -
                       pdfium::CollectionSize<int32_t>(m_chunks);
  for (int32_t i = 0; i < nNeedCount; ++i)
    m_chunks.push_back(pdfium::MakeUnique<ChunkHeader>(m_chunkSize));

  const wchar_t* lpSrcBuf = wsText.c_str();
  int32_t nLeave = nTextLength;
  int32_t nCopyedLength = m_chunkSize;
  for (size_t i = 0; i < m_chunks.size() && nLeave > 0; ++i) {
    if (nLeave < nCopyedLength)
      nCopyedLength = nLeave;

    ChunkHeader* chunk = m_chunks[i].get();
    memcpy(chunk->wChars.data(), lpSrcBuf, nCopyedLength * sizeof(wchar_t));
    nLeave -= nCopyedLength;
    lpSrcBuf += nCopyedLength;
    chunk->nUsed = nCopyedLength;
  }
  m_nTotal = nTextLength;
}

wchar_t CFDE_TxtEdtBuf::GetCharByIndex(int32_t nIndex) const {
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

  return pChunkHeader->wChars[pChunkHeader->nUsed - (nTotal - nIndex)];
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
  CFX_WideString wsText;

  int32_t nCopyLength = m_chunks[chunkIndex]->nUsed - charIndex;
  wchar_t* lpSrcBuf = m_chunks[chunkIndex]->wChars.data() + charIndex;
  while (nLeave > 0) {
    if (nLeave <= nCopyLength)
      nCopyLength = nLeave;

    wsText += CFX_WideStringC(lpSrcBuf, nCopyLength);

    ++chunkIndex;
    if (chunkIndex >= pdfium::CollectionSize<int32_t>(m_chunks))
      break;

    lpSrcBuf = m_chunks[chunkIndex]->wChars.data();
    nLeave -= nCopyLength;
    nCopyLength = m_chunks[chunkIndex]->nUsed;
  }
  return wsText;
}

void CFDE_TxtEdtBuf::Insert(int32_t nPos, const CFX_WideString& wsText) {
  ASSERT(nPos >= 0 && nPos <= m_nTotal);

  int32_t nLength = wsText.GetLength();
  ASSERT(nLength > 0);

  int32_t chunkIndex = 0;
  int32_t charIndex = 0;
  std::tie(chunkIndex, charIndex) = Index2CP(nPos);

  if (charIndex != 0) {
    auto newChunk = pdfium::MakeUnique<ChunkHeader>(m_chunkSize);

    ChunkHeader* chunk = m_chunks[chunkIndex].get();
    int32_t nCopy = chunk->nUsed - charIndex;

    memcpy(newChunk->wChars.data(), chunk->wChars.data() + charIndex,
           nCopy * sizeof(wchar_t));
    chunk->nUsed -= nCopy;
    ++chunkIndex;

    newChunk->nUsed = nCopy;
    m_chunks.insert(m_chunks.begin() + chunkIndex, std::move(newChunk));
    charIndex = 0;
  }

  const wchar_t* lpText = wsText.c_str();
  if (chunkIndex != 0) {
    ChunkHeader* chunk = m_chunks[chunkIndex - 1].get();
    if (chunk->nUsed != m_chunkSize) {
      chunkIndex--;
      int32_t nCopy = std::min(nLength, m_chunkSize - chunk->nUsed);
      memcpy(chunk->wChars.data() + chunk->nUsed, lpText,
             nCopy * sizeof(wchar_t));
      lpText += nCopy;
      nLength -= nCopy;
      chunk->nUsed += nCopy;
      ++chunkIndex;
    }
  }

  while (nLength > 0) {
    auto chunk = pdfium::MakeUnique<ChunkHeader>(m_chunkSize);

    int32_t nCopy = std::min(nLength, m_chunkSize);
    memcpy(chunk->wChars.data(), lpText, nCopy * sizeof(wchar_t));
    lpText += nCopy;
    nLength -= nCopy;
    chunk->nUsed = nCopy;
    m_chunks.insert(m_chunks.begin() + chunkIndex, std::move(chunk));
    ++chunkIndex;
  }

  m_nTotal += wsText.GetLength();
}

void CFDE_TxtEdtBuf::Delete(int32_t nIndex, int32_t nLength) {
  ASSERT(nLength > 0 && nIndex >= 0 && nIndex + nLength <= m_nTotal);

  int32_t endChunkIndex = 0;
  int32_t endCharIndex = 0;
  std::tie(endChunkIndex, endCharIndex) = Index2CP(nIndex + nLength - 1);
  m_nTotal -= nLength;

  int32_t nFirstPart = endCharIndex + 1;
  int32_t nMovePart = m_chunks[endChunkIndex]->nUsed - nFirstPart;
  if (nMovePart != 0) {
    int32_t nDelete = std::min(nFirstPart, nLength);
    memmove(m_chunks[endChunkIndex]->wChars.data() + nFirstPart - nDelete,
            m_chunks[endChunkIndex]->wChars.data() + nFirstPart,
            nMovePart * sizeof(wchar_t));
    m_chunks[endChunkIndex]->nUsed -= nDelete;
    nLength -= nDelete;
    --endChunkIndex;
  }

  while (nLength > 0) {
    int32_t nDeleted = std::min(m_chunks[endChunkIndex]->nUsed, nLength);
    m_chunks[endChunkIndex]->nUsed -= nDeleted;
    if (m_chunks[endChunkIndex]->nUsed == 0)
      m_chunks.erase(m_chunks.begin() + endChunkIndex);

    nLength -= nDeleted;
    --endChunkIndex;
  }
}

void CFDE_TxtEdtBuf::Clear() {
  for (auto& chunk : m_chunks)
    chunk->nUsed = 0;
  m_nTotal = 0;
}

void CFDE_TxtEdtBuf::SetChunkSizeForTesting(size_t size) {
  ASSERT(size > 0);

  m_chunkSize = size;
  m_chunks.clear();
  m_chunks.push_back(pdfium::MakeUnique<ChunkHeader>(m_chunkSize));
}

std::pair<int32_t, int32_t> CFDE_TxtEdtBuf::Index2CP(int32_t nIndex) const {
  ASSERT(nIndex <= GetTextLength());

  if (nIndex == m_nTotal)
    return {m_chunks.size() - 1, m_chunks.back()->nUsed};

  int32_t chunkIndex = 0;
  int32_t nTotal = 0;
  for (auto& chunk : m_chunks) {
    nTotal += chunk->nUsed;
    if (nTotal > nIndex)
      break;

    ++chunkIndex;
  }

  return {chunkIndex, m_chunks[chunkIndex]->nUsed - (nTotal - nIndex)};
}

CFDE_TxtEdtBuf::ChunkHeader::ChunkHeader(int32_t chunkSize) : nUsed(0) {
  wChars.resize(chunkSize);
}

CFDE_TxtEdtBuf::ChunkHeader::~ChunkHeader() {}

CFDE_TxtEdtBuf::Iterator::Iterator(CFDE_TxtEdtBuf* pBuf, wchar_t wcAlias)
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

    if (m_nCurIndex > 0) {
      --m_nCurIndex;
    } else {
      while (m_nCurChunk > 0) {
        --m_nCurChunk;
        ChunkHeader* chunk = m_pBuf->m_chunks[m_nCurChunk].get();
        if (chunk->nUsed > 0) {
          m_nCurIndex = chunk->nUsed - 1;
          break;
        }
      }
    }
    ASSERT(m_nCurChunk >= 0);

    --m_nIndex;
    return true;
  }

  if (m_nIndex >= (m_pBuf->m_nTotal - 1))
    return false;

  ASSERT(m_nCurChunk < pdfium::CollectionSize<int32_t>(m_pBuf->m_chunks));

  if (m_pBuf->m_chunks[m_nCurChunk]->nUsed != (m_nCurIndex + 1)) {
    ++m_nCurIndex;
  } else {
    while (m_nCurChunk <
           pdfium::CollectionSize<int32_t>(m_pBuf->m_chunks) - 1) {
      ++m_nCurChunk;
      if (m_pBuf->m_chunks[m_nCurChunk]->nUsed > 0) {
        m_nCurIndex = 0;
        break;
      }
    }
  }
  ++m_nIndex;
  return true;
}

void CFDE_TxtEdtBuf::Iterator::SetAt(int32_t nIndex) {
  ASSERT(nIndex >= 0 && nIndex < m_pBuf->m_nTotal);

  std::tie(m_nCurChunk, m_nCurIndex) = m_pBuf->Index2CP(nIndex);
  m_nIndex = nIndex;
}

int32_t CFDE_TxtEdtBuf::Iterator::GetAt() const {
  return m_nIndex;
}

wchar_t CFDE_TxtEdtBuf::Iterator::GetChar() const {
  ASSERT(m_nIndex >= 0 && m_nIndex < m_pBuf->m_nTotal);

  if (m_Alias != 0 && m_nIndex != (m_pBuf->m_nTotal - 1))
    return m_Alias;
  return m_pBuf->m_chunks[m_nCurChunk]->wChars[m_nCurIndex];
}

bool CFDE_TxtEdtBuf::Iterator::IsEOF(bool bTail) const {
  return bTail ? m_nIndex == (m_pBuf->GetTextLength() - 2) : m_nIndex == 0;
}

std::unique_ptr<IFX_CharIter> CFDE_TxtEdtBuf::Iterator::Clone() const {
  auto pIter = pdfium::MakeUnique<CFDE_TxtEdtBuf::Iterator>(m_pBuf, 0);
  pIter->m_nCurChunk = m_nCurChunk;
  pIter->m_nCurIndex = m_nCurIndex;
  pIter->m_nIndex = m_nIndex;
  pIter->m_Alias = m_Alias;
  return pIter;
}
