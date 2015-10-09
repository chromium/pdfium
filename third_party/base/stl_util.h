// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDFIUM_THIRD_PARTY_BASE_STL_UTIL_H_
#define PDFIUM_THIRD_PARTY_BASE_STL_UTIL_H_

#include <vector>

namespace pdfium {

// To treat a possibly-empty vector as an array, use these functions.
// If you know the array will never be empty, you can use &*v.begin()
// directly, but that is undefined behaviour if |v| is empty.
template <typename T>
inline T* vector_as_array(std::vector<T>* v) {
  return v->empty() ? nullptr : &*v->begin();
}

template <typename T>
inline const T* vector_as_array(const std::vector<T>* v) {
  return v->empty() ? nullptr : &*v->begin();
}

}  // namespace pdfium

#endif  // PDFIUM_THIRD_PARTY_BASE_STL_UTIL_H_
