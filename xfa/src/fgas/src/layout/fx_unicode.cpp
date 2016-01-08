// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/fgas/src/fgas_base.h"
#include "fx_unicode.h"
void FX_TEXTLAYOUT_PieceSort(CFX_TPOArray& tpos, int32_t iStart, int32_t iEnd) {
  FXSYS_assert(iStart > -1 && iStart < tpos.GetSize());
  FXSYS_assert(iEnd > -1 && iEnd < tpos.GetSize());
  if (iStart >= iEnd) {
    return;
  }
  int32_t i = iStart, j = iEnd;
  FX_TPO *pCur = tpos.GetPtrAt(iStart), *pSort;
  int32_t v = pCur->pos;
  while (i < j) {
    while (j > i) {
      pSort = tpos.GetPtrAt(j);
      if (pSort->pos < v) {
        FX_TPO t = *pSort;
        *pSort = *pCur;
        *pCur = t;
        pCur = pSort;
        break;
      }
      j--;
    }
    while (i < j) {
      pSort = tpos.GetPtrAt(i);
      if (pSort->pos > v) {
        FX_TPO t = *pSort;
        *pSort = *pCur;
        *pCur = t;
        pCur = pSort;
        break;
      }
      i++;
    }
  }
  i--, j++;
  if (iStart < i) {
    FX_TEXTLAYOUT_PieceSort(tpos, iStart, i);
  }
  if (j < iEnd) {
    FX_TEXTLAYOUT_PieceSort(tpos, j, iEnd);
  }
}
static const FX_JAPCHARPROPERTYEX gs_FX_JapCharPropertysEx[] = {
    {0x3001, 0x13}, {0x3002, 0x13}, {0x3041, 0x23}, {0x3043, 0x23},
    {0x3045, 0x23}, {0x3047, 0x23}, {0x3049, 0x23}, {0x3063, 0x23},
    {0x3083, 0x23}, {0x3085, 0x23}, {0x3087, 0x23}, {0x308E, 0x23},
    {0x3095, 0x23}, {0x3096, 0x23}, {0x30A1, 0x23}, {0x30A3, 0x23},
    {0x30A5, 0x23}, {0x30A7, 0x23}, {0x30A9, 0x23}, {0x30C3, 0x23},
    {0x30E3, 0x23}, {0x30E5, 0x23}, {0x30E7, 0x23}, {0x30EE, 0x23},
    {0x30F5, 0x23}, {0x30F6, 0x23}, {0x30FB, 0x22}, {0x31F0, 0x23},
    {0x31F1, 0x23}, {0x31F2, 0x23}, {0x31F3, 0x23}, {0x31F4, 0x23},
    {0x31F5, 0x23}, {0x31F6, 0x23}, {0x31F7, 0x23}, {0x31F8, 0x23},
    {0x31F9, 0x23}, {0x31FA, 0x23}, {0x31FB, 0x23}, {0x31FC, 0x23},
    {0x31FD, 0x23}, {0x31FE, 0x23}, {0x31FF, 0x23},
};
FX_LPCJAPCHARPROPERTYEX FX_GetJapCharPropertyEx(FX_WCHAR wch) {
  int32_t iStart = 0;
  int32_t iEnd =
      sizeof(gs_FX_JapCharPropertysEx) / sizeof(FX_JAPCHARPROPERTYEX);
  while (iStart <= iEnd) {
    int32_t iMid = (iStart + iEnd) / 2;
    FX_WCHAR wJapChar = gs_FX_JapCharPropertysEx[iMid].wChar;
    if (wch == wJapChar) {
      return gs_FX_JapCharPropertysEx + iMid;
    } else if (wch < wJapChar) {
      iEnd = iMid - 1;
    } else {
      iStart = iMid + 1;
    }
  }
  return NULL;
}
FX_BOOL FX_AdjustJapCharDisplayPos(FX_WCHAR wch,
                                   FX_BOOL bMBCSCode,
                                   IFX_Font* pFont,
                                   FX_FLOAT fFontSize,
                                   FX_BOOL bVertical,
                                   CFX_PointF& ptOffset) {
  if (pFont == NULL || !bVertical) {
    return FALSE;
  }
  if (wch < 0x3001 || wch > 0x31FF) {
    return FALSE;
  }
  FX_LPCJAPCHARPROPERTYEX pJapChar = FX_GetJapCharPropertyEx(wch);
  if (pJapChar == NULL) {
    return FALSE;
  }
  CFX_Rect rtBBox;
  rtBBox.Reset();
  if (pFont->GetCharBBox(wch, rtBBox, bMBCSCode)) {
    switch (pJapChar->uAlign & 0xF0) {
      case FX_JAPCHARPROPERTYEX_Top:
        ptOffset.y = fFontSize * (1000 - rtBBox.height) / 1200.0f;
        break;
      case FX_JAPCHARPROPERTYEX_Middle:
        ptOffset.y = fFontSize * (1000 - rtBBox.height) / 6000.0f;
        break;
    }
    switch (pJapChar->uAlign & 0x0F) {
      case FX_JAPCHARPROPERTYEX_Center:
        ptOffset.x = fFontSize * (600 - rtBBox.right()) / 1000.0f;
        break;
      case FX_JAPCHARPROPERTYEX_Right:
        ptOffset.x = fFontSize * (950 - rtBBox.right()) / 1000.0f;
        break;
    }
  }
  return TRUE;
}
