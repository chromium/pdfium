// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/binary_buffer.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/fx_safe_types.h"

namespace fxcrt {

BinaryBuffer::BinaryBuffer() = default;

BinaryBuffer::BinaryBuffer(BinaryBuffer&& that) noexcept
    : m_AllocStep(that.m_AllocStep),
      m_DataSize(that.m_DataSize),
      m_buffer(std::move(that.m_buffer)) {
  // Can't just default, need to leave |that| in a valid state, which means
  // that the size members reflect the (null) moved-from buffer.
  that.m_AllocStep = 0;
  that.m_DataSize = 0;
}

BinaryBuffer::~BinaryBuffer() = default;

BinaryBuffer& BinaryBuffer::operator=(BinaryBuffer&& that) noexcept {
  // Can't just default, need to leave |that| in a valid state, which means
  // that the size members reflect the (null) moved-from buffer.
  m_AllocStep = that.m_AllocStep;
  m_DataSize = that.m_DataSize;
  m_buffer = std::move(that.m_buffer);
  that.m_AllocStep = 0;
  that.m_DataSize = 0;
  return *this;
}

void BinaryBuffer::DeleteBuf(size_t start_index, size_t count) {
  if (m_buffer.empty() || count > GetSize() || start_index > GetSize() - count)
    return;

  memmove(m_buffer.data() + start_index, m_buffer.data() + start_index + count,
          GetSize() - start_index - count);
  m_DataSize -= count;
}

pdfium::span<uint8_t> BinaryBuffer::GetMutableSpan() {
  return {m_buffer.data(), GetSize()};
}

pdfium::span<const uint8_t> BinaryBuffer::GetSpan() const {
  return {m_buffer.data(), GetSize()};
}

size_t BinaryBuffer::GetLength() const {
  return GetSize();
}

void BinaryBuffer::Clear() {
  m_DataSize = 0;
}

DataVector<uint8_t> BinaryBuffer::DetachBuffer() {
  m_buffer.resize(GetSize());
  m_DataSize = 0;
  return std::move(m_buffer);
}

void BinaryBuffer::EstimateSize(size_t size) {
  if (m_buffer.size() < size)
    ExpandBuf(size - GetSize());
}

void BinaryBuffer::ExpandBuf(size_t add_size) {
  FX_SAFE_SIZE_T new_size = GetSize();
  new_size += add_size;
  if (m_buffer.size() >= new_size.ValueOrDie())
    return;

  size_t alloc_step = std::max(static_cast<size_t>(128),
                               m_AllocStep ? m_AllocStep : m_buffer.size() / 4);
  new_size += alloc_step - 1;  // Quantize, don't combine these lines.
  new_size /= alloc_step;
  new_size *= alloc_step;
  m_buffer.resize(new_size.ValueOrDie());
}

void BinaryBuffer::AppendSpan(pdfium::span<const uint8_t> span) {
  return AppendBlock(span.data(), span.size());
}

void BinaryBuffer::AppendBlock(const void* pBuf, size_t size) {
  if (size == 0)
    return;

  ExpandBuf(size);
  if (pBuf) {
    memcpy(m_buffer.data() + GetSize(), pBuf, size);
  } else {
    memset(m_buffer.data() + GetSize(), 0, size);
  }
  m_DataSize += size;
}

void BinaryBuffer::AppendString(const ByteString& str) {
  AppendBlock(str.c_str(), str.GetLength());
}

void BinaryBuffer::AppendByte(uint8_t byte) {
  ExpandBuf(1);
  m_buffer[m_DataSize++] = byte;
}

}  // namespace fxcrt
