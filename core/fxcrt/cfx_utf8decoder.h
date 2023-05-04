// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_UTF8DECODER_H_
#define CORE_FXCRT_CFX_UTF8DECODER_H_

#include "core/fxcrt/string_view_template.h"
#include "core/fxcrt/widestring.h"

class CFX_UTF8Decoder {
 public:
  explicit CFX_UTF8Decoder(ByteStringView input);
  ~CFX_UTF8Decoder();

  WideString TakeResult();

 private:
  void AppendCodePoint(char32_t code_point);

  WideString buffer_;
};

#endif  // CORE_FXCRT_CFX_UTF8DECODER_H_
