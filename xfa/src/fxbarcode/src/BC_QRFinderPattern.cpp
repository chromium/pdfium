// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_ResultPoint.h"
#include "include/BC_QRFinderPattern.h"
CBC_QRFinderPattern::CBC_QRFinderPattern(FX_FLOAT x, FX_FLOAT posY, FX_FLOAT estimatedModuleSize):
    CBC_ResultPoint(x, posY),
    m_estimatedModuleSize(estimatedModuleSize), m_count(1)
{
}
CBC_QRFinderPattern::~CBC_QRFinderPattern()
{
    m_count = 0;
    m_x = 0.0f;
    m_y = 0.0f;
    m_estimatedModuleSize = 0.0f;
}
CBC_QRFinderPattern *CBC_QRFinderPattern::Clone()
{
    CBC_QRFinderPattern *temp = FX_NEW CBC_QRFinderPattern(m_x, m_y, m_estimatedModuleSize);
    temp->m_count = m_count;
    return temp;
}
FX_FLOAT CBC_QRFinderPattern::GetEstimatedModuleSize()
{
    return m_estimatedModuleSize;
}
FX_INT32 CBC_QRFinderPattern::GetCount()
{
    return m_count;
}
void CBC_QRFinderPattern::IncrementCount()
{
    m_count++;
}
FX_BOOL CBC_QRFinderPattern::AboutEquals(FX_FLOAT moduleSize, FX_FLOAT i, FX_FLOAT j)
{
    if((fabs(i - GetY()) <= moduleSize) && (fabs(j - GetX()) <= moduleSize)) {
        FX_FLOAT moduleSizeDiff = fabs(moduleSize - m_estimatedModuleSize);
        return (moduleSizeDiff <= 1.0f) || (moduleSizeDiff / m_estimatedModuleSize <= 1.0f);
    }
    return false;
}
FX_FLOAT CBC_QRFinderPattern::GetX()
{
    return m_x;
}
FX_FLOAT CBC_QRFinderPattern::GetY()
{
    return m_y;
}
