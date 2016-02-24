// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_SRC_FGAS_INCLUDE_FX_UCD_H_
#define XFA_SRC_FGAS_INCLUDE_FX_UCD_H_

#define FX_JAPCHARPROPERTYEX_Left 0x01
#define FX_JAPCHARPROPERTYEX_Center 0x02
#define FX_JAPCHARPROPERTYEX_Right 0x03
#define FX_JAPCHARPROPERTYEX_Top 0x10
#define FX_JAPCHARPROPERTYEX_Middle 0x20
#define FX_JAPCHARPROPERTYEX_Bottom 0x30

struct FX_JAPCHARPROPERTYEX {
  FX_WCHAR wChar;
  uint8_t uAlign;
};

const FX_JAPCHARPROPERTYEX* FX_GetJapCharPropertyEx(FX_WCHAR wch);
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

#endif  // XFA_SRC_FGAS_INCLUDE_FX_UCD_H_
