// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_LUMINANCESOURCE_H
#define _BC_LUMINANCESOURCE_H
class CBC_LuminanceSource;
class CBC_LuminanceSource  : public CFX_Object
{
public:
    CBC_LuminanceSource(FX_INT32 width, FX_INT32 height);
    virtual ~CBC_LuminanceSource();
    FX_INT32 GetWidth();
    FX_INT32 GetHeight();

    virtual CFX_ByteArray *GetRow(FX_INT32 y, CFX_ByteArray &row, FX_INT32 &e) = 0;
    virtual CFX_ByteArray *GetMatrix() = 0;
    virtual FX_BOOL IsCropSupported()
    {
        return FALSE;
    }
    virtual FX_BOOL IsRotateSupported()
    {
        return FALSE;
    }

    virtual CBC_LuminanceSource *RotateCounterClockwise(FX_INT32 &e) = 0;
protected:
    FX_INT32 m_width;
    FX_INT32 m_height;
};
#endif
