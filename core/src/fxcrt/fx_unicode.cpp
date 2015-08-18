// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_ucd.h"

FX_DWORD FX_GetUnicodeProperties(FX_WCHAR wch) {
  size_t idx = static_cast<size_t>(wch);
  if (idx < kTextLayoutCodePropertiesSize)
    return kTextLayoutCodeProperties[(FX_WORD)wch];
  return 0;
}

FX_WCHAR FX_GetMirrorChar(FX_WCHAR wch, FX_BOOL bRTL, FX_BOOL bVertical) {
  FX_DWORD dwProps = FX_GetUnicodeProperties(wch);
  FX_DWORD dwTemp = (dwProps & 0xFF800000);
  if (bRTL && dwTemp < 0xFF800000) {
    size_t idx = dwTemp >> 23;
    if (idx < kFXTextLayoutBidiMirrorSize) {
      wch = kFXTextLayoutBidiMirror[idx];
      dwProps = FX_GetUnicodeProperties(wch);
    }
  }
  if (bVertical) {
    dwTemp = (dwProps & 0x007E0000);
    if (dwTemp < 0x007E0000) {
      size_t idx = dwTemp >> 17;
      if (idx < kFXTextLayoutVerticalMirrorSize)
        wch = kFXTextLayoutVerticalMirror[idx];
    }
  }
  return wch;
}
