// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file defines some bit utilities.

#ifndef THIRD_PARTY_BASE_BITS_H_
#define THIRD_PARTY_BASE_BITS_H_

#include <type_traits>

namespace pdfium {
namespace base {
namespace bits {

// TODO(thestig): When C++20 is required, replace with std::has_single_bit().
// Returns true iff |value| is a power of 2.
template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
constexpr inline bool IsPowerOfTwo(T value) {
  // From "Hacker's Delight": Section 2.1 Manipulating Rightmost Bits.
  //
  // Only positive integers with a single bit set are powers of two. If only one
  // bit is set in x (e.g. 0b00000100000000) then |x-1| will have that bit set
  // to zero and all bits to its right set to 1 (e.g. 0b00000011111111). Hence
  // |x & (x-1)| is 0 iff x is a power of two.
  return value > 0 && (value & (value - 1)) == 0;
}

}  // namespace bits
}  // namespace base
}  // namespace pdfium

#endif  // THIRD_PARTY_BASE_BITS_H_
