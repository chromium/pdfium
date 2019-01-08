// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_unicode.h"

#include "core/fxcrt/fx_ucddata.h"

namespace {

wchar_t GetMirrorChar(wchar_t wch, uint32_t dwProps) {
  uint32_t dwTemp = (dwProps & kMirrorBitMask);
  if (dwTemp == kMirrorBitMask)
    return wch;
  size_t idx = dwTemp >> kMirrorBitPos;
  return idx < kFXTextLayoutBidiMirrorSize ? kFXTextLayoutBidiMirror[idx] : wch;
}

}  // namespace

uint32_t FX_GetUnicodeProperties(wchar_t wch) {
  size_t idx = static_cast<size_t>(wch);
  if (idx < kTextLayoutCodePropertiesSize)
    return kTextLayoutCodeProperties[idx];
  return 0;
}

wchar_t FX_GetMirrorChar(wchar_t wch) {
  return GetMirrorChar(wch, FX_GetUnicodeProperties(wch));
}

#ifdef PDF_ENABLE_XFA
static_assert(FX_CHARTYPEBITS == kCharTypeBitPos, "positions must match");

FX_CHARTYPE GetCharTypeFromProp(uint32_t prop) {
  uint32_t result = (prop & kCharTypeBitMask);
  ASSERT(result <= FX_CHARTYPE_Arabic);
  return static_cast<FX_CHARTYPE>(result);
}

uint32_t GetBreakPropertyFromProp(uint32_t prop) {
  uint32_t result = (prop & kBreakTypeBitMask) >> kBreakTypeBitPos;
  ASSERT(result <= kBreakPropertyTB);
  return result;
}

wchar_t FX_GetMirrorChar(wchar_t wch, uint32_t dwProps) {
  return GetMirrorChar(wch, dwProps);
}
#endif  // PDF_ENABLE_XFA
