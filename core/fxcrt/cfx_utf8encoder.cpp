// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_utf8encoder.h"

#include <stdint.h>

CFX_UTF8Encoder::CFX_UTF8Encoder() = default;

CFX_UTF8Encoder::~CFX_UTF8Encoder() = default;

void CFX_UTF8Encoder::Input(wchar_t code_unit) {
  char32_t code_point = static_cast<char32_t>(code_unit);
  if (code_point > 0x10ffff) {
    // Invalid code point above U+10FFFF.
    return;
  }

  if (code_point < 0x80) {
    // 7-bit code points are unchanged in UTF-8.
    buffer_.push_back(code_point);
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
  buffer_.push_back(kPrefix[byte_size - 2] | (code_point / order));
  for (int i = 0; i < byte_size - 1; i++) {
    code_point = code_point % order;
    order >>= 6;
    buffer_.push_back(0x80 | (code_point / order));
  }
}
