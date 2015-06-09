// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_EXT_H_
#define CORE_INCLUDE_FXCRT_FX_EXT_H_

#include "fx_arb.h"
#include "fx_basic.h"
#include "fx_coordinates.h"
#include "fx_ucd.h"
#include "fx_xml.h"

#ifdef __cplusplus
extern "C" {
#endif

FX_FLOAT		FXSYS_tan(FX_FLOAT a);
FX_FLOAT		FXSYS_logb(FX_FLOAT b, FX_FLOAT x);
FX_FLOAT		FXSYS_strtof(FX_LPCSTR pcsStr, int32_t iLength = -1, int32_t *pUsedLen = NULL);
FX_FLOAT		FXSYS_wcstof(FX_LPCWSTR pwsStr, int32_t iLength = -1, int32_t *pUsedLen = NULL);
FX_LPWSTR		FXSYS_wcsncpy(FX_LPWSTR dstStr, FX_LPCWSTR srcStr, size_t count);
int32_t		FXSYS_wcsnicmp(FX_LPCWSTR s1, FX_LPCWSTR s2, size_t count);
int32_t		FXSYS_strnicmp(FX_LPCSTR s1, FX_LPCSTR s2, size_t count);

inline FX_BOOL	FXSYS_islower(int32_t ch)
{
    return ch >= 'a' && ch <= 'z';
}
inline FX_BOOL	FXSYS_isupper(int32_t ch)
{
    return ch >= 'A' && ch <= 'Z';
}
inline int32_t	FXSYS_tolower(int32_t ch)
{
    return ch < 'A' || ch > 'Z' ? ch : (ch + 0x20);
}
inline int32_t FXSYS_toupper(int32_t ch)
{
    return ch < 'a' || ch > 'z' ? ch : (ch - 0x20);
}

FX_DWORD	FX_HashCode_String_GetA(FX_LPCSTR pStr, int32_t iLength, FX_BOOL bIgnoreCase = FALSE);
FX_DWORD	FX_HashCode_String_GetW(FX_LPCWSTR pStr, int32_t iLength, FX_BOOL bIgnoreCase = FALSE);

#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif

FX_LPVOID	FX_Random_MT_Start(FX_DWORD dwSeed);

FX_DWORD	FX_Random_MT_Generate(FX_LPVOID pContext);

void		FX_Random_MT_Close(FX_LPVOID pContext);

void		FX_Random_GenerateBase(FX_LPDWORD pBuffer, int32_t iCount);

void		FX_Random_GenerateMT(FX_LPDWORD pBuffer, int32_t iCount);

void		FX_Random_GenerateCrypto(FX_LPDWORD pBuffer, int32_t iCount);
#ifdef __cplusplus
}
#endif
template<class baseType>
class CFX_SSortTemplate
{
public:
    void ShellSort(baseType *pArray, int32_t iCount)
    {
        FXSYS_assert(pArray != NULL && iCount > 0);
        int32_t i, j, gap;
        baseType v1, v2;
        gap = iCount >> 1;
        while (gap > 0) {
            for (i = gap; i < iCount; i ++) {
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
