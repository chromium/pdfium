// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_ResultPoint.h"
#include "include/BC_QRAlignmentPattern.h"
CBC_QRAlignmentPattern::CBC_QRAlignmentPattern(FX_FLOAT posX, FX_FLOAT posY, FX_FLOAT estimateModuleSize):
    CBC_ResultPoint(posX, posY), m_moduleSize(estimateModuleSize)
{
}
CBC_QRAlignmentPattern::~CBC_QRAlignmentPattern()
{
}
FX_FLOAT CBC_QRAlignmentPattern::GetX()
{
    return m_x;
}
FX_FLOAT CBC_QRAlignmentPattern::GetY()
{
    return m_y;
}
FX_BOOL CBC_QRAlignmentPattern::AboutEquals(FX_FLOAT moduleSize, FX_FLOAT i, FX_FLOAT j)
{
    if ((FXSYS_fabs(i - GetY()) <= moduleSize) && (FXSYS_fabs(j - GetX()) <= moduleSize)) {
        FX_FLOAT moduleSizeDiff = FXSYS_fabs(moduleSize - m_moduleSize);
        return (moduleSizeDiff <= 1.0f) || (moduleSizeDiff / m_moduleSize <= 1.0f);
    }
    return FALSE;
}
CBC_QRAlignmentPattern *CBC_QRAlignmentPattern::Clone()
{
    return FX_NEW CBC_QRAlignmentPattern(m_x, m_y, m_moduleSize);
}
