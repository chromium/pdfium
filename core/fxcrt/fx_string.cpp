// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_string.h"

#include <stdint.h>

#include <array>
#include <string>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/code_point_view.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/utf16.h"
#include "core/fxcrt/widestring.h"
#include "third_party/fast_float/src/include/fast_float/fast_float.h"

#if !defined(WCHAR_T_IS_16_BIT) && !defined(WCHAR_T_IS_32_BIT)
#error "Unknown wchar_t size"
#endif
#if defined(WCHAR_T_IS_16_BIT) && defined(WCHAR_T_IS_32_BIT)
#error "Conflicting wchar_t sizes"
#endif

namespace {

// Appends a Unicode code point to a `ByteString` using UTF-8.
//
// TODO(crbug.com/pdfium/2041): Migrate to `ByteString`.
void AppendCodePointToByteString(char32_t code_point, ByteString& buffer) {
  if (code_point > pdfium::kMaximumSupplementaryCodePoint) {
    // Invalid code point above U+10FFFF.
    return;
  }

  if (code_point < 0x80) {
    // 7-bit code points are unchanged in UTF-8.
    buffer += code_point;
    return;
  }

  int byte_size;
  if (code_point < 0x800) {
    byte_size = 2;
  } else if (code_point < 0x10000) {
    byte_size = 3;
  } else {
    byte_size = 4;
  }

  static constexpr std::array<uint8_t, 3> kPrefix = {{0xc0, 0xe0, 0xf0}};
  int order = 1 << ((byte_size - 1) * 6);
  buffer += kPrefix[byte_size - 2] | (code_point / order);
  for (int i = 0; i < byte_size - 1; i++) {
    code_point = code_point % order;
    order >>= 6;
    buffer += 0x80 | (code_point / order);
  }
}

}  // namespace

ByteString FX_UTF8Encode(WideStringView wsStr) {
  ByteString buffer;
  for (char32_t code_point : pdfium::CodePointView(wsStr)) {
    AppendCodePointToByteString(code_point, buffer);
  }
  return buffer;
}

std::u16string FX_UTF16Encode(WideStringView wsStr) {
  if (wsStr.IsEmpty()) {
    return {};
  }

  std::u16string result;
  result.reserve(wsStr.GetLength());

  for (wchar_t c : wsStr) {
#if defined(WCHAR_T_IS_32_BIT)
    if (pdfium::IsSupplementary(c)) {
      pdfium::SurrogatePair pair(c);
      result.push_back(pair.high());
      result.push_back(pair.low());
      continue;
    }
#endif  // defined(WCHAR_T_IS_32_BIT)
    result.push_back(c);
  }

  return result;
}

namespace {

template <class T>
T StringTo(ByteStringView strc) {
  // Skip leading whitespaces.
  size_t start = 0;
  size_t len = strc.GetLength();
  while (start < len && strc[start] == ' ') {
    ++start;
  }

  // Skip a leading '+' sign.
  if (start < len && strc[start] == '+') {
    ++start;
  }

  ByteStringView sub_strc = strc.Substr(start, len - start);

  T value;
  auto result = fast_float::from_chars(sub_strc.begin(), sub_strc.end(), value);

  // Return 0 for parsing errors. Some examples of errors are an empty string
  // and a string that cannot be converted to T.
  return result.ec == std::errc() || result.ec == std::errc::result_out_of_range
             ? value
             : 0;
}

}  // namespace

float StringToFloat(ByteStringView strc) {
  return StringTo<float>(strc);
}

float StringToFloat(WideStringView wsStr) {
  return StringToFloat(FX_UTF8Encode(wsStr).AsStringView());
}

double StringToDouble(ByteStringView strc) {
  return StringTo<double>(strc);
}

double StringToDouble(WideStringView wsStr) {
  return StringToDouble(FX_UTF8Encode(wsStr).AsStringView());
}

namespace fxcrt {

template std::vector<ByteString> Split<ByteString>(const ByteString& that,
                                                   ByteString::CharType ch);
template std::vector<WideString> Split<WideString>(const WideString& that,
                                                   WideString::CharType ch);

}  // namespace fxcrt
