// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BARCODEMATRIX_H_
#define _BC_BARCODEMATRIX_H_
class CBC_BarcodeRow;
class CBC_BarcodeMatrix;
class CBC_BarcodeMatrix : public CFX_Object
{
public:
    CBC_BarcodeMatrix();
    CBC_BarcodeMatrix(FX_INT32 height, FX_INT32 width);
    virtual ~CBC_BarcodeMatrix();
    void set(FX_INT32 x, FX_INT32 y, FX_BYTE value);
    void setMatrix(FX_INT32 x, FX_INT32 y, FX_BOOL black);
    void startRow();
    CBC_BarcodeRow* getCurrentRow();
    CFX_ByteArray& getMatrix();
    CFX_ByteArray& getScaledMatrix(FX_INT32 scale);
    CFX_ByteArray& getScaledMatrix(FX_INT32 xScale, FX_INT32 yScale);
    FX_INT32 getWidth();
    FX_INT32 getHeight();
private:
    CFX_PtrArray m_matrix;
    CFX_ByteArray m_matrixOut;
    FX_INT32 m_currentRow;
    FX_INT32 m_height;
    FX_INT32 m_width;
    FX_INT32 m_outWidth;
    FX_INT32 m_outHeight;
};
#endif
