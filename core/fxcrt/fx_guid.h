// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_GUID_H_
#define CORE_FXCRT_FX_GUID_H_

#include "core/fxcrt/fx_string.h"

struct FX_GUID {
  uint32_t data1;
  uint16_t data2;
  uint16_t data3;
  uint8_t data4[8];
};

void FX_GUID_CreateV4(FX_GUID* pGUID);
CFX_ByteString FX_GUID_ToString(const FX_GUID* pGUID, bool bSeparator = true);

#endif  // CORE_FXCRT_FX_GUID_H_
