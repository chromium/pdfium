// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_ResultPoint.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_QRDetectorResult.h"
CBC_QRDetectorResult::CBC_QRDetectorResult(CBC_CommonBitMatrix *bits, CFX_PtrArray *points):
    m_bits(bits), m_points(points)
{
}
CBC_QRDetectorResult::~CBC_QRDetectorResult()
{
    for(FX_INT32 i = 0; i < m_points->GetSize(); i++) {
        delete (CBC_ResultPoint*) (*m_points)[i];
    }
    m_points->RemoveAll();
    delete m_points;
    m_points = NULL;
    if(m_bits != NULL) {
        delete m_bits;
    }
    m_bits = NULL;
}
CBC_CommonBitMatrix* CBC_QRDetectorResult::GetBits()
{
    return m_bits;
}
CFX_PtrArray *CBC_QRDetectorResult::GetPoints()
{
    return m_points;
}
