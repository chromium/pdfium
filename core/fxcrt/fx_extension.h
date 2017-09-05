// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_EXTENSION_H_
#define CORE_FXCRT_FX_EXTENSION_H_

#include <cctype>
#include <cwctype>
#include <memory>

#include "core/fxcrt/fx_string.h"

#define FX_INVALID_OFFSET static_cast<uint32_t>(-1)

#ifdef PDF_ENABLE_XFA
#define FX_IsOdd(a) ((a)&1)
#endif  // PDF_ENABLE_XFA

float FXSYS_wcstof(const wchar_t* pwsStr,
                   int32_t iLength = -1,
                   int32_t* pUsedLen = nullptr);
wchar_t* FXSYS_wcsncpy(wchar_t* dstStr, const wchar_t* srcStr, size_t count);
int32_t FXSYS_wcsnicmp(const wchar_t* s1, const wchar_t* s2, size_t count);

inline bool FXSYS_islower(int32_t ch) {
  return ch >= 'a' && ch <= 'z';
}

inline bool FXSYS_isupper(int32_t ch) {
  return ch >= 'A' && ch <= 'Z';
}

inline int32_t FXSYS_tolower(int32_t ch) {
  return ch < 'A' || ch > 'Z' ? ch : (ch + 0x20);
}

inline int32_t FXSYS_toupper(int32_t ch) {
  return ch < 'a' || ch > 'z' ? ch : (ch - 0x20);
}

inline bool FXSYS_iswalpha(wchar_t wch) {
  return FXSYS_isupper(wch) || FXSYS_islower(wch);
}

inline bool FXSYS_iswalnum(wchar_t wch) {
  return FXSYS_iswalpha(wch) || std::iswdigit(wch);
}

inline bool FXSYS_iswspace(wchar_t c) {
  return (c == 0x20) || (c == 0x0d) || (c == 0x0a) || (c == 0x09);
}

inline bool FXSYS_isHexDigit(const char c) {
  return !((c & 0x80) || !std::isxdigit(c));
}

inline int FXSYS_HexCharToInt(const char c) {
  if (!FXSYS_isHexDigit(c))
    return 0;
  char upchar = std::toupper(c);
  return upchar > '9' ? upchar - 'A' + 10 : upchar - '0';
}

inline bool FXSYS_isDecimalDigit(const char c) {
  return !((c & 0x80) || !std::isdigit(c));
}

inline bool FXSYS_isDecimalDigit(const wchar_t c) {
  return !!std::iswdigit(c);
}

inline int FXSYS_DecimalCharToInt(const char c) {
  return FXSYS_isDecimalDigit(c) ? c - '0' : 0;
}

inline int FXSYS_DecimalCharToInt(const wchar_t c) {
  return std::iswdigit(c) ? c - L'0' : 0;
}

void FXSYS_IntToTwoHexChars(uint8_t c, char* buf);

void FXSYS_IntToFourHexChars(uint16_t c, char* buf);

size_t FXSYS_ToUTF16BE(uint32_t unicode, char* buf);

uint32_t GetBits32(const uint8_t* pData, int bitpos, int nbits);

#endif  // CORE_FXCRT_FX_EXTENSION_H_
