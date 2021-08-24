// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_BC_LIBRARY_H_
#define FXBARCODE_BC_LIBRARY_H_

#include <stdint.h>

enum class BC_TEXT_LOC : uint8_t {
  kNone = 0,
  kAbove,
  kBelow,
  kAboveEmbed,
  kBelowEmbed,
};

enum class BC_CHAR_ENCODING : uint8_t {
  kUTF8 = 0,
  kUnicode,
};

enum BC_TYPE : int8_t {
  BC_UNKNOWN = -1,
  BC_CODE39 = 0,
  BC_CODABAR,
  BC_CODE128,
  BC_CODE128_B,
  BC_CODE128_C,
  BC_EAN8,
  BC_UPCA,
  BC_EAN13,
  BC_QR_CODE,
  BC_PDF417,
  BC_DATAMATRIX,
  BC_LAST = BC_DATAMATRIX
};

void BC_Library_Init();
void BC_Library_Destroy();

#endif  // FXBARCODE_BC_LIBRARY_H_
