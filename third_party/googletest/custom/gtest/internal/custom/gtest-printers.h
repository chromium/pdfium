// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_GOOGLETEST_CUSTOM_GTEST_INTERNAL_CUSTOM_GTEST_PRINTERS_H_
#define THIRD_PARTY_GOOGLETEST_CUSTOM_GTEST_INTERNAL_CUSTOM_GTEST_PRINTERS_H_

#include <string>

namespace fxcrt {
class ByteString;
}

namespace testing {

// If a C string is compared with a PDFium string object, then it is meant to
// point to a NUL-terminated string, and thus print it as a string.

#define GTEST_IMPL_FORMAT_C_STRING_AS_STRING_(CharType, OtherStringType) \
  template <>                                                            \
  class internal::FormatForComparison<CharType*, OtherStringType> {      \
   public:                                                               \
    static std::string Format(CharType* value) {                         \
      return ::testing::PrintToString(value);                            \
    }                                                                    \
  }

GTEST_IMPL_FORMAT_C_STRING_AS_STRING_(char, fxcrt::ByteString);
GTEST_IMPL_FORMAT_C_STRING_AS_STRING_(const char, fxcrt::ByteString);

#undef GTEST_IMPL_FORMAT_C_STRING_AS_STRING_

}  // namespace testing

#endif  // THIRD_PARTY_GOOGLETEST_CUSTOM_GTEST_INTERNAL_CUSTOM_GTEST_PRINTERS_H_
