// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_SPAN_UTIL_H_
#define CORE_FXCRT_SPAN_UTIL_H_

#include <stdint.h>

#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "third_party/base/check_op.h"
#include "third_party/base/containers/span.h"

namespace fxcrt {

// Bounds-checked copies from spans into spans.
template <typename T,
          typename U,
          typename = pdfium::internal::EnableIfLegalSpanConversion<T, U>>
void spancpy(pdfium::span<T> dst, pdfium::span<U> src) {
  CHECK_GE(dst.size_bytes(), src.size_bytes());
  FXSYS_memcpy(dst.data(), src.data(), src.size_bytes());
}

// Bounds-checked moves from spans into spans.
template <typename T,
          typename U,
          typename = pdfium::internal::EnableIfLegalSpanConversion<T, U>>
void spanmove(pdfium::span<T> dst, pdfium::span<U> src) {
  CHECK_GE(dst.size_bytes(), src.size_bytes());
  FXSYS_memmove(dst.data(), src.data(), src.size_bytes());
}

// Bounds-checked sets into spans.
template <typename T>
void spanset(pdfium::span<T> dst, uint8_t val) {
  FXSYS_memset(dst.data(), val, dst.size_bytes());
}

// Bounds-checked zeroing of spans.
template <typename T>
void spanclr(pdfium::span<T> dst) {
  FXSYS_memset(dst.data(), 0, dst.size_bytes());
}

template <typename T,
          typename U,
          typename = typename std::enable_if_t<std::is_const_v<T> ||
                                               !std::is_const_v<U>>>
inline pdfium::span<T> reinterpret_span(pdfium::span<U> s) noexcept {
  CHECK_EQ(s.size_bytes() % sizeof(T), 0u);
  CHECK_EQ(reinterpret_cast<uintptr_t>(s.data()) % alignof(T), 0u);
  return {reinterpret_cast<T*>(s.data()), s.size_bytes() / sizeof(T)};
}

}  // namespace fxcrt

#endif  // CORE_FXCRT_SPAN_UTIL_H_
