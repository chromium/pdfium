// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_CODEWORD_H_
#define _BC_CODEWORD_H_
class CBC_Codeword;
class CBC_Codeword : public CFX_Object
{
public:
    CBC_Codeword(FX_INT32 startX, FX_INT32 endX, FX_INT32 bucket, FX_INT32 value);
    virtual ~CBC_Codeword();
    FX_BOOL hasValidRowNumber() ;
    FX_BOOL isValidRowNumber(FX_INT32 rowNumber);
    void setRowNumberAsRowIndicatorColumn();
    FX_INT32 getWidth();
    FX_INT32 getStartX();
    FX_INT32 getEndX();
    FX_INT32 getBucket();
    FX_INT32 getValue();
    FX_INT32 getRowNumber();
    void setRowNumber(FX_INT32 rowNumber);
    CFX_ByteString toString();
private:
    static FX_INT32 BARCODE_ROW_UNKNOWN;
    FX_INT32 m_startX;
    FX_INT32 m_endX;
    FX_INT32 m_bucket;
    FX_INT32 m_value;
    FX_INT32 m_rowNumber;
};
#endif
