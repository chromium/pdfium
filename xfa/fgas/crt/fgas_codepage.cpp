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
