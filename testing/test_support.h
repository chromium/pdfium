// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_TEST_SUPPORT_H_
#define TESTING_TEST_SUPPORT_H_

#include <stdint.h>

namespace pdfium {

#define STR_IN_TEST_CASE(input_literal, ...)               \
  {                                                        \
    reinterpret_cast<const unsigned char*>(input_literal), \
        sizeof(input_literal) - 1, __VA_ARGS__             \
  }

#define STR_IN_OUT_CASE(input_literal, expected_literal, ...)     \
  {                                                               \
    reinterpret_cast<const unsigned char*>(input_literal),        \
        sizeof(input_literal) - 1,                                \
        reinterpret_cast<const unsigned char*>(expected_literal), \
        sizeof(expected_literal) - 1, __VA_ARGS__                 \
  }

struct StrFuncTestData {
  const unsigned char* input;
  uint32_t input_size;
  const unsigned char* expected;
  uint32_t expected_size;
};

struct DecodeTestData {
  const unsigned char* input;
  uint32_t input_size;
  const unsigned char* expected;
  uint32_t expected_size;
  // The size of input string being processed.
  uint32_t processed_size;
};

struct NullTermWstrFuncTestData {
  const wchar_t* input;
  const wchar_t* expected;
};

}  // namespace pdfium

void InitializePDFTestEnvironment();
void DestroyPDFTestEnvironment();

#endif  // TESTING_TEST_SUPPORT_H_
