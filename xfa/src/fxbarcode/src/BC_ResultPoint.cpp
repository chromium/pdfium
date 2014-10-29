// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_ResultPoint.h"
CBC_ResultPoint::CBC_ResultPoint(FX_FLOAT x, FX_FLOAT y): m_x(x), m_y(y)
{
}
FX_FLOAT CBC_ResultPoint::GetX()
{
    return m_x;
}
FX_FLOAT CBC_ResultPoint::GetY()
{
    return m_y;
}
