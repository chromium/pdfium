// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <stdint.h>

#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256.h"
#include "xfa/fxbarcode/datamatrix/BC_DataMatrixVersion.h"
#include "xfa/fxbarcode/datamatrix/BC_ErrorCorrection.h"
#include "xfa/fxbarcode/datamatrix/BC_SymbolInfo.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417CodewordDecoder.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417DecodedBitStreamParser.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417ECErrorCorrection.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417ECModulusGF.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417HighLevelEncoder.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417ScanningDecoder.h"
#include "xfa/fxbarcode/qrcode/BC_QRCoderErrorCorrectionLevel.h"
#include "xfa/fxbarcode/qrcode/BC_QRCoderMode.h"
#include "xfa/fxbarcode/qrcode/BC_QRCoderVersion.h"
#include "xfa/fxbarcode/qrcode/BC_QRDataMask.h"

void BC_Library_Init() {
  CBC_QRCoderErrorCorrectionLevel::Initialize();
  CBC_QRCoderMode::Initialize();
  CBC_QRCoderVersion::Initialize();
  CBC_QRDataMask::Initialize();
  CBC_ReedSolomonGF256::Initialize();
  CBC_DataMatrixVersion::Initialize();
  CBC_SymbolInfo::Initialize();
  CBC_ErrorCorrection::Initialize();
  CBC_PDF417HighLevelEncoder::Initialize();

  int32_t e = 0;
  CBC_PDF417ECModulusGF::Initialize(e);
  CBC_DecodedBitStreamPaser::Initialize();
  CBC_PDF417CodewordDecoder::Initialize();
  CBC_PDF417ECErrorCorrection::Initialize(e);
  CBC_PDF417ScanningDecoder::Initialize();
}
void BC_Library_Destory() {
  CBC_QRCoderErrorCorrectionLevel::Finalize();
  CBC_QRCoderMode::Finalize();
  CBC_QRCoderVersion::Finalize();
  CBC_QRDataMask::Finalize();
  CBC_ReedSolomonGF256::Finalize();
  CBC_DataMatrixVersion::Finalize();
  CBC_SymbolInfo::Finalize();
  CBC_ErrorCorrection::Finalize();
  CBC_PDF417HighLevelEncoder::Finalize();
  CBC_DecodedBitStreamPaser::Finalize();
  CBC_PDF417CodewordDecoder::Finalize();
  CBC_PDF417ECErrorCorrection::Finalize();
  CBC_PDF417ECModulusGF::Finalize();
  CBC_PDF417ScanningDecoder::Finalize();
}
