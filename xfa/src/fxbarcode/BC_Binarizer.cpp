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

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/BC_LuminanceSource.h"
#include "xfa/src/fxbarcode/BC_Binarizer.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitArray.h"
CBC_Binarizer::CBC_Binarizer(CBC_LuminanceSource* source) {
  m_source = source;
}
CBC_Binarizer::~CBC_Binarizer() {}
CBC_LuminanceSource* CBC_Binarizer::GetLuminanceSource() {
  return m_source;
}
