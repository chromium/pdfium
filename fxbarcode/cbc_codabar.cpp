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

#include "fxbarcode/cbc_codabar.h"

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "fxbarcode/oned/BC_OnedCodaBarWriter.h"

CBC_Codabar::CBC_Codabar()
    : CBC_OneCode(std::make_unique<CBC_OnedCodaBarWriter>()) {}

CBC_Codabar::~CBC_Codabar() = default;

bool CBC_Codabar::Encode(WideStringView contents) {
  auto* pWriter = GetOnedCodaBarWriter();
  if (!pWriter->CheckContentValidity(contents))
    return false;

  m_renderContents = pWriter->FilterContents(contents);
  ByteString byteString = m_renderContents.ToUTF8();
  return pWriter->RenderResult(m_renderContents.AsStringView(),
                               pWriter->Encode(byteString));
}

bool CBC_Codabar::RenderDevice(CFX_RenderDevice* device,
                               const CFX_Matrix& matrix) {
  auto* pWriter = GetOnedCodaBarWriter();
  WideString renderCon =
      pWriter->encodedContents(m_renderContents.AsStringView());
  return pWriter->RenderDeviceResult(device, matrix, renderCon.AsStringView());
}

BC_TYPE CBC_Codabar::GetType() {
  return BC_TYPE::kCodabar;
}

CBC_OnedCodaBarWriter* CBC_Codabar::GetOnedCodaBarWriter() {
  return static_cast<CBC_OnedCodaBarWriter*>(m_pBCWriter.get());
}
