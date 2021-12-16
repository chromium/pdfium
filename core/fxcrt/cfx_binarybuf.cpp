// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_binarybuf.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/fx_safe_types.h"

CFX_BinaryBuf::CFX_BinaryBuf() = default;

CFX_BinaryBuf::CFX_BinaryBuf(CFX_BinaryBuf&& that) noexcept
    : m_AllocStep(that.m_AllocStep),
      m_AllocSize(that.m_AllocSize),
      m_DataSize(that.m_DataSize),
      m_pBuffer(std::move(that.m_pBuffer)) {
  // Can't just default, need to leave |that| in a valid state, which means
  // that the size members reflect the (null) moved-from buffer.
  that.m_AllocStep = 0;
  that.m_AllocSize = 0;
  that.m_DataSize = 0;
}

CFX_BinaryBuf::~CFX_BinaryBuf() = default;

CFX_BinaryBuf& CFX_BinaryBuf::operator=(CFX_BinaryBuf&& that) noexcept {
  // Can't just default, need to leave |that| in a valid state, which means
  // that the size members reflect the (null) moved-from buffer.
  m_AllocStep = that.m_AllocStep;
  m_AllocSize = that.m_AllocSize;
  m_DataSize = that.m_DataSize;
  m_pBuffer = std::move(that.m_pBuffer);
  that.m_AllocStep = 0;
  that.m_AllocSize = 0;
  that.m_DataSize = 0;
  return *this;
}

void CFX_BinaryBuf::Delete(size_t start_index, size_t count) {
  if (!m_pBuffer || count > m_DataSize || start_index > m_DataSize - count)
    return;

  memmove(m_pBuffer.get() + start_index, m_pBuffer.get() + start_index + count,
          m_DataSize - start_index - count);
  m_DataSize -= count;
}

pdfium::span<uint8_t> CFX_BinaryBuf::GetSpan() {
  return {m_pBuffer.get(), GetSize()};
}

pdfium::span<const uint8_t> CFX_BinaryBuf::GetSpan() const {
  return {m_pBuffer.get(), GetSize()};
}

size_t CFX_BinaryBuf::GetLength() const {
  return m_DataSize;
}

void CFX_BinaryBuf::Clear() {
  m_DataSize = 0;
}

std::unique_ptr<uint8_t, FxFreeDeleter> CFX_BinaryBuf::DetachBuffer() {
  m_DataSize = 0;
  m_AllocSize = 0;
  return std::move(m_pBuffer);
}

void CFX_BinaryBuf::EstimateSize(size_t size) {
  if (m_AllocSize < size)
    ExpandBuf(size - m_DataSize);
}

void CFX_BinaryBuf::ExpandBuf(size_t add_size) {
  FX_SAFE_SIZE_T new_size = m_DataSize;
  new_size += add_size;
  if (m_AllocSize >= new_size.ValueOrDie())
    return;

  size_t alloc_step = std::max(static_cast<size_t>(128),
                               m_AllocStep ? m_AllocStep : m_AllocSize / 4);
  new_size += alloc_step - 1;  // Quantize, don't combine these lines.
  new_size /= alloc_step;
  new_size *= alloc_step;
  m_AllocSize = new_size.ValueOrDie();
  m_pBuffer.reset(m_pBuffer
                      ? FX_Realloc(uint8_t, m_pBuffer.release(), m_AllocSize)
                      : FX_Alloc(uint8_t, m_AllocSize));
}

void CFX_BinaryBuf::AppendSpan(pdfium::span<const uint8_t> span) {
  return AppendBlock(span.data(), span.size());
}

void CFX_BinaryBuf::AppendBlock(const void* pBuf, size_t size) {
  if (size == 0)
    return;

  ExpandBuf(size);
  if (pBuf) {
    memcpy(m_pBuffer.get() + m_DataSize, pBuf, size);
  } else {
    memset(m_pBuffer.get() + m_DataSize, 0, size);
  }
  m_DataSize += size;
}
