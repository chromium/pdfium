// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DETECTIONRESULTCOLUMN_H_
#define _BC_DETECTIONRESULTCOLUMN_H_
class CBC_Codeword;
class CBC_BoundingBox;
class CBC_DetectionResultColumn;
class CBC_DetectionResultColumn : public CFX_Object
{
public:
    CBC_DetectionResultColumn(CBC_BoundingBox* boundingBox);
    virtual ~CBC_DetectionResultColumn();
    CBC_Codeword* getCodewordNearby(FX_INT32 imageRow);
    FX_INT32 imageRowToCodewordIndex(FX_INT32 imageRow);
    FX_INT32 codewordIndexToImageRow(FX_INT32 codewordIndex);
    void setCodeword(FX_INT32 imageRow, CBC_Codeword* codeword);
    CBC_Codeword* getCodeword(FX_INT32 imageRow);
    CBC_BoundingBox* getBoundingBox();
    CFX_PtrArray* getCodewords();
    CFX_ByteString toString();
public:
    CBC_BoundingBox* m_boundingBox;
    CFX_PtrArray* m_codewords;
private:
    static FX_INT32 MAX_NEARBY_DISTANCE;
};
#endif
