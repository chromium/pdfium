// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FDRM_FX_CRYPT_AES_H_
#define CORE_FDRM_FX_CRYPT_AES_H_

#include <stdint.h>

#include <array>

#include "core/fxcrt/span.h"

struct CRYPT_aes_context {
  static constexpr size_t kBlockSize = 4;       // Words, not bytes.
  static constexpr size_t kMaxNb = 8;           // Max words in largest key.
  static constexpr size_t kMaxNr = 6 + kMaxNb;  // Max rounds for largest key.
  static constexpr size_t kMaxSchedSize = (kMaxNr + 1) * kBlockSize;

  size_t Nr;  // Actual number of rounds based on keysize.
  std::array<uint32_t, kMaxSchedSize> keysched;
  std::array<uint32_t, kMaxSchedSize> invkeysched;
  std::array<uint32_t, kBlockSize> iv;
};

void CRYPT_AESSetKey(CRYPT_aes_context* ctx, pdfium::span<const uint8_t> key);
void CRYPT_AESSetIV(CRYPT_aes_context* ctx, pdfium::span<const uint8_t, 16> iv);
void CRYPT_AESDecrypt(CRYPT_aes_context* ctx,
                      pdfium::span<uint8_t> dest,
                      pdfium::span<const uint8_t> src);
void CRYPT_AESEncrypt(CRYPT_aes_context* ctx,
                      pdfium::span<uint8_t> dest,
                      pdfium::span<const uint8_t> src);

#endif  // CORE_FDRM_FX_CRYPT_AES_H_
