// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_SYSTEM_H_
#define CORE_INCLUDE_FXCRT_FX_SYSTEM_H_

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
#define _CRT_SECURE_NO_WARNINGS
#include <sal.h>
#include <windows.h>
#endif

#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
#include <libkern/OSAtomic.h>
#include <Carbon/Carbon.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
typedef void* FX_POSITION;       // Keep until fxcrt containers gone
typedef unsigned short FX_WORD;  // Keep - "an efficient small type"
typedef unsigned int FX_DWORD;   // Keep - "an efficient type"
typedef float FX_FLOAT;          // Keep, allow upgrade to doubles.
typedef int FX_BOOL;             // Keep, sadly not always 0 or 1.
typedef char FX_CHAR;            // Keep, questionable signedness.
typedef wchar_t FX_WCHAR;        // Keep, maybe bad platform wchars.

// PDFium string sizes are limited to 2^31-1, and the value is signed to
// allow -1 as a placeholder for "unknown".
// TODO(palmer): it should be a |size_t|, or at least unsigned.
typedef int FX_STRSIZE;

#if defined(DEBUG) && !defined(_DEBUG)
#define _DEBUG
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef __cplusplus
static_assert(TRUE == true, "true_needs_to_be_true");
static_assert(FALSE == false, "false_needs_to_be_false");
#endif

#ifndef NULL
#define NULL 0
#endif

#define FXSYS_assert assert
#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT FXSYS_assert
#else
#define ASSERT(a)
#endif
#endif

#define FX_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define FX_PI 3.1415926535897932384626433832795f

// NOTE: prevent use of the return value from snprintf() since some platforms
// have different return values (e.g. windows _vsnprintf()), and provide
// versions that always NUL-terminate.
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_ && _MSC_VER < 1900
void FXSYS_snprintf(char* str,
                    size_t size,
                    _Printf_format_string_ const char* fmt,
                    ...);
void FXSYS_vsnprintf(char* str, size_t size, const char* fmt, va_list ap);
#else
#define FXSYS_snprintf (void) snprintf
#define FXSYS_vsnprintf (void) vsnprintf
#endif

#define FXSYS_sprintf DO_NOT_USE_SPRINTF_DIE_DIE_DIE
#define FXSYS_vsprintf DO_NOT_USE_VSPRINTF_DIE_DIE_DIE
#define FXSYS_strchr strchr
#define FXSYS_strncmp strncmp
#define FXSYS_strcmp strcmp
#define FXSYS_strcpy strcpy
#define FXSYS_strncpy strncpy
#define FXSYS_strstr strstr
#define FXSYS_FILE FILE
#define FXSYS_fopen fopen
#define FXSYS_fclose fclose
#define FXSYS_SEEK_END SEEK_END
#define FXSYS_SEEK_SET SEEK_SET
#define FXSYS_fseek fseek
#define FXSYS_ftell ftell
#define FXSYS_fread fread
#define FXSYS_fwrite fwrite
#define FXSYS_fprintf fprintf
#define FXSYS_fflush fflush

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#ifdef _NATIVE_WCHAR_T_DEFINED
#define FXSYS_wfopen(f, m) _wfopen((const wchar_t*)(f), (const wchar_t*)(m))
#else
#define FXSYS_wfopen _wfopen
#endif
#else
FXSYS_FILE* FXSYS_wfopen(const FX_WCHAR* filename, const FX_WCHAR* mode);
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

#ifdef __cplusplus
}  // extern "C"
#include "third_party/base/numerics/safe_conversions.h"
#define FXSYS_strlen(ptr) pdfium::base::checked_cast<FX_STRSIZE>(strlen(ptr))
#define FXSYS_wcslen(ptr) pdfium::base::checked_cast<FX_STRSIZE>(wcslen(ptr))
extern "C" {
#else
#define FXSYS_strlen(ptr) ((FX_STRSIZE)strlen(ptr))
#define FXSYS_wcslen(ptr) ((FX_STRSIZE)wcslen(ptr))
#endif

#define FXSYS_wcscmp wcscmp
#define FXSYS_wcschr wcschr
#define FXSYS_wcsstr wcsstr
#define FXSYS_wcsncmp wcsncmp
#define FXSYS_vswprintf vswprintf
#define FXSYS_mbstowcs mbstowcs
#define FXSYS_wcstombs wcstombs
#define FXSYS_memcmp memcmp
#define FXSYS_memcpy memcpy
#define FXSYS_memmove memmove
#define FXSYS_memset memset
#define FXSYS_memchr memchr
#define FXSYS_qsort qsort
#define FXSYS_bsearch bsearch

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define FXSYS_GetACP GetACP
#define FXSYS_itoa _itoa
#define FXSYS_strlwr _strlwr
#define FXSYS_strupr _strupr
#define FXSYS_stricmp _stricmp
#ifdef _NATIVE_WCHAR_T_DEFINED
#define FXSYS_wcsicmp(str1, str2) _wcsicmp((wchar_t*)(str1), (wchar_t*)(str2))
#define FXSYS_WideCharToMultiByte(p1, p2, p3, p4, p5, p6, p7, p8) \
  WideCharToMultiByte(p1, p2, (const wchar_t*)(p3), p4, p5, p6, p7, p8)
#define FXSYS_MultiByteToWideChar(p1, p2, p3, p4, p5, p6) \
  MultiByteToWideChar(p1, p2, p3, p4, (wchar_t*)(p5), p6)
#define FXSYS_wcslwr(str) _wcslwr((wchar_t*)(str))
#define FXSYS_wcsupr(str) _wcsupr((wchar_t*)(str))
#else
#define FXSYS_wcsicmp _wcsicmp
#define FXSYS_WideCharToMultiByte WideCharToMultiByte
#define FXSYS_MultiByteToWideChar MultiByteToWideChar
#define FXSYS_wcslwr _wcslwr
#define FXSYS_wcsupr _wcsupr
#endif
#define FXSYS_GetFullPathName GetFullPathName
#define FXSYS_GetModuleFileName GetModuleFileName
#else
int FXSYS_GetACP(void);
char* FXSYS_itoa(int value, char* string, int radix);
int FXSYS_WideCharToMultiByte(FX_DWORD codepage,
                              FX_DWORD dwFlags,
                              const wchar_t* wstr,
                              int wlen,
                              char* buf,
                              int buflen,
                              const char* default_str,
                              int* pUseDefault);
int FXSYS_MultiByteToWideChar(FX_DWORD codepage,
                              FX_DWORD dwFlags,
                              const char* bstr,
                              int blen,
                              wchar_t* buf,
                              int buflen);
FX_DWORD FXSYS_GetFullPathName(const char* filename,
                               FX_DWORD buflen,
                               char* buf,
                               char** filepart);
FX_DWORD FXSYS_GetModuleFileName(void* hModule, char* buf, FX_DWORD bufsize);
char* FXSYS_strlwr(char* str);
char* FXSYS_strupr(char* str);
int FXSYS_stricmp(const char*, const char*);
int FXSYS_wcsicmp(const wchar_t* string1, const wchar_t* string2);
wchar_t* FXSYS_wcslwr(wchar_t* str);
wchar_t* FXSYS_wcsupr(wchar_t* str);
#endif  // _FXM_PLATFORM == _FXM_PLATFORM_WINDOWS_

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define FXSYS_pow(a, b) (FX_FLOAT) powf(a, b)
#else
#define FXSYS_pow(a, b) (FX_FLOAT) pow(a, b)
#endif
#define FXSYS_sqrt(a) (FX_FLOAT) sqrt(a)
#define FXSYS_fabs(a) (FX_FLOAT) fabs(a)
#define FXSYS_atan2(a, b) (FX_FLOAT) atan2(a, b)
#define FXSYS_ceil(a) (FX_FLOAT) ceil(a)
#define FXSYS_floor(a) (FX_FLOAT) floor(a)
#define FXSYS_cos(a) (FX_FLOAT) cos(a)
#define FXSYS_acos(a) (FX_FLOAT) acos(a)
#define FXSYS_sin(a) (FX_FLOAT) sin(a)
#define FXSYS_log(a) (FX_FLOAT) log(a)
#define FXSYS_log10(a) (FX_FLOAT) log10(a)
#define FXSYS_fmod(a, b) (FX_FLOAT) fmod(a, b)
#define FXSYS_abs abs
#define FXDWORD_FROM_LSBFIRST(i) (i)
#define FXDWORD_FROM_MSBFIRST(i)                        \
  (((uint8_t)(i) << 24) | ((uint8_t)((i) >> 8) << 16) | \
   ((uint8_t)((i) >> 16) << 8) | (uint8_t)((i) >> 24))
#define FXDWORD_GET_LSBFIRST(p) \
  ((p[3] << 24) | (p[2] << 16) | (p[1] << 8) | (p[0]))
#define FXDWORD_GET_MSBFIRST(p) \
  ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | (p[3]))
#define FXSYS_HIBYTE(word) ((uint8_t)((word) >> 8))
#define FXSYS_LOBYTE(word) ((uint8_t)(word))
#define FXSYS_HIWORD(dword) ((FX_WORD)((dword) >> 16))
#define FXSYS_LOWORD(dword) ((FX_WORD)(dword))
int32_t FXSYS_atoi(const FX_CHAR* str);
int32_t FXSYS_wtoi(const FX_WCHAR* str);
int64_t FXSYS_atoi64(const FX_CHAR* str);
int64_t FXSYS_wtoi64(const FX_WCHAR* str);
const FX_CHAR* FXSYS_i64toa(int64_t value, FX_CHAR* str, int radix);
int FXSYS_round(FX_FLOAT f);
#define FXSYS_Mul(a, b) ((a) * (b))
#define FXSYS_Div(a, b) ((a) / (b))
#define FXSYS_MulDiv(a, b, c) ((a) * (b) / (c))
#define FXSYS_sqrt2(a, b) (FX_FLOAT) FXSYS_sqrt((a) * (a) + (b) * (b))
#ifdef __cplusplus
};
#endif

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

#endif  // CORE_INCLUDE_FXCRT_FX_SYSTEM_H_
