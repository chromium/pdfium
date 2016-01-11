// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxcrt/fx_basic.h"
#include "core/include/fxcrt/fx_ext.h"
#include "extension.h"

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#include <wincrypt.h>
#else
#include <ctime>
#endif

CFX_CRTFileStream::CFX_CRTFileStream(IFXCRT_FileAccess* pFA)
    : m_pFile(pFA), m_dwCount(1) {}

CFX_CRTFileStream::~CFX_CRTFileStream() {
  if (m_pFile) {
    m_pFile->Release();
  }
}

IFX_FileStream* CFX_CRTFileStream::Retain() {
  m_dwCount++;
  return this;
}

void CFX_CRTFileStream::Release() {
  FX_DWORD nCount = --m_dwCount;
  if (!nCount) {
    delete this;
  }
}

FX_FILESIZE CFX_CRTFileStream::GetSize() {
  return m_pFile->GetSize();
}

FX_BOOL CFX_CRTFileStream::IsEOF() {
  return GetPosition() >= GetSize();
}

FX_FILESIZE CFX_CRTFileStream::GetPosition() {
  return m_pFile->GetPosition();
}

FX_BOOL CFX_CRTFileStream::ReadBlock(void* buffer,
                                     FX_FILESIZE offset,
                                     size_t size) {
  return (FX_BOOL)m_pFile->ReadPos(buffer, size, offset);
}

size_t CFX_CRTFileStream::ReadBlock(void* buffer, size_t size) {
  return m_pFile->Read(buffer, size);
}

FX_BOOL CFX_CRTFileStream::WriteBlock(const void* buffer,
                                      FX_FILESIZE offset,
                                      size_t size) {
  return (FX_BOOL)m_pFile->WritePos(buffer, size, offset);
}

FX_BOOL CFX_CRTFileStream::Flush() {
  return m_pFile->Flush();
}

#ifdef PDF_ENABLE_XFA
IFX_FileAccess* FX_CreateDefaultFileAccess(const CFX_WideStringC& wsPath) {
  if (wsPath.GetLength() == 0)
    return NULL;

  CFX_CRTFileAccess* pFA = NULL;
  pFA = new CFX_CRTFileAccess;
  if (NULL == pFA)
    return NULL;

  pFA->Init(wsPath);
  return pFA;
}
#endif  // PDF_ENABLE_XFA

IFX_FileStream* FX_CreateFileStream(const FX_CHAR* filename, FX_DWORD dwModes) {
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
IFX_FileStream* FX_CreateFileStream(const FX_WCHAR* filename,
                                    FX_DWORD dwModes) {
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
IFX_FileRead* FX_CreateFileRead(const FX_CHAR* filename) {
  return FX_CreateFileStream(filename, FX_FILEMODE_ReadOnly);
}
IFX_FileRead* FX_CreateFileRead(const FX_WCHAR* filename) {
  return FX_CreateFileStream(filename, FX_FILEMODE_ReadOnly);
}
IFX_MemoryStream* FX_CreateMemoryStream(uint8_t* pBuffer,
                                        size_t dwSize,
                                        FX_BOOL bTakeOver) {
  return new CFX_MemoryStream(pBuffer, dwSize, bTakeOver);
}
IFX_MemoryStream* FX_CreateMemoryStream(FX_BOOL bConsecutive) {
  return new CFX_MemoryStream(bConsecutive);
}

FX_FLOAT FXSYS_tan(FX_FLOAT a) {
  return (FX_FLOAT)tan(a);
}
FX_FLOAT FXSYS_logb(FX_FLOAT b, FX_FLOAT x) {
  return FXSYS_log(x) / FXSYS_log(b);
}
FX_FLOAT FXSYS_strtof(const FX_CHAR* pcsStr,
                      int32_t iLength,
                      int32_t* pUsedLen) {
  FXSYS_assert(pcsStr);
  if (iLength < 0) {
    iLength = (int32_t)FXSYS_strlen(pcsStr);
  }
  CFX_WideString ws = CFX_WideString::FromLocal(pcsStr, iLength);
  return FXSYS_wcstof(ws.c_str(), iLength, pUsedLen);
}
FX_FLOAT FXSYS_wcstof(const FX_WCHAR* pwsStr,
                      int32_t iLength,
                      int32_t* pUsedLen) {
  FXSYS_assert(pwsStr);
  if (iLength < 0) {
    iLength = (int32_t)FXSYS_wcslen(pwsStr);
  }
  if (iLength == 0) {
    return 0.0f;
  }
  int32_t iUsedLen = 0;
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
FX_WCHAR* FXSYS_wcsncpy(FX_WCHAR* dstStr,
                        const FX_WCHAR* srcStr,
                        size_t count) {
  FXSYS_assert(dstStr && srcStr && count > 0);
  for (size_t i = 0; i < count; ++i)
    if ((dstStr[i] = srcStr[i]) == L'\0') {
      break;
    }
  return dstStr;
}
int32_t FXSYS_wcsnicmp(const FX_WCHAR* s1, const FX_WCHAR* s2, size_t count) {
  FXSYS_assert(s1 && s2 && count > 0);
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
int32_t FXSYS_strnicmp(const FX_CHAR* s1, const FX_CHAR* s2, size_t count) {
  FXSYS_assert(s1 && s2 && count > 0);
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
FX_DWORD FX_HashCode_String_GetA(const FX_CHAR* pStr,
                                 int32_t iLength,
                                 FX_BOOL bIgnoreCase) {
  FXSYS_assert(pStr);
  if (iLength < 0) {
    iLength = (int32_t)FXSYS_strlen(pStr);
  }
  const FX_CHAR* pStrEnd = pStr + iLength;
  FX_DWORD dwHashCode = 0;
  if (bIgnoreCase) {
    while (pStr < pStrEnd) {
      dwHashCode = 31 * dwHashCode + FXSYS_tolower(*pStr++);
    }
  } else {
    while (pStr < pStrEnd) {
      dwHashCode = 31 * dwHashCode + *pStr++;
    }
  }
  return dwHashCode;
}
FX_DWORD FX_HashCode_String_GetW(const FX_WCHAR* pStr,
                                 int32_t iLength,
                                 FX_BOOL bIgnoreCase) {
  FXSYS_assert(pStr);
  if (iLength < 0) {
    iLength = (int32_t)FXSYS_wcslen(pStr);
  }
  const FX_WCHAR* pStrEnd = pStr + iLength;
  FX_DWORD dwHashCode = 0;
  if (bIgnoreCase) {
    while (pStr < pStrEnd) {
      dwHashCode = 1313 * dwHashCode + FXSYS_tolower(*pStr++);
    }
  } else {
    while (pStr < pStrEnd) {
      dwHashCode = 1313 * dwHashCode + *pStr++;
    }
  }
  return dwHashCode;
}

void* FX_Random_MT_Start(FX_DWORD dwSeed) {
  FX_LPMTRANDOMCONTEXT pContext = FX_Alloc(FX_MTRANDOMCONTEXT, 1);
  pContext->mt[0] = dwSeed;
  FX_DWORD& i = pContext->mti;
  FX_DWORD* pBuf = pContext->mt;
  for (i = 1; i < MT_N; i++) {
    pBuf[i] = (1812433253UL * (pBuf[i - 1] ^ (pBuf[i - 1] >> 30)) + i);
  }
  pContext->bHaveSeed = TRUE;
  return pContext;
}
FX_DWORD FX_Random_MT_Generate(void* pContext) {
  FXSYS_assert(pContext);
  FX_LPMTRANDOMCONTEXT pMTC = (FX_LPMTRANDOMCONTEXT)pContext;
  FX_DWORD v;
  static FX_DWORD mag[2] = {0, MT_Matrix_A};
  FX_DWORD& mti = pMTC->mti;
  FX_DWORD* pBuf = pMTC->mt;
  if ((int)mti < 0 || mti >= MT_N) {
    if (mti > MT_N && !pMTC->bHaveSeed) {
      return 0;
    }
    FX_DWORD kk;
    for (kk = 0; kk < MT_N - MT_M; kk++) {
      v = (pBuf[kk] & MT_Upper_Mask) | (pBuf[kk + 1] & MT_Lower_Mask);
      pBuf[kk] = pBuf[kk + MT_M] ^ (v >> 1) ^ mag[v & 1];
    }
    for (; kk < MT_N - 1; kk++) {
      v = (pBuf[kk] & MT_Upper_Mask) | (pBuf[kk + 1] & MT_Lower_Mask);
      pBuf[kk] = pBuf[kk + (MT_M - MT_N)] ^ (v >> 1) ^ mag[v & 1];
    }
    v = (pBuf[MT_N - 1] & MT_Upper_Mask) | (pBuf[0] & MT_Lower_Mask);
    pBuf[MT_N - 1] = pBuf[MT_M - 1] ^ (v >> 1) ^ mag[v & 1];
    mti = 0;
  }
  v = pBuf[mti++];
  v ^= (v >> 11);
  v ^= (v << 7) & 0x9d2c5680UL;
  v ^= (v << 15) & 0xefc60000UL;
  v ^= (v >> 18);
  return v;
}
void FX_Random_MT_Close(void* pContext) {
  FXSYS_assert(pContext);
  FX_Free(pContext);
}
void FX_Random_GenerateMT(FX_DWORD* pBuffer, int32_t iCount) {
  FX_DWORD dwSeed;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  if (!FX_GenerateCryptoRandom(&dwSeed, 1)) {
    FX_Random_GenerateBase(&dwSeed, 1);
  }
#else
  FX_Random_GenerateBase(&dwSeed, 1);
#endif
  void* pContext = FX_Random_MT_Start(dwSeed);
  while (iCount-- > 0) {
    *pBuffer++ = FX_Random_MT_Generate(pContext);
  }
  FX_Random_MT_Close(pContext);
}
void FX_Random_GenerateBase(FX_DWORD* pBuffer, int32_t iCount) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  SYSTEMTIME st1, st2;
  ::GetSystemTime(&st1);
  do {
    ::GetSystemTime(&st2);
  } while (FXSYS_memcmp(&st1, &st2, sizeof(SYSTEMTIME)) == 0);
  FX_DWORD dwHash1 =
      FX_HashCode_String_GetA((const FX_CHAR*)&st1, sizeof(st1), TRUE);
  FX_DWORD dwHash2 =
      FX_HashCode_String_GetA((const FX_CHAR*)&st2, sizeof(st2), TRUE);
  ::srand((dwHash1 << 16) | (FX_DWORD)dwHash2);
#else
  time_t tmLast = time(NULL), tmCur;
  while ((tmCur = time(NULL)) == tmLast)
    ;
  ::srand((tmCur << 16) | (tmLast & 0xFFFF));
#endif
  while (iCount-- > 0) {
    *pBuffer++ = (FX_DWORD)((::rand() << 16) | (::rand() & 0xFFFF));
  }
}
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
FX_BOOL FX_GenerateCryptoRandom(FX_DWORD* pBuffer, int32_t iCount) {
  HCRYPTPROV hCP = NULL;
  if (!::CryptAcquireContext(&hCP, NULL, NULL, PROV_RSA_FULL, 0) || !hCP) {
    return FALSE;
  }
  ::CryptGenRandom(hCP, iCount * sizeof(FX_DWORD), (uint8_t*)pBuffer);
  ::CryptReleaseContext(hCP, 0);
  return TRUE;
}
#endif
void FX_Random_GenerateCrypto(FX_DWORD* pBuffer, int32_t iCount) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  FX_GenerateCryptoRandom(pBuffer, iCount);
#else
  FX_Random_GenerateBase(pBuffer, iCount);
#endif
}

#ifdef PDF_ENABLE_XFA
void FX_GUID_CreateV4(FX_LPGUID pGUID) {
#if (_FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
     _FX_OS_ == _FX_WIN64_)
#ifdef _FX_WINAPI_PARTITION_DESKTOP_
  if (!FX_GenerateCryptoRandom((FX_DWORD*)pGUID, 4)) {
    FX_Random_GenerateMT((FX_DWORD*)pGUID, 4);
  }
#else
  FX_Random_GenerateMT((FX_DWORD*)pGUID, 4);
#endif
#else
  FX_Random_GenerateMT((FX_DWORD*)pGUID, 4);
#endif
  uint8_t& b = ((uint8_t*)pGUID)[6];
  b = (b & 0x0F) | 0x40;
}
const FX_CHAR* gs_FX_pHexChars = "0123456789ABCDEF";
void FX_GUID_ToString(FX_LPCGUID pGUID,
                      CFX_ByteString& bsStr,
                      FX_BOOL bSeparator) {
  FX_CHAR* pBuf = bsStr.GetBuffer(40);
  uint8_t b;
  for (int32_t i = 0; i < 16; i++) {
    b = ((const uint8_t*)pGUID)[i];
    *pBuf++ = gs_FX_pHexChars[b >> 4];
    *pBuf++ = gs_FX_pHexChars[b & 0x0F];
    if (bSeparator && (i == 3 || i == 5 || i == 7 || i == 9)) {
      *pBuf++ = L'-';
    }
  }
  bsStr.ReleaseBuffer(bSeparator ? 36 : 32);
}
#endif  // PDF_ENABLE_XFA
