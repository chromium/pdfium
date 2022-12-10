// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_string.h"

#include <iterator>

#include "core/fxcrt/cfx_utf8decoder.h"
#include "core/fxcrt/cfx_utf8encoder.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/span_util.h"
#include "third_party/base/compiler_specific.h"
#include "third_party/base/span.h"

ByteString FX_UTF8Encode(WideStringView wsStr) {
  CFX_UTF8Encoder encoder;
  for (size_t i = 0; i < wsStr.GetLength(); ++i)
    encoder.Input(wsStr[i]);

  return ByteString(encoder.GetResult());
}

WideString FX_UTF8Decode(ByteStringView bsStr) {
  CFX_UTF8Decoder decoder(bsStr);
  return decoder.TakeResult();
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
