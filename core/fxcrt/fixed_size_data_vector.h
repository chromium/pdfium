// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_FIXED_SIZE_DATA_VECTOR_H_
#define CORE_FXCRT_FIXED_SIZE_DATA_VECTOR_H_

#include <stddef.h>

#include <memory>
#include <utility>

#include "core/fxcrt/fx_memory_wrappers.h"
#include "third_party/base/containers/span.h"

namespace fxcrt {

enum class DataVectorAllocOption {
  kInitialized,
  kUninitialized,
  kTryInitialized,
};

// A simple data container that has a fixed size.
// Unlike std::vector, it cannot be implicitly copied and its data is only
// accessible using spans.
// It can either initialize its data with zeros, or leave its data
// uninitialized.
template <typename T, DataVectorAllocOption OPTION>
class FixedSizeDataVector {
 public:
  FixedSizeDataVector() : FixedSizeDataVector(0) {}
  explicit FixedSizeDataVector(size_t size)
      : data_(MaybeInit(size, OPTION)), size_(CalculateSize(size, OPTION)) {}
  FixedSizeDataVector(const FixedSizeDataVector&) = delete;
  FixedSizeDataVector& operator=(const FixedSizeDataVector&) = delete;
  template <DataVectorAllocOption OTHER_OPTION>
  FixedSizeDataVector(FixedSizeDataVector<T, OTHER_OPTION>&& that) noexcept {
    data_ = std::move(that.data_);
    size_ = that.size_;
    that.size_ = 0;
  }
  template <DataVectorAllocOption OTHER_OPTION>
  FixedSizeDataVector& operator=(
      FixedSizeDataVector<T, OTHER_OPTION>&& that) noexcept {
    data_ = std::move(that.data_);
    size_ = that.size_;
    that.size_ = 0;
    return *this;
  }
  ~FixedSizeDataVector() = default;

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
  friend class FixedSizeDataVector<T, DataVectorAllocOption::kInitialized>;
  friend class FixedSizeDataVector<T, DataVectorAllocOption::kUninitialized>;
  friend class FixedSizeDataVector<T, DataVectorAllocOption::kTryInitialized>;

  static T* MaybeInit(size_t size, DataVectorAllocOption alloc_option) {
    if (size == 0)
      return nullptr;
    switch (alloc_option) {
      case DataVectorAllocOption::kInitialized:
        return FX_Alloc(T, size);
      case DataVectorAllocOption::kUninitialized:
        return FX_AllocUninit(T, size);
      case DataVectorAllocOption::kTryInitialized:
        return FX_TryAlloc(T, size);
    }
  }

  size_t CalculateSize(size_t size, DataVectorAllocOption alloc_option) const {
    switch (alloc_option) {
      case DataVectorAllocOption::kInitialized:
      case DataVectorAllocOption::kUninitialized:
        return size;
      case DataVectorAllocOption::kTryInitialized:
        return data_ ? size : 0;
    }
  }

  std::unique_ptr<T, FxFreeDeleter> data_;
  size_t size_;
};

}  // namespace fxcrt

#endif  // CORE_FXCRT_FIXED_SIZE_DATA_VECTOR_H_
