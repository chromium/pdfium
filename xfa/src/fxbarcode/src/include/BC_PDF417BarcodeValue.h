// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BARCODEVALUE_H_
#define _BC_BARCODEVALUE_H_
class CBC_BarcodeValue;
class CBC_BarcodeValue : public CFX_Object
{
public:
    CBC_BarcodeValue();
    virtual ~CBC_BarcodeValue();
    void setValue(FX_INT32 value);
    CFX_Int32Array* getValue();
    FX_INT32 getConfidence(FX_INT32 value);
private:
    CFX_Int32Array m_keys;
    CFX_Int32Array m_values;
};
#endif
