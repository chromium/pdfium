// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/cjs_publicmethods.h"

#include <iterator>

#include "testing/gtest/include/gtest/gtest.h"

TEST(CJSPublicMethodsTest, IsNumber) {
  // TODO(weili): Check whether results from case 0, 1, 10, 15 are intended.
  struct {
    const wchar_t* input;
    bool expected;
  } test_data[] = {
      // Empty string.
      {L"", true},
      // Only whitespaces.
      {L"  ", true},
      // Content with invalid characters.
      {L"xyz00", false},
      {L"1%", false},
      // Hex string.
      {L"0x234", false},
      // Signed numbers.
      {L"+123", true},
      {L"-98765", true},
      // Numbers with whitespaces.
      {L"  345 ", true},
      // Float numbers.
      {L"-1e5", false},
      {L"-2e", false},
      {L"e-5", true},
      {L"0.023", true},
      {L".356089", true},
      {L"1e-9", true},
      {L"-1.23e+23", true},
      // Numbers with commas.
      {L"1,000,000", false},
      {L"560,024", true},
      // Regular numbers.
      {L"0", true},
      {L"0123", true},
      {L"9876123", true},
  };
  for (const auto& element : test_data) {
    EXPECT_EQ(element.expected, CJS_PublicMethods::IsNumber(element.input));
  }
}
