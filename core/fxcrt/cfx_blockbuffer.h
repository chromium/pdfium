// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_BLOCKBUFFER_H_
#define CORE_FXCRT_CFX_BLOCKBUFFER_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "core/fxcrt/fx_string.h"

class CFX_BlockBuffer {
 public:
  explicit CFX_BlockBuffer(int32_t iAllocStep = 1024 * 1024);
  ~CFX_BlockBuffer();

  bool InitBuffer(int32_t iBufferSize = 1024 * 1024);
  bool IsInitialized() { return m_iBufferSize / m_iAllocStep >= 1; }

  wchar_t* GetAvailableBlock(int32_t& iIndexInBlock);
  inline int32_t GetAllocStep() const { return m_iAllocStep; }
  inline int32_t& GetDataLengthRef() { return m_iDataLength; }

  inline void Reset(bool bReserveData = true) {
    if (!bReserveData)
      m_iStartPosition = 0;
    m_iDataLength = 0;
  }

  void SetTextChar(int32_t iIndex, wchar_t ch);
  int32_t DeleteTextChars(int32_t iCount, bool bDirection = true);
  void GetTextData(CFX_WideString& wsTextData,
                   int32_t iStart = 0,
                   int32_t iLength = -1) const;

 private:
  inline void TextDataIndex2BufIndex(const int32_t iIndex,
                                     int32_t& iBlockIndex,
                                     int32_t& iInnerIndex) const;
  void ClearBuffer();

  std::vector<std::unique_ptr<wchar_t, FxFreeDeleter>> m_BlockArray;
  int32_t m_iDataLength;
  int32_t m_iBufferSize;
  int32_t m_iAllocStep;
  int32_t m_iStartPosition;
};

#endif  // CORE_FXCRT_CFX_BLOCKBUFFER_H_
