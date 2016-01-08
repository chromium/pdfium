// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2008 ZXing authors
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
#include "xfa/src/fxbarcode/BC_Reader.h"
#include "BC_OneDReader.h"
#include "BC_OneDimReader.h"
#include "BC_OnedEAN13Reader.h"
#include "BC_OnedUPCAReader.h"
CBC_OnedUPCAReader::CBC_OnedUPCAReader() {
  m_ean13Reader = NULL;
}
void CBC_OnedUPCAReader::Init() {
  m_ean13Reader = new CBC_OnedEAN13Reader;
}
CBC_OnedUPCAReader::~CBC_OnedUPCAReader() {
  if (m_ean13Reader != NULL) {
    delete m_ean13Reader;
  }
  m_ean13Reader = NULL;
}
CFX_ByteString CBC_OnedUPCAReader::DecodeRow(int32_t rowNumber,
                                             CBC_CommonBitArray* row,
                                             int32_t hints,
                                             int32_t& e) {
  CFX_ByteString bytestring =
      m_ean13Reader->DecodeRow(rowNumber, row, hints, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CFX_ByteString temp = MaybeReturnResult(bytestring, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return temp;
}
CFX_ByteString CBC_OnedUPCAReader::DecodeRow(int32_t rowNumber,
                                             CBC_CommonBitArray* row,
                                             CFX_Int32Array* startGuardRange,
                                             int32_t hints,
                                             int32_t& e) {
  CFX_ByteString bytestring =
      m_ean13Reader->DecodeRow(rowNumber, row, startGuardRange, hints, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CFX_ByteString temp = MaybeReturnResult(bytestring, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return temp;
}
CFX_ByteString CBC_OnedUPCAReader::Decode(CBC_BinaryBitmap* image, int32_t& e) {
  CFX_ByteString bytestring = m_ean13Reader->Decode(image, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CFX_ByteString temp = MaybeReturnResult(bytestring, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return temp;
}
CFX_ByteString CBC_OnedUPCAReader::Decode(CBC_BinaryBitmap* image,
                                          int32_t hints,
                                          int32_t& e) {
  CFX_ByteString bytestring = m_ean13Reader->Decode(image, hints, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CFX_ByteString temp = MaybeReturnResult(bytestring, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return temp;
}
int32_t CBC_OnedUPCAReader::DecodeMiddle(CBC_CommonBitArray* row,
                                         CFX_Int32Array* startRange,
                                         CFX_ByteString& resultString,
                                         int32_t& e) {
  int32_t temp = m_ean13Reader->DecodeMiddle(row, startRange, resultString, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, 0);
  return temp;
}
CFX_ByteString CBC_OnedUPCAReader::MaybeReturnResult(CFX_ByteString& result,
                                                     int32_t& e) {
  if (result[0] == '0') {
    result.Delete(0);
    return result;
  } else {
    e = BCExceptionFormatException;
    return "";
  }
  return "";
}
