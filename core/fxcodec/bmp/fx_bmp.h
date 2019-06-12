// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_BMP_FX_BMP_H_
#define CORE_FXCODEC_BMP_FX_BMP_H_

#include <stdint.h>

#pragma pack(1)
struct BmpFileHeader {
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
};

struct BmpCoreHeader {
  uint32_t bcSize;
  uint16_t bcWidth;
  uint16_t bcHeight;
  uint16_t bcPlanes;
  uint16_t bcBitCount;
};

struct BmpInfoHeader {
  uint32_t biSize;
  int32_t biWidth;
  int32_t biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t biXPelsPerMeter;
  int32_t biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
};
#pragma pack()

static_assert(sizeof(BmpFileHeader) == 14, "BmpFileHeader has wrong size");
static_assert(sizeof(BmpCoreHeader) == 12, "BmpCoreHeader has wrong size");
static_assert(sizeof(BmpInfoHeader) == 40, "BmpInfoHeader has wrong size");

#endif  // CORE_FXCODEC_BMP_FX_BMP_H_
