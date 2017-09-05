// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_random.h"

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"

#define MT_N 848
#define MT_M 456
#define MT_Matrix_A 0x9908b0df
#define MT_Upper_Mask 0x80000000
#define MT_Lower_Mask 0x7fffffff

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#include <wincrypt.h>
#else  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#include <ctime>
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

namespace {

struct MTContext {
  MTContext() {
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

void* FX_Random_MT_Start(uint32_t dwSeed) {
  MTContext* pContext = FX_Alloc(MTContext, 1);
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

  MTContext* pMTC = static_cast<MTContext*>(pContext);
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
