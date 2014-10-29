// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERMATRIXUTIL_H_
#define _BC_QRCODERMATRIXUTIL_H_
class CBC_CommonByteMatrix;
class CBC_QRCoderErrorCorrectionLevel;
class CBC_QRCoderBitVector;
class CBC_QRCoderMatrixUtil;
class CBC_QRCoderMatrixUtil  : public CFX_Object
{
private:
    const static FX_INT32 POSITION_DETECTION_PATTERN[7][7];
    const static FX_INT32 VERTICAL_SEPARATION_PATTERN[7][1];
    const static FX_INT32 HORIZONTAL_SEPARATION_PATTERN[1][8];
    const static FX_INT32 POSITION_ADJUSTMENT_PATTERN[5][5];
    const static FX_INT32 POSITION_ADJUSTMENT_PATTERN_COORDINATE_TABLE[40][7];
    const static FX_INT32 TYPE_INFO_COORDINATES[15][2];
    const static FX_INT32 VERSION_INFO_POLY;
    const static FX_INT32 TYPE_INFO_POLY;
    const static FX_INT32 TYPE_INFO_MASK_PATTERN;
public:
    CBC_QRCoderMatrixUtil();
    virtual ~CBC_QRCoderMatrixUtil();
    static void ClearMatrix(CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static void BuildMatrix(CBC_QRCoderBitVector* dataBits, CBC_QRCoderErrorCorrectionLevel* ecLevel,
                            FX_INT32 version, FX_INT32 maskPattern, CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static void EmbedBasicPatterns(FX_INT32 version, CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static void EmbedTypeInfo(CBC_QRCoderErrorCorrectionLevel* ecLevel, FX_INT32 maskPattern, CBC_CommonByteMatrix *matrix, FX_INT32 &e);
    static void EmbedDataBits(CBC_QRCoderBitVector* dataBits, FX_INT32 maskPattern, CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static void MaybeEmbedVersionInfo(FX_INT32 version, CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static FX_INT32 FindMSBSet(FX_INT32 value);
    static FX_INT32 CalculateBCHCode(FX_INT32 code, FX_INT32 poly);
    static void MakeTypeInfoBits(CBC_QRCoderErrorCorrectionLevel* ecLevel, FX_INT32 maskPattern, CBC_QRCoderBitVector* bits, FX_INT32 &e);
    static void MakeVersionInfoBits(FX_INT32 version, CBC_QRCoderBitVector* bits, FX_INT32 &e);
    static FX_BOOL IsEmpty(FX_INT32 value);
    static FX_BOOL IsValidValue(FX_INT32 value);
    static void EmbedTimingPatterns(CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static void EmbedDarkDotAtLeftBottomCorner(CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static void EmbedHorizontalSeparationPattern(FX_INT32 xStart, FX_INT32 yStart, CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static void EmbedVerticalSeparationPattern(FX_INT32 xStart, FX_INT32 yStart, CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static void EmbedPositionAdjustmentPattern(FX_INT32 xStart, FX_INT32 yStart, CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static void EmbedPositionDetectionPattern(FX_INT32 xStart, FX_INT32 yStart, CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static void EmbedPositionDetectionPatternsAndSeparators(CBC_CommonByteMatrix* matrix, FX_INT32 &e);
    static void MaybeEmbedPositionAdjustmentPatterns(FX_INT32 version, CBC_CommonByteMatrix* matrix, FX_INT32 &e);
};
#endif
