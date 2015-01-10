// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRDATAMASK_H_
#define _BC_QRDATAMASK_H_
class CBC_CommonBitMatrix;
class CBC_QRDataMask;
class CBC_QRDataMask  : public CFX_Object
{
public:
    static CFX_PtrArray *DATA_MASKS;
    CBC_QRDataMask();
    virtual ~CBC_QRDataMask();
    static void Initialize();
    static void Finalize();
    virtual FX_BOOL IsMasked(FX_INT32 i, FX_INT32 j) = 0;
    void UnmaskBitMatirx(CBC_CommonBitMatrix *bits, FX_INT32 dimension);
    static CBC_QRDataMask* ForReference(FX_INT32 reference, FX_INT32 &e);
    static FX_INT32 BuildDataMasks();
    static void Destroy();
};
#endif
