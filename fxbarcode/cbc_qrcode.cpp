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

#include "fxbarcode/cbc_qrcode.h"

#include <memory>

#include "fxbarcode/qrcode/BC_QRCodeWriter.h"

CBC_QRCode::CBC_QRCode() : CBC_CodeBase(new CBC_QRCodeWriter) {}

CBC_QRCode::~CBC_QRCode() {}

bool CBC_QRCode::SetErrorCorrectionLevel(int32_t level) {
  if (level < 0 || level > 3)
    return false;
  return m_pBCWriter && writer()->SetErrorCorrectionLevel(level);
}

bool CBC_QRCode::Encode(const CFX_WideStringC& contents,
                        bool isDevice,
                        int32_t& e) {
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  CBC_QRCodeWriter* pWriter = writer();
  std::unique_ptr<uint8_t, FxFreeDeleter> data(pWriter->Encode(
      CFX_WideString(contents), pWriter->GetErrorCorrectionLevel(), outWidth,
      outHeight, e));
  if (e != BCExceptionNO)
    return false;
  pWriter->RenderResult(data.get(), outWidth, outHeight, e);
  return e == BCExceptionNO;
}

bool CBC_QRCode::RenderDevice(CFX_RenderDevice* device,
                              const CFX_Matrix* matrix,
                              int32_t& e) {
  writer()->RenderDeviceResult(device, matrix);
  return true;
}

bool CBC_QRCode::RenderBitmap(CFX_RetainPtr<CFX_DIBitmap>& pOutBitmap,
                              int32_t& e) {
  writer()->RenderBitmapResult(pOutBitmap, e);
  return e == BCExceptionNO;
}

BC_TYPE CBC_QRCode::GetType() {
  return BC_QR_CODE;
}

CBC_QRCodeWriter* CBC_QRCode::writer() {
  return static_cast<CBC_QRCodeWriter*>(m_pBCWriter.get());
}
