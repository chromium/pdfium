// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_COMMONECI_H_
#define _BC_COMMONECI_H_
class CBC_CommonECI;
class CBC_CommonECI  : public CFX_Object
{
public:
    CBC_CommonECI(FX_INT32 value);
    virtual ~CBC_CommonECI();

    FX_INT32 GetValue();
    static CBC_CommonECI* GetEICByValue(FX_INT32 value, FX_INT32 &e);
private:
    FX_INT32 m_value;
};
#endif
