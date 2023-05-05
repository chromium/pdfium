// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_UTF8ENCODER_H_
#define CORE_FXCRT_CFX_UTF8ENCODER_H_

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/string_view_template.h"

class CFX_UTF8Encoder {
 public:
  // `input` may be UTF-16 or UTF-32, depending on the platform.
  // TODO(crbug.com/pdfium/2031): Always use UTF-16.
  explicit CFX_UTF8Encoder(WideStringView input);
  ~CFX_UTF8Encoder();

  ByteString TakeResult();

 private:
  void AppendCodePoint(char32_t code_point);

  ByteString buffer_;
};

#endif  // CORE_FXCRT_CFX_UTF8ENCODER_H_
