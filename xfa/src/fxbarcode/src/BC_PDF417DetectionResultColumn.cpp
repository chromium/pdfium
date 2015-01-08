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

#include "barcode.h"
#include "include/BC_PDF417Codeword.h"
#include "include/BC_PDF417BoundingBox.h"
#include "include/BC_PDF417DetectionResultColumn.h"
FX_INT32 CBC_DetectionResultColumn::MAX_NEARBY_DISTANCE = 5;
CBC_DetectionResultColumn::CBC_DetectionResultColumn(CBC_BoundingBox* boundingBox)
{
    m_boundingBox = boundingBox;
    m_codewords = FX_NEW CFX_PtrArray;
    m_codewords->SetSize(boundingBox->getMaxY() - boundingBox->getMinY() + 1);
}
CBC_DetectionResultColumn::~CBC_DetectionResultColumn()
{
    for (FX_INT32 i = 0; i < m_codewords->GetSize(); i++) {
        delete (CBC_Codeword*)m_codewords->GetAt(i);
    }
    m_codewords->RemoveAll();
    delete m_codewords;
}
CBC_Codeword* CBC_DetectionResultColumn::getCodewordNearby(FX_INT32 imageRow)
{
    CBC_Codeword* codeword = getCodeword(imageRow);
    if (codeword != NULL) {
        return codeword;
    }
    for (FX_INT32 i = 1; i < MAX_NEARBY_DISTANCE; i++) {
        FX_INT32 nearImageRow = imageRowToCodewordIndex(imageRow) - i;
        if (nearImageRow >= 0) {
            codeword = (CBC_Codeword*)m_codewords->GetAt(nearImageRow);
            if (codeword != NULL) {
                return codeword;
            }
        }
        nearImageRow = imageRowToCodewordIndex(imageRow) + i;
        if (nearImageRow < m_codewords->GetSize()) {
            codeword = (CBC_Codeword*)m_codewords->GetAt(nearImageRow);
            if (codeword != NULL) {
                return codeword;
            }
        }
    }
    return NULL;
}
FX_INT32 CBC_DetectionResultColumn::imageRowToCodewordIndex(FX_INT32 imageRow)
{
    return imageRow - m_boundingBox->getMinY();
}
FX_INT32 CBC_DetectionResultColumn::codewordIndexToImageRow(FX_INT32 codewordIndex)
{
    return m_boundingBox->getMinY() + codewordIndex;
}
void CBC_DetectionResultColumn::setCodeword(FX_INT32 imageRow, CBC_Codeword* codeword)
{
    m_codewords->SetAt(imageRowToCodewordIndex(imageRow), codeword);
}
CBC_Codeword* CBC_DetectionResultColumn::getCodeword(FX_INT32 imageRow)
{
    return (CBC_Codeword*)m_codewords->GetAt(imageRowToCodewordIndex(imageRow));
}
CBC_BoundingBox* CBC_DetectionResultColumn::getBoundingBox()
{
    return m_boundingBox;
}
CFX_PtrArray* CBC_DetectionResultColumn::getCodewords()
{
    return m_codewords;
}
CFX_ByteString CBC_DetectionResultColumn::toString()
{
    CFX_ByteString result;
    FX_INT32 row = 0;
    for (FX_INT32 i = 0; i < m_codewords->GetSize(); i++) {
        CBC_Codeword* codeword = (CBC_Codeword*)m_codewords->GetAt(i);
        if (codeword == NULL) {
            result += (FX_CHAR) row;
            row++;
            continue;
        }
        result += (FX_CHAR) row;
        result += codeword->getRowNumber();
        result += codeword->getValue();
        row++;
    }
    return result;
}
