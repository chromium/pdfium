// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/utils/hash.h"

#include "core/fdrm/fx_crypt.h"

std::string CryptToBase16(const uint8_t* digest) {
  static char const zEncode[] = "0123456789abcdef";
  std::string ret;
  ret.resize(32);
  for (int i = 0, j = 0; i < 16; i++, j += 2) {
    uint8_t a = digest[i];
    ret[j] = zEncode[(a >> 4) & 0xf];
    ret[j + 1] = zEncode[a & 0xf];
  }
  return ret;
}

std::string GenerateMD5Base16(const uint8_t* data, uint32_t size) {
  uint8_t digest[16];
  CRYPT_MD5Generate({data, size}, digest);
  return CryptToBase16(digest);
}
