// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_CODEPAGE_H_
#define CORE_FXCRT_FX_CODEPAGE_H_

#include <stdint.h>

// Prove consistency with incomplete forward definitions.
#include "core/fxcrt/fx_codepage_forward.h"

enum class FX_CodePage : uint16_t {
  kDefANSI = 0,
  kSymbol = 42,
  kMSDOS_US = 437,
  kArabic_ASMO708 = 708,
  kMSDOS_Greek1 = 737,
  kMSDOS_Baltic = 775,
  kMSDOS_WesternEuropean = 850,
  kMSDOS_EasternEuropean = 852,
  kMSDOS_Cyrillic = 855,
  kMSDOS_Turkish = 857,
  kMSDOS_Portuguese = 860,
  kMSDOS_Icelandic = 861,
  kMSDOS_Hebrew = 862,
  kMSDOS_FrenchCanadian = 863,
  kMSDOS_Arabic = 864,
  kMSDOS_Norwegian = 865,
  kMSDOS_Russian = 866,
  kMSDOS_Greek2 = 869,
  kMSDOS_Thai = 874,
  kShiftJIS = 932,
  kChineseSimplified = 936,
  kHangul = 949,
  kChineseTraditional = 950,
  kUTF16LE = 1200,
  kUTF16BE = 1201,
  kMSWin_EasternEuropean = 1250,
  kMSWin_Cyrillic = 1251,
  kMSWin_WesternEuropean = 1252,
  kMSWin_Greek = 1253,
  kMSWin_Turkish = 1254,
  kMSWin_Hebrew = 1255,
  kMSWin_Arabic = 1256,
  kMSWin_Baltic = 1257,
  kMSWin_Vietnamese = 1258,
  kJohab = 1361,
  kMAC_Roman = 10000,
  kMAC_ShiftJIS = 10001,
  kMAC_ChineseTraditional = 10002,
  kMAC_Korean = 10003,
  kMAC_Arabic = 10004,
  kMAC_Hebrew = 10005,
  kMAC_Greek = 10006,
  kMAC_Cyrillic = 10007,
  kMAC_ChineseSimplified = 10008,
  kMAC_Thai = 10021,
  kMAC_EasternEuropean = 10029,
  kMAC_Turkish = 10081,
  kUTF8 = 65001,
  kFailure = 65535,
};

#define FX_CHARSET_ANSI 0
#define FX_CHARSET_Default 1
#define FX_CHARSET_Symbol 2
#define FX_CHARSET_MAC_Roman 77
#define FX_CHARSET_MAC_ShiftJIS 78
#define FX_CHARSET_MAC_Korean 79
#define FX_CHARSET_MAC_ChineseSimplified 80
#define FX_CHARSET_MAC_ChineseTraditional 81
#define FX_CHARSET_MAC_Hebrew 83
#define FX_CHARSET_MAC_Arabic 84
#define FX_CHARSET_MAC_Greek 85
#define FX_CHARSET_MAC_Turkish 86
#define FX_CHARSET_MAC_Thai 87
#define FX_CHARSET_MAC_EasternEuropean 88
#define FX_CHARSET_MAC_Cyrillic 89
#define FX_CHARSET_ShiftJIS 128
#define FX_CHARSET_Hangul 129
#define FX_CHARSET_Johab 130
#define FX_CHARSET_ChineseSimplified 134
#define FX_CHARSET_ChineseTraditional 136
#define FX_CHARSET_MSWin_Greek 161
#define FX_CHARSET_MSWin_Turkish 162
#define FX_CHARSET_MSWin_Vietnamese 163
#define FX_CHARSET_MSWin_Hebrew 177
#define FX_CHARSET_MSWin_Arabic 178
#define FX_CHARSET_MSWin_Baltic 186
#define FX_CHARSET_MSWin_Cyrillic 204
#define FX_CHARSET_Thai 222
#define FX_CHARSET_MSWin_EasternEuropean 238
#define FX_CHARSET_US 254
#define FX_CHARSET_OEM 255

// Hi-bytes to unicode codepoint mapping for various code pages.
struct FX_CharsetUnicodes {
  uint8_t m_Charset;
  const uint16_t* m_pUnicodes;  // Raw, POD struct.
};

extern const FX_CharsetUnicodes g_FX_CharsetUnicodes[8];

FX_CodePage FX_GetACP();
FX_CodePage FX_GetCodePageFromCharset(uint8_t charset);
uint8_t FX_GetCharsetFromCodePage(FX_CodePage codepage);
bool FX_CharSetIsCJK(uint8_t uCharset);
int FX_WideCharToMultiByte(FX_CodePage codepage,
                           uint32_t dwFlags,
                           const wchar_t* wstr,
                           int wlen,
                           char* buf,
                           int buflen,
                           const char* default_str,
                           int* pUseDefault);
int FX_MultiByteToWideChar(FX_CodePage codepage,
                           uint32_t dwFlags,
                           const char* bstr,
                           int blen,
                           wchar_t* buf,
                           int buflen);

#endif  // CORE_FXCRT_FX_CODEPAGE_H_
