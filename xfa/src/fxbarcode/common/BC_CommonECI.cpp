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
CBC_CommonECI::CBC_CommonECI(int32_t value) {
  m_value = value;
}
CBC_CommonECI::~CBC_CommonECI() {}
int32_t CBC_CommonECI::GetValue() {
  return m_value;
}
CBC_CommonECI* CBC_CommonECI::GetEICByValue(int32_t value, int32_t& e) {
  if (value < 0 || value > 999999) {
    e = BCExceptionBadECI;
    return NULL;
  }
  if (value < 900) {
  }
  return NULL;
}
