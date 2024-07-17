// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_ZIP_H_
#define CORE_FXCRT_ZIP_H_

#include <stdint.h>

#include <utility>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/span.h"

namespace fxcrt {

// Vastly simplified implementation of ideas from C++23 zip_view<>. Allows
// safe traversal of two ranges with a single bounds check per iteration.

// Example usage:
//   struct RGB { uint8_t r; uint8_t g; uint8_t b; };
//   const uint8_t gray[256] = { ... };
//   RGB rgbs[260];
//   for (auto [in, out] : Zip(gray, rgbs)) {
//     out.r = in;
//     out.g = in;
//     out.b = in;
//   }
// which fills the first 256 elements of rgbs with the corresponding gray
// value in each component, say.

// Differences include:
// - Only zips together two views instead of N.
// - Size is determined by the first view, which must be smaller than the
//   second view.
// - First view is presumed to be "input-like" and is const, second view is
//   presumed to be "output-like" and is non-const.
// - Only those methods required to support use in a range-based for-loop
//   are provided.

template <typename T, typename U>
class ZipView {
 public:
  struct Iter {
    bool operator==(const Iter& that) const { return first == that.first; }

    bool operator!=(const Iter& that) const { return first != that.first; }

    UNSAFE_BUFFER_USAGE Iter& operator++() {
      // SAFETY: required from caller, enforced by UNSAFE_BUFFER_USAGE.
      UNSAFE_BUFFERS(++first);
      UNSAFE_BUFFERS(++second);
      return *this;
    }

    std::pair<typename T::reference, typename U::reference> operator*() const {
      return {*first, *second};
    }

    typename T::iterator first;
    typename U::iterator second;
  };

  ZipView(T first, U second) : first_(first), second_(second) {
    CHECK_LE(first.size(), second.size());
  }

  Iter begin() { return {first_.begin(), second_.begin()}; }
  Iter end() { return {first_.end(), second_.end()}; }

 private:
  T first_;
  U second_;
};

template <typename T, typename U>
auto Zip(const T& first, U&& second) {
  return ZipView(pdfium::span(first), pdfium::span(second));
}

}  // namespace fxcrt

#endif  // CORE_FXCRT_ZIP_H_
