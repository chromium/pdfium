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
  bool IsInitialized() { return m_iBufferSize / GetAllocStep() >= 1; }

  std::pair<wchar_t*, int32_t> GetAvailableBlock();
  int32_t GetAllocStep() const;

  // This is ... scary. This returns a ref, which the XMLSyntaxParser stores
  // and modifies.
  int32_t& GetDataLengthRef() { return m_iDataLength; }

  void Reset(bool bReserveData) {
    if (!bReserveData)
      m_iStartPosition = 0;
    m_iDataLength = 0;
  }

  void SetTextChar(int32_t iIndex, wchar_t ch);
  int32_t DeleteTextChars(int32_t iCount);
  CFX_WideString GetTextData(int32_t iStart, int32_t iLength) const;

 private:
  std::pair<int32_t, int32_t> TextDataIndex2BufIndex(
      const int32_t iIndex) const;

  std::vector<std::unique_ptr<wchar_t, FxFreeDeleter>> m_BlockArray;
  int32_t m_iDataLength;
  int32_t m_iBufferSize;
  int32_t m_iStartPosition;
};

#endif  // CORE_FXCRT_CFX_BLOCKBUFFER_H_
