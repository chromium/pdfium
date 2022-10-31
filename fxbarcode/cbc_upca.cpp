// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
/*
 * Copyright 2011 ZXing authors
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

#include "fxbarcode/cbc_upca.h"

#include <memory>

#include "fxbarcode/oned/BC_OnedUPCAWriter.h"

CBC_UPCA::CBC_UPCA() : CBC_EANCode(std::make_unique<CBC_OnedUPCAWriter>()) {}

CBC_UPCA::~CBC_UPCA() = default;

BC_TYPE CBC_UPCA::GetType() {
  return BC_TYPE::kUPCA;
}

size_t CBC_UPCA::GetMaxLength() const {
  return 11;
}
