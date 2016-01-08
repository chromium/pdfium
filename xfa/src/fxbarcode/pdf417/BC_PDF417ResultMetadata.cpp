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
#include "BC_PDF417ResultMetadata.h"
CBC_PDF417ResultMetadata::CBC_PDF417ResultMetadata() {}
CBC_PDF417ResultMetadata::~CBC_PDF417ResultMetadata() {}
int32_t CBC_PDF417ResultMetadata::getSegmentIndex() {
  return m_segmentIndex;
}
void CBC_PDF417ResultMetadata::setSegmentIndex(int32_t segmentIndex) {
  m_segmentIndex = segmentIndex;
}
CFX_ByteString CBC_PDF417ResultMetadata::getFileId() {
  return m_fileId;
}
void CBC_PDF417ResultMetadata::setFileId(CFX_ByteString fileId) {
  m_fileId = fileId;
}
CFX_Int32Array& CBC_PDF417ResultMetadata::getOptionalData() {
  return m_optionalData;
}
void CBC_PDF417ResultMetadata::setOptionalData(CFX_Int32Array& optionalData) {
  m_optionalData.Copy(optionalData);
}
FX_BOOL CBC_PDF417ResultMetadata::isLastSegment() {
  return m_lastSegment;
}
void CBC_PDF417ResultMetadata::setLastSegment(FX_BOOL lastSegment) {
  m_lastSegment = lastSegment;
}
