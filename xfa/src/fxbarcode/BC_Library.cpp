// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "BC_Reader.h"
#include "BC_UtilCodingConvert.h"
#include "BC_BinaryBitmap.h"
#include "BC_LuminanceSource.h"
#include "BC_BufferedImageLuminanceSource.h"
#include "BC_Binarizer.h"
#include "BC_Writer.h"
#include "BC_Dimension.h"
#include "BC_UtilCodingConvert.h"
#include "BC_ResultPoint.h"
#include "BC_BinaryBitmap.h"
#include "BC_DecoderResult.h"
#include "BC_TwoDimWriter.h"
#include "common/BC_GlobalHistogramBinarizer.h"
#include "common/BC_CommonBitMatrix.h"
#include "common/reedsolomon/BC_ReedSolomonGF256.h"
#include "common/BC_CommonByteMatrix.h"
#include "common/BC_CommonBitMatrix.h"
#include "common/BC_CommonBitArray.h"
#include "common/BC_CommonBitMatrix.h"
#include "common/BC_CommonDecoderResult.h"
#include "datamatrix/BC_DataMatrixVersion.h"
#include "datamatrix/BC_DataMatrixReader.h"
#include "datamatrix/BC_Encoder.h"
#include "datamatrix/BC_DefaultPlacement.h"
#include "datamatrix/BC_SymbolShapeHint.h"
#include "datamatrix/BC_SymbolInfo.h"
#include "datamatrix/BC_DataMatrixSymbolInfo144.h"
#include "datamatrix/BC_ErrorCorrection.h"
#include "datamatrix/BC_EncoderContext.h"
#include "datamatrix/BC_C40Encoder.h"
#include "datamatrix/BC_TextEncoder.h"
#include "datamatrix/BC_X12Encoder.h"
#include "datamatrix/BC_EdifactEncoder.h"
#include "datamatrix/BC_Base256Encoder.h"
#include "datamatrix/BC_ASCIIEncoder.h"
#include "datamatrix/BC_HighLevelEncoder.h"
#include "datamatrix/BC_DataMatrixWriter.h"
#include "oned/BC_OneDReader.h"
#include "oned/BC_OnedCode128Reader.h"
#include "oned/BC_OnedCode39Reader.h"
#include "oned/BC_OneDimReader.h"
#include "oned/BC_OnedEAN13Reader.h"
#include "oned/BC_OnedEAN8Reader.h"
#include "oned/BC_OnedUPCAReader.h"
#include "oned/BC_OnedCodaBarReader.h"
#include "oned/BC_OneDimWriter.h"
#include "oned/BC_OnedCode128Writer.h"
#include "oned/BC_OnedCode39Writer.h"
#include "oned/BC_OnedEAN13Writer.h"
#include "oned/BC_OnedEAN8Writer.h"
#include "oned/BC_OnedUPCAWriter.h"
#include "pdf417/BC_PDF417DetectorResult.h"
#include "pdf417/BC_PDF417Compaction.h"
#include "pdf417/BC_PDF417HighLevelEncoder.h"
#include "pdf417/BC_PDF417Detector.h"
#include "pdf417/BC_PDF417DetectorResult.h"
#include "pdf417/BC_PDF417Codeword.h"
#include "pdf417/BC_PDF417Common.h"
#include "pdf417/BC_PDF417BarcodeValue.h"
#include "pdf417/BC_PDF417BarcodeMetadata.h"
#include "pdf417/BC_PDF417BoundingBox.h"
#include "pdf417/BC_PDF417DetectionResultColumn.h"
#include "pdf417/BC_PDF417DetectionResultRowIndicatorColumn.h"
#include "pdf417/BC_PDF417DetectionResult.h"
#include "pdf417/BC_PDF417DecodedBitStreamParser.h"
#include "pdf417/BC_PDF417CodewordDecoder.h"
#include "pdf417/BC_PDF417DecodedBitStreamParser.h"
#include "pdf417/BC_PDF417ECModulusPoly.h"
#include "pdf417/BC_PDF417ECModulusGF.h"
#include "pdf417/BC_PDF417ECErrorCorrection.h"
#include "pdf417/BC_PDF417DecodedBitStreamParser.h"
#include "pdf417/BC_PDF417ScanningDecoder.h"
#include "pdf417/BC_PDF417Reader.h"
#include "qrcode/BC_QRCodeReader.h"
#include "qrcode/BC_QRCodeWriter.h"
#include "qrcode/BC_QRCoderErrorCorrectionLevel.h"
#include "qrcode/BC_QRCoderMode.h"
#include "qrcode/BC_QRCoderVersion.h"
#include "qrcode/BC_QRDataMask.h"
#include "qrcode/BC_QRDecodedBitStreamParser.h"
void BC_Library_Init()
{
    CBC_QRCoderErrorCorrectionLevel::Initialize();
    CBC_QRCoderMode::Initialize();
    CBC_QRCoderVersion::Initialize();
    CBC_QRDataMask::Initialize();
    CBC_ReedSolomonGF256::Initialize();
    CBC_DataMatrixVersion::Initialize();
    CBC_SymbolInfo::Initialize();
    CBC_ErrorCorrection::Initialize();
    CBC_PDF417HighLevelEncoder::Initialize();
    FX_INT32 e = 0;
    CBC_PDF417ECModulusGF::Initialize(e);
    CBC_DecodedBitStreamPaser::Initialize();
    CBC_PDF417CodewordDecoder::Initialize();
    CBC_PDF417ECErrorCorrection::Initialize(e);
    CBC_PDF417ScanningDecoder::Initialize();
}
void BC_Library_Destory()
{
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
