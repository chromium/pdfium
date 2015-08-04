// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_EDTECTIONRESULT_H_
#define _BC_EDTECTIONRESULT_H_
class CBC_BarcodeMetadata;
class CBC_BoundingBox;
class CBC_Codeword;
class CBC_DetectionResultColumn;
class CBC_DetectionResult {
 public:
  CBC_DetectionResult(CBC_BarcodeMetadata* barcodeMetadata,
                      CBC_BoundingBox* boundingBox);
  virtual ~CBC_DetectionResult();
  CFX_PtrArray& getDetectionResultColumns();
  void setBoundingBox(CBC_BoundingBox* boundingBox);
  CBC_BoundingBox* getBoundingBox();
  void setDetectionResultColumn(
      int32_t barcodeColumn,
      CBC_DetectionResultColumn* detectionResultColumn);
  CBC_DetectionResultColumn* getDetectionResultColumn(int32_t barcodeColumn);
  CFX_ByteString toString();

  int32_t getBarcodeColumnCount();
  int32_t getBarcodeRowCount();
  int32_t getBarcodeECLevel();

 private:
  static int32_t ADJUST_ROW_NUMBER_SKIP;
  CBC_BarcodeMetadata* m_barcodeMetadata;
  CFX_PtrArray m_detectionResultColumns;
  CBC_BoundingBox* m_boundingBox;
  int32_t m_barcodeColumnCount;

 private:
  void adjustIndicatorColumnRowNumbers(
      CBC_DetectionResultColumn* detectionResultColumn);
  int32_t adjustRowNumbers();
  int32_t adjustRowNumbersByRow();
  int32_t adjustRowNumbersFromBothRI();
  int32_t adjustRowNumbersFromRRI();
  int32_t adjustRowNumbersFromLRI();
  static int32_t adjustRowNumberIfValid(int32_t rowIndicatorRowNumber,
                                        int32_t invalidRowCounts,
                                        CBC_Codeword* codeword);
  void adjustRowNumbers(int32_t barcodeColumn,
                        int32_t codewordsRow,
                        CFX_PtrArray* codewords);
  static FX_BOOL adjustRowNumber(CBC_Codeword* codeword,
                                 CBC_Codeword* otherCodeword);
};
#endif
