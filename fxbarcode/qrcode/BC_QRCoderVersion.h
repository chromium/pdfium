// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_QRCODE_BC_QRCODERVERSION_H_
#define FXBARCODE_QRCODE_BC_QRCODERVERSION_H_

#include <vector>

#include "core/fxcrt/fx_basic.h"

class CBC_CommonBitMatrix;
class CBC_QRCoderECBlocks;
class CBC_QRCoderErrorCorrectionLevel;

class CBC_QRCoderVersion {
 public:
  ~CBC_QRCoderVersion();
  static void Initialize();
  static void Finalize();

  static CBC_QRCoderVersion* GetVersionForNumber(int32_t versionNumber);
  static void Destroy();

  int32_t GetVersionNumber();
  int32_t GetTotalCodeWords();
  int32_t GetDimensionForVersion();
  CBC_CommonBitMatrix* BuildFunctionPattern(int32_t& e);
  std::vector<int32_t>* GetAlignmentPatternCenters();
  CBC_QRCoderECBlocks* GetECBlocksForLevel(
      CBC_QRCoderErrorCorrectionLevel* ecLevel);

 private:
  CBC_QRCoderVersion();
  CBC_QRCoderVersion(int32_t versionNumber,
                     CBC_QRCoderECBlocks* ecBlocks1,
                     CBC_QRCoderECBlocks* ecBlocks2,
                     CBC_QRCoderECBlocks* ecBlocks3,
                     CBC_QRCoderECBlocks* ecBlocks4);

  static const int32_t VERSION_DECODE_INFO[34];
  static std::vector<CBC_QRCoderVersion*>* VERSION;

  int32_t m_versionNumber;
  int32_t m_totalCodeWords;
  std::vector<int32_t> m_alignmentPatternCenters;
  std::vector<CBC_QRCoderECBlocks*> m_ecBlocksArray;
};

#endif  // FXBARCODE_QRCODE_BC_QRCODERVERSION_H_
