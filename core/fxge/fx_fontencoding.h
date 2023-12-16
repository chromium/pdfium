// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXGE_FX_FONTENCODING_H_
#define CORE_FXGE_FX_FONTENCODING_H_

#include <stdint.h>

#include "core/fxcrt/fx_string.h"

namespace fxge {

// The values here match FreeType FT_Encoding values with similar names.
enum class FontEncoding : uint32_t {
  kAdobeCustom = FXBSTR_ID('A', 'D', 'B', 'C'),
  kAdobeExpert = FXBSTR_ID('A', 'D', 'B', 'E'),
  kAdobeStandard = FXBSTR_ID('A', 'D', 'O', 'B'),
  kAppleRoman = FXBSTR_ID('a', 'r', 'm', 'n'),
  kBig5 = FXBSTR_ID('b', 'i', 'g', '5'),
  kGB2312 = FXBSTR_ID('g', 'b', ' ', ' '),
  kJohab = FXBSTR_ID('j', 'o', 'h', 'a'),
  kLatin1 = FXBSTR_ID('l', 'a', 't', '1'),
  kOldLatin2 = FXBSTR_ID('l', 'a', 't', '2'),
  kSjis = FXBSTR_ID('s', 'j', 'i', 's'),
  kSymbol = FXBSTR_ID('s', 'y', 'm', 'b'),
  kUnicode = FXBSTR_ID('u', 'n', 'i', 'c'),
  kWansung = FXBSTR_ID('w', 'a', 'n', 's'),
};

}  // namespace fxge

#endif  // CORE_FXGE_FX_FONTENCODING_H_
