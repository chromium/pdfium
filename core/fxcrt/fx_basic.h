// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_BASIC_H_
#define CORE_FXCRT_FX_BASIC_H_

#include <algorithm>
#include <memory>
#include <vector>

#include "core/fxcrt/cfx_binarybuf.h"
#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"

#ifdef PDF_ENABLE_XFA
#define FX_IsOdd(a) ((a)&1)
#endif  // PDF_ENABLE_XFA

template <class DataType, int FixedSize>
class CFX_FixedBufGrow {
 public:
  explicit CFX_FixedBufGrow(int data_size) {
    if (data_size > FixedSize) {
      m_pGrowData.reset(FX_Alloc(DataType, data_size));
      return;
    }
    memset(m_FixedData, 0, sizeof(DataType) * FixedSize);
  }
  operator DataType*() { return m_pGrowData ? m_pGrowData.get() : m_FixedData; }

 private:
  DataType m_FixedData[FixedSize];
  std::unique_ptr<DataType, FxFreeDeleter> m_pGrowData;
};

class IFX_Pause {
 public:
  virtual ~IFX_Pause() {}
  virtual bool NeedToPauseNow() = 0;
};

uint32_t GetBits32(const uint8_t* pData, int bitpos, int nbits);

#endif  // CORE_FXCRT_FX_BASIC_H_
