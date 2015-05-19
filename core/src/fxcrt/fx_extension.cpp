// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_ext.h"
#include "extension.h"
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#include <wincrypt.h>
#else
#include <ctime>
#endif
FX_HFILE FX_File_Open(FX_BSTR fileName, FX_DWORD dwMode)
{
    IFXCRT_FileAccess* pFA = FXCRT_FileAccess_Create();
    if (pFA && !pFA->Open(fileName, dwMode)) {
        pFA->Release();
        return NULL;
    }
    return (FX_HFILE)pFA;
}
FX_HFILE FX_File_Open(FX_WSTR fileName, FX_DWORD dwMode)
{
    IFXCRT_FileAccess* pFA = FXCRT_FileAccess_Create();
    if (pFA && !pFA->Open(fileName, dwMode)) {
        pFA->Release();
        return NULL;
    }
    return (FX_HFILE)pFA;
}
void FX_File_Close(FX_HFILE hFile)
{
    FXSYS_assert(hFile != NULL);
    ((IFXCRT_FileAccess*)hFile)->Close();
    ((IFXCRT_FileAccess*)hFile)->Release();
}
FX_FILESIZE FX_File_GetSize(FX_HFILE hFile)
{
    FXSYS_assert(hFile != NULL);
    return ((IFXCRT_FileAccess*)hFile)->GetSize();
}
FX_FILESIZE FX_File_GetPosition(FX_HFILE hFile)
{
    FXSYS_assert(hFile != NULL);
    return ((IFXCRT_FileAccess*)hFile)->GetPosition();
}
FX_FILESIZE FX_File_SetPosition(FX_HFILE hFile, FX_FILESIZE pos)
{
    FXSYS_assert(hFile != NULL);
    return ((IFXCRT_FileAccess*)hFile)->SetPosition(pos);
}
size_t FX_File_Read(FX_HFILE hFile, void* pBuffer, size_t szBuffer)
{
    FXSYS_assert(hFile != NULL);
    return ((IFXCRT_FileAccess*)hFile)->Read(pBuffer, szBuffer);
}
size_t FX_File_ReadPos(FX_HFILE hFile, void* pBuffer, size_t szBuffer, FX_FILESIZE pos)
{
    FXSYS_assert(hFile != NULL);
    return ((IFXCRT_FileAccess*)hFile)->ReadPos(pBuffer, szBuffer, pos);
}
size_t FX_File_Write(FX_HFILE hFile, const void* pBuffer, size_t szBuffer)
{
    FXSYS_assert(hFile != NULL);
    return ((IFXCRT_FileAccess*)hFile)->Write(pBuffer, szBuffer);
}
size_t FX_File_WritePos(FX_HFILE hFile, const void* pBuffer, size_t szBuffer, FX_FILESIZE pos)
{
    FXSYS_assert(hFile != NULL);
    return ((IFXCRT_FileAccess*)hFile)->WritePos(pBuffer, szBuffer, pos);
}
FX_BOOL FX_File_Flush(FX_HFILE hFile)
{
    FXSYS_assert(hFile != NULL);
    return ((IFXCRT_FileAccess*)hFile)->Flush();
}
FX_BOOL FX_File_Truncate(FX_HFILE hFile, FX_FILESIZE szFile)
{
    FXSYS_assert(hFile != NULL);
    return ((IFXCRT_FileAccess*)hFile)->Truncate(szFile);
}
IFX_FileStream* FX_CreateFileStream(FX_LPCSTR filename, FX_DWORD dwModes)
{
    IFXCRT_FileAccess* pFA = FXCRT_FileAccess_Create();
    if (!pFA) {
        return NULL;
    }
    if (!pFA->Open(filename, dwModes)) {
        pFA->Release();
        return NULL;
    }
    return new CFX_CRTFileStream(pFA);
}
IFX_FileStream* FX_CreateFileStream(FX_LPCWSTR filename, FX_DWORD dwModes)
{
    IFXCRT_FileAccess* pFA = FXCRT_FileAccess_Create();
    if (!pFA) {
        return NULL;
    }
    if (!pFA->Open(filename, dwModes)) {
        pFA->Release();
        return NULL;
    }
    return new CFX_CRTFileStream(pFA);
}
IFX_FileWrite* FX_CreateFileWrite(FX_LPCSTR filename)
{
    return FX_CreateFileStream(filename, FX_FILEMODE_Truncate);
}
IFX_FileWrite* FX_CreateFileWrite(FX_LPCWSTR filename)
{
    return FX_CreateFileStream(filename, FX_FILEMODE_Truncate);
}
IFX_FileRead* FX_CreateFileRead(FX_LPCSTR filename)
{
    return FX_CreateFileStream(filename, FX_FILEMODE_ReadOnly);
}
IFX_FileRead* FX_CreateFileRead(FX_LPCWSTR filename)
{
    return FX_CreateFileStream(filename, FX_FILEMODE_ReadOnly);
}
IFX_MemoryStream* FX_CreateMemoryStream(FX_LPBYTE pBuffer, size_t dwSize, FX_BOOL bTakeOver)
{
    return new CFX_MemoryStream(pBuffer, dwSize, bTakeOver);
}
IFX_MemoryStream* FX_CreateMemoryStream(FX_BOOL bConsecutive)
{
    return new CFX_MemoryStream(bConsecutive);
}
#ifdef __cplusplus
extern "C" {
#endif
FX_FLOAT FXSYS_tan(FX_FLOAT a)
{
    return (FX_FLOAT)tan(a);
}
FX_FLOAT FXSYS_logb(FX_FLOAT b, FX_FLOAT x)
{
    return FXSYS_log(x) / FXSYS_log(b);
}
FX_FLOAT FXSYS_strtof(FX_LPCSTR pcsStr, FX_INT32 iLength, FX_INT32 *pUsedLen)
{
    FXSYS_assert(pcsStr != NULL);
    if (iLength < 0) {
        iLength = (FX_INT32)FXSYS_strlen(pcsStr);
    }
    CFX_WideString ws = CFX_WideString::FromLocal(pcsStr, iLength);
    return FXSYS_wcstof(ws.c_str(), iLength, pUsedLen);
}
FX_FLOAT FXSYS_wcstof(FX_LPCWSTR pwsStr, FX_INT32 iLength, FX_INT32 *pUsedLen)
{
    FXSYS_assert(pwsStr != NULL);
    if (iLength < 0) {
        iLength = (FX_INT32)FXSYS_wcslen(pwsStr);
    }
    if (iLength == 0) {
        return 0.0f;
    }
    FX_INT32 iUsedLen = 0;
    FX_BOOL bNegtive = FALSE;
    switch (pwsStr[iUsedLen]) {
        case '-':
            bNegtive = TRUE;
        case '+':
            iUsedLen++;
            break;
    }
    FX_FLOAT fValue = 0.0f;
    while (iUsedLen < iLength) {
        FX_WCHAR wch = pwsStr[iUsedLen];
        if (wch >= L'0' && wch <= L'9') {
            fValue = fValue * 10.0f + (wch - L'0');
        } else {
            break;
        }
        iUsedLen++;
    }
    if (iUsedLen < iLength && pwsStr[iUsedLen] == L'.') {
        FX_FLOAT fPrecise = 0.1f;
        while (++iUsedLen < iLength) {
            FX_WCHAR wch = pwsStr[iUsedLen];
            if (wch >= L'0' && wch <= L'9') {
                fValue += (wch - L'0') * fPrecise;
                fPrecise *= 0.1f;
            } else {
                break;
            }
        }
    }
    if (pUsedLen) {
        *pUsedLen = iUsedLen;
    }
    return bNegtive ? -fValue : fValue;
}
FX_LPWSTR FXSYS_wcsncpy(FX_LPWSTR dstStr, FX_LPCWSTR srcStr, size_t count)
{
    FXSYS_assert(dstStr != NULL && srcStr != NULL && count > 0);
    for (size_t i = 0; i < count; ++i)
        if ((dstStr[i] = srcStr[i]) == L'\0') {
            break;
        }
    return dstStr;
}
FX_INT32 FXSYS_wcsnicmp(FX_LPCWSTR s1, FX_LPCWSTR s2, size_t count)
{
    FXSYS_assert(s1 != NULL && s2 != NULL && count > 0);
    FX_WCHAR wch1 = 0, wch2 = 0;
    while (count-- > 0) {
        wch1 = (FX_WCHAR)FXSYS_tolower(*s1++);
        wch2 = (FX_WCHAR)FXSYS_tolower(*s2++);
        if (wch1 != wch2) {
            break;
        }
    }
    return wch1 - wch2;
}
FX_INT32 FXSYS_strnicmp(FX_LPCSTR s1, FX_LPCSTR s2, size_t count)
{
    FXSYS_assert(s1 != NULL && s2 != NULL && count > 0);
    FX_CHAR ch1 = 0, ch2 = 0;
    while (count-- > 0) {
        ch1 = (FX_CHAR)FXSYS_tolower(*s1++);
        ch2 = (FX_CHAR)FXSYS_tolower(*s2++);
        if (ch1 != ch2) {
            break;
        }
    }
    return ch1 - ch2;
}
FX_DWORD FX_HashCode_String_GetA(FX_LPCSTR pStr, FX_INT32 iLength, FX_BOOL bIgnoreCase)
{
    FXSYS_assert(pStr != NULL);
    if (iLength < 0) {
        iLength = (FX_INT32)FXSYS_strlen(pStr);
    }
    FX_LPCSTR pStrEnd = pStr + iLength;
    FX_DWORD dwHashCode = 0;
    if (bIgnoreCase) {
        while (pStr < pStrEnd) {
            dwHashCode = 31 * dwHashCode + FXSYS_tolower(*pStr++);
        }
    } else {
        while (pStr < pStrEnd) {
            dwHashCode = 31 * dwHashCode + *pStr ++;
        }
    }
    return dwHashCode;
}
FX_DWORD FX_HashCode_String_GetW(FX_LPCWSTR pStr, FX_INT32 iLength, FX_BOOL bIgnoreCase)
{
    FXSYS_assert(pStr != NULL);
    if (iLength < 0) {
        iLength = (FX_INT32)FXSYS_wcslen(pStr);
    }
    FX_LPCWSTR pStrEnd = pStr + iLength;
    FX_DWORD dwHashCode = 0;
    if (bIgnoreCase) {
        while (pStr < pStrEnd) {
            dwHashCode = 1313 * dwHashCode + FXSYS_tolower(*pStr++);
        }
    } else {
        while (pStr < pStrEnd) {
            dwHashCode = 1313 * dwHashCode + *pStr ++;
        }
    }
    return dwHashCode;
}
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
FX_LPVOID FX_Random_MT_Start(FX_DWORD dwSeed)
{
    FX_LPMTRANDOMCONTEXT pContext = FX_Alloc(FX_MTRANDOMCONTEXT, 1);
    pContext->mt[0] = dwSeed;
    FX_DWORD &i = pContext->mti;
    FX_LPDWORD pBuf = pContext->mt;
    for (i = 1; i < MT_N; i ++) {
        pBuf[i] = (1812433253UL * (pBuf[i - 1] ^ (pBuf[i - 1] >> 30)) + i);
    }
    pContext->bHaveSeed = TRUE;
    return pContext;
}
FX_DWORD FX_Random_MT_Generate(FX_LPVOID pContext)
{
    FXSYS_assert(pContext != NULL);
    FX_LPMTRANDOMCONTEXT pMTC = (FX_LPMTRANDOMCONTEXT)pContext;
    FX_DWORD v;
    static FX_DWORD mag[2] = {0, MT_Matrix_A};
    FX_DWORD &mti = pMTC->mti;
    FX_LPDWORD pBuf = pMTC->mt;
    if ((int)mti < 0 || mti >= MT_N) {
        if (mti > MT_N && !pMTC->bHaveSeed) {
            return 0;
        }
        FX_DWORD kk;
        for (kk = 0; kk < MT_N - MT_M; kk ++) {
            v = (pBuf[kk] & MT_Upper_Mask) | (pBuf[kk + 1] & MT_Lower_Mask);
            pBuf[kk] = pBuf[kk + MT_M] ^ (v >> 1) ^ mag[v & 1];
        }
        for (; kk < MT_N - 1; kk ++) {
            v = (pBuf[kk] & MT_Upper_Mask) | (pBuf[kk + 1] & MT_Lower_Mask);
            pBuf[kk] = pBuf[kk + (MT_M - MT_N)] ^ (v >> 1) ^ mag[v & 1];
        }
        v = (pBuf[MT_N - 1] & MT_Upper_Mask) | (pBuf[0] & MT_Lower_Mask);
        pBuf[MT_N - 1] = pBuf[MT_M - 1] ^ (v >> 1) ^ mag[v & 1];
        mti = 0;
    }
    v = pBuf[mti ++];
    v ^= (v >> 11);
    v ^= (v << 7) & 0x9d2c5680UL;
    v ^= (v << 15) & 0xefc60000UL;
    v ^= (v >> 18);
    return v;
}
void FX_Random_MT_Close(FX_LPVOID pContext)
{
    FXSYS_assert(pContext != NULL);
    FX_Free(pContext);
}
void FX_Random_GenerateMT(FX_LPDWORD pBuffer, FX_INT32 iCount)
{
    FX_DWORD dwSeed;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    if (!FX_GenerateCryptoRandom(&dwSeed, 1)) {
        FX_Random_GenerateBase(&dwSeed, 1);
    }
#else
    FX_Random_GenerateBase(&dwSeed, 1);
#endif
    FX_LPVOID pContext = FX_Random_MT_Start(dwSeed);
    while (iCount -- > 0) {
        *pBuffer ++ = FX_Random_MT_Generate(pContext);
    }
    FX_Random_MT_Close(pContext);
}
void FX_Random_GenerateBase(FX_LPDWORD pBuffer, FX_INT32 iCount)
{
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    SYSTEMTIME st1, st2;
    ::GetSystemTime(&st1);
    do {
        ::GetSystemTime(&st2);
    } while (FXSYS_memcmp32(&st1, &st2, sizeof(SYSTEMTIME)) == 0);
    FX_DWORD dwHash1 = FX_HashCode_String_GetA((FX_LPCSTR)&st1, sizeof(st1), TRUE);
    FX_DWORD dwHash2 = FX_HashCode_String_GetA((FX_LPCSTR)&st2, sizeof(st2), TRUE);
    ::srand((dwHash1 << 16) | (FX_DWORD)dwHash2);
#else
    time_t tmLast = time(NULL), tmCur;
    while ((tmCur = time(NULL)) == tmLast);
    ::srand((tmCur << 16) | (tmLast & 0xFFFF));
#endif
    while (iCount -- > 0) {
        *pBuffer ++ = (FX_DWORD)((::rand() << 16) | (::rand() & 0xFFFF));
    }
}
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
FX_BOOL FX_GenerateCryptoRandom(FX_LPDWORD pBuffer, FX_INT32 iCount)
{
    HCRYPTPROV hCP = NULL;
    if (!::CryptAcquireContext(&hCP, NULL, NULL, PROV_RSA_FULL, 0) || hCP == NULL) {
        return FALSE;
    }
    ::CryptGenRandom(hCP, iCount * sizeof(FX_DWORD), (FX_LPBYTE)pBuffer);
    ::CryptReleaseContext(hCP, 0);
    return TRUE;
}
#endif
void FX_Random_GenerateCrypto(FX_LPDWORD pBuffer, FX_INT32 iCount)
{
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    FX_GenerateCryptoRandom(pBuffer, iCount);
#else
    FX_Random_GenerateBase(pBuffer, iCount);
#endif
}
#ifdef __cplusplus
}
#endif
