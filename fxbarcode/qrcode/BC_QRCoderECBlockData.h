// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_QRCODE_BC_QRCODERECBLOCKSDATA_H_
#define FXBARCODE_QRCODE_BC_QRCODERECBLOCKSDATA_H_

#include <stdint.h>

#include <array>

struct CBC_QRCoderECBlockData {
  int32_t GetNumBlocks() const;
  int32_t GetTotalECCodeWords() const;
  int32_t GetTotalDataCodeWords() const;

  uint8_t ecCodeWordsPerBlock;
  uint8_t count1;
  uint8_t dataCodeWords1;
  uint8_t count2;
  uint8_t dataCodeWords2;

 private:
  int32_t GetECCodeWordsPerBlock() const;
};

using ECBlockDataRow = std::array<CBC_QRCoderECBlockData, 4>;
extern const std::array<const ECBlockDataRow, 40> kQRCoderECBDataTable;

#endif  // FXBARCODE_QRCODE_BC_QRCODERECBLOCKSDATA_H_
