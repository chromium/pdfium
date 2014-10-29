// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Reader.h"
#include "include/BC_OneDReader.h"
#include "include/BC_UtilCodingConvert.h"
#include "include/BC_DataMatrixVersion.h"
#include "include/BC_BinaryBitmap.h"
#include "include/BC_LuminanceSource.h"
#include "include/BC_BufferedImageLuminanceSource.h"
#include "include/BC_Binarizer.h"
#include "include/BC_GlobalHistogramBinarizer.h"
#include "include/BC_OnedCode128Reader.h"
#include "include/BC_OnedCode39Reader.h"
#include "include/BC_OneDimReader.h"
#include "include/BC_OnedEAN13Reader.h"
#include "include/BC_OnedEAN8Reader.h"
#include "include/BC_OnedUPCAReader.h"
#include "include/BC_OnedCodaBarReader.h"
#include "include/BC_DataMatrixReader.h"
#include "include/BC_QRCodeReader.h"
#include "include/BC_Writer.h"
#include "include/BC_OneDimWriter.h"
#include "include/BC_OnedCode128Writer.h"
#include "include/BC_OnedCode39Writer.h"
#include "include/BC_OnedEAN13Writer.h"
#include "include/BC_OnedEAN8Writer.h"
#include "include/BC_OnedUPCAWriter.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_QRCodeWriter.h"
#include "include/BC_QRCoderErrorCorrectionLevel.h"
#include "include/BC_QRCoderMode.h"
#include "include/BC_QRCoderVersion.h"
#include "include/BC_QRDataMask.h"
#include "include/BC_QRDecodedBitStreamParser.h"
#include "include/BC_ReedSolomonGF256.h"
#include "include/BC_Encoder.h"
#include "include/BC_DefaultPlacement.h"
#include "include/BC_SymbolShapeHint.h"
#include "include/BC_SymbolInfo.h"
#include "include/BC_DataMatrixSymbolInfo144.h"
#include "include/BC_ErrorCorrection.h"
#include "include/BC_Dimension.h"
#include "include/BC_EncoderContext.h"
#include "include/BC_C40Encoder.h"
#include "include/BC_TextEncoder.h"
#include "include/BC_X12Encoder.h"
#include "include/BC_EdifactEncoder.h"
#include "include/BC_Base256Encoder.h"
#include "include/BC_ASCIIEncoder.h"
#include "include/BC_HighLevelEncoder.h"
#include "include/BC_CommonByteMatrix.h"
#include "include/BC_DataMatrixWriter.h"
#include "include/BC_PDF417Compaction.h"
#include "include/BC_UtilCodingConvert.h"
#include "include/BC_PDF417HighLevelEncoder.h"
#include "include/BC_ResultPoint.h"
#include "include/BC_PDF417DetectorResult.h"
#include "include/BC_BinaryBitmap.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_CommonBitArray.h"
#include "include/BC_PDF417Detector.h"
#include "include/BC_PDF417DetectorResult.h"
#include "include/BC_DecoderResult.h"
#include "include/BC_PDF417Codeword.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_PDF417Common.h"
#include "include/BC_PDF417BarcodeValue.h"
#include "include/BC_PDF417BarcodeMetadata.h"
#include "include/BC_PDF417BoundingBox.h"
#include "include/BC_PDF417DetectionResultColumn.h"
#include "include/BC_PDF417DetectionResultRowIndicatorColumn.h"
#include "include/BC_PDF417DetectionResult.h"
#include "include/BC_PDF417DecodedBitStreamParser.h"
#include "include/BC_PDF417CodewordDecoder.h"
#include "include/BC_PDF417DecodedBitStreamParser.h"
#include "include/BC_PDF417ECModulusPoly.h"
#include "include/BC_PDF417ECModulusGF.h"
#include "include/BC_PDF417ECErrorCorrection.h"
#include "include/BC_PDF417DecodedBitStreamParser.h"
#include "include/BC_CommonDecoderResult.h"
#include "include/BC_PDF417ScanningDecoder.h"
#include "include/BC_PDF417Reader.h"
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
