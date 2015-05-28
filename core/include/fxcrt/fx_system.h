// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_SYSTEM_H_
#define CORE_INCLUDE_FXCRT_FX_SYSTEM_H_

#define _FX_WIN32_DESKTOP_		1
#define _FX_LINUX_DESKTOP_		4
#define _FX_MACOSX_				7
#define _FX_ANDROID_			12

#define _FXM_PLATFORM_WINDOWS_		1
#define _FXM_PLATFORM_LINUX_		2
#define _FXM_PLATFORM_APPLE_		3
#define _FXM_PLATFORM_ANDROID_		4

#ifndef _FX_OS_
#if defined(__ANDROID__)
#define _FX_OS_ _FX_ANDROID_
#define _FXM_PLATFORM_ _FXM_PLATFORM_ANDROID_
#elif defined(_WIN32) || defined(_WIN64)
#define _FX_OS_ _FX_WIN32_DESKTOP_
#define _FXM_PLATFORM_ _FXM_PLATFORM_WINDOWS_
#elif defined(__linux__)
#define _FX_OS_ _FX_LINUX_DESKTOP_
#define _FXM_PLATFORM_ _FXM_PLATFORM_LINUX_
#elif defined(__APPLE__)
#define _FX_OS_ _FX_MACOSX_
#define _FXM_PLATFORM_ _FXM_PLATFORM_APPLE_
#endif
#endif

#if !defined(_FX_OS_) || _FX_OS_ == 0
#error Sorry, can not figure out what OS you are targeting to. Please specify _FX_OS_ macro.
#endif

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define _CRT_SECURE_NO_WARNINGS
#include <sal.h>
#include <windows.h>
#endif  // _FXM_PLATFORM_WINDOWS_
#define _FX_W32_		1
#define _FX_W64_		2
#ifndef _FX_WORDSIZE_
#if defined(_WIN64) || defined(__arm64) || defined(__arm64__) || defined(_M_AMD64) || defined(_M_X64) || defined(_M_IA64) || defined(__powerpc64__) || defined(__x86_64__) || __WORDSIZE == 64 || defined(__LP64__)
#define _FX_WORDSIZE_	_FX_W64_
#else
#define _FX_WORDSIZE_	_FX_W32_
#endif
#endif

#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <wchar.h>

#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
#include <libkern/OSAtomic.h>
#if _FX_OS_ == _FX_MACOSX_
#include <Carbon/Carbon.h>
#elif _FX_OS_ == _FX_IOS_
#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
typedef void*					FX_LPVOID;
typedef void const*				FX_LPCVOID;
typedef void*					FX_POSITION;
typedef signed char				FX_INT8;
typedef unsigned char			FX_UINT8;
typedef unsigned char			FX_BYTE;
typedef unsigned char*			FX_LPBYTE;
typedef unsigned char const*	FX_LPCBYTE;
typedef short					FX_INT16;
typedef unsigned short			FX_UINT16;
typedef short					FX_SHORT;
typedef unsigned short			FX_WORD;
typedef unsigned short*			FX_LPWORD;
typedef unsigned short const*	FX_LPCWORD;
typedef int						FX_INT32;
typedef float					FX_FLOAT;
typedef int						FX_BOOL;
typedef int						FX_ERR;
#define FX_SUCCEEDED(Status)	((FX_ERR)(Status) >= 0)
#define FX_FAILED(Status)		((FX_ERR)(Status) < 0)
typedef char					FX_CHAR;
typedef char*					FX_LPSTR;
typedef char const*				FX_LPCSTR;
#define FX_DWORD_MAX UINT_MAX
typedef unsigned int		FX_DWORD;
typedef unsigned int*		FX_LPDWORD;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
typedef __int64				FX_INT64;
typedef unsigned __int64	FX_UINT64;
#else
typedef long long int		FX_INT64;
typedef unsigned long long	FX_UINT64;
#endif
#if _FX_WORDSIZE_ == _FX_W64_
typedef FX_INT64			FX_INTPTR;
typedef FX_UINT64			FX_UINTPTR;
#else
typedef int					FX_INTPTR;
typedef unsigned int		FX_UINTPTR;
#endif
typedef wchar_t					FX_WCHAR;
typedef wchar_t*				FX_LPWSTR;
typedef wchar_t const*			FX_LPCWSTR;
typedef FX_DWORD				FX_UINT32;
typedef FX_UINT64				FX_QWORD;

// PDFium string sizes are limited to 2^31-1, and the value is signed to
// allow -1 as a placeholder for "unknown".
// TODO(palmer): it should be a |size_t|, or at least unsigned.
typedef int FX_STRSIZE;

#if defined(DEBUG) && !defined(_DEBUG)
#define _DEBUG
#endif
#ifndef TRUE

#define TRUE	1
#endif
#ifndef FALSE

#define FALSE	0
#endif
#ifndef NULL

#define NULL	0
#endif
#define FXSYS_assert assert
#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT FXSYS_assert
#else

#define ASSERT(a)
#endif
#endif
#define FX_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define FX_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define FX_PI	3.1415926535897932384626433832795f

// NOTE: prevent use of the return value from snprintf() since some platforms
// have different return values (e.g. windows _vsnprintf()), and provide
// versions that always NUL-terminate.
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_ && _MSC_VER < 1900
void FXSYS_snprintf(char *str, size_t size, _Printf_format_string_ const char* fmt, ...);
void FXSYS_vsnprintf(char *str, size_t size, const char* fmt, va_list ap);
#else
#define FXSYS_snprintf	(void) snprintf
#define FXSYS_vsnprintf	(void) vsnprintf
#endif

#define FXSYS_sprintf	DO_NOT_USE_SPRINTF_DIE_DIE_DIE
#define FXSYS_vsprintf	DO_NOT_USE_VSPRINTF_DIE_DIE_DIE
#define FXSYS_strchr	strchr
#define FXSYS_strncmp	strncmp
#define FXSYS_strcmp	strcmp
#define FXSYS_strcpy	strcpy
#define FXSYS_strncpy	strncpy
#define FXSYS_strstr	strstr
#define FXSYS_FILE		FILE
#define FXSYS_fopen		fopen
#define FXSYS_fclose	fclose
#define FXSYS_SEEK_END	SEEK_END
#define FXSYS_SEEK_SET	SEEK_SET
#define FXSYS_fseek		fseek
#define FXSYS_ftell		ftell
#define FXSYS_fread		fread
#define FXSYS_fwrite	fwrite
#define FXSYS_fprintf	fprintf
#define FXSYS_fflush	fflush

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#ifdef _NATIVE_WCHAR_T_DEFINED
#define FXSYS_wfopen(f, m) _wfopen((const wchar_t*)(f), (const wchar_t*)(m))
#else
#define FXSYS_wfopen _wfopen
#endif
#else
FXSYS_FILE* FXSYS_wfopen(FX_LPCWSTR filename, FX_LPCWSTR mode);
#endif

#ifdef __cplusplus
} // extern "C"
#include "../../../third_party/base/numerics/safe_conversions.h"
#define FXSYS_strlen(ptr) pdfium::base::checked_cast<FX_STRSIZE>(strlen(ptr))
#define FXSYS_wcslen(ptr) pdfium::base::checked_cast<FX_STRSIZE>(wcslen(ptr))
extern "C" {
#else
#define FXSYS_strlen(ptr) ((FX_STRSIZE)strlen(ptr))
#define FXSYS_wcslen(ptr) ((FX_STRSIZE)wcslen(ptr))
#endif

#define FXSYS_wcscmp	wcscmp
#define FXSYS_wcschr	wcschr
#define FXSYS_wcsstr	wcsstr
#define FXSYS_wcsncmp	wcsncmp
#define FXSYS_vswprintf	vswprintf
#define FXSYS_mbstowcs	mbstowcs
#define FXSYS_wcstombs	wcstombs
#define FXSYS_memcmp	memcmp
#define FXSYS_memcpy	memcpy
#define FXSYS_memmove	memmove
#define FXSYS_memset	memset
#define FXSYS_memchr	memchr
#define FXSYS_qsort		qsort
#define FXSYS_bsearch	bsearch
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define FXSYS_GetACP GetACP
#define FXSYS_itoa _itoa
#define FXSYS_strlwr _strlwr
#define FXSYS_strupr _strupr
#define FXSYS_stricmp _stricmp
#ifdef _NATIVE_WCHAR_T_DEFINED
#define FXSYS_wcsicmp(str1, str2) _wcsicmp((wchar_t*)(str1), (wchar_t*)(str2))
#define FXSYS_WideCharToMultiByte(p1, p2, p3, p4, p5, p6, p7, p8) WideCharToMultiByte(p1, p2, (const wchar_t*)(p3), p4, p5, p6, p7, p8)
#define FXSYS_MultiByteToWideChar(p1, p2, p3, p4, p5, p6) MultiByteToWideChar(p1, p2, p3, p4, (wchar_t*)(p5), p6)
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
int			FXSYS_GetACP(void);
char*		FXSYS_itoa(int value, char* string, int radix);
int			FXSYS_WideCharToMultiByte(FX_DWORD codepage, FX_DWORD dwFlags, const wchar_t* wstr, int wlen,
                                      char* buf, int buflen, const char* default_str, int* pUseDefault);
int			FXSYS_MultiByteToWideChar(FX_DWORD codepage, FX_DWORD dwFlags, const char* bstr, int blen,
                                      wchar_t* buf, int buflen);
FX_DWORD	FXSYS_GetFullPathName(const char* filename, FX_DWORD buflen, char* buf, char** filepart);
FX_DWORD	FXSYS_GetModuleFileName(void* hModule, char* buf, FX_DWORD bufsize);
char*		FXSYS_strlwr(char* str);
char*		FXSYS_strupr(char* str);
int			FXSYS_stricmp(const char*, const char*);
int			FXSYS_wcsicmp(const wchar_t *string1, const wchar_t *string2);
wchar_t*	FXSYS_wcslwr(wchar_t* str);
wchar_t*	FXSYS_wcsupr(wchar_t* str);
#endif
#define FXSYS_memcpy32		FXSYS_memcpy
#define FXSYS_memcmp32		FXSYS_memcmp
#define FXSYS_memset32		FXSYS_memset
#define FXSYS_memset8		FXSYS_memset
#define FXSYS_memmove32		FXSYS_memmove
#ifdef __cplusplus
}
#endif
#include <math.h>
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define FXSYS_pow(a, b)		(FX_FLOAT)powf(a, b)
#else
#define FXSYS_pow(a, b)		(FX_FLOAT)pow(a, b)
#endif
#define FXSYS_sqrt(a)		(FX_FLOAT)sqrt(a)
#define FXSYS_fabs(a)		(FX_FLOAT)fabs(a)
#define FXSYS_atan2(a, b)	(FX_FLOAT)atan2(a, b)
#define FXSYS_ceil(a)		(FX_FLOAT)ceil(a)
#define FXSYS_floor(a)		(FX_FLOAT)floor(a)
#define FXSYS_cos(a)		(FX_FLOAT)cos(a)
#define FXSYS_acos(a)		(FX_FLOAT)acos(a)
#define FXSYS_sin(a)		(FX_FLOAT)sin(a)
#define FXSYS_log(a)		(FX_FLOAT)log(a)
#define FXSYS_log10(a)		(FX_FLOAT)log10(a)
#define FXSYS_fmod(a, b)	(FX_FLOAT)fmod(a, b)
#define FXSYS_abs			abs
#ifdef __cplusplus
extern "C" {
#endif
#define _FX_LSB_FIRST_
#define FXDWORD_FROM_LSBFIRST(i)	(i)
#define FXDWORD_FROM_MSBFIRST(i)	(((FX_BYTE)(i) << 24) | ((FX_BYTE)((i) >> 8) << 16) | ((FX_BYTE)((i) >> 16) << 8) | (FX_BYTE)((i) >> 24))
#define FXDWORD_GET_LSBFIRST(p)		((((FX_LPBYTE)(p))[3] << 24) | (((FX_LPBYTE)(p))[2] << 16) | (((FX_LPBYTE)(p))[1] << 8) | (((FX_LPBYTE)(p))[0]))
#define FXDWORD_GET_MSBFIRST(p) ((((FX_LPBYTE)(p))[0] << 24) | (((FX_LPBYTE)(p))[1] << 16) | (((FX_LPBYTE)(p))[2] << 8) | (((FX_LPBYTE)(p))[3]))
#define FXSYS_HIBYTE(word)	((FX_BYTE)((word) >> 8))
#define FXSYS_LOBYTE(word)	((FX_BYTE)(word))
#define FXSYS_HIWORD(dword)	((FX_WORD)((dword) >> 16))
#define FXSYS_LOWORD(dword)	((FX_WORD)(dword))
FX_INT32	FXSYS_atoi(FX_LPCSTR str);
FX_INT32	FXSYS_wtoi(FX_LPCWSTR str);
FX_INT64	FXSYS_atoi64(FX_LPCSTR str);
FX_INT64	FXSYS_wtoi64(FX_LPCWSTR str);
FX_LPCSTR	FXSYS_i64toa(FX_INT64 value, FX_LPSTR str, int radix);
FX_LPCWSTR	FXSYS_i64tow(FX_INT64 value, FX_LPWSTR str, int radix);
int			FXSYS_round(FX_FLOAT f);
#define		FXSYS_Mul(a, b) ((a) * (b))
#define		FXSYS_Div(a, b) ((a) / (b))
#define		FXSYS_MulDiv(a, b, c) ((a) * (b) / (c))
#define		FXSYS_sqrt2(a, b) (FX_FLOAT)FXSYS_sqrt((a)*(a) + (b)*(b))
#ifdef __cplusplus
};

#if defined(__clang__) || _MSC_VER >= 1700
#define FX_FINAL final
#define FX_OVERRIDE override
#elif defined(__GNUC__) && __cplusplus >= 201103 && \
      (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40700
#define FX_FINAL final
#define FX_OVERRIDE override
#else
#define FX_FINAL
#define FX_OVERRIDE
#endif
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
#if  _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define NEVER_INLINE __declspec(noinline)
#else  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#define NEVER_INLINE __attribute__((__noinline__))
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

#endif  // CORE_INCLUDE_FXCRT_FX_SYSTEM_H_
