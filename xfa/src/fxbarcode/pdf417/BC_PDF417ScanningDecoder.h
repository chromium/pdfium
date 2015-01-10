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
class CBC_PDF417ScanningDecoder;
class CBC_PDF417ScanningDecoder  : public CFX_Object
{
public:
    CBC_PDF417ScanningDecoder();
    virtual ~CBC_PDF417ScanningDecoder();
    static void Initialize();
    static void Finalize();
    static CBC_CommonDecoderResult* decode(CBC_CommonBitMatrix* image, CBC_ResultPoint* imageTopLeft, CBC_ResultPoint* imageBottomLeft, CBC_ResultPoint* imageTopRight,
                                           CBC_ResultPoint* imageBottomRight, FX_INT32 minCodewordWidth, FX_INT32 maxCodewordWidth, FX_INT32 &e);
    static CFX_ByteString toString(CFX_PtrArray* barcodeMatrix);
private:
    static FX_INT32 CODEWORD_SKEW_SIZE;
    static FX_INT32 MAX_ERRORS;
    static FX_INT32 MAX_EC_CODEWORDS;
    static CBC_PDF417ECErrorCorrection* errorCorrection;
    static CBC_DetectionResult* merge(CBC_DetectionResultRowIndicatorColumn* leftRowIndicatorColumn, CBC_DetectionResultRowIndicatorColumn* rightRowIndicatorColumn, FX_INT32 &e);
    static CBC_BoundingBox* adjustBoundingBox(CBC_DetectionResultRowIndicatorColumn* rowIndicatorColumn, FX_INT32 &e);
    static FX_INT32 getMax(CFX_Int32Array& values);
    static CBC_BarcodeMetadata* getBarcodeMetadata(CBC_DetectionResultRowIndicatorColumn* leftRowIndicatorColumn, CBC_DetectionResultRowIndicatorColumn* rightRowIndicatorColumn);
    static CBC_DetectionResultRowIndicatorColumn* getRowIndicatorColumn(CBC_CommonBitMatrix* image, CBC_BoundingBox* boundingBox, CBC_ResultPoint startPoint,
            FX_BOOL leftToRight, FX_INT32 minCodewordWidth, FX_INT32 maxCodewordWidth);
    static void adjustCodewordCount(CBC_DetectionResult* detectionResult, CFX_PtrArray* barcodeMatrix, FX_INT32 &e);
    static CBC_CommonDecoderResult* createDecoderResult(CBC_DetectionResult* detectionResult, FX_INT32 &e);
    static CBC_CommonDecoderResult* createDecoderResultFromAmbiguousValues(FX_INT32 ecLevel, CFX_Int32Array &codewords, CFX_Int32Array &erasureArray, CFX_Int32Array &ambiguousIndexes,
            CFX_PtrArray& ambiguousIndexValues, FX_INT32 &e);
    static CFX_PtrArray* createBarcodeMatrix(CBC_DetectionResult* detectionResult);
    static FX_BOOL isValidBarcodeColumn(CBC_DetectionResult* detectionResult, FX_INT32 barcodeColumn);
    static FX_INT32 getStartColumn(CBC_DetectionResult* detectionResult, FX_INT32 barcodeColumn, FX_INT32 imageRow, FX_BOOL leftToRight);
    static CBC_Codeword* detectCodeword(CBC_CommonBitMatrix* image, FX_INT32 minColumn, FX_INT32 maxColumn, FX_BOOL leftToRight, FX_INT32 startColumn,
                                        FX_INT32 imageRow, FX_INT32 minCodewordWidth, FX_INT32 maxCodewordWidth);
    static CFX_Int32Array* getModuleBitCount(CBC_CommonBitMatrix* image, FX_INT32 minColumn, FX_INT32 maxColumn, FX_BOOL leftToRight, FX_INT32 startColumn, FX_INT32 imageRow);
    static FX_INT32 getNumberOfECCodeWords(FX_INT32 barcodeECLevel);
    static FX_INT32 adjustCodewordStartColumn(CBC_CommonBitMatrix* image, FX_INT32 minColumn, FX_INT32 maxColumn, FX_BOOL leftToRight, FX_INT32 codewordStartColumn, FX_INT32 imageRow);
    static FX_BOOL checkCodewordSkew(FX_INT32 codewordSize, FX_INT32 minCodewordWidth, FX_INT32 maxCodewordWidth);
    static CBC_CommonDecoderResult* decodeCodewords(CFX_Int32Array &codewords, FX_INT32 ecLevel, CFX_Int32Array &erasures, FX_INT32 &e);
    static FX_INT32 correctErrors(CFX_Int32Array &codewords, CFX_Int32Array &erasures, FX_INT32 numECCodewords, FX_INT32 &e);
    static void verifyCodewordCount(CFX_Int32Array &codewords, FX_INT32 numECCodewords, FX_INT32 &e);
    static CFX_Int32Array* getBitCountForCodeword(FX_INT32 codeword);
    static FX_INT32 getCodewordBucketNumber(FX_INT32 codeword);
    static FX_INT32 getCodewordBucketNumber(CFX_Int32Array& moduleBitCount);
};
#endif
