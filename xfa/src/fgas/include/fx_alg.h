// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_ALGORITHM
#define _FX_ALGORITHM
#define FX_IsOdd(a) ((a)&1)
#ifdef __cplusplus
extern "C" {
#endif
int32_t FX_Base64EncodeA(const uint8_t* pSrc, int32_t iSrcLen, FX_CHAR* pDst);
int32_t FX_Base64DecodeA(const FX_CHAR* pSrc, int32_t iSrcLen, uint8_t* pDst);
int32_t FX_Base64DecodeW(const FX_WCHAR* pSrc, int32_t iSrcLen, uint8_t* pDst);
uint8_t FX_Hex2Dec(uint8_t hexHigh, uint8_t hexLow);
int32_t FX_SeparateStringW(const FX_WCHAR* pStr,
                           int32_t iStrLen,
                           FX_WCHAR delimiter,
                           CFX_WideStringArray& pieces);
#ifdef __cplusplus
};
#endif
template <class baseType>
class CFX_DSPATemplate {
 public:
  int32_t Lookup(const baseType& find, const baseType* pArray, int32_t iCount) {
    FXSYS_assert(pArray != NULL);
    if (iCount < 1) {
      return -1;
    }
    int32_t iStart = 0, iEnd = iCount - 1, iMid;
    do {
      iMid = (iStart + iEnd) / 2;
      const baseType& v = pArray[iMid];
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
