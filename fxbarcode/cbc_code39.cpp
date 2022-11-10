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

#include "fxbarcode/cbc_code39.h"

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "fxbarcode/oned/BC_OnedCode39Writer.h"

CBC_Code39::CBC_Code39()
    : CBC_OneCode(std::make_unique<CBC_OnedCode39Writer>()) {}

CBC_Code39::~CBC_Code39() = default;

bool CBC_Code39::Encode(WideStringView contents) {
  auto* pWriter = GetOnedCode39Writer();
  if (!pWriter->CheckContentValidity(contents))
    return false;

  WideString filtercontents = pWriter->FilterContents(contents);
  m_renderContents = pWriter->RenderTextContents(contents);
  ByteString byteString = filtercontents.ToUTF8();
  return pWriter->RenderResult(m_renderContents.AsStringView(),
                               pWriter->Encode(byteString));
}

bool CBC_Code39::RenderDevice(CFX_RenderDevice* device,
                              const CFX_Matrix& matrix) {
  auto* pWriter = GetOnedCode39Writer();
  WideString renderCon;
  if (!pWriter->encodedContents(m_renderContents.AsStringView(), &renderCon))
    return false;
  return pWriter->RenderDeviceResult(device, matrix, renderCon.AsStringView());
}

BC_TYPE CBC_Code39::GetType() {
  return BC_TYPE::kCode39;
}

CBC_OnedCode39Writer* CBC_Code39::GetOnedCode39Writer() {
  return static_cast<CBC_OnedCode39Writer*>(m_pBCWriter.get());
}
