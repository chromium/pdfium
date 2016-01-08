// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/BC_Reader.h"
#include "xfa/src/fxbarcode/BC_UtilCodingConvert.h"
#include "xfa/src/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/src/fxbarcode/BC_LuminanceSource.h"
#include "xfa/src/fxbarcode/BC_BufferedImageLuminanceSource.h"
#include "xfa/src/fxbarcode/BC_Binarizer.h"
#include "xfa/src/fxbarcode/BC_Writer.h"
#include "xfa/src/fxbarcode/BC_Dimension.h"
#include "xfa/src/fxbarcode/BC_UtilCodingConvert.h"
#include "xfa/src/fxbarcode/BC_ResultPoint.h"
#include "xfa/src/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/src/fxbarcode/BC_DecoderResult.h"
#include "xfa/src/fxbarcode/BC_TwoDimWriter.h"
#include "xfa/src/fxbarcode/common/BC_GlobalHistogramBinarizer.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/src/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256.h"
#include "xfa/src/fxbarcode/common/BC_CommonByteMatrix.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitArray.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/src/fxbarcode/common/BC_CommonDecoderResult.h"
#include "xfa/src/fxbarcode/datamatrix/BC_DataMatrixVersion.h"
#include "xfa/src/fxbarcode/datamatrix/BC_DataMatrixReader.h"
#include "xfa/src/fxbarcode/datamatrix/BC_Encoder.h"
#include "xfa/src/fxbarcode/datamatrix/BC_DefaultPlacement.h"
#include "xfa/src/fxbarcode/datamatrix/BC_SymbolShapeHint.h"
#include "xfa/src/fxbarcode/datamatrix/BC_SymbolInfo.h"
#include "xfa/src/fxbarcode/datamatrix/BC_DataMatrixSymbolInfo144.h"
#include "xfa/src/fxbarcode/datamatrix/BC_ErrorCorrection.h"
#include "xfa/src/fxbarcode/datamatrix/BC_EncoderContext.h"
#include "xfa/src/fxbarcode/datamatrix/BC_C40Encoder.h"
#include "xfa/src/fxbarcode/datamatrix/BC_TextEncoder.h"
#include "xfa/src/fxbarcode/datamatrix/BC_X12Encoder.h"
#include "xfa/src/fxbarcode/datamatrix/BC_EdifactEncoder.h"
#include "xfa/src/fxbarcode/datamatrix/BC_Base256Encoder.h"
#include "xfa/src/fxbarcode/datamatrix/BC_ASCIIEncoder.h"
#include "xfa/src/fxbarcode/datamatrix/BC_HighLevelEncoder.h"
#include "xfa/src/fxbarcode/datamatrix/BC_DataMatrixWriter.h"
#include "xfa/src/fxbarcode/oned/BC_OneDReader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCode128Reader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCode39Reader.h"
#include "xfa/src/fxbarcode/oned/BC_OneDimReader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedEAN13Reader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedEAN8Reader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedUPCAReader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCodaBarReader.h"
#include "xfa/src/fxbarcode/oned/BC_OneDimWriter.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCode128Writer.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCode39Writer.h"
#include "xfa/src/fxbarcode/oned/BC_OnedEAN13Writer.h"
#include "xfa/src/fxbarcode/oned/BC_OnedEAN8Writer.h"
#include "xfa/src/fxbarcode/oned/BC_OnedUPCAWriter.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DetectorResult.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417Compaction.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417HighLevelEncoder.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417Detector.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DetectorResult.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417Codeword.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417Common.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417BarcodeValue.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417BarcodeMetadata.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417BoundingBox.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DetectionResultColumn.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DetectionResultRowIndicatorColumn.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DetectionResult.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DecodedBitStreamParser.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417CodewordDecoder.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DecodedBitStreamParser.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417ECModulusPoly.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417ECModulusGF.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417ECErrorCorrection.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DecodedBitStreamParser.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417ScanningDecoder.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417Reader.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRCodeReader.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRCodeWriter.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRCoderErrorCorrectionLevel.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRCoderMode.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRCoderVersion.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRDataMask.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRDecodedBitStreamParser.h"
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
