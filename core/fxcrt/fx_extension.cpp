// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_extension.h"

#include <cwctype>

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#include <wincrypt.h>
#else
#include <ctime>
#endif

#define MT_N 848
#define MT_M 456
#define MT_Matrix_A 0x9908b0df
#define MT_Upper_Mask 0x80000000
#define MT_Lower_Mask 0x7fffffff

namespace {

struct FX_MTRANDOMCONTEXT {
  FX_MTRANDOMCONTEXT() {
    mti = MT_N + 1;
    bHaveSeed = false;
  }

  uint32_t mti;
  bool bHaveSeed;
  uint32_t mt[MT_N];
};

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
bool GenerateCryptoRandom(uint32_t* pBuffer, int32_t iCount) {
  HCRYPTPROV hCP = 0;
  if (!::CryptAcquireContext(&hCP, nullptr, nullptr, PROV_RSA_FULL, 0) ||
      !hCP) {
    return false;
  }
  ::CryptGenRandom(hCP, iCount * sizeof(uint32_t),
                   reinterpret_cast<uint8_t*>(pBuffer));
  ::CryptReleaseContext(hCP, 0);
  return true;
}
#endif

}  // namespace

float FXSYS_wcstof(const wchar_t* pwsStr, int32_t iLength, int32_t* pUsedLen) {
  ASSERT(pwsStr);
  if (iLength < 0)
    iLength = static_cast<int32_t>(FXSYS_wcslen(pwsStr));
  if (iLength == 0)
    return 0.0f;

  int32_t iUsedLen = 0;
  bool bNegtive = false;
  switch (pwsStr[iUsedLen]) {
    case '-':
      bNegtive = true;
    case '+':
      iUsedLen++;
      break;
  }

  float fValue = 0.0f;
  while (iUsedLen < iLength) {
    wchar_t wch = pwsStr[iUsedLen];
    if (!std::iswdigit(wch))
      break;

    fValue = fValue * 10.0f + (wch - L'0');
    iUsedLen++;
  }

  if (iUsedLen < iLength && pwsStr[iUsedLen] == L'.') {
    float fPrecise = 0.1f;
    while (++iUsedLen < iLength) {
      wchar_t wch = pwsStr[iUsedLen];
      if (!std::iswdigit(wch))
        break;

      fValue += (wch - L'0') * fPrecise;
      fPrecise *= 0.1f;
    }
  }
  if (pUsedLen)
    *pUsedLen = iUsedLen;

  return bNegtive ? -fValue : fValue;
}

wchar_t* FXSYS_wcsncpy(wchar_t* dstStr, const wchar_t* srcStr, size_t count) {
  ASSERT(dstStr && srcStr && count > 0);
  for (size_t i = 0; i < count; ++i)
    if ((dstStr[i] = srcStr[i]) == L'\0')
      break;
  return dstStr;
}

int32_t FXSYS_wcsnicmp(const wchar_t* s1, const wchar_t* s2, size_t count) {
  ASSERT(s1 && s2 && count > 0);
  wchar_t wch1 = 0, wch2 = 0;
  while (count-- > 0) {
    wch1 = static_cast<wchar_t>(FXSYS_tolower(*s1++));
    wch2 = static_cast<wchar_t>(FXSYS_tolower(*s2++));
    if (wch1 != wch2)
      break;
  }
  return wch1 - wch2;
}

uint32_t FX_HashCode_GetA(const CFX_ByteStringC& str, bool bIgnoreCase) {
  uint32_t dwHashCode = 0;
  if (bIgnoreCase) {
    for (const auto& c : str)
      dwHashCode = 31 * dwHashCode + FXSYS_tolower(c);
  } else {
    for (const auto& c : str)
      dwHashCode = 31 * dwHashCode + c;
  }
  return dwHashCode;
}

uint32_t FX_HashCode_GetW(const CFX_WideStringC& str, bool bIgnoreCase) {
  uint32_t dwHashCode = 0;
  if (bIgnoreCase) {
    for (const auto& c : str)
      dwHashCode = 1313 * dwHashCode + FXSYS_tolower(c);
  } else {
    for (const auto& c : str)
      dwHashCode = 1313 * dwHashCode + c;
  }
  return dwHashCode;
}

void FXSYS_IntToTwoHexChars(uint8_t n, char* buf) {
  static const char kHex[] = "0123456789ABCDEF";
  buf[0] = kHex[n / 16];
  buf[1] = kHex[n % 16];
}

void FXSYS_IntToFourHexChars(uint16_t n, char* buf) {
  FXSYS_IntToTwoHexChars(n / 256, buf);
  FXSYS_IntToTwoHexChars(n % 256, buf + 2);
}

size_t FXSYS_ToUTF16BE(uint32_t unicode, char* buf) {
  ASSERT(unicode <= 0xD7FF || (unicode > 0xDFFF && unicode <= 0x10FFFF));
  if (unicode <= 0xFFFF) {
    FXSYS_IntToFourHexChars(unicode, buf);
    return 4;
  }
  unicode -= 0x010000;
  // High ten bits plus 0xD800
  FXSYS_IntToFourHexChars(0xD800 + unicode / 0x400, buf);
  // Low ten bits plus 0xDC00
  FXSYS_IntToFourHexChars(0xDC00 + unicode % 0x400, buf + 4);
  return 8;
}

void* FX_Random_MT_Start(uint32_t dwSeed) {
  FX_MTRANDOMCONTEXT* pContext = FX_Alloc(FX_MTRANDOMCONTEXT, 1);
  pContext->mt[0] = dwSeed;
  uint32_t& i = pContext->mti;
  uint32_t* pBuf = pContext->mt;
  for (i = 1; i < MT_N; i++)
    pBuf[i] = (1812433253UL * (pBuf[i - 1] ^ (pBuf[i - 1] >> 30)) + i);

  pContext->bHaveSeed = true;
  return pContext;
}

uint32_t FX_Random_MT_Generate(void* pContext) {
  ASSERT(pContext);
  FX_MTRANDOMCONTEXT* pMTC = static_cast<FX_MTRANDOMCONTEXT*>(pContext);
  uint32_t v;
  static uint32_t mag[2] = {0, MT_Matrix_A};
  uint32_t& mti = pMTC->mti;
  uint32_t* pBuf = pMTC->mt;
  if ((int)mti < 0 || mti >= MT_N) {
    if (mti > MT_N && !pMTC->bHaveSeed)
      return 0;

    uint32_t kk;
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
  ASSERT(pContext);
  FX_Free(pContext);
}

void FX_Random_GenerateMT(uint32_t* pBuffer, int32_t iCount) {
  uint32_t dwSeed;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  if (!GenerateCryptoRandom(&dwSeed, 1))
    FX_Random_GenerateBase(&dwSeed, 1);
#else
  FX_Random_GenerateBase(&dwSeed, 1);
#endif
  void* pContext = FX_Random_MT_Start(dwSeed);
  while (iCount-- > 0)
    *pBuffer++ = FX_Random_MT_Generate(pContext);

  FX_Random_MT_Close(pContext);
}

void FX_Random_GenerateBase(uint32_t* pBuffer, int32_t iCount) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  SYSTEMTIME st1, st2;
  ::GetSystemTime(&st1);
  do {
    ::GetSystemTime(&st2);
  } while (memcmp(&st1, &st2, sizeof(SYSTEMTIME)) == 0);
  uint32_t dwHash1 =
      FX_HashCode_GetA(CFX_ByteStringC((uint8_t*)&st1, sizeof(st1)), true);
  uint32_t dwHash2 =
      FX_HashCode_GetA(CFX_ByteStringC((uint8_t*)&st2, sizeof(st2)), true);
  ::srand((dwHash1 << 16) | (uint32_t)dwHash2);
#else
  time_t tmLast = time(nullptr);
  time_t tmCur;
  while ((tmCur = time(nullptr)) == tmLast)
    continue;

  ::srand((tmCur << 16) | (tmLast & 0xFFFF));
#endif
  while (iCount-- > 0)
    *pBuffer++ = static_cast<uint32_t>((::rand() << 16) | (::rand() & 0xFFFF));
}

#ifdef PDF_ENABLE_XFA
void FX_GUID_CreateV4(FX_GUID* pGUID) {
  FX_Random_GenerateMT((uint32_t*)pGUID, 4);
  uint8_t& b = ((uint8_t*)pGUID)[6];
  b = (b & 0x0F) | 0x40;
}

CFX_ByteString FX_GUID_ToString(const FX_GUID* pGUID, bool bSeparator) {
  CFX_ByteString bsStr;
  char* pBuf = bsStr.GetBuffer(40);
  for (int32_t i = 0; i < 16; i++) {
    uint8_t b = reinterpret_cast<const uint8_t*>(pGUID)[i];
    FXSYS_IntToTwoHexChars(b, pBuf);
    pBuf += 2;
    if (bSeparator && (i == 3 || i == 5 || i == 7 || i == 9))
      *pBuf++ = L'-';
  }
  bsStr.ReleaseBuffer(bSeparator ? 36 : 32);
  return bsStr;
}
#endif  // PDF_ENABLE_XFA
