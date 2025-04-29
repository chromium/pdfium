// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/binary_buffer.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"

namespace fxcrt {

BinaryBuffer::BinaryBuffer() = default;

BinaryBuffer::BinaryBuffer(BinaryBuffer&& that) noexcept
    : alloc_step_(that.alloc_step_),
      data_size_(that.data_size_),
      buffer_(std::move(that.buffer_)) {
  // Can't just default, need to leave |that| in a valid state, which means
  // that the size members reflect the (null) moved-from buffer.
  that.alloc_step_ = 0;
  that.data_size_ = 0;
}

BinaryBuffer::~BinaryBuffer() = default;

BinaryBuffer& BinaryBuffer::operator=(BinaryBuffer&& that) noexcept {
  // Can't just default, need to leave |that| in a valid state, which means
  // that the size members reflect the (null) moved-from buffer.
  alloc_step_ = that.alloc_step_;
  data_size_ = that.data_size_;
  buffer_ = std::move(that.buffer_);
  that.alloc_step_ = 0;
  that.data_size_ = 0;
  return *this;
}

void BinaryBuffer::DeleteBuf(size_t start_index, size_t count) {
  if (buffer_.empty() || count > GetSize() || start_index > GetSize() - count) {
    return;
  }

  auto buffer_span = GetMutableSpan();
  fxcrt::spanmove(buffer_span.subspan(start_index),
                  buffer_span.subspan(start_index + count));
  data_size_ -= count;
}

pdfium::span<uint8_t> BinaryBuffer::GetMutableSpan() {
  return pdfium::span(buffer_).first(GetSize());
}

pdfium::span<const uint8_t> BinaryBuffer::GetSpan() const {
  return pdfium::span(buffer_).first(GetSize());
}

size_t BinaryBuffer::GetLength() const {
  return GetSize();
}

void BinaryBuffer::Clear() {
  data_size_ = 0;
}

DataVector<uint8_t> BinaryBuffer::DetachBuffer() {
  buffer_.resize(GetSize());
  data_size_ = 0;
  return std::move(buffer_);
}

void BinaryBuffer::EstimateSize(size_t size) {
  if (buffer_.size() < size) {
    ExpandBuf(size - GetSize());
  }
}

void BinaryBuffer::ExpandBuf(size_t add_size) {
  FX_SAFE_SIZE_T new_size = GetSize();
  new_size += add_size;
  if (buffer_.size() >= new_size.ValueOrDie()) {
    return;
  }

  size_t alloc_step = std::max(static_cast<size_t>(128),
                               alloc_step_ ? alloc_step_ : buffer_.size() / 4);
  new_size += alloc_step - 1;  // Quantize, don't combine these lines.
  new_size /= alloc_step;
  new_size *= alloc_step;
  buffer_.resize(new_size.ValueOrDie());
}

void BinaryBuffer::AppendSpan(pdfium::span<const uint8_t> span) {
  if (span.empty()) {
    return;
  }

  ExpandBuf(span.size());
  fxcrt::Copy(span, pdfium::span(buffer_).subspan(GetSize()));
  data_size_ += span.size();
}

void BinaryBuffer::AppendString(const ByteString& str) {
  AppendSpan(str.unsigned_span());
}

void BinaryBuffer::AppendUint8(uint8_t value) {
  AppendSpan(pdfium::span_from_ref(value));
}

void BinaryBuffer::AppendUint16(uint16_t value) {
  AppendSpan(pdfium::as_bytes(pdfium::span_from_ref(value)));
}

void BinaryBuffer::AppendUint32(uint32_t value) {
  AppendSpan(pdfium::as_bytes(pdfium::span_from_ref(value)));
}

void BinaryBuffer::AppendDouble(double value) {
  AppendSpan(pdfium::as_bytes(pdfium::allow_nonunique_obj,
                              pdfium::span_from_ref(value)));
}

}  // namespace fxcrt
