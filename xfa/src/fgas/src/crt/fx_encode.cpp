// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/fgas/src/fgas_base.h"
void FX_SwapByteOrder(FX_WCHAR* pStr, int32_t iLength) {
  FXSYS_assert(pStr != NULL);
  if (iLength < 0) {
    iLength = FXSYS_wcslen(pStr);
  }
  FX_WORD wch;
  if (sizeof(FX_WCHAR) > 2) {
    while (iLength-- > 0) {
      wch = (FX_WORD)*pStr;
      wch = (wch >> 8) | (wch << 8);
      wch &= 0x00FF;
      *pStr++ = wch;
    }
  } else {
    while (iLength-- > 0) {
      wch = (FX_WORD)*pStr;
      wch = (wch >> 8) | (wch << 8);
      *pStr++ = wch;
    }
  }
}
void FX_SwapByteOrderCopy(const FX_WCHAR* pSrc,
                          FX_WCHAR* pDst,
                          int32_t iLength) {
  FXSYS_assert(pSrc != NULL && pDst != NULL);
  if (iLength < 0) {
    iLength = FXSYS_wcslen(pSrc);
  }
  FX_WORD wch;
  if (sizeof(FX_WCHAR) > 2) {
    while (iLength-- > 0) {
      wch = (FX_WORD)*pSrc++;
      wch = (wch >> 8) | (wch << 8);
      wch &= 0x00FF;
      *pDst++ = wch;
    }
  } else {
    while (iLength-- > 0) {
      wch = (FX_WORD)*pSrc++;
      wch = (wch >> 8) | (wch << 8);
      *pDst++ = wch;
    }
  }
}
void FX_UTF16ToWChar(void* pBuffer, int32_t iLength) {
  FXSYS_assert(pBuffer != NULL && iLength > 0);
  if (sizeof(FX_WCHAR) == 2) {
    return;
  }
  FX_WORD* pSrc = (FX_WORD*)pBuffer;
  FX_WCHAR* pDst = (FX_WCHAR*)pBuffer;
  while (--iLength >= 0) {
    pDst[iLength] = (FX_WCHAR)pSrc[iLength];
  }
}
void FX_UTF16ToWCharCopy(const FX_WORD* pUTF16,
                         FX_WCHAR* pWChar,
                         int32_t iLength) {
  FXSYS_assert(pUTF16 != NULL && pWChar != NULL && iLength > 0);
  if (sizeof(FX_WCHAR) == 2) {
    FXSYS_memcpy(pWChar, pUTF16, iLength * sizeof(FX_WCHAR));
  } else {
    while (--iLength >= 0) {
      pWChar[iLength] = (FX_WCHAR)pUTF16[iLength];
    }
  }
}
void FX_WCharToUTF16(void* pBuffer, int32_t iLength) {
  FXSYS_assert(pBuffer != NULL && iLength > 0);
  if (sizeof(FX_WCHAR) == 2) {
    return;
  }
  const FX_WCHAR* pSrc = (const FX_WCHAR*)pBuffer;
  FX_WORD* pDst = (FX_WORD*)pBuffer;
  while (--iLength >= 0) {
    *pDst++ = (FX_WORD)*pSrc++;
  }
}
void FX_WCharToUTF16Copy(const FX_WCHAR* pWChar,
                         FX_WORD* pUTF16,
                         int32_t iLength) {
  FXSYS_assert(pWChar != NULL && pUTF16 != NULL && iLength > 0);
  if (sizeof(FX_WCHAR) == 2) {
    FXSYS_memcpy(pUTF16, pWChar, iLength * sizeof(FX_WCHAR));
  } else {
    while (--iLength >= 0) {
      *pUTF16++ = (FX_WORD)*pWChar++;
    }
  }
}
inline FX_DWORD FX_DWordFromBytes(const uint8_t* pStr) {
  return FXBSTR_ID(pStr[3], pStr[2], pStr[1], pStr[0]);
}
inline FX_WORD FX_WordFromBytes(const uint8_t* pStr) {
  return (pStr[1] << 8 | pStr[0]);
}
int32_t FX_DecodeString(FX_WORD wCodePage,
                        const FX_CHAR* pSrc,
                        int32_t* pSrcLen,
                        FX_WCHAR* pDst,
                        int32_t* pDstLen,
                        FX_BOOL bErrBreak) {
  if (wCodePage == FX_CODEPAGE_UTF8) {
    return FX_UTF8Decode(pSrc, pSrcLen, pDst, pDstLen);
  }
  return -1;
}
int32_t FX_UTF8Decode(const FX_CHAR* pSrc,
                      int32_t* pSrcLen,
                      FX_WCHAR* pDst,
                      int32_t* pDstLen) {
  if (pSrcLen == NULL || pDstLen == NULL) {
    return -1;
  }
  int32_t iSrcLen = *pSrcLen;
  if (iSrcLen < 1) {
    *pSrcLen = *pDstLen = 0;
    return 1;
  }
  int32_t iDstLen = *pDstLen;
  FX_BOOL bValidDst = (pDst != NULL && iDstLen > 0);
  FX_DWORD dwCode = 0;
  int32_t iPending = 0;
  int32_t iSrcNum = 0, iDstNum = 0;
  int32_t k = 0;
  int32_t iIndex = 0;
  k = 1;
  while (iIndex < iSrcLen) {
    uint8_t byte = (uint8_t) * (pSrc + iIndex);
    if (byte < 0x80) {
      iPending = 0;
      k = 1;
      iDstNum++;
      iSrcNum += k;
      if (bValidDst) {
        *pDst++ = byte;
        if (iDstNum >= iDstLen) {
          break;
        }
      }
    } else if (byte < 0xc0) {
      if (iPending < 1) {
        break;
      }
      iPending--;
      dwCode |= (byte & 0x3f) << (iPending * 6);
      if (iPending == 0) {
        iDstNum++;
        iSrcNum += k;
        if (bValidDst) {
          *pDst++ = dwCode;
          if (iDstNum >= iDstLen) {
            break;
          }
        }
      }
    } else if (byte < 0xe0) {
      iPending = 1;
      k = 2;
      dwCode = (byte & 0x1f) << 6;
    } else if (byte < 0xf0) {
      iPending = 2;
      k = 3;
      dwCode = (byte & 0x0f) << 12;
    } else if (byte < 0xf8) {
      iPending = 3;
      k = 4;
      dwCode = (byte & 0x07) << 18;
    } else if (byte < 0xfc) {
      iPending = 4;
      k = 5;
      dwCode = (byte & 0x03) << 24;
    } else if (byte < 0xfe) {
      iPending = 5;
      k = 6;
      dwCode = (byte & 0x01) << 30;
    } else {
      break;
    }
    iIndex++;
  }
  *pSrcLen = iSrcNum;
  *pDstLen = iDstNum;
  return 1;
}
