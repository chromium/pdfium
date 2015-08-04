// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417SCANNINGDECODER_H_
#define _BC_PDF417SCANNINGDECODER_H_
class CBC_CommonDecoderResult;
class CBC_CommonBitMatrix;
class CBC_ErrorCorrection;
class CBC_Codeword;
class CBC_BoundingBox;
class CBC_ResultPoint;
class CBC_BarcodeMetadata;
class CBC_BarcodeValue;
class CBC_DetectionResult;
class CBC_DetectionResultRowIndicatorColumn;

class CBC_PDF417ScanningDecoder {
 public:
  CBC_PDF417ScanningDecoder();
  virtual ~CBC_PDF417ScanningDecoder();
  static void Initialize();
  static void Finalize();
  static CBC_CommonDecoderResult* decode(CBC_CommonBitMatrix* image,
                                         CBC_ResultPoint* imageTopLeft,
                                         CBC_ResultPoint* imageBottomLeft,
                                         CBC_ResultPoint* imageTopRight,
                                         CBC_ResultPoint* imageBottomRight,
                                         int32_t minCodewordWidth,
                                         int32_t maxCodewordWidth,
                                         int32_t& e);
  static CFX_ByteString toString(CFX_PtrArray* barcodeMatrix);

 private:
  static int32_t CODEWORD_SKEW_SIZE;
  static int32_t MAX_ERRORS;
  static int32_t MAX_EC_CODEWORDS;
  static CBC_PDF417ECErrorCorrection* errorCorrection;
  static CBC_DetectionResult* merge(
      CBC_DetectionResultRowIndicatorColumn* leftRowIndicatorColumn,
      CBC_DetectionResultRowIndicatorColumn* rightRowIndicatorColumn,
      int32_t& e);
  static CBC_BoundingBox* adjustBoundingBox(
      CBC_DetectionResultRowIndicatorColumn* rowIndicatorColumn,
      int32_t& e);
  static int32_t getMax(CFX_Int32Array& values);
  static CBC_BarcodeMetadata* getBarcodeMetadata(
      CBC_DetectionResultRowIndicatorColumn* leftRowIndicatorColumn,
      CBC_DetectionResultRowIndicatorColumn* rightRowIndicatorColumn);
  static CBC_DetectionResultRowIndicatorColumn* getRowIndicatorColumn(
      CBC_CommonBitMatrix* image,
      CBC_BoundingBox* boundingBox,
      CBC_ResultPoint startPoint,
      FX_BOOL leftToRight,
      int32_t minCodewordWidth,
      int32_t maxCodewordWidth);
  static void adjustCodewordCount(CBC_DetectionResult* detectionResult,
                                  CFX_PtrArray* barcodeMatrix,
                                  int32_t& e);
  static CBC_CommonDecoderResult* createDecoderResult(
      CBC_DetectionResult* detectionResult,
      int32_t& e);
  static CBC_CommonDecoderResult* createDecoderResultFromAmbiguousValues(
      int32_t ecLevel,
      CFX_Int32Array& codewords,
      CFX_Int32Array& erasureArray,
      CFX_Int32Array& ambiguousIndexes,
      CFX_PtrArray& ambiguousIndexValues,
      int32_t& e);
  static CFX_PtrArray* createBarcodeMatrix(
      CBC_DetectionResult* detectionResult);
  static FX_BOOL isValidBarcodeColumn(CBC_DetectionResult* detectionResult,
                                      int32_t barcodeColumn);
  static int32_t getStartColumn(CBC_DetectionResult* detectionResult,
                                int32_t barcodeColumn,
                                int32_t imageRow,
                                FX_BOOL leftToRight);
  static CBC_Codeword* detectCodeword(CBC_CommonBitMatrix* image,
                                      int32_t minColumn,
                                      int32_t maxColumn,
                                      FX_BOOL leftToRight,
                                      int32_t startColumn,
                                      int32_t imageRow,
                                      int32_t minCodewordWidth,
                                      int32_t maxCodewordWidth);
  static CFX_Int32Array* getModuleBitCount(CBC_CommonBitMatrix* image,
                                           int32_t minColumn,
                                           int32_t maxColumn,
                                           FX_BOOL leftToRight,
                                           int32_t startColumn,
                                           int32_t imageRow);
  static int32_t getNumberOfECCodeWords(int32_t barcodeECLevel);
  static int32_t adjustCodewordStartColumn(CBC_CommonBitMatrix* image,
                                           int32_t minColumn,
                                           int32_t maxColumn,
                                           FX_BOOL leftToRight,
                                           int32_t codewordStartColumn,
                                           int32_t imageRow);
  static FX_BOOL checkCodewordSkew(int32_t codewordSize,
                                   int32_t minCodewordWidth,
                                   int32_t maxCodewordWidth);
  static CBC_CommonDecoderResult* decodeCodewords(CFX_Int32Array& codewords,
                                                  int32_t ecLevel,
                                                  CFX_Int32Array& erasures,
                                                  int32_t& e);
  static int32_t correctErrors(CFX_Int32Array& codewords,
                               CFX_Int32Array& erasures,
                               int32_t numECCodewords,
                               int32_t& e);
  static void verifyCodewordCount(CFX_Int32Array& codewords,
                                  int32_t numECCodewords,
                                  int32_t& e);
  static CFX_Int32Array* getBitCountForCodeword(int32_t codeword);
  static int32_t getCodewordBucketNumber(int32_t codeword);
  static int32_t getCodewordBucketNumber(CFX_Int32Array& moduleBitCount);
};
#endif
