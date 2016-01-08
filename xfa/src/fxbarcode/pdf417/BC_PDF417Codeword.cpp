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
int32_t CBC_Codeword::BARCODE_ROW_UNKNOWN = -1;
CBC_Codeword::CBC_Codeword(int32_t startX,
                           int32_t endX,
                           int32_t bucket,
                           int32_t cvalue) {
  m_startX = startX;
  m_endX = endX;
  m_bucket = bucket;
  m_value = cvalue;
  m_rowNumber = BARCODE_ROW_UNKNOWN;
}
CBC_Codeword::~CBC_Codeword() {}
FX_BOOL CBC_Codeword::hasValidRowNumber() {
  return isValidRowNumber(m_rowNumber);
}
FX_BOOL CBC_Codeword::isValidRowNumber(int32_t rowNumber) {
  return m_rowNumber != BARCODE_ROW_UNKNOWN &&
         m_bucket == (m_rowNumber % 3) * 3;
}
void CBC_Codeword::setRowNumberAsRowIndicatorColumn() {
  m_rowNumber = (m_value / 30) * 3 + m_bucket / 3;
}
int32_t CBC_Codeword::getWidth() {
  return m_endX - m_startX;
}
int32_t CBC_Codeword::getStartX() {
  return m_startX;
}
int32_t CBC_Codeword::getEndX() {
  return m_endX;
}
int32_t CBC_Codeword::getBucket() {
  return m_bucket;
}
int32_t CBC_Codeword::getValue() {
  return m_value;
}
int32_t CBC_Codeword::getRowNumber() {
  return m_rowNumber;
}
void CBC_Codeword::setRowNumber(int32_t rowNumber) {
  m_rowNumber = rowNumber;
}
CFX_ByteString CBC_Codeword::toString() {
  return m_rowNumber + (FX_CHAR)'|' + m_value;
}
