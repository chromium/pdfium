// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_STL_UTIL_H_
#define CORE_FXCRT_STL_UTIL_H_

#include <algorithm>
#include <iterator>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/numerics/safe_conversions.h"

namespace fxcrt {

// Convenience routine for "int-fected" code, so that the stl collection
// size_t size() method return values will be checked.
template <typename ResultType, typename Collection>
ResultType CollectionSize(const Collection& collection) {
  return pdfium::checked_cast<ResultType>(collection.size());
}

// Convenience routine for "int-fected" code, to handle signed indicies. The
// compiler can deduce the type, making this more convenient than the above.
template <typename IndexType, typename Collection>
bool IndexInBounds(const Collection& collection, IndexType index) {
  return index >= 0 && index < CollectionSize<IndexType>(collection);
}

// Non-flawed version of C++20 std::ranges::copy(), which takes an output
// range as the second parameter and CHECKS() if it not sufficiently sized.
template <typename T, typename U>
void Copy(const T& source_container, U&& dest_container) {
  static_assert(sizeof(source_container[0]) == sizeof(dest_container[0]));
  CHECK_GE(std::size(dest_container), std::size(source_container));
  std::copy(std::begin(source_container), std::end(source_container),
            std::begin(dest_container));
}

}  // namespace fxcrt

#endif  // CORE_FXCRT_STL_UTIL_H_
