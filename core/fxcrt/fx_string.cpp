// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_string.h"

#include <stdint.h>

#include <iterator>

#include "build/build_config.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/code_point_view.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/string_view_template.h"
#include "core/fxcrt/utf16.h"
#include "core/fxcrt/widestring.h"
#include "third_party/base/compiler_specific.h"
#include "third_party/base/containers/span.h"

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

  static constexpr uint8_t kPrefix[] = {0xc0, 0xe0, 0xf0};
  int order = 1 << ((byte_size - 1) * 6);
  buffer += kPrefix[byte_size - 2] | (code_point / order);
  for (int i = 0; i < byte_size - 1; i++) {
    code_point = code_point % order;
    order >>= 6;
    buffer += 0x80 | (code_point / order);
  }
}

// Appends a Unicode code point to a `WideString` using either UTF-16 or UTF-32,
// depending on the platform's definition of `wchar_t`.
//
// TODO(crbug.com/pdfium/2031): Always use UTF-16.
// TODO(crbug.com/pdfium/2041): Migrate to `WideString`.
void AppendCodePointToWideString(char32_t code_point, WideString& buffer) {
  if (code_point > pdfium::kMaximumSupplementaryCodePoint) {
    // Invalid code point above U+10FFFF.
    return;
  }

#if defined(WCHAR_T_IS_UTF16)
  if (code_point < pdfium::kMinimumSupplementaryCodePoint) {
    buffer += static_cast<wchar_t>(code_point);
  } else {
    // Encode as UTF-16 surrogate pair.
    pdfium::SurrogatePair surrogate_pair(code_point);
    buffer += surrogate_pair.high();
    buffer += surrogate_pair.low();
  }
#else
  buffer += static_cast<wchar_t>(code_point);
#endif  // defined(WCHAR_T_IS_UTF16)
}

}  // namespace

ByteString FX_UTF8Encode(WideStringView wsStr) {
  ByteString buffer;
  for (char32_t code_point : pdfium::CodePointView(wsStr)) {
    AppendCodePointToByteString(code_point, buffer);
  }
  return buffer;
}

WideString FX_UTF8Decode(ByteStringView bsStr) {
  WideString buffer;

  int remaining = 0;
  char32_t code_point = 0;
  for (char byte : bsStr) {
    uint8_t code_unit = static_cast<uint8_t>(byte);
    if (code_unit < 0x80) {
      remaining = 0;
      AppendCodePointToWideString(code_unit, buffer);
    } else if (code_unit < 0xc0) {
      if (remaining > 0) {
        --remaining;
        code_point = (code_point << 6) | (code_unit & 0x3f);
        if (remaining == 0) {
          AppendCodePointToWideString(code_point, buffer);
        }
      }
    } else if (code_unit < 0xe0) {
      remaining = 1;
      code_point = code_unit & 0x1f;
    } else if (code_unit < 0xf0) {
      remaining = 2;
      code_point = code_unit & 0x0f;
    } else if (code_unit < 0xf8) {
      remaining = 3;
      code_point = code_unit & 0x07;
    } else {
      remaining = 0;
    }
  }

  return buffer;
}

namespace {

constexpr float kFractionScalesFloat[] = {
    0.1f,         0.01f,         0.001f,        0.0001f,
    0.00001f,     0.000001f,     0.0000001f,    0.00000001f,
    0.000000001f, 0.0000000001f, 0.00000000001f};

const double kFractionScalesDouble[] = {
    0.1,       0.01,       0.001,       0.0001,       0.00001,      0.000001,
    0.0000001, 0.00000001, 0.000000001, 0.0000000001, 0.00000000001};

template <class T>
T StringTo(ByteStringView strc, pdfium::span<const T> fractional_scales) {
  if (strc.IsEmpty())
    return 0;

  bool bNegative = false;
  size_t cc = 0;
  size_t len = strc.GetLength();
  if (strc[0] == '+') {
    cc++;
  } else if (strc[0] == '-') {
    bNegative = true;
    cc++;
  }
  while (cc < len) {
    if (strc[cc] != '+' && strc[cc] != '-')
      break;
    cc++;
  }
  T value = 0;
  while (cc < len) {
    if (strc[cc] == '.')
      break;
    value = value * 10 + FXSYS_DecimalCharToInt(strc.CharAt(cc));
    cc++;
  }
  size_t scale = 0;
  if (cc < len && strc[cc] == '.') {
    cc++;
    while (cc < len) {
      value +=
          fractional_scales[scale] * FXSYS_DecimalCharToInt(strc.CharAt(cc));
      scale++;
      if (scale == fractional_scales.size())
        break;
      cc++;
    }
  }
  return bNegative ? -value : value;
}

template <class T>
size_t ToString(T value, int (*round_func)(T), pdfium::span<char> buf) {
  buf[0] = '0';
  buf[1] = '\0';
  if (value == 0) {
    return 1;
  }
  bool bNegative = false;
  if (value < 0) {
    bNegative = true;
    value = -value;
  }
  int scale = 1;
  int scaled = round_func(value);
  while (scaled < 100000) {
    if (scale == 1000000) {
      break;
    }
    scale *= 10;
    scaled = round_func(value * scale);
  }
  if (scaled == 0) {
    return 1;
  }
  char buf2[32];
  size_t buf_size = 0;
  if (bNegative) {
    buf[buf_size++] = '-';
  }
  int i = scaled / scale;
  FXSYS_itoa(i, buf2, 10);
  size_t len = strlen(buf2);
  fxcrt::spancpy(buf.subspan(buf_size), pdfium::make_span(buf2).first(len));
  buf_size += len;
  int fraction = scaled % scale;
  if (fraction == 0) {
    return buf_size;
  }
  buf[buf_size++] = '.';
  scale /= 10;
  while (fraction) {
    buf[buf_size++] = '0' + fraction / scale;
    fraction %= scale;
    scale /= 10;
  }
  return buf_size;
}

}  // namespace

float StringToFloat(ByteStringView strc) {
  return StringTo<float>(strc, kFractionScalesFloat);
}

float StringToFloat(WideStringView wsStr) {
  return StringToFloat(FX_UTF8Encode(wsStr).AsStringView());
}

size_t FloatToString(float f, pdfium::span<char> buf) {
  return ToString<float>(f, FXSYS_roundf, buf);
}

double StringToDouble(ByteStringView strc) {
  return StringTo<double>(strc, kFractionScalesDouble);
}

double StringToDouble(WideStringView wsStr) {
  return StringToDouble(FX_UTF8Encode(wsStr).AsStringView());
}

size_t DoubleToString(double d, pdfium::span<char> buf) {
  return ToString<double>(d, FXSYS_round, buf);
}

namespace fxcrt {

template std::vector<ByteString> Split<ByteString>(const ByteString& that,
                                                   ByteString::CharType ch);
template std::vector<WideString> Split<WideString>(const WideString& that,
                                                   WideString::CharType ch);

}  // namespace fxcrt
