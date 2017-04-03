// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_blockbuffer.h"

#include <algorithm>

#include "third_party/base/stl_util.h"

CFX_BlockBuffer::CFX_BlockBuffer(int32_t iAllocStep)
    : m_iDataLength(0),
      m_iBufferSize(0),
      m_iAllocStep(iAllocStep),
      m_iStartPosition(0) {}

CFX_BlockBuffer::~CFX_BlockBuffer() {
  ClearBuffer();
}

wchar_t* CFX_BlockBuffer::GetAvailableBlock(int32_t& iIndexInBlock) {
  iIndexInBlock = 0;
  if (m_BlockArray.empty())
    return nullptr;

  int32_t iRealIndex = m_iStartPosition + m_iDataLength;
  if (iRealIndex == m_iBufferSize) {
    m_BlockArray.emplace_back(FX_Alloc(wchar_t, m_iAllocStep));
    m_iBufferSize += m_iAllocStep;
    return m_BlockArray.back().get();
  }
  iIndexInBlock = iRealIndex % m_iAllocStep;
  return m_BlockArray[iRealIndex / m_iAllocStep].get();
}

bool CFX_BlockBuffer::InitBuffer(int32_t iBufferSize) {
  ClearBuffer();
  int32_t iNumOfBlock = (iBufferSize - 1) / m_iAllocStep + 1;
  for (int32_t i = 0; i < iNumOfBlock; i++)
    m_BlockArray.emplace_back(FX_Alloc(wchar_t, m_iAllocStep));

  m_iBufferSize = iNumOfBlock * m_iAllocStep;
  return true;
}

void CFX_BlockBuffer::SetTextChar(int32_t iIndex, wchar_t ch) {
  if (iIndex < 0) {
    return;
  }
  int32_t iRealIndex = m_iStartPosition + iIndex;
  int32_t iBlockIndex = iRealIndex / m_iAllocStep;
  int32_t iInnerIndex = iRealIndex % m_iAllocStep;
  int32_t iBlockSize = pdfium::CollectionSize<int32_t>(m_BlockArray);
  if (iBlockIndex >= iBlockSize) {
    int32_t iNewBlocks = iBlockIndex - iBlockSize + 1;
    do {
      m_BlockArray.emplace_back(FX_Alloc(wchar_t, m_iAllocStep));
      m_iBufferSize += m_iAllocStep;
    } while (--iNewBlocks);
  }
  wchar_t* pTextData = m_BlockArray[iBlockIndex].get();
  pTextData[iInnerIndex] = ch;
  m_iDataLength = std::max(m_iDataLength, iIndex + 1);
}

int32_t CFX_BlockBuffer::DeleteTextChars(int32_t iCount, bool bDirection) {
  if (iCount <= 0)
    return m_iDataLength;

  if (iCount >= m_iDataLength) {
    Reset(false);
    return 0;
  }
  if (bDirection) {
    m_iStartPosition += iCount;
    m_iDataLength -= iCount;
  } else {
    m_iDataLength -= iCount;
  }
  return m_iDataLength;
}

void CFX_BlockBuffer::GetTextData(CFX_WideString& wsTextData,
                                  int32_t iStart,
                                  int32_t iLength) const {
  wsTextData.clear();
  int32_t iMaybeDataLength = m_iBufferSize - 1 - m_iStartPosition;
  if (iStart < 0 || iStart > iMaybeDataLength) {
    return;
  }
  if (iLength == -1 || iLength > iMaybeDataLength) {
    iLength = iMaybeDataLength;
  }
  if (iLength <= 0) {
    return;
  }
  wchar_t* pBuf = wsTextData.GetBuffer(iLength);
  if (!pBuf) {
    return;
  }
  int32_t iStartBlockIndex = 0;
  int32_t iStartInnerIndex = 0;
  TextDataIndex2BufIndex(iStart, iStartBlockIndex, iStartInnerIndex);
  int32_t iEndBlockIndex = 0;
  int32_t iEndInnerIndex = 0;
  TextDataIndex2BufIndex(iStart + iLength, iEndBlockIndex, iEndInnerIndex);
  int32_t iPointer = 0;
  for (int32_t i = iStartBlockIndex; i <= iEndBlockIndex; i++) {
    int32_t iBufferPointer = 0;
    int32_t iCopyLength = m_iAllocStep;
    if (i == iStartBlockIndex) {
      iCopyLength -= iStartInnerIndex;
      iBufferPointer = iStartInnerIndex;
    }
    if (i == iEndBlockIndex) {
      iCopyLength -= ((m_iAllocStep - 1) - iEndInnerIndex);
    }
    wchar_t* pBlockBuf = m_BlockArray[i].get();
    memcpy(pBuf + iPointer, pBlockBuf + iBufferPointer,
           iCopyLength * sizeof(wchar_t));
    iPointer += iCopyLength;
  }
  wsTextData.ReleaseBuffer(iLength);
}

void CFX_BlockBuffer::TextDataIndex2BufIndex(const int32_t iIndex,
                                             int32_t& iBlockIndex,
                                             int32_t& iInnerIndex) const {
  ASSERT(iIndex >= 0);
  int32_t iRealIndex = m_iStartPosition + iIndex;
  iBlockIndex = iRealIndex / m_iAllocStep;
  iInnerIndex = iRealIndex % m_iAllocStep;
}

void CFX_BlockBuffer::ClearBuffer() {
  m_iBufferSize = 0;
  m_BlockArray.clear();
}
