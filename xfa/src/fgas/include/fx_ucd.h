// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FGAS_UNICODE_
#define _FGAS_UNICODE_
#define FX_JAPCHARPROPERTYEX_Left 0x01
#define FX_JAPCHARPROPERTYEX_Center 0x02
#define FX_JAPCHARPROPERTYEX_Right 0x03
#define FX_JAPCHARPROPERTYEX_Top 0x10
#define FX_JAPCHARPROPERTYEX_Middle 0x20
#define FX_JAPCHARPROPERTYEX_Bottom 0x30
typedef struct _FX_JAPCHARPROPERTYEX {
  FX_WCHAR wChar;
  uint8_t uAlign;
} FX_JAPCHARPROPERTYEX, *FX_LPJAPCHARPROPERTYEX;
typedef FX_JAPCHARPROPERTYEX const* FX_LPCJAPCHARPROPERTYEX;
FX_LPCJAPCHARPROPERTYEX FX_GetJapCharPropertyEx(FX_WCHAR wch);
typedef FX_BOOL (*FX_AdjustCharDisplayPos)(FX_WCHAR wch,
                                           FX_BOOL bMBCSCode,
                                           IFX_Font* pFont,
                                           FX_FLOAT fFontSize,
                                           FX_BOOL bVertical,
                                           CFX_PointF& ptOffset);
FX_BOOL FX_AdjustJapCharDisplayPos(FX_WCHAR wch,
                                   FX_BOOL bMBCSCode,
                                   IFX_Font* pFont,
                                   FX_FLOAT fFontSize,
                                   FX_BOOL bVertical,
                                   CFX_PointF& ptOffset);
#endif
