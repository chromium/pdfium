// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdftext/fpdf_text.h"
extern const FX_WCHAR g_UnicodeData_Normalization[];
extern const FX_WCHAR g_UnicodeData_Normalization_Map1[];
extern const FX_WCHAR g_UnicodeData_Normalization_Map2[];
extern const FX_WCHAR g_UnicodeData_Normalization_Map3[];
extern const FX_WCHAR g_UnicodeData_Normalization_Map4[];
FX_LPCWSTR g_UnicodeData_Normalization_Maps[5] = {
    NULL,
    g_UnicodeData_Normalization_Map1,
    g_UnicodeData_Normalization_Map2,
    g_UnicodeData_Normalization_Map3,
    g_UnicodeData_Normalization_Map4
};
FX_STRSIZE FX_Unicode_GetNormalization(FX_WCHAR wch, FX_LPWSTR pDst)
{
    wch = wch & 0xFFFF;
    FX_WCHAR wFind = g_UnicodeData_Normalization[wch];
    if (!wFind) {
        if (pDst) {
            *pDst = wch;
        }
        return 1;
    }
    if(wFind >= 0x8000) {
        wch = wFind - 0x8000;
        wFind = 1;
    } else {
        wch = wFind & 0x0FFF;
        wFind >>= 12;
    }
    FX_LPCWSTR pMap = g_UnicodeData_Normalization_Maps[wFind];
    if (pMap == g_UnicodeData_Normalization_Map4) {
        pMap = g_UnicodeData_Normalization_Map4 + wch;
        wFind = (FX_WCHAR)(*pMap ++);
    } else {
        pMap += wch;
    }
    if (pDst) {
        FX_WCHAR n = wFind;
        while (n --) {
            *pDst ++ = *pMap ++;
        }
    }
    return (FX_STRSIZE)wFind;
}
FX_STRSIZE FX_WideString_GetNormalization(FX_WSTR wsSrc, FX_LPWSTR pDst)
{
    FX_STRSIZE nCount = 0;
    for (FX_STRSIZE len = 0; len < wsSrc.GetLength(); len ++) {
        FX_WCHAR wch = wsSrc.GetAt(len);
        if(pDst) {
            nCount += FX_Unicode_GetNormalization(wch, pDst + nCount);
        } else {
            nCount += FX_Unicode_GetNormalization(wch, pDst);
        }
    }
    return nCount;
}
FX_STRSIZE FX_WideString_GetNormalization(FX_WSTR wsSrc, CFX_WideString &wsDst)
{
    FX_STRSIZE nLen = FX_WideString_GetNormalization(wsSrc, (FX_LPWSTR)NULL);
    if (!nLen) {
        return 0;
    }
    FX_LPWSTR pBuf = wsDst.GetBuffer(nLen);
    FX_WideString_GetNormalization(wsSrc, pBuf);
    wsDst.ReleaseBuffer(nLen);
    return nLen;
}
