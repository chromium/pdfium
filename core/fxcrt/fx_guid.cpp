// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_guid.h"

#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_random.h"

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
