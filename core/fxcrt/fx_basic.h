// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_BASIC_H_
#define CORE_FXCRT_FX_BASIC_H_

#include <algorithm>
#include <memory>
#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"

#ifdef PDF_ENABLE_XFA
#define FX_IsOdd(a) ((a)&1)
#endif  // PDF_ENABLE_XFA

class CFX_BinaryBuf {
 public:
  CFX_BinaryBuf();
  explicit CFX_BinaryBuf(FX_STRSIZE size);
  ~CFX_BinaryBuf();

  uint8_t* GetBuffer() const { return m_pBuffer.get(); }
  FX_STRSIZE GetSize() const { return m_DataSize; }

  void Clear();
  void EstimateSize(FX_STRSIZE size, FX_STRSIZE alloc_step = 0);
  void AppendBlock(const void* pBuf, FX_STRSIZE size);
  void AppendString(const CFX_ByteString& str) {
    AppendBlock(str.c_str(), str.GetLength());
  }

  void AppendByte(uint8_t byte) {
    ExpandBuf(1);
    m_pBuffer.get()[m_DataSize++] = byte;
  }

  void InsertBlock(FX_STRSIZE pos, const void* pBuf, FX_STRSIZE size);
  void Delete(FX_STRSIZE start_index, FX_STRSIZE count);

  // Releases ownership of |m_pBuffer| and returns it.
  std::unique_ptr<uint8_t, FxFreeDeleter> DetachBuffer();

 protected:
  void ExpandBuf(FX_STRSIZE size);

  FX_STRSIZE m_AllocStep;
  FX_STRSIZE m_AllocSize;
  FX_STRSIZE m_DataSize;
  std::unique_ptr<uint8_t, FxFreeDeleter> m_pBuffer;
};

class CFX_WideTextBuf : public CFX_BinaryBuf {
 public:
  void AppendChar(wchar_t wch);
  FX_STRSIZE GetLength() const { return m_DataSize / sizeof(wchar_t); }
  wchar_t* GetBuffer() const {
    return reinterpret_cast<wchar_t*>(m_pBuffer.get());
  }

  CFX_WideStringC AsStringC() const {
    return CFX_WideStringC(reinterpret_cast<const wchar_t*>(m_pBuffer.get()),
                           m_DataSize / sizeof(wchar_t));
  }
  CFX_WideString MakeString() const {
    return CFX_WideString(reinterpret_cast<const wchar_t*>(m_pBuffer.get()),
                          m_DataSize / sizeof(wchar_t));
  }

  void Delete(int start_index, int count) {
    CFX_BinaryBuf::Delete(start_index * sizeof(wchar_t),
                          count * sizeof(wchar_t));
  }

  CFX_WideTextBuf& operator<<(int i);
  CFX_WideTextBuf& operator<<(double f);
  CFX_WideTextBuf& operator<<(const wchar_t* lpsz);
  CFX_WideTextBuf& operator<<(const CFX_WideStringC& str);
  CFX_WideTextBuf& operator<<(const CFX_WideString& str);
  CFX_WideTextBuf& operator<<(const CFX_WideTextBuf& buf);
};

class CFX_UTF8Decoder {
 public:
  CFX_UTF8Decoder() { m_PendingBytes = 0; }

  void Clear();
  void Input(uint8_t byte);
  void AppendCodePoint(uint32_t ch);
  void ClearStatus() { m_PendingBytes = 0; }
  CFX_WideStringC GetResult() const { return m_Buffer.AsStringC(); }

 private:
  int m_PendingBytes;
  uint32_t m_PendingChar;
  CFX_WideTextBuf m_Buffer;
};

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
