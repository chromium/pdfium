// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DIMENSIONS_H_
#define _BC_DIMENSIONS_H_
class CBC_Dimensions;
class CBC_Dimensions : public CFX_Object
{
public:
    CBC_Dimensions(FX_INT32 minCols, FX_INT32 maxCols, FX_INT32 minRows, FX_INT32 maxRows);
    virtual ~CBC_Dimensions();
    FX_INT32 getMinCols();
    FX_INT32 getMaxCols();
    FX_INT32 getMinRows();
    FX_INT32 getMaxRows();
private:
    FX_INT32 m_minCols;
    FX_INT32 m_maxCols;
    FX_INT32 m_minRows;
    FX_INT32 m_maxRows;
};
#endif
