// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_BINARYBUF_H_
#define CORE_FXCRT_CFX_BINARYBUF_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "third_party/base/span.h"

class CFX_BinaryBuf {
 public:
  CFX_BinaryBuf();
  CFX_BinaryBuf(CFX_BinaryBuf&& that) noexcept;
  virtual ~CFX_BinaryBuf();

  // Moved-from value will be left empty.
  CFX_BinaryBuf& operator=(CFX_BinaryBuf&& that) noexcept;

  pdfium::span<uint8_t> GetSpan();
  pdfium::span<const uint8_t> GetSpan() const;
  bool IsEmpty() const { return GetLength() == 0; }
  size_t GetSize() const { return m_DataSize; }  // In bytes.
  virtual size_t GetLength() const;              // In subclass-specific units.

  void Clear();
  void SetAllocStep(size_t step) { m_AllocStep = step; }
  void EstimateSize(size_t size);
  void AppendSpan(pdfium::span<const uint8_t> span);
  void AppendBlock(const void* pBuf, size_t size);
  void AppendString(const ByteString& str) {
    AppendBlock(str.c_str(), str.GetLength());
  }

  void AppendByte(uint8_t byte) {
    ExpandBuf(1);
    m_pBuffer.get()[m_DataSize++] = byte;
  }

  // Releases ownership of |m_pBuffer| and returns it.
  std::unique_ptr<uint8_t, FxFreeDeleter> DetachBuffer();

 protected:
  void ExpandBuf(size_t size);
  void DeleteBuf(size_t start_index, size_t count);

  size_t m_AllocStep = 0;
  size_t m_AllocSize = 0;
  size_t m_DataSize = 0;
  std::unique_ptr<uint8_t, FxFreeDeleter> m_pBuffer;
};

#endif  // CORE_FXCRT_CFX_BINARYBUF_H_
