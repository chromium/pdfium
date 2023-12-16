// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/fx_fontencoding.h"

#include "core/fxge/freetype/fx_freetype.h"

namespace fxge {

static_assert(static_cast<uint32_t>(FontEncoding::kAdobeCustom) ==
                  FT_ENCODING_ADOBE_CUSTOM,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kAdobeExpert) ==
                  FT_ENCODING_ADOBE_EXPERT,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kAdobeStandard) ==
                  FT_ENCODING_ADOBE_STANDARD,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kAppleRoman) ==
                  FT_ENCODING_APPLE_ROMAN,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kBig5) == FT_ENCODING_BIG5,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kGB2312) == FT_ENCODING_PRC,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kJohab) == FT_ENCODING_JOHAB,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kLatin1) ==
                  FT_ENCODING_ADOBE_LATIN_1,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kOldLatin2) ==
                  FT_ENCODING_OLD_LATIN_2,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kSjis) == FT_ENCODING_SJIS,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kSymbol) ==
                  FT_ENCODING_MS_SYMBOL,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kUnicode) ==
                  FT_ENCODING_UNICODE,
              "wrong encoding");
static_assert(static_cast<uint32_t>(FontEncoding::kWansung) ==
                  FT_ENCODING_WANSUNG,
              "wrong encoding");

}  // namespace fxge
