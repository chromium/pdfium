// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2009 ZXing authors
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

#include "xfa/fxbarcode/BC_Binarizer.h"
#include "xfa/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/fxbarcode/BC_LuminanceSource.h"
#include "xfa/fxbarcode/common/BC_CommonBitArray.h"
#include "xfa/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/fxbarcode/utils.h"

CBC_BinaryBitmap::CBC_BinaryBitmap(CBC_Binarizer* binarizer)
    : m_binarizer(binarizer), m_matrix(NULL) {}
CBC_BinaryBitmap::~CBC_BinaryBitmap() {
  delete m_matrix;
}
int32_t CBC_BinaryBitmap::GetHeight() {
  return m_binarizer->GetLuminanceSource()->GetHeight();
}
int32_t CBC_BinaryBitmap::GetWidth() {
  return m_binarizer->GetLuminanceSource()->GetWidth();
}
CBC_CommonBitMatrix* CBC_BinaryBitmap::GetMatrix(int32_t& e) {
  if (m_matrix == NULL) {
    m_matrix = m_binarizer->GetBlackMatrix(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  }
  return m_matrix;
}
CBC_CommonBitArray* CBC_BinaryBitmap::GetBlackRow(int32_t y,
                                                  CBC_CommonBitArray* row,
                                                  int32_t& e) {
  CBC_CommonBitArray* temp = m_binarizer->GetBlackRow(y, row, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return temp;
}
CBC_CommonBitMatrix* CBC_BinaryBitmap::GetBlackMatrix(int32_t& e) {
  if (m_matrix == NULL) {
    m_matrix = m_binarizer->GetBlackMatrix(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  }
  return m_matrix;
}
