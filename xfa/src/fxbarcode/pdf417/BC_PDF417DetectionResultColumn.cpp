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

#include "xfa/src/fxbarcode/barcode.h"
#include "BC_PDF417Codeword.h"
#include "BC_PDF417BoundingBox.h"
#include "BC_PDF417DetectionResultColumn.h"
int32_t CBC_DetectionResultColumn::MAX_NEARBY_DISTANCE = 5;
CBC_DetectionResultColumn::CBC_DetectionResultColumn(
    CBC_BoundingBox* boundingBox) {
  m_boundingBox = boundingBox;
  m_codewords = new CFX_PtrArray;
  m_codewords->SetSize(boundingBox->getMaxY() - boundingBox->getMinY() + 1);
}
CBC_DetectionResultColumn::~CBC_DetectionResultColumn() {
  for (int32_t i = 0; i < m_codewords->GetSize(); i++) {
    delete (CBC_Codeword*)m_codewords->GetAt(i);
  }
  m_codewords->RemoveAll();
  delete m_codewords;
}
CBC_Codeword* CBC_DetectionResultColumn::getCodewordNearby(int32_t imageRow) {
  CBC_Codeword* codeword = getCodeword(imageRow);
  if (codeword != NULL) {
    return codeword;
  }
  for (int32_t i = 1; i < MAX_NEARBY_DISTANCE; i++) {
    int32_t nearImageRow = imageRowToCodewordIndex(imageRow) - i;
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
int32_t CBC_DetectionResultColumn::imageRowToCodewordIndex(int32_t imageRow) {
  return imageRow - m_boundingBox->getMinY();
}
int32_t CBC_DetectionResultColumn::codewordIndexToImageRow(
    int32_t codewordIndex) {
  return m_boundingBox->getMinY() + codewordIndex;
}
void CBC_DetectionResultColumn::setCodeword(int32_t imageRow,
                                            CBC_Codeword* codeword) {
  m_codewords->SetAt(imageRowToCodewordIndex(imageRow), codeword);
}
CBC_Codeword* CBC_DetectionResultColumn::getCodeword(int32_t imageRow) {
  return (CBC_Codeword*)m_codewords->GetAt(imageRowToCodewordIndex(imageRow));
}
CBC_BoundingBox* CBC_DetectionResultColumn::getBoundingBox() {
  return m_boundingBox;
}
CFX_PtrArray* CBC_DetectionResultColumn::getCodewords() {
  return m_codewords;
}
CFX_ByteString CBC_DetectionResultColumn::toString() {
  CFX_ByteString result;
  int32_t row = 0;
  for (int32_t i = 0; i < m_codewords->GetSize(); i++) {
    CBC_Codeword* codeword = (CBC_Codeword*)m_codewords->GetAt(i);
    if (codeword == NULL) {
      result += (FX_CHAR)row;
      row++;
      continue;
    }
    result += (FX_CHAR)row;
    result += codeword->getRowNumber();
    result += codeword->getValue();
    row++;
  }
  return result;
}
