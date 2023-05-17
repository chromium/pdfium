// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_utf8decoder.h"

#include <stdint.h>

#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/string_view_template.h"
#include "core/fxcrt/utf16.h"
#include "core/fxcrt/widestring.h"

CFX_UTF8Decoder::CFX_UTF8Decoder(ByteStringView input) {
  int remaining = 0;
  char32_t code_point = 0;

  for (char byte : input) {
    uint8_t code_unit = static_cast<uint8_t>(byte);
    if (code_unit < 0x80) {
      remaining = 0;
      AppendCodePoint(code_unit);
    } else if (code_unit < 0xc0) {
      if (remaining > 0) {
        --remaining;
        code_point = (code_point << 6) | (code_unit & 0x3f);
        if (remaining == 0) {
          AppendCodePoint(code_point);
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
}

CFX_UTF8Decoder::~CFX_UTF8Decoder() = default;

WideString CFX_UTF8Decoder::TakeResult() {
  return std::move(buffer_);
}

void CFX_UTF8Decoder::AppendCodePoint(char32_t code_point) {
  if (code_point > pdfium::kMaximumSupplementaryCodePoint) {
    // Invalid code point above U+10FFFF.
    return;
  }

#if defined(WCHAR_T_IS_UTF16)
  if (code_point < pdfium::kMinimumSupplementaryCodePoint) {
    buffer_ += static_cast<wchar_t>(code_point);
  } else {
    // Encode as UTF-16 surrogate pair.
    pdfium::SurrogatePair surrogate_pair(code_point);
    buffer_ += surrogate_pair.high();
    buffer_ += surrogate_pair.low();
  }
#else
  buffer_ += static_cast<wchar_t>(code_point);
#endif  // defined(WCHAR_T_IS_UTF16)
}
