// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_SPAN_UTIL_H_
#define CORE_FXCRT_SPAN_UTIL_H_

#include <stdint.h>

#include <type_traits>

#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "third_party/base/check_op.h"
#include "third_party/base/containers/span.h"

namespace fxcrt {

// Bounds-checked byte-for-byte copies from spans into spans. Returns a
// span describing the remaining portion of the destination span.
template <typename T,
          typename U,
          typename = std::enable_if_t<sizeof(T) == sizeof(U) &&
                                      std::is_trivially_copyable_v<T> &&
                                      std::is_trivially_copyable_v<U>>>
pdfium::span<T> spancpy(pdfium::span<T> dst, pdfium::span<U> src) {
  CHECK_GE(dst.size(), src.size());
  FXSYS_memcpy(dst.data(), src.data(), src.size_bytes());
  return dst.subspan(src.size());
}

// Bounds-checked byte-for-byte moves from spans into spans. Returns a
// span describing the remaining portion of the destination span.
template <typename T,
          typename U,
          typename = std::enable_if_t<sizeof(T) == sizeof(U) &&
                                      std::is_trivially_copyable_v<T> &&
                                      std::is_trivially_copyable_v<U>>>
pdfium::span<T> spanmove(pdfium::span<T> dst, pdfium::span<U> src) {
  CHECK_GE(dst.size(), src.size());
  FXSYS_memmove(dst.data(), src.data(), src.size_bytes());
  return dst.subspan(src.size());
}

// Bounds-checked sets into spans.
template <typename T,
          typename = std::enable_if_t<std::is_trivially_constructible_v<T> &&
                                      std::is_trivially_destructible_v<T>>>
void spanset(pdfium::span<T> dst, uint8_t val) {
  FXSYS_memset(dst.data(), val, dst.size_bytes());
}

// Bounds-checked zeroing of spans.
template <typename T,
          typename = std::enable_if_t<std::is_trivially_constructible_v<T> &&
                                      std::is_trivially_destructible_v<T>>>
void spanclr(pdfium::span<T> dst) {
  FXSYS_memset(dst.data(), 0, dst.size_bytes());
}

// Bounds-checked byte-for-byte equality of same-sized spans. This is
// helpful because span does not (yet) have an operator==().
template <typename T,
          typename U,
          typename = std::enable_if_t<sizeof(T) == sizeof(U) &&
                                      std::is_trivially_copyable_v<T> &&
                                      std::is_trivially_copyable_v<U>>>
bool span_equals(pdfium::span<T> s1, pdfium::span<U> s2) {
  return s1.size_bytes() == s2.size_bytes() &&
         memcmp(s1.data(), s2.data(), s1.size_bytes()) == 0;
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
