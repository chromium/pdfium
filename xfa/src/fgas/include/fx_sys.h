// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_SYSTEM
#define _FX_SYSTEM
#ifdef __cplusplus
extern "C" {
#endif
#define FX_RAD2DEG(r)		((r) * 180.0f / FX_PI)
#define FX_DEG2RAD(a)		((a) * FX_PI / 180.0f)
typedef FX_INT8 *			FX_LPINT8;
typedef FX_INT8 const *		FX_LPCINT8;
typedef FX_INT32 *			FX_LPINT32;
typedef FX_INT32 const *	FX_LPCINT32;
typedef long				FX_LONG;
typedef FX_LONG *			FX_LPLONG;
typedef FX_LONG const *		FX_LPCLONG;
typedef FX_FLOAT const *	FX_LPCFLOAT;
typedef double				FX_DOUBLE;
typedef FX_DOUBLE *			FX_LPDOUBLE;
typedef FX_DOUBLE const *	FX_LPCDOUBLE;
FX_FLOAT	FX_tan(FX_FLOAT a);
FX_FLOAT	FX_log(FX_FLOAT b, FX_FLOAT x);
FX_FLOAT	FX_strtof(FX_LPCSTR pcsStr, FX_INT32 iLength = -1, FX_INT32 *pUsedLen = NULL);
FX_FLOAT	FX_wcstof(FX_LPCWSTR pwsStr, FX_INT32 iLength = -1, FX_INT32 *pUsedLen = NULL);
FX_LPWSTR	FX_wcsncpy(FX_LPWSTR dstStr, FX_LPCWSTR srcStr, size_t count);
FX_INT32	FX_wcsnicmp(FX_LPCWSTR s1, FX_LPCWSTR s2, size_t count);
FX_INT32	FX_strnicmp(FX_LPCSTR s1, FX_LPCSTR s2, size_t count);
inline FX_BOOL	FX_islower(FX_INT32 ch)
{
    return ch >= 'a' && ch <= 'z';
}
inline FX_BOOL	FX_isupper(FX_INT32 ch)
{
    return ch >= 'A' && ch <= 'Z';
}
inline FX_INT32	FX_tolower(FX_INT32 ch)
{
    return FX_isupper(ch) ? (ch + 0x20) : ch;
}
inline FX_INT32 FX_toupper(FX_INT32 ch)
{
    return FX_islower(ch) ? (ch - 0x20) : ch;
}
FX_INT32	FX_filelength(FXSYS_FILE *file);
FX_BOOL		FX_fsetsize(FXSYS_FILE *file, FX_INT32 size);
void		FX_memset(FX_LPVOID pBuf, FX_INT32 iValue, size_t size);
void		FX_memcpy(FX_LPVOID pDst, FX_LPCVOID pSrc, size_t size);
FX_BOOL		FX_IsRelativePath(const CFX_WideStringC &wsPath);
FX_BOOL		FX_JoinPath(const CFX_WideStringC &wsBasePath, const CFX_WideStringC &wsRelativePath, CFX_WideString &wsAbsolutePath);
typedef struct _FX_VERSION {
    FX_DWORD	dwMajorVersion;
    FX_DWORD	dwMinorVersion;
    FX_DWORD	dwBuildVersion;
} FX_VERSION, * FX_LPVERSION;
typedef FX_VERSION const * FX_LPCVERSION;
#ifdef __cplusplus
};
#endif
#endif
