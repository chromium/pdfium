// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_SYSTEM_H_
#define CORE_FXCRT_FX_SYSTEM_H_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "build/build_config.h"
#include "core/fxcrt/fx_types.h"

#if defined(_MSC_VER) && _MSC_VER < 1900
#error Sorry, VC++ 2015 or later is required to compile PDFium.
#endif  // defined(_MSC_VER) && _MSC_VER < 1900

#if defined(__wasm__) && defined(PDF_ENABLE_V8)
#error Cannot compile v8 with wasm.
#endif  // PDF_ENABLE_V8

#if defined(OS_WIN)
#include <windows.h>
#endif  // defined(OS_WIN)

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define FXSYS_IsFloatZero(f) ((f) < 0.0001 && (f) > -0.0001)
#define FXSYS_IsFloatBigger(fa, fb) \
  ((fa) > (fb) && !FXSYS_IsFloatZero((fa) - (fb)))
#define FXSYS_IsFloatSmaller(fa, fb) \
  ((fa) < (fb) && !FXSYS_IsFloatZero((fa) - (fb)))
#define FXSYS_IsFloatEqual(fa, fb) FXSYS_IsFloatZero((fa) - (fb))

// M_PI not universally present on all platforms.
#define FXSYS_PI 3.1415926535897932384626433832795f
#define FXSYS_BEZIER 0.5522847498308f

// NOTE: prevent use of the return value from snprintf() since some platforms
// have different return values.
#define FXSYS_snprintf (void)snprintf
#define FXSYS_vsnprintf (void)vsnprintf
#define FXSYS_sprintf DO_NOT_USE_SPRINTF_DIE_DIE_DIE
#define FXSYS_vsprintf DO_NOT_USE_VSPRINTF_DIE_DIE_DIE

#ifdef __cplusplus
}  // extern "C"

// Overloaded functions for C++ templates
inline size_t FXSYS_len(const char* ptr) {
  return strlen(ptr);
}

inline size_t FXSYS_len(const wchar_t* ptr) {
  return wcslen(ptr);
}

inline int FXSYS_cmp(const char* ptr1, const char* ptr2, size_t len) {
  return memcmp(ptr1, ptr2, len);
}

inline int FXSYS_cmp(const wchar_t* ptr1, const wchar_t* ptr2, size_t len) {
  return wmemcmp(ptr1, ptr2, len);
}

inline const char* FXSYS_chr(const char* ptr, char ch, size_t len) {
  return reinterpret_cast<const char*>(memchr(ptr, ch, len));
}

inline const wchar_t* FXSYS_chr(const wchar_t* ptr, wchar_t ch, size_t len) {
  return wmemchr(ptr, ch, len);
}

extern "C" {
#endif  // __cplusplus

#if defined(OS_WIN)
#define FXSYS_itoa _itoa
#define FXSYS_strlwr _strlwr
#define FXSYS_strupr _strupr
#define FXSYS_stricmp _stricmp
#define FXSYS_wcsicmp _wcsicmp
#define FXSYS_wcslwr _wcslwr
#define FXSYS_wcsupr _wcsupr
size_t FXSYS_wcsftime(wchar_t* strDest,
                      size_t maxsize,
                      const wchar_t* format,
                      const struct tm* timeptr);
#define FXSYS_SetLastError SetLastError
#define FXSYS_GetLastError GetLastError
#else  // defined(OS_WIN)
char* FXSYS_itoa(int value, char* str, int radix);
char* FXSYS_strlwr(char* str);
char* FXSYS_strupr(char* str);
int FXSYS_stricmp(const char* str1, const char* str2);
int FXSYS_wcsicmp(const wchar_t* str1, const wchar_t* str2);
wchar_t* FXSYS_wcslwr(wchar_t* str);
wchar_t* FXSYS_wcsupr(wchar_t* str);
#define FXSYS_wcsftime wcsftime
void FXSYS_SetLastError(uint32_t err);
uint32_t FXSYS_GetLastError();
#endif  // defined(OS_WIN)

#define FXSYS_UINT16_GET_LSBFIRST(p)                            \
  (static_cast<uint16_t>((static_cast<uint32_t>((p)[1]) << 8) | \
                         (static_cast<uint32_t>((p)[0]))))
#define FXSYS_UINT16_GET_MSBFIRST(p)                            \
  (static_cast<uint16_t>((static_cast<uint32_t>((p)[0]) << 8) | \
                         (static_cast<uint32_t>((p)[1]))))
#define FXSYS_UINT32_GET_LSBFIRST(p)       \
  ((static_cast<uint32_t>((p)[3]) << 24) | \
   (static_cast<uint32_t>((p)[2]) << 16) | \
   (static_cast<uint32_t>((p)[1]) << 8) | (static_cast<uint32_t>((p)[0])))
#define FXSYS_UINT32_GET_MSBFIRST(p)       \
  ((static_cast<uint32_t>((p)[0]) << 24) | \
   (static_cast<uint32_t>((p)[1]) << 16) | \
   (static_cast<uint32_t>((p)[2]) << 8) | (static_cast<uint32_t>((p)[3])))

int32_t FXSYS_atoi(const char* str);
uint32_t FXSYS_atoui(const char* str);
int32_t FXSYS_wtoi(const wchar_t* str);
int64_t FXSYS_atoi64(const char* str);
const char* FXSYS_i64toa(int64_t value, char* str, int radix);
int FXSYS_roundf(float f);
int FXSYS_round(double d);
float FXSYS_sqrt2(float a, float b);

#ifdef __cplusplus
}  // extern C
#endif  // __cplusplus

#endif  // CORE_FXCRT_FX_SYSTEM_H_
