// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FDRM_FX_CRYPT_H_
#define CORE_FDRM_FX_CRYPT_H_

#include <stdint.h>

#include "third_party/base/containers/span.h"

struct CRYPT_rc4_context {
  static constexpr int32_t kPermutationLength = 256;

  int32_t x;
  int32_t y;
  int32_t m[kPermutationLength];
};

struct CRYPT_aes_context {
  static constexpr int kMaxNb = 8;
  static constexpr int kMaxNr = 14;
  static constexpr int kSchedSize = (kMaxNr + 1) * kMaxNb;

  int Nb;
  int Nr;
  unsigned int keysched[kSchedSize];
  unsigned int invkeysched[kSchedSize];
  unsigned int iv[kMaxNb];
};

struct CRYPT_md5_context {
  uint32_t total[2];
  uint32_t state[4];
  uint8_t buffer[64];
};

struct CRYPT_sha1_context {
  uint64_t total_bytes;
  uint32_t blkused;  // Constrained to [0, 64).
  uint32_t h[5];
  uint8_t block[64];
};

struct CRYPT_sha2_context {
  uint64_t total_bytes;
  uint64_t state[8];
  uint8_t buffer[128];
};

void CRYPT_ArcFourCryptBlock(pdfium::span<uint8_t> data,
                             pdfium::span<const uint8_t> key);
void CRYPT_ArcFourSetup(CRYPT_rc4_context* context,
                        pdfium::span<const uint8_t> key);
void CRYPT_ArcFourCrypt(CRYPT_rc4_context* context, pdfium::span<uint8_t> data);

void CRYPT_AESSetKey(CRYPT_aes_context* context,
                     const uint8_t* key,
                     uint32_t keylen);
void CRYPT_AESSetIV(CRYPT_aes_context* context, const uint8_t* iv);
void CRYPT_AESDecrypt(CRYPT_aes_context* context,
                      uint8_t* dest,
                      const uint8_t* src,
                      uint32_t size);
void CRYPT_AESEncrypt(CRYPT_aes_context* context,
                      uint8_t* dest,
                      const uint8_t* src,
                      uint32_t size);

CRYPT_md5_context CRYPT_MD5Start();
void CRYPT_MD5Update(CRYPT_md5_context* context,
                     pdfium::span<const uint8_t> data);
void CRYPT_MD5Finish(CRYPT_md5_context* context, uint8_t digest[16]);
void CRYPT_MD5Generate(pdfium::span<const uint8_t> data, uint8_t digest[16]);

void CRYPT_SHA1Start(CRYPT_sha1_context* context);
void CRYPT_SHA1Update(CRYPT_sha1_context* context,
                      const uint8_t* data,
                      uint32_t size);
void CRYPT_SHA1Finish(CRYPT_sha1_context* context, uint8_t digest[20]);
void CRYPT_SHA1Generate(const uint8_t* data, uint32_t size, uint8_t digest[20]);

void CRYPT_SHA256Start(CRYPT_sha2_context* context);
void CRYPT_SHA256Update(CRYPT_sha2_context* context,
                        const uint8_t* data,
                        uint32_t size);
void CRYPT_SHA256Finish(CRYPT_sha2_context* context, uint8_t digest[32]);
void CRYPT_SHA256Generate(const uint8_t* data,
                          uint32_t size,
                          uint8_t digest[32]);

void CRYPT_SHA384Start(CRYPT_sha2_context* context);
void CRYPT_SHA384Update(CRYPT_sha2_context* context,
                        const uint8_t* data,
                        uint32_t size);
void CRYPT_SHA384Finish(CRYPT_sha2_context* context, uint8_t digest[48]);
void CRYPT_SHA384Generate(const uint8_t* data,
                          uint32_t size,
                          uint8_t digest[48]);

void CRYPT_SHA512Start(CRYPT_sha2_context* context);
void CRYPT_SHA512Update(CRYPT_sha2_context* context,
                        const uint8_t* data,
                        uint32_t size);
void CRYPT_SHA512Finish(CRYPT_sha2_context* context, uint8_t digest[64]);
void CRYPT_SHA512Generate(const uint8_t* data,
                          uint32_t size,
                          uint8_t digest[64]);

#endif  // CORE_FDRM_FX_CRYPT_H_
