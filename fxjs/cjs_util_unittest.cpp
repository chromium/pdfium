// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/cjs_util.h"

#include <iterator>

#include "testing/gtest/include/gtest/gtest.h"

TEST(CJS_Util, ParseDataType) {
  struct ParseDataTypeCase {
    const wchar_t* const input_string;
    const CJS_Util::DataType expected;
  };

  // Commented out tests follow the spec but are not passing.
  const ParseDataTypeCase cases[] = {
      // Not conversions
      {L"", CJS_Util::DataType::kInvalid},
      {L"d", CJS_Util::DataType::kInvalid},

      // Simple cases
      {L"%d", CJS_Util::DataType::kInt},
      {L"%x", CJS_Util::DataType::kInt},
      {L"%f", CJS_Util::DataType::kDouble},
      {L"%s", CJS_Util::DataType::kString},

      // nDecSep Not implemented
      // {L"%,0d", CJS_Util::DataType::kInt},
      // {L"%,1d", CJS_Util::DataType::kInt},
      // {L"%,2d", CJS_Util::DataType::kInt},
      // {L"%,3d", CJS_Util::DataType::kInt},
      // {L"%,4d", -1},
      // {L"%,d", -1},

      // cFlags("+ 0#"") are only valid for numeric conversions.
      {L"%+d", CJS_Util::DataType::kInt},
      {L"%+x", CJS_Util::DataType::kInt},
      {L"%+f", CJS_Util::DataType::kDouble},
      // {L"%+s", -1},
      {L"% d", CJS_Util::DataType::kInt},
      {L"% x", CJS_Util::DataType::kInt},
      {L"% f", CJS_Util::DataType::kDouble},
      // {L"% s", -1},
      {L"%0d", CJS_Util::DataType::kInt},
      {L"%0x", CJS_Util::DataType::kInt},
      {L"%0f", CJS_Util::DataType::kDouble},
      // {L"%0s", -1},
      {L"%#d", CJS_Util::DataType::kInt},
      {L"%#x", CJS_Util::DataType::kInt},
      {L"%#f", CJS_Util::DataType::kDouble},
      // {L"%#s", -1},

      // nWidth should work. for all conversions, can be combined with cFlags=0
      // for numbers.
      {L"%5d", CJS_Util::DataType::kInt},
      {L"%05d", CJS_Util::DataType::kInt},
      {L"%5x", CJS_Util::DataType::kInt},
      {L"%05x", CJS_Util::DataType::kInt},
      {L"%5f", CJS_Util::DataType::kDouble},
      {L"%05f", CJS_Util::DataType::kDouble},
      {L"%5s", CJS_Util::DataType::kString},
      // {L"%05s", -1},

      // nPrecision should only work for float
      // {L"%.5d", -1},
      // {L"%.5x", -1},
      {L"%.5f", CJS_Util::DataType::kDouble},
      // {L"%.5s", -1},
      // {L"%.14d", -1},
      // {L"%.14x", -1},
      {L"%.14f", CJS_Util::DataType::kDouble},
      // {L"%.14s", -1},
      // {L"%.f", -1},

      // See https://crbug.com/740166
      // nPrecision too large (> 260) causes crashes in Windows.
      // Avoid this by limiting to two digits
      {L"%.1d", CJS_Util::DataType::kInt},
      {L"%.10d", CJS_Util::DataType::kInt},
      {L"%.100d", CJS_Util::DataType::kInvalid},

      // Unexpected characters
      {L"%ad", CJS_Util::DataType::kInvalid},
      {L"%bx", CJS_Util::DataType::kInvalid},
      // {L"%cf", CJS_Util::DataType::kInvalid},
      // {L"%es", CJS_Util::DataType::kInvalid},
      // {L"%gd", CJS_Util::DataType::kInvalid},
      {L"%hx", CJS_Util::DataType::kInvalid},
      // {L"%if", CJS_Util::DataType::kInvalid},
      {L"%js", CJS_Util::DataType::kInvalid},
      {L"%@d", CJS_Util::DataType::kInvalid},
      {L"%~x", CJS_Util::DataType::kInvalid},
      {L"%[f", CJS_Util::DataType::kInvalid},
      {L"%\0s", CJS_Util::DataType::kInvalid},
      {L"%\nd", CJS_Util::DataType::kInvalid},
      {L"%\rx", CJS_Util::DataType::kInvalid},
      // {L"%%f", CJS_Util::DataType::kInvalid},
      // {L"%  s", CJS_Util::DataType::kInvalid},

      // Combine multiple valid components
      {L"%+6d", CJS_Util::DataType::kInt},
      {L"% 7x", CJS_Util::DataType::kInt},
      {L"%#9.3f", CJS_Util::DataType::kDouble},
      {L"%10s", CJS_Util::DataType::kString},
  };

  for (size_t i = 0; i < std::size(cases); i++) {
    WideString input(cases[i].input_string);
    EXPECT_EQ(cases[i].expected, CJS_Util::ParseDataType(&input))
        << cases[i].input_string;
  }
}
