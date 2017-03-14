// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_ext.h"
#include "xfa/fgas/crt/fgas_codepage.h"
#include "xfa/fgas/crt/fgas_language.h"

namespace {

struct FX_CHARSET_MAP {
  uint16_t charset;
  uint16_t codepage;
};

const FX_CHARSET_MAP g_FXCharset2CodePageTable[] = {
    {FX_CHARSET_ANSI, FX_CODEPAGE_MSWin_WesternEuropean},
    {FX_CHARSET_Default, FX_CODEPAGE_DefANSI},
    {FX_CHARSET_Symbol, FX_CODEPAGE_Symbol},
    {FX_CHARSET_MAC_Roman, FX_CODEPAGE_MAC_Roman},
    {FX_CHARSET_MAC_ShiftJIS, FX_CODEPAGE_MAC_ShiftJIS},
    {FX_CHARSET_MAC_Korean, FX_CODEPAGE_MAC_Korean},
    {FX_CHARSET_MAC_ChineseSimplified, FX_CODEPAGE_MAC_ChineseSimplified},
    {FX_CHARSET_MAC_ChineseTriditional, FX_CODEPAGE_MAC_ChineseTraditional},
    {FX_CHARSET_MAC_Hebrew, FX_CODEPAGE_MAC_Hebrew},
    {FX_CHARSET_MAC_Arabic, FX_CODEPAGE_MAC_Arabic},
    {FX_CHARSET_MAC_Greek, FX_CODEPAGE_MAC_Greek},
    {FX_CHARSET_MAC_Turkish, FX_CODEPAGE_MAC_Turkish},
    {FX_CHARSET_MAC_Thai, FX_CODEPAGE_MAC_Thai},
    {FX_CHARSET_MAC_EasternEuropean, FX_CODEPAGE_MAC_EasternEuropean},
    {FX_CHARSET_MAC_Cyrillic, FX_CODEPAGE_MAC_Cyrillic},
    {FX_CHARSET_ShiftJIS, FX_CODEPAGE_ShiftJIS},
    {FX_CHARSET_Korean, FX_CODEPAGE_Korean},
    {FX_CHARSET_Johab, FX_CODEPAGE_Johab},
    {FX_CHARSET_ChineseSimplified, FX_CODEPAGE_ChineseSimplified},
    {FX_CHARSET_ChineseTriditional, FX_CODEPAGE_ChineseTraditional},
    {FX_CHARSET_MSWin_Greek, FX_CODEPAGE_MSWin_Greek},
    {FX_CHARSET_MSWin_Turkish, FX_CODEPAGE_MSWin_Turkish},
    {FX_CHARSET_MSWin_Vietnamese, FX_CODEPAGE_MSWin_Vietnamese},
    {FX_CHARSET_MSWin_Hebrew, FX_CODEPAGE_MSWin_Hebrew},
    {FX_CHARSET_MSWin_Arabic, FX_CODEPAGE_MSWin_Arabic},
    {FX_CHARSET_MSWin_Baltic, FX_CODEPAGE_MSWin_Baltic},
    {FX_CHARSET_MSWin_Cyrillic, FX_CODEPAGE_MSWin_Cyrillic},
    {FX_CHARSET_Thai, FX_CODEPAGE_MSDOS_Thai},
    {FX_CHARSET_MSWin_EasterEuropean, FX_CODEPAGE_MSWin_EasternEuropean},
    {FX_CHARSET_US, FX_CODEPAGE_MSDOS_US},
    {FX_CHARSET_OEM, FX_CODEPAGE_MSDOS_WesternEuropean},
};

}  // namespace

uint16_t FX_GetCodePageFromCharset(uint8_t charset) {
  int32_t iEnd = sizeof(g_FXCharset2CodePageTable) / sizeof(FX_CHARSET_MAP) - 1;
  ASSERT(iEnd >= 0);

  int32_t iStart = 0, iMid;
  do {
    iMid = (iStart + iEnd) / 2;
    const FX_CHARSET_MAP& cp = g_FXCharset2CodePageTable[iMid];
    if (charset == cp.charset)
      return cp.codepage;
    if (charset < cp.charset)
      iEnd = iMid - 1;
    else
      iStart = iMid + 1;
  } while (iStart <= iEnd);
  return 0xFFFF;
}

void FX_SwapByteOrder(wchar_t* pStr, int32_t iLength) {
  ASSERT(pStr);

  if (iLength < 0)
    iLength = FXSYS_wcslen(pStr);

  uint16_t wch;
  if (sizeof(wchar_t) > 2) {
    while (iLength-- > 0) {
      wch = (uint16_t)*pStr;
      wch = (wch >> 8) | (wch << 8);
      wch &= 0x00FF;
      *pStr++ = wch;
    }
    return;
  }

  while (iLength-- > 0) {
    wch = (uint16_t)*pStr;
    wch = (wch >> 8) | (wch << 8);
    *pStr++ = wch;
  }
}

void FX_UTF16ToWChar(void* pBuffer, int32_t iLength) {
  ASSERT(pBuffer && iLength > 0);
  if (sizeof(wchar_t) == 2)
    return;

  uint16_t* pSrc = static_cast<uint16_t*>(pBuffer);
  wchar_t* pDst = static_cast<wchar_t*>(pBuffer);
  while (--iLength >= 0)
    pDst[iLength] = static_cast<wchar_t>(pSrc[iLength]);
}

int32_t FX_DecodeString(uint16_t wCodePage,
                        const char* pSrc,
                        int32_t* pSrcLen,
                        wchar_t* pDst,
                        int32_t* pDstLen,
                        bool bErrBreak) {
  if (wCodePage == FX_CODEPAGE_UTF8)
    return FX_UTF8Decode(pSrc, pSrcLen, pDst, pDstLen);
  return -1;
}

int32_t FX_UTF8Decode(const char* pSrc,
                      int32_t* pSrcLen,
                      wchar_t* pDst,
                      int32_t* pDstLen) {
  if (!pSrcLen || !pDstLen)
    return -1;

  int32_t iSrcLen = *pSrcLen;
  if (iSrcLen < 1) {
    *pSrcLen = *pDstLen = 0;
    return 1;
  }

  int32_t iDstLen = *pDstLen;
  bool bValidDst = (pDst && iDstLen > 0);
  uint32_t dwCode = 0;
  int32_t iPending = 0;
  int32_t iSrcNum = 0;
  int32_t iDstNum = 0;
  int32_t iIndex = 0;
  int32_t k = 1;
  while (iIndex < iSrcLen) {
    uint8_t byte = static_cast<uint8_t>(*(pSrc + iIndex));
    if (byte < 0x80) {
      iPending = 0;
      k = 1;
      iDstNum++;
      iSrcNum += k;
      if (bValidDst) {
        *pDst++ = byte;
        if (iDstNum >= iDstLen)
          break;
      }
    } else if (byte < 0xc0) {
      if (iPending < 1)
        break;

      iPending--;
      dwCode |= (byte & 0x3f) << (iPending * 6);
      if (iPending == 0) {
        iDstNum++;
        iSrcNum += k;
        if (bValidDst) {
          *pDst++ = dwCode;
          if (iDstNum >= iDstLen)
            break;
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
