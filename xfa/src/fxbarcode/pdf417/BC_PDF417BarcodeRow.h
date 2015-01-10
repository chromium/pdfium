// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BARCODEROW_H_
#define _BC_BARCODEROW_H_
class CBC_BarcodeRow;
class CBC_BarcodeRow : public CFX_Object
{
public:
    CBC_BarcodeRow(FX_INT32 width);
    virtual ~CBC_BarcodeRow();
    void set(FX_INT32 x, FX_BYTE value);
    void set(FX_INT32 x, FX_BOOL black);
    void addBar(FX_BOOL black, FX_INT32 width);
    CFX_ByteArray& getRow();
    CFX_ByteArray& getScaledRow(FX_INT32 scale);
private:
    CFX_ByteArray m_row;
    CFX_ByteArray m_output;
    FX_INT32 m_currentLocation;
};
#endif
