// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_ALGORITHM
#define _FX_ALGORITHM
#define FX_IsOdd(a)	((a) & 1)
#ifdef __cplusplus
extern "C" {
#endif
FX_INT32	FX_Base64EncodeA(FX_LPCBYTE pSrc, FX_INT32 iSrcLen, FX_LPSTR pDst);
FX_INT32	FX_Base64DecodeA(FX_LPCSTR pSrc, FX_INT32 iSrcLen, FX_LPBYTE pDst);
FX_INT32	FX_Base64DecodeW(FX_LPCWSTR pSrc, FX_INT32 iSrcLen, FX_LPBYTE pDst);
FX_BYTE		FX_Hex2Dec(FX_BYTE hexHigh, FX_BYTE hexLow);
FX_INT32	FX_SeparateStringW(FX_LPCWSTR pStr, FX_INT32 iStrLen, FX_WCHAR delimiter, CFX_WideStringArray &pieces);
#ifdef __cplusplus
};
#endif
template<class baseType>
class CFX_DSPATemplate
{
public:
    FX_INT32 Lookup(const baseType &find, const baseType *pArray, FX_INT32 iCount)
    {
        FXSYS_assert(pArray != NULL);
        if (iCount < 1) {
            return -1;
        }
        FX_INT32 iStart = 0, iEnd = iCount - 1, iMid;
        do {
            iMid = (iStart + iEnd) / 2;
            const baseType &v = pArray[iMid];
            if (find == v) {
                return iMid;
            } else if (find < v) {
                iEnd = iMid - 1;
            } else {
                iStart = iMid + 1;
            }
        } while (iStart <= iEnd);
        return -1;
    }
};
#endif
