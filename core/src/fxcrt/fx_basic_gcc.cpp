// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <limits>
#include "../../include/fxcrt/fx_ext.h"
template <class T, class STR_T>
T FXSYS_StrToInt(STR_T str)
{
    FX_BOOL neg = FALSE;
    if (str == NULL) {
        return 0;
    }
    if (*str == '-') {
        neg = TRUE;
        str ++;
    }
    T num = 0;
    while (*str) {
        if ((*str) < '0' || (*str) > '9') {
            break;
        }
        if (num > (std::numeric_limits<T>::max() - 9) / 10) {
            break;
        }
        num = num * 10 + (*str) - '0';
        str ++;
    }
    return neg ? -num : num;
}
template <typename T, typename STR_T>
STR_T FXSYS_IntToStr(T value, STR_T string, int radix)
{
    int i = 0;
    if (value < 0) {
        string[i++] = '-';
        value = -value;
    } else if (value == 0) {
        string[0] = '0';
        string[1] = 0;
        return string;
    }
    int digits = 1;
    T order = value / 10;
    while(order > 0) {
        digits++;
        order = order / 10;
    }
    for (int d = digits - 1; d > -1; d--) {
        string[d + i] = "0123456789abcdef"[value % 10];
        value /= 10;
    }
    string[digits + i] = 0;
    return string;
}
#ifdef __cplusplus
extern "C" {
#endif
FX_INT32 FXSYS_atoi(FX_LPCSTR str)
{
    return FXSYS_StrToInt<FX_INT32, FX_LPCSTR>(str);
}
FX_INT32 FXSYS_wtoi(FX_LPCWSTR str)
{
    return FXSYS_StrToInt<FX_INT32, FX_LPCWSTR>(str);
}
FX_INT64 FXSYS_atoi64(FX_LPCSTR str)
{
    return FXSYS_StrToInt<FX_INT64, FX_LPCSTR>(str);
}
FX_INT64 FXSYS_wtoi64(FX_LPCWSTR str)
{
    return FXSYS_StrToInt<FX_INT64, FX_LPCWSTR>(str);
}
FX_LPCSTR FXSYS_i64toa(FX_INT64 value, FX_LPSTR str, int radix)
{
    return FXSYS_IntToStr<FX_INT64, FX_LPSTR>(value, str, radix);
}
FX_LPCWSTR FXSYS_i64tow(FX_INT64 value, FX_LPWSTR str, int radix)
{
    return FXSYS_IntToStr<FX_INT64, FX_LPWSTR>(value, str, radix);
}
#ifdef __cplusplus
}
#endif
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
#ifdef __cplusplus
extern "C" {
#endif
int FXSYS_GetACP()
{
    return 0;
}
FX_DWORD FXSYS_GetFullPathName(FX_LPCSTR filename, FX_DWORD buflen, FX_LPSTR buf, FX_LPSTR* filepart)
{
    int srclen = FXSYS_strlen(filename);
    if (buf == NULL || (int)buflen < srclen + 1) {
        return srclen + 1;
    }
    FXSYS_strcpy(buf, filename);
    return srclen;
}
FX_DWORD FXSYS_GetModuleFileName(FX_LPVOID hModule, char* buf, FX_DWORD bufsize)
{
    return (FX_DWORD) - 1;
}
#ifdef __cplusplus
}
#endif
#endif
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
#ifdef __cplusplus
extern "C" {
#endif
FXSYS_FILE* FXSYS_wfopen(FX_LPCWSTR filename, FX_LPCWSTR mode)
{
    return FXSYS_fopen(CFX_ByteString::FromUnicode(filename), CFX_ByteString::FromUnicode(mode));
}
char* FXSYS_strlwr(char* str)
{
    if (str == NULL) {
        return NULL;
    }
    char* s = str;
    while (*str) {
        *str = FXSYS_tolower(*str);
        str ++;
    }
    return s;
}
char* FXSYS_strupr(char* str)
{
    if (str == NULL) {
        return NULL;
    }
    char* s = str;
    while (*str) {
        *str = FXSYS_toupper(*str);
        str ++;
    }
    return s;
}
FX_WCHAR* FXSYS_wcslwr(FX_WCHAR* str)
{
    if (str == NULL) {
        return NULL;
    }
    FX_WCHAR* s = str;
    while (*str) {
        *str = FXSYS_tolower(*str);
        str ++;
    }
    return s;
}
FX_WCHAR* FXSYS_wcsupr(FX_WCHAR* str)
{
    if (str == NULL) {
        return NULL;
    }
    FX_WCHAR* s = str;
    while (*str) {
        *str = FXSYS_toupper(*str);
        str ++;
    }
    return s;
}
int FXSYS_stricmp(const char*dst, const char*src)
{
    int f, l;
    do {
        if ( ((f = (unsigned char)(*(dst++))) >= 'A') && (f <= 'Z') ) {
            f -= ('A' - 'a');
        }
        if ( ((l = (unsigned char)(*(src++))) >= 'A') && (l <= 'Z') ) {
            l -= ('A' - 'a');
        }
    } while ( f && (f == l) );
    return(f - l);
}
int FXSYS_wcsicmp(const FX_WCHAR *dst, const FX_WCHAR *src)
{
    FX_WCHAR f, l;
    do {
        if ( ((f = (FX_WCHAR)(*(dst++))) >= 'A') && (f <= 'Z') ) {
            f -= ('A' - 'a');
        }
        if ( ((l = (FX_WCHAR)(*(src++))) >= 'A') && (l <= 'Z') ) {
            l -= ('A' - 'a');
        }
    } while ( f && (f == l) );
    return(f - l);
}
char* FXSYS_itoa(int value, char* string, int radix)
{
    return FXSYS_IntToStr<FX_INT32, FX_LPSTR>(value, string, radix);
}
#ifdef __cplusplus
}
#endif
#endif
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
#ifdef __cplusplus
extern "C" {
#endif
int FXSYS_WideCharToMultiByte(FX_DWORD codepage, FX_DWORD dwFlags, FX_LPCWSTR wstr, int wlen,
                              FX_LPSTR buf, int buflen, FX_LPCSTR default_str, FX_BOOL* pUseDefault)
{
    int len = 0;
    for (int i = 0; i < wlen; i ++) {
        if (wstr[i] < 0x100) {
            if (buf && len < buflen) {
                buf[len] = (FX_CHAR)wstr[i];
            }
            len ++;
        }
    }
    return len;
}
int FXSYS_MultiByteToWideChar(FX_DWORD codepage, FX_DWORD dwFlags, FX_LPCSTR bstr, int blen,
                              FX_LPWSTR buf, int buflen)
{
    int wlen = 0;
    for (int i = 0; i < blen; i ++) {
        if (buf && wlen < buflen) {
            buf[wlen] = bstr[i];
        }
        wlen ++;
    }
    return wlen;
}
#ifdef __cplusplus
}
#endif
#endif
