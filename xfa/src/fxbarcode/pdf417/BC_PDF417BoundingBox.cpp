// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2013 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../barcode.h"
#include "../BC_ResultPoint.h"
#include "../common/BC_CommonBitMatrix.h"
#include "BC_PDF417BoundingBox.h"
CBC_BoundingBox::CBC_BoundingBox(CBC_CommonBitMatrix* image, CBC_ResultPoint* topLeft, CBC_ResultPoint* bottomLeft, CBC_ResultPoint* topRight, CBC_ResultPoint* bottomRight, FX_INT32 &e)
{
    if ((topLeft == NULL && topRight == NULL) ||
            (bottomLeft == NULL && bottomRight == NULL) ||
            (topLeft != NULL && bottomLeft == NULL) ||
            (topRight != NULL && bottomRight == NULL)) {
        e = BCExceptionNotFoundInstance;
    }
    init(image, topLeft, bottomLeft, topRight, bottomRight);
}
CBC_BoundingBox::CBC_BoundingBox(CBC_BoundingBox* boundingBox)
{
    init(boundingBox->m_image, boundingBox->m_topLeft, boundingBox->m_bottomLeft, boundingBox->m_topRight, boundingBox->m_bottomRight);
}
CBC_BoundingBox::~CBC_BoundingBox()
{
    if (m_topLeft) {
        delete m_topLeft;
    }
    if (m_bottomLeft) {
        delete m_bottomLeft;
    }
    if (m_topRight) {
        delete m_topRight;
    }
    if (m_bottomRight) {
        delete m_bottomRight;
    }
}
CBC_BoundingBox* CBC_BoundingBox::merge(CBC_BoundingBox* leftBox, CBC_BoundingBox* rightBox, FX_INT32 &e)
{
    CBC_BoundingBox* boundingBox = NULL;
    if (leftBox == NULL) {
        boundingBox = FX_NEW CBC_BoundingBox(rightBox);
        return boundingBox;
    }
    if (rightBox == NULL) {
        boundingBox = FX_NEW CBC_BoundingBox(leftBox);
        return boundingBox;
    }
    boundingBox = FX_NEW CBC_BoundingBox(leftBox->m_image, leftBox->m_topLeft, leftBox->m_bottomLeft, rightBox->m_topRight, rightBox->m_bottomRight, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return boundingBox;
}
CBC_BoundingBox* CBC_BoundingBox::addMissingRows(FX_INT32 missingStartRows, FX_INT32 missingEndRows, FX_BOOL isLeft, FX_INT32 &e)
{
    CBC_ResultPoint* newTopLeft = m_topLeft;
    CBC_ResultPoint* newBottomLeft = m_bottomLeft;
    CBC_ResultPoint* newTopRight = m_topRight;
    CBC_ResultPoint* newBottomRight = m_bottomRight;
    CBC_ResultPoint* newTop = NULL;
    CBC_ResultPoint* newBottom = NULL;
    if (missingStartRows > 0) {
        CBC_ResultPoint* top = isLeft ? m_topLeft : m_topRight;
        FX_INT32 newMinY = (FX_INT32) top->GetY() - missingStartRows;
        if (newMinY < 0) {
            newMinY = 0;
        }
        newTop = FX_NEW CBC_ResultPoint((FX_FLOAT)top->GetX(), (FX_FLOAT)newMinY);
        if (isLeft) {
            newTopLeft = newTop;
        } else {
            newTopRight = newTop;
        }
    }
    if (missingEndRows > 0) {
        CBC_ResultPoint* bottom = isLeft ? m_bottomLeft : m_bottomRight;
        FX_INT32 newMaxY = (FX_INT32) bottom->GetY() + missingEndRows;
        if (newMaxY >= m_image->GetHeight()) {
            newMaxY = m_image->GetHeight() - 1;
        }
        newBottom = FX_NEW CBC_ResultPoint((FX_FLOAT)bottom->GetX(), (FX_FLOAT)newMaxY);
        if (isLeft) {
            newBottomLeft = newBottom;
        } else {
            newBottomRight = newBottom;
        }
    }
    calculateMinMaxValues();
    CBC_BoundingBox* boundingBox = FX_NEW CBC_BoundingBox(m_image, newTopLeft, newBottomLeft, newTopRight, newBottomRight, e);
    delete newTop;
    delete newBottom;
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return boundingBox;
}
void CBC_BoundingBox::setTopRight(CBC_ResultPoint topRight)
{
    if (m_topRight) {
        delete m_topRight;
    }
    m_topRight = FX_NEW CBC_ResultPoint(topRight.GetX(), topRight.GetY());
    calculateMinMaxValues();
}
void CBC_BoundingBox::setBottomRight(CBC_ResultPoint bottomRight)
{
    if (m_bottomRight) {
        delete m_bottomRight;
    }
    m_bottomRight = FX_NEW CBC_ResultPoint(bottomRight.GetX(), bottomRight.GetY());
    calculateMinMaxValues();
}
FX_INT32 CBC_BoundingBox::getMinX()
{
    return m_minX;
}
FX_INT32 CBC_BoundingBox::getMaxX()
{
    return m_maxX;
}
FX_INT32 CBC_BoundingBox::getMinY()
{
    return m_minY;
}
FX_INT32 CBC_BoundingBox::getMaxY()
{
    return m_maxY;
}
CBC_ResultPoint* CBC_BoundingBox::getTopLeft()
{
    return m_topLeft;
}
CBC_ResultPoint* CBC_BoundingBox::getTopRight()
{
    return m_topRight;
}
CBC_ResultPoint* CBC_BoundingBox::getBottomLeft()
{
    return m_bottomLeft;
}
CBC_ResultPoint* CBC_BoundingBox::getBottomRight()
{
    return m_bottomRight;
}
void CBC_BoundingBox::init(CBC_CommonBitMatrix* image, CBC_ResultPoint* topLeft, CBC_ResultPoint* bottomLeft, CBC_ResultPoint* topRight, CBC_ResultPoint* bottomRight)
{
    m_topLeft = NULL;
    m_bottomLeft = NULL;
    m_topRight = NULL;
    m_bottomRight = NULL;
    m_image = image;
    if (topLeft) {
        m_topLeft = FX_NEW CBC_ResultPoint(topLeft->GetX(), topLeft->GetY());
    }
    if (bottomLeft) {
        m_bottomLeft = FX_NEW CBC_ResultPoint(bottomLeft->GetX(), bottomLeft->GetY());
    }
    if (topRight) {
        m_topRight = FX_NEW CBC_ResultPoint(topRight->GetX(), topRight->GetY());
    }
    if (bottomRight) {
        m_bottomRight = FX_NEW CBC_ResultPoint(bottomRight->GetX(), bottomRight->GetY());
    }
    calculateMinMaxValues();
}
void CBC_BoundingBox::calculateMinMaxValues()
{
    if (m_topLeft == NULL) {
        m_topLeft = FX_NEW CBC_ResultPoint(0, m_topRight->GetY());
        m_bottomLeft = FX_NEW CBC_ResultPoint(0, m_bottomRight->GetY());
    } else if (m_topRight == NULL) {
        m_topRight = FX_NEW CBC_ResultPoint((FX_FLOAT)m_image->GetWidth() - 1, (FX_FLOAT)m_topLeft->GetY());
        m_bottomRight = FX_NEW CBC_ResultPoint((FX_FLOAT)m_image->GetWidth() - 1, (FX_FLOAT)m_bottomLeft->GetY());
    }
    m_minX = (FX_INT32) (m_topLeft->GetX() < m_bottomLeft->GetX() ? m_topLeft->GetX() : m_bottomLeft->GetX());
    m_maxX = (FX_INT32) (m_topRight->GetX() > m_bottomRight->GetX() ? m_topRight->GetX() : m_bottomRight->GetX());
    m_minY = (FX_INT32) (m_topLeft->GetY() <  m_topRight->GetY() ? m_topLeft->GetY() : m_topRight->GetY());
    m_maxY = (FX_INT32) (m_bottomLeft->GetY() >  m_bottomRight->GetY() ? m_bottomLeft->GetY() : m_bottomRight->GetY());
}
