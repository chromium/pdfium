// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_BLOCKBUFFER_H_
#define CORE_FXCRT_CFX_BLOCKBUFFER_H_

#include <stdint.h>

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_string.h"

class CFX_BlockBuffer {
 public:
  CFX_BlockBuffer();
  ~CFX_BlockBuffer();

  bool InitBuffer();
  bool IsInitialized() { return m_BufferSize / GetAllocStep() >= 1; }

  std::pair<wchar_t*, size_t> GetAvailableBlock();
  size_t GetAllocStep() const;
  size_t GetDataLength() const { return m_DataLength; }
  void IncrementDataLength() { m_DataLength++; }
  bool IsEmpty() const { return m_DataLength == 0; }

  void Reset(bool bReserveData) {
    if (!bReserveData)
      m_StartPosition = 0;
    m_DataLength = 0;
  }

  void SetTextChar(size_t iIndex, wchar_t ch);
  void DeleteTextChars(size_t iCount);
  WideString GetTextData(size_t iStart, size_t iLength) const;

 private:
  std::pair<size_t, size_t> TextDataIndex2BufIndex(const size_t iIndex) const;

  std::vector<std::unique_ptr<wchar_t, FxFreeDeleter>> m_BlockArray;
  size_t m_DataLength;
  size_t m_BufferSize;
  size_t m_StartPosition;
};

#endif  // CORE_FXCRT_CFX_BLOCKBUFFER_H_
