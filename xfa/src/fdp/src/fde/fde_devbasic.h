// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_DEVICE_BASIC_IMP
#define _FDE_DEVICE_BASIC_IMP
struct FDE_HATCHDATA {
  int32_t iWidth;
  int32_t iHeight;
  uint8_t MaskBits[64];
};
typedef FDE_HATCHDATA const* FDE_LPCHATCHDATA;
FDE_LPCHATCHDATA FDE_DEVGetHatchData(int32_t iHatchStyle);
#endif
