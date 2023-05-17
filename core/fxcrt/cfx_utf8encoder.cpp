// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_utf8encoder.h"

#include <stdint.h>

#include <utility>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/code_point_view.h"
#include "core/fxcrt/string_view_template.h"
#include "core/fxcrt/utf16.h"

CFX_UTF8Encoder::CFX_UTF8Encoder(WideStringView input) {
  for (char32_t code_point : pdfium::CodePointView(input)) {
    AppendCodePoint(code_point);
  }
}

CFX_UTF8Encoder::~CFX_UTF8Encoder() = default;

ByteString CFX_UTF8Encoder::TakeResult() {
  return std::move(buffer_);
}

void CFX_UTF8Encoder::AppendCodePoint(char32_t code_point) {
  if (code_point > pdfium::kMaximumSupplementaryCodePoint) {
    // Invalid code point above U+10FFFF.
    return;
  }

  if (code_point < 0x80) {
    // 7-bit code points are unchanged in UTF-8.
    buffer_ += code_point;
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
  buffer_ += kPrefix[byte_size - 2] | (code_point / order);
  for (int i = 0; i < byte_size - 1; i++) {
    code_point = code_point % order;
    order >>= 6;
    buffer_ += 0x80 | (code_point / order);
  }
}
