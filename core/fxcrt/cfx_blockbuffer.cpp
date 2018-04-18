// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_blockbuffer.h"

#include <algorithm>
#include <utility>

namespace {

const size_t kAllocStep = 1024 * 1024;

}  // namespace

CFX_BlockBuffer::CFX_BlockBuffer()
    : m_DataLength(0), m_BufferSize(0), m_StartPosition(0) {}

CFX_BlockBuffer::~CFX_BlockBuffer() {}

size_t CFX_BlockBuffer::GetAllocStep() const {
  return kAllocStep;
}

std::pair<wchar_t*, size_t> CFX_BlockBuffer::GetAvailableBlock() {
  if (m_BlockArray.empty())
    return {nullptr, 0};

  size_t realIndex = m_StartPosition + m_DataLength;
  if (realIndex == m_BufferSize) {
    m_BlockArray.emplace_back(FX_Alloc(wchar_t, kAllocStep));
    m_BufferSize += kAllocStep;
    return {m_BlockArray.back().get(), 0};
  }
  return {m_BlockArray[realIndex / kAllocStep].get(), realIndex % kAllocStep};
}

bool CFX_BlockBuffer::InitBuffer() {
  m_BlockArray.clear();
  m_BlockArray.emplace_back(FX_Alloc(wchar_t, kAllocStep));
  m_BufferSize = kAllocStep;
  return true;
}

void CFX_BlockBuffer::SetTextChar(size_t index, wchar_t ch) {
  size_t realIndex = m_StartPosition + index;
  size_t blockIndex = realIndex / kAllocStep;
  if (blockIndex >= m_BlockArray.size()) {
    size_t newBlocks = blockIndex - m_BlockArray.size() + 1;
    do {
      m_BlockArray.emplace_back(FX_Alloc(wchar_t, kAllocStep));
      m_BufferSize += kAllocStep;
    } while (--newBlocks);
  }
  wchar_t* pTextData = m_BlockArray[blockIndex].get();
  pTextData[realIndex % kAllocStep] = ch;
  m_DataLength = std::max(m_DataLength, index + 1);
}

void CFX_BlockBuffer::DeleteTextChars(size_t count) {
  if (count == 0)
    return;

  if (count >= m_DataLength) {
    Reset(false);
    return;
  }
  m_DataLength -= count;
}

WideString CFX_BlockBuffer::GetTextData(size_t start, size_t length) const {
  if (m_BufferSize <= m_StartPosition + 1 || length == 0)
    return WideString();

  size_t maybeDataLength = m_BufferSize - 1 - m_StartPosition;
  if (start > maybeDataLength)
    return WideString();
  length = std::min(length, maybeDataLength);

  WideString wsTextData;
  wchar_t* pBuf = wsTextData.GetBuffer(length);
  if (!pBuf)
    return WideString();

  size_t startBlock = 0;
  size_t startInner = 0;
  std::tie(startBlock, startInner) = TextDataIndex2BufIndex(start);

  size_t endBlock = 0;
  size_t endInner = 0;
  std::tie(endBlock, endInner) = TextDataIndex2BufIndex(start + length);

  size_t pointer = 0;
  for (size_t i = startBlock; i <= endBlock; ++i) {
    size_t bufferPointer = 0;
    size_t copyLength = kAllocStep;
    if (i == startBlock) {
      copyLength -= startInner;
      bufferPointer = startInner;
    }
    if (i == endBlock)
      copyLength -= ((kAllocStep - 1) - endInner);

    wchar_t* pBlockBuf = m_BlockArray[i].get();
    memcpy(pBuf + pointer, pBlockBuf + bufferPointer,
           copyLength * sizeof(wchar_t));
    pointer += copyLength;
  }
  wsTextData.ReleaseBuffer(length);
  return wsTextData;
}

std::pair<size_t, size_t> CFX_BlockBuffer::TextDataIndex2BufIndex(
    const size_t iIndex) const {
  ASSERT(iIndex >= 0);

  size_t realIndex = m_StartPosition + iIndex;
  return {realIndex / kAllocStep, realIndex % kAllocStep};
}
