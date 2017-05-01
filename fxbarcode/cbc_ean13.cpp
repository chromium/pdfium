// Copyright 2016 PDFium Authors. All rights reserved.
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

#include "fxbarcode/cbc_ean13.h"

#include <memory>

#include "fxbarcode/oned/BC_OnedEAN13Writer.h"
#include "third_party/base/ptr_util.h"

CBC_EAN13::CBC_EAN13()
    : CBC_OneCode(pdfium::MakeUnique<CBC_OnedEAN13Writer>()) {}

CBC_EAN13::~CBC_EAN13() {}

CFX_WideString CBC_EAN13::Preprocess(const CFX_WideStringC& contents) {
  auto* pWriter = GetOnedEAN13Writer();
  CFX_WideString encodeContents = pWriter->FilterContents(contents);
  int32_t length = encodeContents.GetLength();
  if (length <= 12) {
    for (int32_t i = 0; i < 12 - length; i++)
      encodeContents = wchar_t('0') + encodeContents;

    CFX_ByteString byteString = encodeContents.UTF8Encode();
    int32_t checksum = pWriter->CalcChecksum(byteString);
    byteString += checksum - 0 + '0';
    encodeContents = byteString.UTF8Decode();
  }
  if (length > 13)
    encodeContents = encodeContents.Mid(0, 13);

  return encodeContents;
}

bool CBC_EAN13::Encode(const CFX_WideStringC& contents, bool isDevice) {
  if (contents.IsEmpty())
    return false;

  BCFORMAT format = BCFORMAT_EAN_13;
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  CFX_WideString encodeContents = Preprocess(contents);
  CFX_ByteString byteString = encodeContents.UTF8Encode();
  m_renderContents = encodeContents;
  auto* pWriter = GetOnedEAN13Writer();
  std::unique_ptr<uint8_t, FxFreeDeleter> data(
      pWriter->Encode(byteString, format, outWidth, outHeight));
  if (!data)
    return false;
  return pWriter->RenderResult(encodeContents.AsStringC(), data.get(), outWidth,
                               isDevice);
}

bool CBC_EAN13::RenderDevice(CFX_RenderDevice* device,
                             const CFX_Matrix* matrix) {
  return GetOnedEAN13Writer()->RenderDeviceResult(device, matrix,
                                                  m_renderContents.AsStringC());
}

BC_TYPE CBC_EAN13::GetType() {
  return BC_EAN13;
}

CBC_OnedEAN13Writer* CBC_EAN13::GetOnedEAN13Writer() {
  return static_cast<CBC_OnedEAN13Writer*>(m_pBCWriter.get());
}
