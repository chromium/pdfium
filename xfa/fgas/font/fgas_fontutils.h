// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_FONT_FGAS_FONTUTILS_H_
#define XFA_FGAS_FONT_FGAS_FONTUTILS_H_

#include "core/include/fxcrt/fx_string.h"

struct FGAS_FONTUSB {
  uint16_t wStartUnicode;
  uint16_t wEndUnicode;
  uint16_t wBitField;
  uint16_t wCodePage;
};

FX_DWORD FGAS_GetFontHashCode(uint16_t wCodePage, FX_DWORD dwFontStyles);
FX_DWORD FGAS_GetFontFamilyHash(const FX_WCHAR* pszFontFamily,
                                FX_DWORD dwFontStyles,
                                uint16_t wCodePage);
const FGAS_FONTUSB* FGAS_GetUnicodeBitField(FX_WCHAR wUnicode);

#endif  // XFA_FGAS_FONT_FGAS_FONTUTILS_H_
