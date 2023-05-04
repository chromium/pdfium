// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_UTF8ENCODER_H_
#define CORE_FXCRT_CFX_UTF8ENCODER_H_

#include "build/build_config.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/string_view_template.h"

class CFX_UTF8Encoder {
 public:
  CFX_UTF8Encoder();
  ~CFX_UTF8Encoder();

  // `code_unit` may be UTF-16 or UTF-32, depending on the platform.
  // TODO(crbug.com/pdfium/2031): Accept `char16_t` instead of `wchar_t`.
  void Input(wchar_t code_unit);

  // The data returned by `GetResult()` is invalidated when this is modified by
  // appending any data.
  ByteStringView GetResult() const {
    return ByteStringView(buffer_.data(), buffer_.size());
  }

 private:
  void AppendCodePoint(char32_t code_point);

  DataVector<char> buffer_;

#if defined(WCHAR_T_IS_UTF16)
  char16_t high_surrogate_ = 0;
#endif  // defined(WCHAR_T_IS_UTF16)
};

#endif  // CORE_FXCRT_CFX_UTF8ENCODER_H_
