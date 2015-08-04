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
class CBC_Detector {
 public:
  CBC_Detector();
  virtual ~CBC_Detector();
  static CBC_PDF417DetectorResult* detect(CBC_BinaryBitmap* image,
                                          int32_t hints,
                                          FX_BOOL multiple,
                                          int32_t& e);
  static void rotate180(CBC_CommonBitMatrix* bitMatrix);
  static CBC_CommonBitArray* mirror(CBC_CommonBitArray* input,
                                    CBC_CommonBitArray* result);

 private:
  static int32_t INDEXES_START_PATTERN[];
  static int32_t INDEXES_STOP_PATTERN[];
  static int32_t INTEGER_MATH_SHIFT;
  static int32_t PATTERN_MATCH_RESULT_SCALE_FACTOR;
  static int32_t MAX_AVG_VARIANCE;
  static int32_t MAX_INDIVIDUAL_VARIANCE;
  static int32_t START_PATTERN[];
  static int32_t STOP_PATTERN[];
  static int32_t MAX_PIXEL_DRIFT;
  static int32_t MAX_PATTERN_DRIFT;
  static int32_t SKIPPED_ROW_COUNT_MAX;
  static int32_t ROW_STEP;
  static int32_t BARCODE_MIN_HEIGHT;
  static CFX_PtrArray* detect(FX_BOOL multiple, CBC_CommonBitMatrix* bitMatrix);
  static CFX_PtrArray* findVertices(CBC_CommonBitMatrix* matrix,
                                    int32_t startRow,
                                    int32_t startColumn);
  static void copyToResult(CFX_PtrArray* result,
                           CFX_PtrArray* tmpResult,
                           int32_t* destinationIndexes,
                           int32_t destinationLength);
  static CFX_PtrArray* findRowsWithPattern(CBC_CommonBitMatrix* matrix,
                                           int32_t height,
                                           int32_t width,
                                           int32_t startRow,
                                           int32_t startColumn,
                                           int32_t* pattern,
                                           int32_t patternLength);
  static CFX_Int32Array* findGuardPattern(CBC_CommonBitMatrix* matrix,
                                          int32_t column,
                                          int32_t row,
                                          int32_t width,
                                          FX_BOOL whiteFirst,
                                          int32_t* pattern,
                                          int32_t patternLength,
                                          CFX_Int32Array& counters);
  static int32_t patternMatchVariance(CFX_Int32Array& counters,
                                      int32_t* pattern,
                                      int32_t maxIndividualVariance);
};
#endif
