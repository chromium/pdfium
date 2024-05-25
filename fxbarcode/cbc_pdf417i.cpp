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

#include "fxbarcode/cbc_pdf417i.h"

#include <stdint.h>

#include <memory>

#include "core/fxcrt/data_vector.h"
#include "fxbarcode/pdf417/BC_PDF417Writer.h"

namespace {

// Multiple source say PDF417 can encode about 1100 bytes, 1800 ASCII characters
// or 2710 numerical digits.
constexpr size_t kMaxPDF417InputLengthBytes = 2710;

}  // namespace

CBC_PDF417I::CBC_PDF417I()
    : CBC_CodeBase(std::make_unique<CBC_PDF417Writer>()) {}

CBC_PDF417I::~CBC_PDF417I() = default;

bool CBC_PDF417I::Encode(WideStringView contents) {
  if (contents.GetLength() > kMaxPDF417InputLengthBytes)
    return false;

  auto* pWriter = GetPDF417Writer();
  CBC_PDF417Writer::EncodeResult result = pWriter->Encode(contents);
  return pWriter->RenderResult(result.data, result.width, result.height);
}

bool CBC_PDF417I::RenderDevice(CFX_RenderDevice* device,
                               const CFX_Matrix& matrix) {
  GetPDF417Writer()->RenderDeviceResult(device, matrix);
  return true;
}

BC_TYPE CBC_PDF417I::GetType() {
  return BC_TYPE::kPDF417;
}

CBC_PDF417Writer* CBC_PDF417I::GetPDF417Writer() {
  return static_cast<CBC_PDF417Writer*>(m_pBCWriter.get());
}
