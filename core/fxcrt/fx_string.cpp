// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <limits>
#include <vector>

#include "core/fxcrt/cfx_utf8decoder.h"
#include "core/fxcrt/cfx_utf8encoder.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_string.h"
#include "third_party/base/compiler_specific.h"

ByteString FX_UTF8Encode(const WideStringView& wsStr) {
  size_t len = wsStr.GetLength();
  const wchar_t* pStr = wsStr.unterminated_c_str();
  CFX_UTF8Encoder encoder;
  while (len-- > 0)
    encoder.Input(*pStr++);

  return ByteString(encoder.GetResult());
}

WideString FX_UTF8Decode(const ByteStringView& bsStr) {
  if (bsStr.IsEmpty())
    return WideString();

  CFX_UTF8Decoder decoder;
  for (size_t i = 0; i < bsStr.GetLength(); i++)
    decoder.Input(bsStr[i]);

  return WideString(decoder.GetResult());
}

namespace {

const float fraction_scales[] = {0.1f,          0.01f,         0.001f,
                                 0.0001f,       0.00001f,      0.000001f,
                                 0.0000001f,    0.00000001f,   0.000000001f,
                                 0.0000000001f, 0.00000000001f};

float FractionalScale(size_t scale_factor, int value) {
  return fraction_scales[scale_factor] * value;
}

}  // namespace

bool FX_atonum(const ByteStringView& strc, void* pData) {
  if (strc.Contains('.')) {
    float* pFloat = static_cast<float*>(pData);
    *pFloat = FX_atof(strc);
    return false;
  }

  // Note, numbers in PDF are typically of the form 123, -123, etc. But,
  // for things like the Permissions on the encryption hash the number is
  // actually an unsigned value. We use a uint32_t so we can deal with the
  // unsigned and then check for overflow if the user actually signed the value.
  // The Permissions flag is listed in Table 3.20 PDF 1.7 spec.
  pdfium::base::CheckedNumeric<uint32_t> integer = 0;
  bool bNegative = false;
  bool bSigned = false;
  size_t cc = 0;
  if (strc[0] == '+') {
    cc++;
    bSigned = true;
  } else if (strc[0] == '-') {
    bNegative = true;
    bSigned = true;
    cc++;
  }

  while (cc < strc.GetLength() && std::isdigit(strc[cc])) {
    integer = integer * 10 + FXSYS_DecimalCharToInt(strc.CharAt(cc));
    if (!integer.IsValid())
      break;
    cc++;
  }

  // We have a sign, and the value was greater then a regular integer
  // we've overflowed, reset to the default value.
  if (bSigned) {
    if (bNegative) {
      if (integer.ValueOrDefault(0) >
          static_cast<uint32_t>(std::numeric_limits<int>::max()) + 1) {
        integer = 0;
      }
    } else if (integer.ValueOrDefault(0) >
               static_cast<uint32_t>(std::numeric_limits<int>::max())) {
      integer = 0;
    }
  }

  // Switch back to the int space so we can flip to a negative if we need.
  uint32_t uValue = integer.ValueOrDefault(0);
  int32_t value = static_cast<int>(uValue);
  if (bNegative) {
    // |value| is usually positive, except in the corner case of "-2147483648",
    // where |uValue| is 2147483648. When it gets casted to an int, |value|
    // becomes -2147483648. For this case, avoid undefined behavior, because an
    // integer cannot represent 2147483648.
    static constexpr int kMinInt = std::numeric_limits<int>::min();
    value = LIKELY(value != kMinInt) ? -value : kMinInt;
  }

  int* pInt = static_cast<int*>(pData);
  *pInt = value;
  return true;
}

float FX_atof(const ByteStringView& strc) {
  if (strc.IsEmpty())
    return 0.0;

  int cc = 0;
  bool bNegative = false;
  int len = strc.GetLength();
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
  float value = 0;
  while (cc < len) {
    if (strc[cc] == '.')
      break;
    value = value * 10 + FXSYS_DecimalCharToInt(strc.CharAt(cc));
    cc++;
  }
  int scale = 0;
  if (cc < len && strc[cc] == '.') {
    cc++;
    while (cc < len) {
      value += FractionalScale(scale, FXSYS_DecimalCharToInt(strc.CharAt(cc)));
      scale++;
      if (scale == FX_ArraySize(fraction_scales))
        break;
      cc++;
    }
  }
  return bNegative ? -value : value;
}

float FX_atof(const WideStringView& wsStr) {
  return FX_atof(FX_UTF8Encode(wsStr).c_str());
}

size_t FX_ftoa(float d, char* buf) {
  buf[0] = '0';
  buf[1] = '\0';
  if (d == 0.0f) {
    return 1;
  }
  bool bNegative = false;
  if (d < 0) {
    bNegative = true;
    d = -d;
  }
  int scale = 1;
  int scaled = FXSYS_round(d);
  while (scaled < 100000) {
    if (scale == 1000000) {
      break;
    }
    scale *= 10;
    scaled = FXSYS_round(d * scale);
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
  memcpy(buf + buf_size, buf2, len);
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
