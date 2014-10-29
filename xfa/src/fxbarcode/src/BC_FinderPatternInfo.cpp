// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_ResultPoint.h"
#include "include/BC_QRFinderPattern.h"
#include "include/BC_FinderPatternInfo.h"
CBC_QRFinderPatternInfo::CBC_QRFinderPatternInfo(CFX_PtrArray *patternCenters)
{
    m_bottomLeft = (CBC_QRFinderPattern*)(*patternCenters)[0];
    m_topLeft = (CBC_QRFinderPattern*)(*patternCenters)[1];
    m_topRight = (CBC_QRFinderPattern*)(*patternCenters)[2];
}
CBC_QRFinderPatternInfo::~CBC_QRFinderPatternInfo()
{
}
CBC_QRFinderPattern* CBC_QRFinderPatternInfo::GetBottomLeft()
{
    return m_bottomLeft;
}
CBC_QRFinderPattern* CBC_QRFinderPatternInfo::GetTopLeft()
{
    return m_topLeft;
}
CBC_QRFinderPattern* CBC_QRFinderPatternInfo::GetTopRight()
{
    return m_topRight;
}
