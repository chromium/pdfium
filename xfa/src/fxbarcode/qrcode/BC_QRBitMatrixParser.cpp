// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2007 ZXing authors
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
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "BC_QRCoderVersion.h"
#include "BC_QRCoderFormatInformation.h"
#include "BC_QRDataMask.h"
#include "BC_QRBitMatrixParser.h"
CBC_QRBitMatrixParser::CBC_QRBitMatrixParser() {}
void CBC_QRBitMatrixParser::Init(CBC_CommonBitMatrix* bitMatrix, int32_t& e) {
  m_dimension = bitMatrix->GetDimension(e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  m_tempBitMatrix = bitMatrix;
  if (m_dimension < 21 || (m_dimension & 0x03) != 1) {
    e = BCExceptionRead;
    BC_EXCEPTION_CHECK_ReturnVoid(e);
  }
  m_bitMatrix = m_tempBitMatrix;
  m_parsedFormatInfo = NULL;
  m_version = NULL;
}
CBC_QRBitMatrixParser::~CBC_QRBitMatrixParser() {
  if (m_parsedFormatInfo != NULL) {
    delete m_parsedFormatInfo;
    m_parsedFormatInfo = NULL;
  }
  m_version = NULL;
}
CBC_QRCoderFormatInformation* CBC_QRBitMatrixParser::ReadFormatInformation(
    int32_t& e) {
  if (m_parsedFormatInfo != NULL) {
    return m_parsedFormatInfo;
  }
  int32_t formatInfoBits = 0;
  int32_t j;
  for (j = 0; j < 6; j++) {
    formatInfoBits = CopyBit(8, j, formatInfoBits);
  }
  formatInfoBits = CopyBit(8, 7, formatInfoBits);
  formatInfoBits = CopyBit(8, 8, formatInfoBits);
  formatInfoBits = CopyBit(7, 8, formatInfoBits);
  for (int32_t i = 5; i >= 0; i--) {
    formatInfoBits = CopyBit(i, 8, formatInfoBits);
  }
  m_parsedFormatInfo =
      CBC_QRCoderFormatInformation::DecodeFormatInformation(formatInfoBits);
  if (m_parsedFormatInfo != NULL) {
    return m_parsedFormatInfo;
  }
  int32_t dimension = m_bitMatrix->GetDimension(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  formatInfoBits = 0;
  int32_t iMin = dimension - 8;
  for (j = dimension - 1; j >= iMin; j--) {
    formatInfoBits = CopyBit(j, 8, formatInfoBits);
  }
  for (int32_t k = dimension - 7; k < dimension; k++) {
    formatInfoBits = CopyBit(8, k, formatInfoBits);
  }
  m_parsedFormatInfo =
      CBC_QRCoderFormatInformation::DecodeFormatInformation(formatInfoBits);
  if (m_parsedFormatInfo != NULL) {
    return m_parsedFormatInfo;
  }
  e = BCExceptionRead;
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return NULL;
}
CBC_QRCoderVersion* CBC_QRBitMatrixParser::ReadVersion(int32_t& e) {
  if (m_version != NULL) {
    return m_version;
  }
  int32_t dimension = m_bitMatrix->GetDimension(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  int32_t provisionVersion = (dimension - 17) >> 2;
  if (provisionVersion <= 6) {
    CBC_QRCoderVersion* qrv =
        CBC_QRCoderVersion::GetVersionForNumber(provisionVersion, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return qrv;
  }
  int32_t versionBits = 0;
  for (int32_t i = 5; i >= 0; i--) {
    int32_t jMin = dimension - 11;
    for (int32_t j = dimension - 9; j >= jMin; j--) {
      versionBits = CopyBit(i, j, versionBits);
    }
  }
  m_version = CBC_QRCoderVersion::DecodeVersionInformation(versionBits, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  if (m_version != NULL && m_version->GetDimensionForVersion() == dimension) {
    return m_version;
  }
  versionBits = 0;
  for (int32_t j = 5; j >= 0; j--) {
    int32_t iMin = dimension - 11;
    for (int32_t i = dimension - 9; i >= iMin; i--) {
      versionBits = CopyBit(i, j, versionBits);
    }
  }
  m_version = CBC_QRCoderVersion::DecodeVersionInformation(versionBits, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  if (m_version != NULL && m_version->GetDimensionForVersion() == dimension) {
    return m_version;
  }
  e = BCExceptionRead;
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return NULL;
}
int32_t CBC_QRBitMatrixParser::CopyBit(int32_t i,
                                       int32_t j,
                                       int32_t versionBits) {
  return m_bitMatrix->Get(j, i) ? (versionBits << 1) | 0x1 : versionBits << 1;
}
CFX_ByteArray* CBC_QRBitMatrixParser::ReadCodewords(int32_t& e) {
  CBC_QRCoderFormatInformation* formatInfo = ReadFormatInformation(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL) CBC_QRCoderVersion* version =
      ReadVersion(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_QRDataMask* dataMask =
      CBC_QRDataMask::ForReference((int32_t)(formatInfo->GetDataMask()), e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  int32_t dimension = m_bitMatrix->GetDimension(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  dataMask->UnmaskBitMatirx(m_bitMatrix, dimension);
  CBC_CommonBitMatrix* cbm = version->BuildFunctionPattern(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_AutoPtr<CBC_CommonBitMatrix> functionPattern(cbm);
  FX_BOOL readingUp = TRUE;
  CFX_ByteArray* temp = new CFX_ByteArray;
  temp->SetSize(version->GetTotalCodeWords());
  CBC_AutoPtr<CFX_ByteArray> result(temp);
  int32_t resultOffset = 0;
  int32_t currentByte = 0;
  int32_t bitsRead = 0;
  for (int32_t j = dimension - 1; j > 0; j -= 2) {
    if (j == 6) {
      j--;
    }
    for (int32_t count = 0; count < dimension; count++) {
      int32_t i = readingUp ? dimension - 1 - count : count;
      for (int32_t col = 0; col < 2; col++) {
        if (!functionPattern->Get(j - col, i)) {
          bitsRead++;
          currentByte <<= 1;
          if (m_bitMatrix->Get(j - col, i)) {
            currentByte |= 1;
          }
          if (bitsRead == 8) {
            (*result)[resultOffset++] = (uint8_t)currentByte;
            bitsRead = 0;
            currentByte = 0;
          }
        }
      }
    }
    readingUp ^= TRUE;
  }
  if (resultOffset != version->GetTotalCodeWords()) {
    e = BCExceptionRead;
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  }
  return result.release();
}
