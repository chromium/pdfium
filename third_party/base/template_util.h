// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDFIUM_THIRD_PARTY_BASE_TEMPLATE_UTIL_H_
#define PDFIUM_THIRD_PARTY_BASE_TEMPLATE_UTIL_H_

#include <cstddef>  // For size_t.

namespace pdfium {
namespace base {

template<class T, T v>
struct integral_constant {
  static const T value = v;
  typedef T value_type;
  typedef integral_constant<T, v> type;
};

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

template <class T, class U> struct is_same : public false_type {};
template <class T> struct is_same<T,T> : true_type {};

template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };

}  // namespace base
}  // namespace pdfium

#endif  // PDFIUM_THIRD_PARTY_BASE_TEMPLATE_UTIL_H_
