// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_defaultappearance.h"

#include <iterator>

#include "core/fpdfapi/parser/cpdf_simple_parser.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/span.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

namespace {

struct FindTagTestStruct {
  pdfium::span<const uint8_t> input_span() const {
    // SAFETY: STR_IN_TEST_CASE macro extracts size of literal.
    return UNSAFE_BUFFERS(pdfium::make_span(input, input_size));
  }
  const unsigned char* input;
  unsigned int input_size;
  const char* token;
  int num_params;
  bool result;
  unsigned int result_position;
};

}  // namespace

TEST(CPDFDefaultAppearanceTest, FindTagParamFromStart) {
  const FindTagTestStruct test_data[] = {
      // Empty strings.
      STR_IN_TEST_CASE("", "Tj", 1, false, 0),
      STR_IN_TEST_CASE("", "", 1, false, 0),
      // Empty token.
      STR_IN_TEST_CASE("  T j", "", 1, false, 5),
      // No parameter.
      STR_IN_TEST_CASE("Tj", "Tj", 1, false, 2),
      STR_IN_TEST_CASE("(Tj", "Tj", 1, false, 3),
      // Partial token match.
      STR_IN_TEST_CASE("\r12\t34  56 78Tj", "Tj", 1, false, 15),
      // Regular cases with various parameters.
      STR_IN_TEST_CASE("\r\0abd Tj", "Tj", 1, true, 0),
      STR_IN_TEST_CASE("12 4 Tj 3 46 Tj", "Tj", 1, true, 2),
      STR_IN_TEST_CASE("er^ 2 (34) (5667) Tj", "Tj", 2, true, 5),
      STR_IN_TEST_CASE("<344> (232)\t343.4\n12 45 Tj", "Tj", 3, true, 11),
      STR_IN_TEST_CASE("1 2 3 4 5 6 7 8 cm", "cm", 6, true, 3),
  };

  for (const auto& item : test_data) {
    CPDF_SimpleParser parser(item.input_span());
    EXPECT_EQ(item.result,
              CPDF_DefaultAppearance::FindTagParamFromStartForTesting(
                  &parser, item.token, item.num_params))
        << " for case " << item.input;
    EXPECT_EQ(item.result_position, parser.GetCurrentPosition())
        << " for case " << item.input;
  }
}
