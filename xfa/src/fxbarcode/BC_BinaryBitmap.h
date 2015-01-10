// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BINARYBITMAP_H_
#define _BC_BINARYBITMAP_H_
class CBC_Binarizer;
class CBC_CommonBitMatrix;
class CBC_CommonBitArray;
class CBC_BinaryBitmap;
class CBC_BinaryBitmap : public CFX_Object
{
public:
    CBC_BinaryBitmap(CBC_Binarizer *binarizer);
    virtual ~CBC_BinaryBitmap();
    FX_INT32				GetWidth();
    FX_INT32				GetHeight();
    CBC_CommonBitMatrix *	GetMatrix(FX_INT32 &e);
    CBC_CommonBitArray *	GetBlackRow(FX_INT32 y, CBC_CommonBitArray *row, FX_INT32 &e);
    CBC_CommonBitMatrix *	GetBlackMatrix(FX_INT32 &e);
    FX_BOOL					IsCropSupported();
    FX_BOOL					IsRotateSupported();
private:
    CBC_Binarizer *			m_binarizer;
    CBC_CommonBitMatrix *	m_matrix;
};
#endif
