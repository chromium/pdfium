// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_FIXED_ZEROED_DATA_VECTOR_H_
#define CORE_FXCRT_FIXED_ZEROED_DATA_VECTOR_H_

#include <stddef.h>

#include <memory>
#include <utility>

#include "core/fxcrt/fx_memory_wrappers.h"
#include "third_party/base/span.h"

namespace fxcrt {

// A simple data container that has a fixed size.
// Unlike std::vector, it cannot be implicitly copied.
// Access its data using spans.
template <typename T>
class FixedZeroedDataVector {
 public:
  FixedZeroedDataVector() : FixedZeroedDataVector(0) {}
  explicit FixedZeroedDataVector(size_t size)
      : data_(size ? FX_Alloc(T, size) : nullptr), size_(size) {}
  FixedZeroedDataVector(const FixedZeroedDataVector&) = delete;
  FixedZeroedDataVector& operator=(const FixedZeroedDataVector&) = delete;
  FixedZeroedDataVector(FixedZeroedDataVector&& that) noexcept {
    data_ = std::move(that.data_);
    size_ = that.size_;
    that.size_ = 0;
  }
  FixedZeroedDataVector& operator=(FixedZeroedDataVector&& that) noexcept {
    data_ = std::move(that.data_);
    size_ = that.size_;
    that.size_ = 0;
    return *this;
  }
  ~FixedZeroedDataVector() = default;

  operator pdfium::span<const T>() const { return span(); }

  pdfium::span<T> writable_span() {
    return pdfium::make_span(data_.get(), size_);
  }

  pdfium::span<const T> span() const {
    return pdfium::make_span(data_.get(), size_);
  }

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

 private:
  std::unique_ptr<T, FxFreeDeleter> data_;
  size_t size_;
};

}  // namespace fxcrt

using fxcrt::FixedZeroedDataVector;

#endif  // CORE_FXCRT_FIXED_ZEROED_DATA_VECTOR_H_
