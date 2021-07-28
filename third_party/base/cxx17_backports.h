// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BASE_CXX17_BACKPORTS_H_
#define THIRD_PARTY_BASE_CXX17_BACKPORTS_H_

#include <stddef.h>

namespace pdfium {

// C++14 implementation of C++17's std::size():
// http://en.cppreference.com/w/cpp/iterator/size
template <typename Container>
constexpr auto size(const Container& c) -> decltype(c.size()) {
  return c.size();
}

template <typename T, size_t N>
constexpr size_t size(const T (&array)[N]) noexcept {
  return N;
}

}  // namespace pdfium

#endif  // THIRD_PARTY_BASE_CXX17_BACKPORTS_H_
