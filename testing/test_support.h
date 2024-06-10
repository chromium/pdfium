// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_TEST_SUPPORT_H_
#define TESTING_TEST_SUPPORT_H_

#include <stdint.h>

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/span.h"

namespace pdfium {

#define STR_IN_TEST_CASE(input_literal, ...)         \
  {                                                  \
    reinterpret_cast<const uint8_t*>(input_literal), \
        sizeof(input_literal) - 1, __VA_ARGS__       \
  }

#define STR_IN_OUT_CASE(input_literal, expected_literal, ...) \
  {                                                           \
    reinterpret_cast<const uint8_t*>(input_literal),          \
        sizeof(input_literal) - 1,                            \
        reinterpret_cast<const uint8_t*>(expected_literal),   \
        sizeof(expected_literal) - 1, __VA_ARGS__             \
  }

struct StrFuncTestData {
  pdfium::span<const uint8_t> input_span() const {
    // SAFETY: size determined from literal via macro above.
    return UNSAFE_BUFFERS(pdfium::make_span(input, input_size));
  }
  pdfium::span<const uint8_t> expected_span() const {
    // SAFETY: size determined from literal via macro above.
    return UNSAFE_BUFFERS(pdfium::make_span(expected, expected_size));
  }

  const uint8_t* input;
  uint32_t input_size;
  const uint8_t* expected;
  uint32_t expected_size;
};

struct DecodeTestData {
  pdfium::span<const uint8_t> input_span() const {
    // SAFETY: size determined from literal via macro above.
    return UNSAFE_BUFFERS(pdfium::make_span(input, input_size));
  }
  pdfium::span<const uint8_t> expected_span() const {
    // SAFETY: size determined from literal via macro above.
    return UNSAFE_BUFFERS(pdfium::make_span(expected, expected_size));
  }

  const uint8_t* input;
  uint32_t input_size;
  const uint8_t* expected;
  uint32_t expected_size;
  // The size of input string being processed.
  uint32_t processed_size;
};

struct NullTermWstrFuncTestData {
  const wchar_t* input;
  const wchar_t* expected;
};

}  // namespace pdfium

#endif  // TESTING_TEST_SUPPORT_H_
