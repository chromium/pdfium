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
#include "BC_CommonECI.h"
#include "BC_CommonCharacterSetECI.h"
void CBC_CommonCharacterSetECI::initialize() {}
CBC_CommonCharacterSetECI::CBC_CommonCharacterSetECI(
    int32_t value,
    CFX_ByteString encodingName)
    : CBC_CommonECI(value), m_encodingName(encodingName) {}
CBC_CommonCharacterSetECI::~CBC_CommonCharacterSetECI() {}
CFX_ByteString CBC_CommonCharacterSetECI::GetEncodingName() {
  return m_encodingName;
}
void CBC_CommonCharacterSetECI::AddCharacterSet(int32_t value,
                                                CFX_ByteString encodingName) {}
CBC_CommonCharacterSetECI* CBC_CommonCharacterSetECI::GetCharacterSetECIByValue(
    int32_t value) {
  return NULL;
}
CBC_CommonCharacterSetECI* CBC_CommonCharacterSetECI::GetCharacterSetECIByName(
    const CFX_ByteString& name) {
  return NULL;
}
