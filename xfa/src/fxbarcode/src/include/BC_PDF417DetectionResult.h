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
class CBC_DetectionResult;
class CBC_DetectionResult : public CFX_Object
{
public:
    CBC_DetectionResult(CBC_BarcodeMetadata* barcodeMetadata, CBC_BoundingBox* boundingBox);
    virtual ~CBC_DetectionResult();
    CFX_PtrArray& getDetectionResultColumns();
    void setBoundingBox(CBC_BoundingBox* boundingBox);
    CBC_BoundingBox* getBoundingBox();
    void setDetectionResultColumn(FX_INT32 barcodeColumn, CBC_DetectionResultColumn* detectionResultColumn);
    CBC_DetectionResultColumn* getDetectionResultColumn(FX_INT32 barcodeColumn);
    CFX_ByteString toString();

    FX_INT32 getBarcodeColumnCount();
    FX_INT32 getBarcodeRowCount();
    FX_INT32 getBarcodeECLevel();
private:
    static FX_INT32 ADJUST_ROW_NUMBER_SKIP;
    CBC_BarcodeMetadata* m_barcodeMetadata;
    CFX_PtrArray m_detectionResultColumns;
    CBC_BoundingBox* m_boundingBox;
    FX_INT32 m_barcodeColumnCount;
private:
    void adjustIndicatorColumnRowNumbers(CBC_DetectionResultColumn* detectionResultColumn);
    FX_INT32 adjustRowNumbers();
    FX_INT32 adjustRowNumbersByRow();
    FX_INT32 adjustRowNumbersFromBothRI();
    FX_INT32 adjustRowNumbersFromRRI();
    FX_INT32 adjustRowNumbersFromLRI();
    static FX_INT32 adjustRowNumberIfValid(FX_INT32 rowIndicatorRowNumber, FX_INT32 invalidRowCounts, CBC_Codeword* codeword);
    void adjustRowNumbers(FX_INT32 barcodeColumn, FX_INT32 codewordsRow, CFX_PtrArray* codewords);
    static FX_BOOL adjustRowNumber(CBC_Codeword* codeword, CBC_Codeword* otherCodeword);


};
#endif
