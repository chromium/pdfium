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
template <class T>
struct is_same<T, T> : true_type {};

template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };

namespace internal {

// Types YesType and NoType are guaranteed such that sizeof(YesType) <
// sizeof(NoType).
typedef char YesType;

struct NoType {
  YesType dummy[2];
};

// This class is an implementation detail for is_convertible, and you
// don't need to know how it works to use is_convertible. For those
// who care: we declare two different functions, one whose argument is
// of type To and one with a variadic argument list. We give them
// return types of different size, so we can use sizeof to trick the
// compiler into telling us which function it would have chosen if we
// had called it with an argument of type From.  See Alexandrescu's
// _Modern C++ Design_ for more details on this sort of trick.

struct ConvertHelper {
  template <typename To>
  static YesType Test(To);

  template <typename To>
  static NoType Test(...);

  template <typename From>
  static From& Create();
};

}  // namespace internal

// Inherits from true_type if From is convertible to To, false_type otherwise.
//
// Note that if the type is convertible, this will be a true_type REGARDLESS
// of whether or not the conversion would emit a warning.
template <typename From, typename To>
struct is_convertible
    : integral_constant<bool,
                        sizeof(internal::ConvertHelper::Test<To>(
                            internal::ConvertHelper::Create<From>())) ==
                            sizeof(internal::YesType)> {};

}  // namespace base
}  // namespace pdfium

#endif  // PDFIUM_THIRD_PARTY_BASE_TEMPLATE_UTIL_H_
