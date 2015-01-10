// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DIMENSION_H_
#define _BC_DIMENSION_H_
class CBC_Dimension;
class CBC_Dimension : public CFX_Object
{
public:
    CBC_Dimension();
    CBC_Dimension(FX_INT32 width, FX_INT32 height, FX_INT32 &e);
    virtual ~CBC_Dimension();
    FX_INT32 getWidth();
    FX_INT32 getHeight();
    FX_INT32 hashCode();
    CFX_WideString toString();
private:
    FX_INT32 m_width;
    FX_INT32 m_height;
};
#endif
