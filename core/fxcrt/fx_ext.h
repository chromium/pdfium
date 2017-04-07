// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_EXT_H_
#define CORE_FXCRT_FX_EXT_H_

#include <cctype>
#include <cwctype>
#include <memory>

#include "core/fxcrt/fx_basic.h"

#define FX_INVALID_OFFSET static_cast<uint32_t>(-1)

float FXSYS_strtof(const char* pcsStr,
                   int32_t iLength = -1,
                   int32_t* pUsedLen = nullptr);
float FXSYS_wcstof(const wchar_t* pwsStr,
                   int32_t iLength = -1,
                   int32_t* pUsedLen = nullptr);
wchar_t* FXSYS_wcsncpy(wchar_t* dstStr, const wchar_t* srcStr, size_t count);
int32_t FXSYS_wcsnicmp(const wchar_t* s1, const wchar_t* s2, size_t count);
int32_t FXSYS_strnicmp(const char* s1, const char* s2, size_t count);

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
  return (wch >= L'A' && wch <= L'Z') || (wch >= L'a' && wch <= L'z');
}

inline bool FXSYS_iswdigit(wchar_t wch) {
  return wch >= L'0' && wch <= L'9';
}

inline bool FXSYS_iswalnum(wchar_t wch) {
  return FXSYS_iswalpha(wch) || FXSYS_iswdigit(wch);
}

inline bool FXSYS_iswspace(wchar_t c) {
  return (c == 0x20) || (c == 0x0d) || (c == 0x0a) || (c == 0x09);
}

inline int FXSYS_toHexDigit(const char c) {
  if (!std::isxdigit(c))
    return 0;
  char upchar = std::toupper(c);
  return upchar > '9' ? upchar - 'A' + 10 : upchar - '0';
}

inline bool FXSYS_isDecimalDigit(const char c) {
  return !!std::isdigit(c);
}

inline bool FXSYS_isDecimalDigit(const wchar_t c) {
  return !!std::iswdigit(c);
}

inline int FXSYS_toDecimalDigit(const char c) {
  return std::isdigit(c) ? c - '0' : 0;
}

inline int FXSYS_toDecimalDigit(const wchar_t c) {
  return std::iswdigit(c) ? c - L'0' : 0;
}

float FXSYS_FractionalScale(size_t scale_factor, int value);
int FXSYS_FractionalScaleCount();

void* FX_Random_MT_Start(uint32_t dwSeed);
void FX_Random_MT_Close(void* pContext);
uint32_t FX_Random_MT_Generate(void* pContext);
void FX_Random_GenerateBase(uint32_t* pBuffer, int32_t iCount);
void FX_Random_GenerateMT(uint32_t* pBuffer, int32_t iCount);
void FX_Random_GenerateCrypto(uint32_t* pBuffer, int32_t iCount);

#ifdef PDF_ENABLE_XFA
struct FX_GUID {
  uint32_t data1;
  uint16_t data2;
  uint16_t data3;
  uint8_t data4[8];
};
void FX_GUID_CreateV4(FX_GUID* pGUID);
CFX_ByteString FX_GUID_ToString(const FX_GUID* pGUID, bool bSeparator = true);
#endif  // PDF_ENABLE_XFA

#endif  // CORE_FXCRT_FX_EXT_H_
