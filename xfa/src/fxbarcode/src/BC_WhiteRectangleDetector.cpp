// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_WhiteRectangleDetector.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_ResultPoint.h"
const FX_INT32 CBC_WhiteRectangleDetector::INIT_SIZE = 30;
const FX_INT32 CBC_WhiteRectangleDetector::CORR = 1;
CBC_WhiteRectangleDetector::CBC_WhiteRectangleDetector(CBC_CommonBitMatrix *image)
{
    m_image = image;
    m_height = image->GetHeight();
    m_width = image->GetWidth();
    m_leftInit = (m_width - INIT_SIZE) >> 1;
    m_rightInit = (m_width + INIT_SIZE) >> 1;
    m_upInit = (m_height - INIT_SIZE) >> 1;
    m_downInit = (m_height + INIT_SIZE) >> 1;
}
void CBC_WhiteRectangleDetector::Init(FX_INT32 &e)
{
    if (m_upInit < 0 || m_leftInit < 0 || m_downInit >= m_height || m_rightInit >= m_width) {
        e = BCExceptionNotFound;
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
}
CBC_WhiteRectangleDetector::CBC_WhiteRectangleDetector(CBC_CommonBitMatrix *image, FX_INT32 initSize, FX_INT32 x, FX_INT32 y)
{
    m_image = image;
    m_height = image->GetHeight();
    m_width = image->GetWidth();
    FX_INT32 halfsize = initSize >> 1;
    m_leftInit = x - halfsize;
    m_rightInit = x + halfsize;
    m_upInit = y - halfsize;
    m_downInit = y + halfsize;
}
CBC_WhiteRectangleDetector::~CBC_WhiteRectangleDetector()
{
}
CFX_PtrArray *CBC_WhiteRectangleDetector::Detect(FX_INT32 &e)
{
    FX_INT32 left = m_leftInit;
    FX_INT32 right = m_rightInit;
    FX_INT32 up = m_upInit;
    FX_INT32 down = m_downInit;
    FX_BOOL sizeExceeded = FALSE;
    FX_BOOL aBlackPointFoundOnBorder = TRUE;
    FX_BOOL atLeastOneBlackPointFoundOnBorder = FALSE;
    while (aBlackPointFoundOnBorder) {
        aBlackPointFoundOnBorder = FALSE;
        FX_BOOL rightBorderNotWhite = TRUE;
        while (rightBorderNotWhite && right < m_width) {
            rightBorderNotWhite = ContainsBlackPoint(up, down, right, FALSE);
            if (rightBorderNotWhite) {
                right++;
                aBlackPointFoundOnBorder = TRUE;
            }
        }
        if (right >= m_width) {
            sizeExceeded = TRUE;
            break;
        }
        FX_BOOL bottomBorderNotWhite = TRUE;
        while (bottomBorderNotWhite && down < m_height) {
            bottomBorderNotWhite = ContainsBlackPoint(left, right, down, TRUE);
            if (bottomBorderNotWhite) {
                down++;
                aBlackPointFoundOnBorder = TRUE;
            }
        }
        if (down >= m_height) {
            sizeExceeded = TRUE;
            break;
        }
        FX_BOOL leftBorderNotWhite = TRUE;
        while (leftBorderNotWhite && left >= 0) {
            leftBorderNotWhite = ContainsBlackPoint(up, down, left, FALSE);
            if (leftBorderNotWhite) {
                left--;
                aBlackPointFoundOnBorder = TRUE;
            }
        }
        if (left < 0) {
            sizeExceeded = TRUE;
            break;
        }
        FX_BOOL topBorderNotWhite = TRUE;
        while (topBorderNotWhite && up >= 0) {
            topBorderNotWhite = ContainsBlackPoint(left, right, up, TRUE);
            if (topBorderNotWhite) {
                up--;
                aBlackPointFoundOnBorder = TRUE;
            }
        }
        if (up < 0) {
            sizeExceeded = TRUE;
            break;
        }
        if (aBlackPointFoundOnBorder) {
            atLeastOneBlackPointFoundOnBorder = TRUE;
        }
    }
    if (!sizeExceeded && atLeastOneBlackPointFoundOnBorder) {
        FX_INT32 maxSize = right - left;
        CBC_AutoPtr<CBC_ResultPoint> z(NULL);
        for (FX_INT32 i = 1; i < maxSize; i++) {
            z = CBC_AutoPtr<CBC_ResultPoint>(GetBlackPointOnSegment((FX_FLOAT)left, (FX_FLOAT)(down - i), (FX_FLOAT)(left + i), (FX_FLOAT)(down)) );
            if (z.get() != NULL) {
                break;
            }
        }
        if (z.get() == NULL) {
            e = BCExceptionNotFound;
            BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        }
        CBC_AutoPtr<CBC_ResultPoint> t(NULL);
        for (FX_INT32 j = 1; j < maxSize; j++) {
            t = CBC_AutoPtr<CBC_ResultPoint>(GetBlackPointOnSegment((FX_FLOAT)left, (FX_FLOAT)(up + j), (FX_FLOAT)(left + j), (FX_FLOAT)up));
            if (t.get() != NULL) {
                break;
            }
        }
        if (t.get() == NULL) {
            e = BCExceptionNotFound;
            BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        }
        CBC_AutoPtr<CBC_ResultPoint> x(NULL);
        for (FX_INT32 k = 1; k < maxSize; k++) {
            x = CBC_AutoPtr<CBC_ResultPoint>(GetBlackPointOnSegment((FX_FLOAT)right, (FX_FLOAT)(up + k), (FX_FLOAT)(right - k), (FX_FLOAT)up));
            if (x.get() != NULL) {
                break;
            }
        }
        if (x.get() == NULL) {
            e = BCExceptionNotFound;
            BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        }
        CBC_AutoPtr<CBC_ResultPoint> y(NULL);
        for (FX_INT32 m = 1;	m < maxSize; m++) {
            y = CBC_AutoPtr<CBC_ResultPoint>(GetBlackPointOnSegment((FX_FLOAT)right, (FX_FLOAT)(down - m), (FX_FLOAT)(right - m), (FX_FLOAT) down));
            if (y.get() != NULL) {
                break;
            }
        }
        if (y.get() == NULL) {
            e = BCExceptionNotFound;
            BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        }
        return CenterEdges(y.get(), z.get(), x.get(), t.get());
    } else {
        e = BCExceptionNotFound;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    return NULL;
}
FX_INT32 CBC_WhiteRectangleDetector::Round(FX_FLOAT d)
{
    return (FX_INT32) (d + 0.5f);
}
CBC_ResultPoint *CBC_WhiteRectangleDetector::GetBlackPointOnSegment(FX_FLOAT aX, FX_FLOAT aY, FX_FLOAT bX, FX_FLOAT bY)
{
    FX_INT32 dist = DistanceL2(aX, aY, bX, bY);
    float xStep = (bX - aX) / dist;
    float yStep = (bY - aY) / dist;
    for (FX_INT32 i = 0; i < dist; i++) {
        FX_INT32 x = Round(aX + i * xStep);
        FX_INT32 y = Round(aY + i * yStep);
        if (m_image->Get(x, y)) {
            return FX_NEW CBC_ResultPoint((FX_FLOAT)x, (FX_FLOAT) y);
        }
    }
    return NULL;
}
FX_INT32 CBC_WhiteRectangleDetector::DistanceL2(FX_FLOAT aX, FX_FLOAT aY, FX_FLOAT bX, FX_FLOAT bY)
{
    float xDiff = aX - bX;
    float yDiff = aY - bY;
    return Round((float)sqrt(xDiff * xDiff + yDiff * yDiff));
}
CFX_PtrArray *CBC_WhiteRectangleDetector::CenterEdges(CBC_ResultPoint *y, CBC_ResultPoint *z, CBC_ResultPoint *x, CBC_ResultPoint *t)
{
    float yi = y->GetX();
    float yj = y->GetY();
    float zi = z->GetX();
    float zj = z->GetY();
    float xi = x->GetX();
    float xj = x->GetY();
    float ti = t->GetX();
    float tj = t->GetY();
    if (yi < m_width / 2) {
        CFX_PtrArray *result = FX_NEW CFX_PtrArray;
        result->SetSize(4);
        (*result)[0] = FX_NEW CBC_ResultPoint(ti - CORR, tj + CORR);
        (*result)[1] = FX_NEW CBC_ResultPoint(zi + CORR, zj + CORR);
        (*result)[2] = FX_NEW CBC_ResultPoint(xi - CORR, xj - CORR);
        (*result)[3] = FX_NEW CBC_ResultPoint(yi + CORR, yj - CORR);
        return result;
    } else {
        CFX_PtrArray *result = FX_NEW CFX_PtrArray;
        result->SetSize(4);
        (*result)[0] = FX_NEW CBC_ResultPoint(ti + CORR, tj + CORR);
        (*result)[1] = FX_NEW CBC_ResultPoint(zi + CORR, zj - CORR);
        (*result)[2] = FX_NEW CBC_ResultPoint(xi - CORR, xj + CORR);
        (*result)[3] = FX_NEW CBC_ResultPoint(yi - CORR, yj - CORR);
        return result;
    }
}
FX_BOOL CBC_WhiteRectangleDetector::ContainsBlackPoint(FX_INT32 a, FX_INT32 b, FX_INT32 fixed, FX_BOOL horizontal)
{
    if (horizontal) {
        for (FX_INT32 x = a; x <= b; x++) {
            if (m_image->Get(x, fixed)) {
                return TRUE;
            }
        }
    } else {
        for (FX_INT32 y = a; y <= b; y++) {
            if (m_image->Get(fixed, y)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}
