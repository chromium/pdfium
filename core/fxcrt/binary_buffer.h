// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_BINARY_BUFFER_H_
#define CORE_FXCRT_BINARY_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "third_party/base/span.h"

namespace fxcrt {

class BinaryBuffer {
 public:
  BinaryBuffer();
  BinaryBuffer(BinaryBuffer&& that) noexcept;
  BinaryBuffer(const BinaryBuffer& that) = delete;
  virtual ~BinaryBuffer();

  // Moved-from value will be left empty.
  BinaryBuffer& operator=(BinaryBuffer&& that) noexcept;

  BinaryBuffer& operator=(const BinaryBuffer& that) = delete;

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
  void AppendString(const ByteString& str);
  void AppendByte(uint8_t byte);

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

}  // namespace fxcrt

using fxcrt::BinaryBuffer;

#endif  // CORE_FXCRT_BINARY_BUFFER_H_
