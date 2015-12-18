// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_EXT_H_
#define CORE_INCLUDE_FXCRT_FX_EXT_H_

#include <cctype>
#include <cwctype>

#include "core/include/fxcrt/fx_basic.h"

FX_FLOAT FXSYS_tan(FX_FLOAT a);
FX_FLOAT FXSYS_logb(FX_FLOAT b, FX_FLOAT x);
FX_FLOAT FXSYS_strtof(const FX_CHAR* pcsStr,
                      int32_t iLength = -1,
                      int32_t* pUsedLen = NULL);
FX_FLOAT FXSYS_wcstof(const FX_WCHAR* pwsStr,
                      int32_t iLength = -1,
                      int32_t* pUsedLen = NULL);
FX_WCHAR* FXSYS_wcsncpy(FX_WCHAR* dstStr, const FX_WCHAR* srcStr, size_t count);
int32_t FXSYS_wcsnicmp(const FX_WCHAR* s1, const FX_WCHAR* s2, size_t count);
int32_t FXSYS_strnicmp(const FX_CHAR* s1, const FX_CHAR* s2, size_t count);

inline FX_BOOL FXSYS_islower(int32_t ch) {
  return ch >= 'a' && ch <= 'z';
}
inline FX_BOOL FXSYS_isupper(int32_t ch) {
  return ch >= 'A' && ch <= 'Z';
}
inline int32_t FXSYS_tolower(int32_t ch) {
  return ch < 'A' || ch > 'Z' ? ch : (ch + 0x20);
}
inline int32_t FXSYS_toupper(int32_t ch) {
  return ch < 'a' || ch > 'z' ? ch : (ch - 0x20);
}
inline FX_BOOL FXSYS_iswalpha(wchar_t wch) {
  return (wch >= L'A' && wch <= L'Z') || (wch >= L'a' && wch <= L'z');
}
inline FX_BOOL FXSYS_iswdigit(wchar_t wch) {
  return wch >= L'0' && wch <= L'9';
}
inline FX_BOOL FXSYS_iswalnum(wchar_t wch) {
  return FXSYS_iswalpha(wch) || FXSYS_iswdigit(wch);
}

inline int FXSYS_toHexDigit(const FX_CHAR c) {
  if (!std::isxdigit(c))
    return 0;
  char upchar = std::toupper(c);
  return upchar > '9' ? upchar - 'A' + 10 : upchar - '0';
}

inline int FXSYS_toDecimalDigit(const FX_CHAR c) {
  if (!std::isdigit(c))
    return 0;
  return c - '0';
}

inline int FXSYS_toDecimalDigitWide(const FX_WCHAR c) {
  if (!std::iswdigit(c))
    return 0;
  return c - L'0';
}

FX_DWORD FX_HashCode_String_GetA(const FX_CHAR* pStr,
                                 int32_t iLength,
                                 FX_BOOL bIgnoreCase = FALSE);
FX_DWORD FX_HashCode_String_GetW(const FX_WCHAR* pStr,
                                 int32_t iLength,
                                 FX_BOOL bIgnoreCase = FALSE);

void* FX_Random_MT_Start(FX_DWORD dwSeed);

FX_DWORD FX_Random_MT_Generate(void* pContext);

void FX_Random_MT_Close(void* pContext);

void FX_Random_GenerateBase(FX_DWORD* pBuffer, int32_t iCount);

void FX_Random_GenerateMT(FX_DWORD* pBuffer, int32_t iCount);

void FX_Random_GenerateCrypto(FX_DWORD* pBuffer, int32_t iCount);

#ifdef PDF_ENABLE_XFA
typedef struct FX_GUID {
  FX_DWORD data1;
  FX_WORD data2;
  FX_WORD data3;
  uint8_t data4[8];
} FX_GUID, *FX_LPGUID;
typedef FX_GUID const* FX_LPCGUID;
void FX_GUID_CreateV4(FX_LPGUID pGUID);
void FX_GUID_ToString(FX_LPCGUID pGUID,
                      CFX_ByteString& bsStr,
                      FX_BOOL bSeparator = TRUE);
#endif  // PDF_ENABLE_XFA

template <class baseType>
class CFX_SSortTemplate {
 public:
  void ShellSort(baseType* pArray, int32_t iCount) {
    FXSYS_assert(pArray && iCount > 0);
    int32_t i, j, gap;
    baseType v1, v2;
    gap = iCount >> 1;
    while (gap > 0) {
      for (i = gap; i < iCount; i++) {
        j = i - gap;
        v1 = pArray[i];
        while (j > -1 && (v2 = pArray[j]) > v1) {
          pArray[j + gap] = v2;
          j -= gap;
        }
        pArray[j + gap] = v1;
      }
      gap >>= 1;
    }
  }
};

#endif  // CORE_INCLUDE_FXCRT_FX_EXT_H_
