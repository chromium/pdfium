// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_SYSTEM_H_
#define CORE_FXCRT_FX_SYSTEM_H_

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// _FX_OS_ values:
#define _FX_WIN32_DESKTOP_ 1
#define _FX_WIN64_DESKTOP_ 2
#define _FX_LINUX_DESKTOP_ 4
#define _FX_MACOSX_ 7
#define _FX_ANDROID_ 12

// _FXM_PLATFORM_ values;
#define _FXM_PLATFORM_WINDOWS_ 1  // _FX_WIN32_DESKTOP_ or _FX_WIN64_DESKTOP_.
#define _FXM_PLATFORM_LINUX_ 2    // _FX_LINUX_DESKTOP_ always.
#define _FXM_PLATFORM_APPLE_ 3    // _FX_MACOSX_ always.
#define _FXM_PLATFORM_ANDROID_ 4  // _FX_ANDROID_ always.

#ifndef _FX_OS_
#if defined(__ANDROID__)
#define _FX_OS_ _FX_ANDROID_
#define _FXM_PLATFORM_ _FXM_PLATFORM_ANDROID_
#elif defined(_WIN32)
#define _FX_OS_ _FX_WIN32_DESKTOP_
#define _FXM_PLATFORM_ _FXM_PLATFORM_WINDOWS_
#elif defined(_WIN64)
#define _FX_OS_ _FX_WIN64_DESKTOP_
#define _FXM_PLATFORM_ _FXM_PLATFORM_WINDOWS_
#elif defined(__linux__)
#define _FX_OS_ _FX_LINUX_DESKTOP_
#define _FXM_PLATFORM_ _FXM_PLATFORM_LINUX_
#elif defined(__APPLE__)
#define _FX_OS_ _FX_MACOSX_
#define _FXM_PLATFORM_ _FXM_PLATFORM_APPLE_
#endif
#endif  // _FX_OS_

#if !defined(_FX_OS_) || _FX_OS_ == 0
#error Sorry, can not figure out target OS. Please specify _FX_OS_ macro.
#endif

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#include <windows.h>
#include <sal.h>
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
#include <Carbon/Carbon.h>
#include <libkern/OSAtomic.h>
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define IsFloatZero(f) ((f) < 0.0001 && (f) > -0.0001)
#define IsFloatBigger(fa, fb) ((fa) > (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatSmaller(fa, fb) ((fa) < (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatEqual(fa, fb) IsFloatZero((fa) - (fb))

// PDFium string sizes are limited to 2^31-1, and the value is signed to
// allow -1 as a placeholder for "unknown".
// TODO(palmer): it should be a |size_t|, or at least unsigned.
typedef int FX_STRSIZE;

// PDFium file sizes match the platform, but PDFium itself does not support
// files larger than 2GB even if the platform does. The value must be signed
// to support -1 error returns.
// TODO(tsepez): support larger files.
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define FX_FILESIZE int32_t
#else  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define FX_FILESIZE off_t
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

#ifndef ASSERT
#ifndef NDEBUG
#define ASSERT assert
#else
#define ASSERT(a)
#endif  // NDEBUG
#endif  // ASSERT

#if defined(__clang__) || defined(__GNUC__)
#define PDFIUM_IMMEDIATE_CRASH() __builtin_trap()
#else
#define PDFIUM_IMMEDIATE_CRASH() ((void)(*(volatile char*)0 = 0))
#endif  // defined(__clang__) || defined(__GNUC__)

// M_PI not universally present on all platforms.
#define FX_PI 3.1415926535897932384626433832795f
#define FX_BEZIER 0.5522847498308f

// NOTE: prevent use of the return value from snprintf() since some platforms
// have different return values (e.g. windows _vsnprintf()), and provide
// versions that always NUL-terminate.
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_ && _MSC_VER < 1900
void FXSYS_snprintf(char* str,
                    size_t size,
                    _Printf_format_string_ const char* fmt,
                    ...);
void FXSYS_vsnprintf(char* str, size_t size, const char* fmt, va_list ap);
#else  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_ && _MSC_VER < 1900
#define FXSYS_snprintf (void)snprintf
#define FXSYS_vsnprintf (void)vsnprintf
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_ && _MSC_VER < 1900

#define FXSYS_sprintf DO_NOT_USE_SPRINTF_DIE_DIE_DIE
#define FXSYS_vsprintf DO_NOT_USE_VSPRINTF_DIE_DIE_DIE

#ifdef __cplusplus
}  // extern "C"

#include "third_party/base/numerics/safe_conversions.h"

#define FXSYS_strlen(ptr) pdfium::base::checked_cast<FX_STRSIZE>(strlen(ptr))
#define FXSYS_wcslen(ptr) pdfium::base::checked_cast<FX_STRSIZE>(wcslen(ptr))

// Overloaded functions for C++ templates
inline FX_STRSIZE FXSYS_len(const char* ptr) {
  return FXSYS_strlen(ptr);
}

inline FX_STRSIZE FXSYS_len(const wchar_t* ptr) {
  return FXSYS_wcslen(ptr);
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
#else
#define FXSYS_strlen(ptr) ((FX_STRSIZE)strlen(ptr))
#define FXSYS_wcslen(ptr) ((FX_STRSIZE)wcslen(ptr))
#endif  // __cplusplus

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define FXSYS_GetACP GetACP
#define FXSYS_itoa _itoa
#define FXSYS_strlwr _strlwr
#define FXSYS_strupr _strupr
#define FXSYS_stricmp _stricmp
#define FXSYS_pow(a, b) (float)powf(a, b)
#define FXSYS_GetFullPathName GetFullPathName
#define FXSYS_GetModuleFileName GetModuleFileName
size_t FXSYS_wcsftime(wchar_t* strDest,
                      size_t maxsize,
                      const wchar_t* format,
                      const struct tm* timeptr);
#ifdef _NATIVE_WCHAR_T_DEFINED
#define FXSYS_wcsicmp(str1, str2) _wcsicmp((wchar_t*)(str1), (wchar_t*)(str2))
#define FXSYS_WideCharToMultiByte(p1, p2, p3, p4, p5, p6, p7, p8) \
  WideCharToMultiByte(p1, p2, (const wchar_t*)(p3), p4, p5, p6, p7, p8)
#define FXSYS_MultiByteToWideChar(p1, p2, p3, p4, p5, p6) \
  MultiByteToWideChar(p1, p2, p3, p4, (wchar_t*)(p5), p6)
#define FXSYS_wcslwr(str) _wcslwr((wchar_t*)(str))
#define FXSYS_wcsupr(str) _wcsupr((wchar_t*)(str))
#else  // _NATIVE_WCHAR_T_DEFINED
#define FXSYS_wcsicmp _wcsicmp
#define FXSYS_WideCharToMultiByte WideCharToMultiByte
#define FXSYS_MultiByteToWideChar MultiByteToWideChar
#define FXSYS_wcslwr _wcslwr
#define FXSYS_wcsupr _wcsupr
#endif  // _NATIVE_WCHAR_T_DEFINED
#else   // _FXM_PLATFORM == _FXM_PLATFORM_WINDOWS_
int FXSYS_GetACP();
char* FXSYS_itoa(int value, char* str, int radix);
int FXSYS_WideCharToMultiByte(uint32_t codepage,
                              uint32_t dwFlags,
                              const wchar_t* wstr,
                              int wlen,
                              char* buf,
                              int buflen,
                              const char* default_str,
                              int* pUseDefault);
int FXSYS_MultiByteToWideChar(uint32_t codepage,
                              uint32_t dwFlags,
                              const char* bstr,
                              int blen,
                              wchar_t* buf,
                              int buflen);
uint32_t FXSYS_GetFullPathName(const char* filename,
                               uint32_t buflen,
                               char* buf,
                               char** filepart);
uint32_t FXSYS_GetModuleFileName(void* hModule, char* buf, uint32_t bufsize);
char* FXSYS_strlwr(char* str);
char* FXSYS_strupr(char* str);
int FXSYS_stricmp(const char*, const char*);
int FXSYS_wcsicmp(const wchar_t* str1, const wchar_t* str2);
wchar_t* FXSYS_wcslwr(wchar_t* str);
wchar_t* FXSYS_wcsupr(wchar_t* str);
#define FXSYS_pow(a, b) (float)pow(a, b)
#define FXSYS_wcsftime wcsftime
#endif  // _FXM_PLATFORM == _FXM_PLATFORM_WINDOWS_

#define FXDWORD_GET_LSBFIRST(p)                                                \
  ((static_cast<uint32_t>(p[3]) << 24) | (static_cast<uint32_t>(p[2]) << 16) | \
   (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[0])))
#define FXDWORD_GET_MSBFIRST(p)                                                \
  ((static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) | \
   (static_cast<uint32_t>(p[2]) << 8) | (static_cast<uint32_t>(p[3])))
int32_t FXSYS_atoi(const char* str);
uint32_t FXSYS_atoui(const char* str);
int32_t FXSYS_wtoi(const wchar_t* str);
int64_t FXSYS_atoi64(const char* str);
int64_t FXSYS_wtoi64(const wchar_t* str);
const char* FXSYS_i64toa(int64_t value, char* str, int radix);
int FXSYS_round(float f);
#define FXSYS_sqrt2(a, b) (float)sqrt((a) * (a) + (b) * (b))
#ifdef __cplusplus
};      // extern C
#endif  // __cplusplus

// To print a size_t value in a portable way:
//   size_t size;
//   printf("xyz: %" PRIuS, size);
// The "u" in the macro corresponds to %u, and S is for "size".
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_

#if (defined(_INTTYPES_H) || defined(_INTTYPES_H_)) && !defined(PRId64)
#error "inttypes.h has already been included before this header file, but "
#error "without __STDC_FORMAT_MACROS defined."
#endif

#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

#if !defined(PRIuS)
#define PRIuS "zu"
#endif

#else  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_

#if !defined(PRIuS)
#define PRIuS "Iu"
#endif

#endif  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_

// Prevent a function from ever being inlined, typically because we'd
// like it to appear in stack traces.
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define NEVER_INLINE __declspec(noinline)
#else  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define NEVER_INLINE __attribute__((__noinline__))
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

// Handle differnces between platform's variadic function implementations.
#if defined(__ARMCC_VERSION) ||                                              \
    (!defined(_MSC_VER) && (_FX_CPU_ == _FX_X64_ || _FX_CPU_ == _FX_IA64_ || \
                            _FX_CPU_ == _FX_ARM64_)) ||                      \
    defined(__native_client__)
#define FX_VA_COPY(dst, src) va_copy((dst), (src))
#else
#define FX_VA_COPY(dst, src) ((dst) = (src))
#endif

#endif  // CORE_FXCRT_FX_SYSTEM_H_
