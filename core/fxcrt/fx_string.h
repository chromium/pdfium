// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_STRING_H_
#define CORE_FXCRT_FX_STRING_H_

#include <stdint.h>

#include <vector>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/widestring.h"

#define FXBSTR_ID(c1, c2, c3, c4)                                      \
  (((uint32_t)c1 << 24) | ((uint32_t)c2 << 16) | ((uint32_t)c3 << 8) | \
   ((uint32_t)c4))

ByteString FX_UTF8Encode(WideStringView wsStr);
WideString FX_UTF8Decode(ByteStringView bsStr);

float StringToFloat(ByteStringView str);
float StringToFloat(WideStringView wsStr);
size_t FloatToString(float f, char* buf);

namespace fxcrt {

template <typename StrType>
std::vector<StrType> Split(const StrType& that, typename StrType::CharType ch) {
  std::vector<StrType> result;
  StringViewTemplate<typename StrType::CharType> remaining(that.span());
  while (1) {
    Optional<size_t> index = remaining.Find(ch);
    if (!index.has_value())
      break;
    result.emplace_back(remaining.Left(index.value()));
    remaining = remaining.Right(remaining.GetLength() - index.value() - 1);
  }
  result.emplace_back(remaining);
  return result;
}

}  // namespace fxcrt

#endif  // CORE_FXCRT_FX_STRING_H_
