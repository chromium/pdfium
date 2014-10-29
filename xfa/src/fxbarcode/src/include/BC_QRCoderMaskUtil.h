// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERMASKUTIL_H_
#define _BC_QRCODERMASKUTIL_H_
class CBC_CommonByteMatrix;
class CBC_QRCoderMaskUtil;
class CBC_QRCoderMaskUtil  : public CFX_Object
{
public:
    CBC_QRCoderMaskUtil();
    virtual ~CBC_QRCoderMaskUtil();
    static FX_BOOL GetDataMaskBit(FX_INT32 maskPattern, FX_INT32 x, FX_INT32 y, FX_INT32 &e);

    static FX_INT32 ApplyMaskPenaltyRule1(CBC_CommonByteMatrix* matrix);
    static FX_INT32 ApplyMaskPenaltyRule2(CBC_CommonByteMatrix* matrix);
    static FX_INT32 ApplyMaskPenaltyRule3(CBC_CommonByteMatrix* matrix);
    static FX_INT32 ApplyMaskPenaltyRule4(CBC_CommonByteMatrix* matrix);
    static FX_INT32 ApplyMaskPenaltyRule1Internal(CBC_CommonByteMatrix* matrix, FX_BOOL isHorizontal);
};
#endif
