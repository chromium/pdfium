// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_SYSTEM
#define _FX_SYSTEM
#ifdef __cplusplus
extern "C" {
#endif
#define FX_RAD2DEG(r) ((r)*180.0f / FX_PI)
#define FX_DEG2RAD(a) ((a)*FX_PI / 180.0f)
typedef int8_t* FX_LPINT8;
typedef int8_t const* FX_LPCINT8;
typedef int32_t* FX_LPINT32;
typedef int32_t const* FX_LPCINT32;
typedef long FX_LONG;
typedef FX_LONG* FX_LPLONG;
typedef FX_LONG const* FX_LPCLONG;
typedef FX_FLOAT const* FX_LPCFLOAT;
typedef double FX_DOUBLE;
typedef FX_DOUBLE* FX_LPDOUBLE;
typedef FX_DOUBLE const* FX_LPCDOUBLE;
FX_FLOAT FX_tan(FX_FLOAT a);
FX_FLOAT FX_log(FX_FLOAT b, FX_FLOAT x);
FX_FLOAT FX_strtof(const FX_CHAR* pcsStr,
                   int32_t iLength = -1,
                   int32_t* pUsedLen = NULL);
FX_FLOAT FX_wcstof(const FX_WCHAR* pwsStr,
                   int32_t iLength = -1,
                   int32_t* pUsedLen = NULL);
FX_WCHAR* FX_wcsncpy(FX_WCHAR* dstStr, const FX_WCHAR* srcStr, size_t count);
int32_t FX_wcsnicmp(const FX_WCHAR* s1, const FX_WCHAR* s2, size_t count);
int32_t FX_strnicmp(const FX_CHAR* s1, const FX_CHAR* s2, size_t count);
inline FX_BOOL FX_islower(int32_t ch) {
  return ch >= 'a' && ch <= 'z';
}
inline FX_BOOL FX_isupper(int32_t ch) {
  return ch >= 'A' && ch <= 'Z';
}
inline int32_t FX_tolower(int32_t ch) {
  return FX_isupper(ch) ? (ch + 0x20) : ch;
}
inline int32_t FX_toupper(int32_t ch) {
  return FX_islower(ch) ? (ch - 0x20) : ch;
}
int32_t FX_filelength(FXSYS_FILE* file);
FX_BOOL FX_fsetsize(FXSYS_FILE* file, int32_t size);
void FX_memset(void* pBuf, int32_t iValue, size_t size);
void FX_memcpy(void* pDst, const void* pSrc, size_t size);
FX_BOOL FX_IsRelativePath(const CFX_WideStringC& wsPath);
FX_BOOL FX_JoinPath(const CFX_WideStringC& wsBasePath,
                    const CFX_WideStringC& wsRelativePath,
                    CFX_WideString& wsAbsolutePath);
typedef struct _FX_VERSION {
  FX_DWORD dwMajorVersion;
  FX_DWORD dwMinorVersion;
  FX_DWORD dwBuildVersion;
} FX_VERSION, *FX_LPVERSION;
typedef FX_VERSION const* FX_LPCVERSION;
#ifdef __cplusplus
};
#endif
#endif
