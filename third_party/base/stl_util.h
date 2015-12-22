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

// Test to see if a set, map, hash_set or hash_map contains a particular key.
// Returns true if the key is in the collection.
template <typename Collection, typename Key>
bool ContainsKey(const Collection& collection, const Key& key) {
  return collection.find(key) != collection.end();
}

// Test to see if a collection like a vector contains a particular value.
// Returns true if the value is in the collection.
template <typename Collection, typename Value>
bool ContainsValue(const Collection& collection, const Value& value) {
  return std::find(collection.begin(), collection.end(), value) !=
         collection.end();
}

}  // namespace pdfium

#endif  // PDFIUM_THIRD_PARTY_BASE_STL_UTIL_H_
