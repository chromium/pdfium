// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_Encoder.h"
#include "include/BC_Dimension.h"
CBC_Dimension::CBC_Dimension()
{
}
CBC_Dimension::CBC_Dimension(FX_INT32 width, FX_INT32 height, FX_INT32 &e)
{
    if (width < 0 || height < 0) {
        e = BCExceptionHeightAndWidthMustBeAtLeast1;
    }
    m_width = width;
    m_height = height;
}
CBC_Dimension::~CBC_Dimension()
{
}
FX_INT32 CBC_Dimension::getWidth()
{
    return m_width;
}
FX_INT32 CBC_Dimension::getHeight()
{
    return m_height;
}
FX_INT32 CBC_Dimension::hashCode()
{
    return m_width * 32713 + m_height;
}
CFX_WideString CBC_Dimension::toString()
{
    return (FX_WCHAR)(m_width + (FX_WCHAR)'x' + m_height);
}
