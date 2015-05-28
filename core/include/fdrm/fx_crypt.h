// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FDRM_FX_CRYPT_H_
#define CORE_INCLUDE_FDRM_FX_CRYPT_H_

#include "../fxcrt/fx_basic.h"

#ifdef __cplusplus
extern "C" {
#endif

void CRYPT_ArcFourCryptBlock(FX_LPBYTE data, FX_DWORD size, FX_LPCBYTE key, FX_DWORD keylen);
void CRYPT_ArcFourSetup(FX_LPVOID context, FX_LPCBYTE key, FX_DWORD length);
void CRYPT_ArcFourCrypt(FX_LPVOID context, FX_LPBYTE data, FX_DWORD size);
void CRYPT_AESSetKey(FX_LPVOID context, FX_DWORD blocklen, FX_LPCBYTE key, FX_DWORD keylen, FX_BOOL bEncrypt);
void CRYPT_AESSetIV(FX_LPVOID context, FX_LPCBYTE iv);
void CRYPT_AESDecrypt(FX_LPVOID context, FX_LPBYTE dest, FX_LPCBYTE src, FX_DWORD size);
void CRYPT_AESEncrypt(FX_LPVOID context, FX_LPBYTE dest, FX_LPCBYTE src, FX_DWORD size);
void CRYPT_MD5Generate(FX_LPCBYTE data, FX_DWORD size, FX_BYTE digest[16]);
void CRYPT_MD5Start(FX_LPVOID context);
void CRYPT_MD5Update(FX_LPVOID context, FX_LPCBYTE data, FX_DWORD size);
void CRYPT_MD5Finish(FX_LPVOID context, FX_BYTE digest[16]);
void CRYPT_SHA1Generate(FX_LPCBYTE data, FX_DWORD size, FX_BYTE digest[20]);
void CRYPT_SHA1Start(FX_LPVOID context);
void CRYPT_SHA1Update(FX_LPVOID context, FX_LPCBYTE data, FX_DWORD size);
void CRYPT_SHA1Finish(FX_LPVOID context, FX_BYTE digest[20]);
void CRYPT_SHA256Generate(FX_LPCBYTE data, FX_DWORD size, FX_BYTE digest[32]);
void CRYPT_SHA256Start(FX_LPVOID context);
void CRYPT_SHA256Update(FX_LPVOID context, FX_LPCBYTE data, FX_DWORD size);
void CRYPT_SHA256Finish(FX_LPVOID context, FX_BYTE digest[32]);
void CRYPT_SHA384Start(FX_LPVOID context);
void CRYPT_SHA384Update(FX_LPVOID context, FX_LPCBYTE data, FX_DWORD size);
void CRYPT_SHA384Finish(FX_LPVOID context, FX_BYTE digest[48]);
void CRYPT_SHA384Generate(FX_LPCBYTE data, FX_DWORD size, FX_BYTE digest[48]);
void CRYPT_SHA512Start(FX_LPVOID context);
void CRYPT_SHA512Update(FX_LPVOID context, FX_LPCBYTE data, FX_DWORD size);
void CRYPT_SHA512Finish(FX_LPVOID context, FX_BYTE digest[64]);
void CRYPT_SHA512Generate(FX_LPCBYTE data, FX_DWORD size, FX_BYTE digest[64]);
void CRYPT_SetPubKeyDecryptor(FX_BOOL (*func)(FX_LPCBYTE pData, FX_DWORD size, FX_LPBYTE data_buf, FX_DWORD& data_len));

#ifdef __cplusplus
};
#endif

#endif  // CORE_INCLUDE_FDRM_FX_CRYPT_H_
