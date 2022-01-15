// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_STL_UTIL_H_
#define CORE_FXCRT_STL_UTIL_H_

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "core/fxcrt/fx_memory_wrappers.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "third_party/base/numerics/safe_math.h"

namespace fxcrt {

// Means of generating a key for searching STL collections of std::unique_ptr
// that avoids the side effect of deleting the pointer.
template <class T>
class FakeUniquePtr : public std::unique_ptr<T> {
 public:
  using std::unique_ptr<T>::unique_ptr;
  ~FakeUniquePtr() { std::unique_ptr<T>::release(); }
};

// Convenience routine for "int-fected" code, so that the stl collection
// size_t size() method return values will be checked.
template <typename ResultType, typename Collection>
ResultType CollectionSize(const Collection& collection) {
  return pdfium::base::checked_cast<ResultType>(collection.size());
}

// Convenience routine for "int-fected" code, to handle signed indicies. The
// compiler can deduce the type, making this more convenient than the above.
template <typename IndexType, typename Collection>
bool IndexInBounds(const Collection& collection, IndexType index) {
  return index >= 0 && index < CollectionSize<IndexType>(collection);
}

// Safely allocate a 1-dim vector big enough for |w| by |h| or die.
template <typename T, typename A = std::allocator<T>>
std::vector<T, A> Vector2D(size_t w, size_t h) {
  pdfium::base::CheckedNumeric<size_t> safe_size = w;
  safe_size *= h;
  return std::vector<T, A>(safe_size.ValueOrDie());
}

// String stream that uses PartitionAlloc for backing store.
using ostringstream = std::
    basic_ostringstream<char, std::char_traits<char>, FxStringAllocator<char>>;

}  // namespace fxcrt

#endif  // CORE_FXCRT_STL_UTIL_H_
