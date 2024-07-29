// Copyright 2016 The PDFium Authors
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

enum class BC_TYPE : int8_t {
  kUnknown = -1,
  kCode39 = 0,
  kCodabar,
  kCode128,
  kCode128B,
  kCode128C,
  kEAN8,
  kUPCA,
  kEAN13,
  kQRCode,
  kPDF417,
  kDataMatrix,
  kLast = kDataMatrix
};

void BC_Library_Init();
void BC_Library_Destroy();

#endif  // FXBARCODE_BC_LIBRARY_H_
