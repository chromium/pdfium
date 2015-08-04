// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FGAS_FONTUTIL_H_
#define _FGAS_FONTUTIL_H_
typedef struct _FGAS_FONTUSB {
  FX_WCHAR wStartUnicode;
  FX_WCHAR wEndUnicode;
  FX_WORD wBitField;
  FX_WORD wCodePage;
} FGAS_FONTUSB, *FGAS_LPFONTUSB;
typedef FGAS_FONTUSB const* FGAS_LPCFONTUSB;
FX_DWORD FGAS_GetFontHashCode(FX_WORD wCodePage, FX_DWORD dwFontStyles);
FX_DWORD FGAS_GetFontFamilyHash(const FX_WCHAR* pszFontFamily,
                                FX_DWORD dwFontStyles,
                                FX_WORD wCodePage);
FGAS_LPCFONTUSB FGAS_GetUnicodeBitField(FX_WCHAR wUnicode);
#endif
