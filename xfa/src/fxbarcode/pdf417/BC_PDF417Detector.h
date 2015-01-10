// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DETECTOR_H_
#define _BC_DETECTOR_H_
class CBC_PDF417DetectorResult;
class CBC_BinaryBitmap;
class CBC_CommonBitMatrix;
class CBC_CommonBitArray;
class CBC_Detector;
class CBC_Detector : public CFX_Object
{
public:
    CBC_Detector();
    virtual ~CBC_Detector();
    static CBC_PDF417DetectorResult* detect(CBC_BinaryBitmap* image, FX_INT32 hints, FX_BOOL multiple, FX_INT32 &e);
    static void rotate180(CBC_CommonBitMatrix* bitMatrix);
    static CBC_CommonBitArray* mirror(CBC_CommonBitArray* input, CBC_CommonBitArray* result);
private:
    static FX_INT32 INDEXES_START_PATTERN[];
    static FX_INT32 INDEXES_STOP_PATTERN[];
    static FX_INT32 INTEGER_MATH_SHIFT;
    static FX_INT32 PATTERN_MATCH_RESULT_SCALE_FACTOR;
    static FX_INT32 MAX_AVG_VARIANCE;
    static FX_INT32 MAX_INDIVIDUAL_VARIANCE;
    static FX_INT32 START_PATTERN[];
    static FX_INT32 STOP_PATTERN[];
    static FX_INT32 MAX_PIXEL_DRIFT;
    static FX_INT32 MAX_PATTERN_DRIFT;
    static FX_INT32 SKIPPED_ROW_COUNT_MAX;
    static FX_INT32 ROW_STEP;
    static FX_INT32 BARCODE_MIN_HEIGHT;
    static CFX_PtrArray* detect(FX_BOOL multiple, CBC_CommonBitMatrix* bitMatrix);
    static CFX_PtrArray* findVertices(CBC_CommonBitMatrix* matrix, FX_INT32 startRow, FX_INT32 startColumn);
    static void copyToResult(CFX_PtrArray* result, CFX_PtrArray* tmpResult, FX_INT32* destinationIndexes, FX_INT32 destinationLength);
    static CFX_PtrArray* findRowsWithPattern(CBC_CommonBitMatrix* matrix, FX_INT32 height, FX_INT32 width, FX_INT32 startRow, FX_INT32 startColumn, FX_INT32* pattern, FX_INT32 patternLength);
    static CFX_Int32Array* findGuardPattern(CBC_CommonBitMatrix* matrix, FX_INT32 column, FX_INT32 row, FX_INT32 width, FX_BOOL whiteFirst, FX_INT32* pattern, FX_INT32 patternLength, CFX_Int32Array &counters);
    static FX_INT32 patternMatchVariance(CFX_Int32Array &counters, FX_INT32* pattern, FX_INT32 maxIndividualVariance);
};
#endif
