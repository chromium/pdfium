// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/fgas/src/fgas_base.h"
#ifdef __cplusplus
extern "C" {
#endif
const static FX_CHAR g_FXBase64EncoderMap[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
};
typedef struct _FX_BASE64DATA {
  FX_DWORD data1 : 2;
  FX_DWORD data2 : 6;
  FX_DWORD data3 : 4;
  FX_DWORD data4 : 4;
  FX_DWORD data5 : 6;
  FX_DWORD data6 : 2;
  FX_DWORD data7 : 8;
} FX_BASE64DATA, *FX_LPBASE64ENCODEDATA;
typedef FX_BASE64DATA const* FX_LPCBASE64DATA;
static void FX_Base64EncodePiece(const FX_BASE64DATA& src,
                                 int32_t iBytes,
                                 FX_CHAR dst[4]) {
  dst[0] = g_FXBase64EncoderMap[src.data2];
  FX_DWORD b = src.data1 << 4;
  if (iBytes > 1) {
    b |= src.data4;
  }
  dst[1] = g_FXBase64EncoderMap[b];
  if (iBytes > 1) {
    b = src.data3 << 2;
    if (iBytes > 2) {
      b |= src.data6;
    }
    dst[2] = g_FXBase64EncoderMap[b];
    if (iBytes > 2) {
      dst[3] = g_FXBase64EncoderMap[src.data5];
    } else {
      dst[3] = '=';
    }
  } else {
    dst[2] = dst[3] = '=';
  }
}
int32_t FX_Base64EncodeA(const uint8_t* pSrc, int32_t iSrcLen, FX_CHAR* pDst) {
  FXSYS_assert(pSrc != NULL);
  if (iSrcLen < 1) {
    return 0;
  }
  if (pDst == NULL) {
    int32_t iDstLen = iSrcLen / 3 * 4;
    if ((iSrcLen % 3) != 0) {
      iDstLen += 4;
    }
    return iDstLen;
  }
  FX_BASE64DATA srcData;
  int32_t iBytes = 3;
  FX_CHAR* pDstEnd = pDst;
  while (iSrcLen > 0) {
    if (iSrcLen > 2) {
      ((uint8_t*)&srcData)[0] = *pSrc++;
      ((uint8_t*)&srcData)[1] = *pSrc++;
      ((uint8_t*)&srcData)[2] = *pSrc++;
      iSrcLen -= 3;
    } else {
      *((FX_DWORD*)&srcData) = 0;
      ((uint8_t*)&srcData)[0] = *pSrc++;
      if (iSrcLen > 1) {
        ((uint8_t*)&srcData)[1] = *pSrc++;
      }
      iBytes = iSrcLen;
      iSrcLen = 0;
    }
    FX_Base64EncodePiece(srcData, iBytes, pDstEnd);
    pDstEnd += 4;
  }
  return pDstEnd - pDst;
}
const static uint8_t g_FXBase64DecoderMap[256] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12,
    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
};
static void FX_Base64DecodePiece(const FX_CHAR src[4],
                                 int32_t iChars,
                                 FX_BASE64DATA& dst,
                                 int32_t& iBytes) {
  FXSYS_assert(iChars > 0 && iChars < 5);
  iBytes = 1;
  dst.data2 = g_FXBase64DecoderMap[(uint8_t)src[0]];
  if (iChars > 1) {
    uint8_t b = g_FXBase64DecoderMap[(uint8_t)src[1]];
    dst.data1 = b >> 4;
    dst.data4 = b;
    if (iChars > 2) {
      iBytes = 2;
      b = g_FXBase64DecoderMap[(uint8_t)src[2]];
      dst.data3 = b >> 2;
      dst.data6 = b;
      if (iChars > 3) {
        iBytes = 3;
        dst.data5 = g_FXBase64DecoderMap[(uint8_t)src[3]];
      } else {
        dst.data5 = 0;
      }
    } else {
      dst.data3 = 0;
    }
  } else {
    dst.data1 = 0;
  }
}
int32_t FX_Base64DecodeA(const FX_CHAR* pSrc, int32_t iSrcLen, uint8_t* pDst) {
  FXSYS_assert(pSrc != NULL);
  if (iSrcLen < 1) {
    return 0;
  }
  while (iSrcLen > 0 && pSrc[iSrcLen - 1] == '=') {
    iSrcLen--;
  }
  if (iSrcLen < 1) {
    return 0;
  }
  if (pDst == NULL) {
    int32_t iDstLen = iSrcLen / 4 * 3;
    iSrcLen %= 4;
    if (iSrcLen == 1) {
      iDstLen += 1;
    } else if (iSrcLen == 2) {
      iDstLen += 1;
    } else if (iSrcLen == 3) {
      iDstLen += 2;
    }
    return iDstLen;
  }
  FX_CHAR srcData[4];
  FX_BASE64DATA dstData;
  int32_t iChars = 4, iBytes;
  uint8_t* pDstEnd = pDst;
  while (iSrcLen > 0) {
    if (iSrcLen > 3) {
      *((FX_DWORD*)srcData) = *((FX_DWORD*)pSrc);
      pSrc += 4;
      iSrcLen -= 4;
    } else {
      *((FX_DWORD*)&dstData) = 0;
      *((FX_DWORD*)srcData) = 0;
      srcData[0] = *pSrc++;
      if (iSrcLen > 1) {
        srcData[1] = *pSrc++;
      }
      if (iSrcLen > 2) {
        srcData[2] = *pSrc++;
      }
      iChars = iSrcLen;
      iSrcLen = 0;
    }
    FX_Base64DecodePiece(srcData, iChars, dstData, iBytes);
    *pDstEnd++ = ((uint8_t*)&dstData)[0];
    if (iBytes > 1) {
      *pDstEnd++ = ((uint8_t*)&dstData)[1];
    }
    if (iBytes > 2) {
      *pDstEnd++ = ((uint8_t*)&dstData)[2];
    }
  }
  return pDstEnd - pDst;
}
int32_t FX_Base64DecodeW(const FX_WCHAR* pSrc, int32_t iSrcLen, uint8_t* pDst) {
  FXSYS_assert(pSrc != NULL);
  if (iSrcLen < 1) {
    return 0;
  }
  while (iSrcLen > 0 && pSrc[iSrcLen - 1] == '=') {
    iSrcLen--;
  }
  if (iSrcLen < 1) {
    return 0;
  }
  if (pDst == NULL) {
    int32_t iDstLen = iSrcLen / 4 * 3;
    iSrcLen %= 4;
    if (iSrcLen == 1) {
      iDstLen += 1;
    } else if (iSrcLen == 2) {
      iDstLen += 1;
    } else if (iSrcLen == 3) {
      iDstLen += 2;
    }
    return iDstLen;
  }
  FX_CHAR srcData[4];
  FX_BASE64DATA dstData;
  int32_t iChars = 4, iBytes;
  uint8_t* pDstEnd = pDst;
  while (iSrcLen > 0) {
    if (iSrcLen > 3) {
      srcData[0] = (FX_CHAR)*pSrc++;
      srcData[1] = (FX_CHAR)*pSrc++;
      srcData[2] = (FX_CHAR)*pSrc++;
      srcData[3] = (FX_CHAR)*pSrc++;
      iSrcLen -= 4;
    } else {
      *((FX_DWORD*)&dstData) = 0;
      *((FX_DWORD*)srcData) = 0;
      srcData[0] = (FX_CHAR)*pSrc++;
      if (iSrcLen > 1) {
        srcData[1] = (FX_CHAR)*pSrc++;
      }
      if (iSrcLen > 2) {
        srcData[2] = (FX_CHAR)*pSrc++;
      }
      iChars = iSrcLen;
      iSrcLen = 0;
    }
    FX_Base64DecodePiece(srcData, iChars, dstData, iBytes);
    *pDstEnd++ = ((uint8_t*)&dstData)[0];
    if (iBytes > 1) {
      *pDstEnd++ = ((uint8_t*)&dstData)[1];
    }
    if (iBytes > 2) {
      *pDstEnd++ = ((uint8_t*)&dstData)[2];
    }
  }
  return pDstEnd - pDst;
}
const static uint8_t g_FXHex2DecMap[256] = {
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  1,  2,  3, 4, 5, 6, 7, 8, 9,  0,  0,
    0,  0,  0,  0, 0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 10, 11, 12,
    13, 14, 15, 0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
};
uint8_t FX_Hex2Dec(uint8_t hexHigh, uint8_t hexLow) {
  return (g_FXHex2DecMap[hexHigh] << 4) + g_FXHex2DecMap[hexLow];
}
int32_t FX_SeparateStringW(const FX_WCHAR* pStr,
                           int32_t iStrLen,
                           FX_WCHAR delimiter,
                           CFX_WideStringArray& pieces) {
  if (pStr == NULL) {
    return 0;
  }
  if (iStrLen < 0) {
    iStrLen = FXSYS_wcslen(pStr);
  }
  const FX_WCHAR* pToken = pStr;
  const FX_WCHAR* pEnd = pStr + iStrLen;
  while (TRUE) {
    if (pStr >= pEnd || delimiter == *pStr) {
      CFX_WideString sub(pToken, pStr - pToken);
      pieces.Add(sub);
      pToken = pStr + 1;
      if (pStr >= pEnd) {
        break;
      }
    }
    pStr++;
  }
  return pieces.GetSize();
}
#ifdef __cplusplus
};
#endif
