// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BASE_NUMERICS_RANGES_H_
#define THIRD_PARTY_BASE_NUMERICS_RANGES_H_

#include <algorithm>

namespace pdfium {

// To be replaced with std::clamp() from C++17, someday.
template <class T>
constexpr const T& clamp(const T& value, const T& min, const T& max) {
  return std::min(std::max(value, min), max);
}

}  // namespace pdfium

#endif  // THIRD_PARTY_BASE_NUMERICS_RANGES_H_
