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
#include "xfa/src/fxbarcode/qrcode/BC_QRCoderErrorCorrectionLevel.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417ResultMetadata.h"
#include "BC_CommonDecoderResult.h"
CBC_CommonDecoderResult::CBC_CommonDecoderResult() {}
void CBC_CommonDecoderResult::Init(const CFX_ByteArray& rawBytes,
                                   const CFX_ByteString& text,
                                   const CFX_Int32Array& byteSegments,
                                   CBC_QRCoderErrorCorrectionLevel* ecLevel,
                                   int32_t& e) {
  if (text.IsEmpty()) {
    e = BCExceptionIllegalArgument;
    return;
  }
  m_rawBytes.Copy(rawBytes);
  m_text = text;
  m_byteSegments.Copy(byteSegments);
  m_ecLevel = ecLevel;
  m_other = NULL;
}
void CBC_CommonDecoderResult::Init(const CFX_ByteArray& rawBytes,
                                   const CFX_ByteString& text,
                                   const CFX_PtrArray& byteSegments,
                                   const CFX_ByteString& ecLevel,
                                   int32_t& e) {
  if (text.IsEmpty()) {
    e = BCExceptionIllegalArgument;
    return;
  }
  m_rawBytes.Copy(rawBytes);
  m_text = text;
  m_pdf417byteSegments.Copy(byteSegments);
  m_pdf417ecLevel = ecLevel;
  m_other = NULL;
}
void CBC_CommonDecoderResult::setOther(CBC_PDF417ResultMetadata* other) {
  m_other = other;
}
CBC_CommonDecoderResult::~CBC_CommonDecoderResult() {
  if (m_other != NULL) {
    delete m_other;
  }
}
const CFX_ByteArray& CBC_CommonDecoderResult::GetRawBytes() {
  return m_rawBytes;
}
const CFX_Int32Array& CBC_CommonDecoderResult::GetByteSegments() {
  return m_byteSegments;
}
const CFX_ByteString& CBC_CommonDecoderResult::GetText() {
  return m_text;
}
CBC_QRCoderErrorCorrectionLevel* CBC_CommonDecoderResult::GetECLevel() {
  return m_ecLevel;
}
