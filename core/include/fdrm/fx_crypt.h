// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FDRM_FX_CRYPT_H_
#define CORE_INCLUDE_FDRM_FX_CRYPT_H_

#include "core/include/fxcrt/fx_system.h"

#ifdef __cplusplus
extern "C" {
#endif

void CRYPT_ArcFourCryptBlock(uint8_t* data,
                             FX_DWORD size,
                             const uint8_t* key,
                             FX_DWORD keylen);
void CRYPT_ArcFourSetup(void* context, const uint8_t* key, FX_DWORD length);
void CRYPT_ArcFourCrypt(void* context, uint8_t* data, FX_DWORD size);
void CRYPT_AESSetKey(void* context,
                     FX_DWORD blocklen,
                     const uint8_t* key,
                     FX_DWORD keylen,
                     FX_BOOL bEncrypt);
void CRYPT_AESSetIV(void* context, const uint8_t* iv);
void CRYPT_AESDecrypt(void* context,
                      uint8_t* dest,
                      const uint8_t* src,
                      FX_DWORD size);
void CRYPT_AESEncrypt(void* context,
                      uint8_t* dest,
                      const uint8_t* src,
                      FX_DWORD size);
void CRYPT_MD5Generate(const uint8_t* data, FX_DWORD size, uint8_t digest[16]);
void CRYPT_MD5Start(void* context);
void CRYPT_MD5Update(void* context, const uint8_t* data, FX_DWORD size);
void CRYPT_MD5Finish(void* context, uint8_t digest[16]);
void CRYPT_SHA1Generate(const uint8_t* data, FX_DWORD size, uint8_t digest[20]);
void CRYPT_SHA1Start(void* context);
void CRYPT_SHA1Update(void* context, const uint8_t* data, FX_DWORD size);
void CRYPT_SHA1Finish(void* context, uint8_t digest[20]);
void CRYPT_SHA256Generate(const uint8_t* data,
                          FX_DWORD size,
                          uint8_t digest[32]);
void CRYPT_SHA256Start(void* context);
void CRYPT_SHA256Update(void* context, const uint8_t* data, FX_DWORD size);
void CRYPT_SHA256Finish(void* context, uint8_t digest[32]);
void CRYPT_SHA384Start(void* context);
void CRYPT_SHA384Update(void* context, const uint8_t* data, FX_DWORD size);
void CRYPT_SHA384Finish(void* context, uint8_t digest[48]);
void CRYPT_SHA384Generate(const uint8_t* data,
                          FX_DWORD size,
                          uint8_t digest[48]);
void CRYPT_SHA512Start(void* context);
void CRYPT_SHA512Update(void* context, const uint8_t* data, FX_DWORD size);
void CRYPT_SHA512Finish(void* context, uint8_t digest[64]);
void CRYPT_SHA512Generate(const uint8_t* data,
                          FX_DWORD size,
                          uint8_t digest[64]);
void CRYPT_SetPubKeyDecryptor(FX_BOOL (*func)(const uint8_t* pData,
                                              FX_DWORD size,
                                              uint8_t* data_buf,
                                              FX_DWORD& data_len));

#ifdef __cplusplus
};
#endif

#endif  // CORE_INCLUDE_FDRM_FX_CRYPT_H_
