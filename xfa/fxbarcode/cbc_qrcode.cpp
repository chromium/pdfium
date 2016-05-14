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

#include "xfa/fxbarcode/cbc_qrcode.h"

#include "xfa/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/fxbarcode/BC_BufferedImageLuminanceSource.h"
#include "xfa/fxbarcode/common/BC_GlobalHistogramBinarizer.h"
#include "xfa/fxbarcode/qrcode/BC_QRCodeReader.h"
#include "xfa/fxbarcode/qrcode/BC_QRCodeWriter.h"

CBC_QRCode::CBC_QRCode() {
  m_pBCReader = (CBC_Reader*)new (CBC_QRCodeReader);
  ((CBC_QRCodeReader*)m_pBCReader)->Init();
  m_pBCWriter = (CBC_Writer*)new (CBC_QRCodeWriter);
}

CBC_QRCode::~CBC_QRCode() {
  delete (m_pBCReader);
  delete (m_pBCWriter);
}

FX_BOOL CBC_QRCode::SetVersion(int32_t version) {
  if (version < 0 || version > 40)
    return FALSE;
  if (!m_pBCWriter)
    return FALSE;
  return ((CBC_QRCodeWriter*)m_pBCWriter)->SetVersion(version);
}

FX_BOOL CBC_QRCode::SetErrorCorrectionLevel(int32_t level) {
  if (level < 0 || level > 3)
    return FALSE;
  if (!m_pBCWriter)
    return FALSE;
  return ((CBC_TwoDimWriter*)m_pBCWriter)->SetErrorCorrectionLevel(level);
}

FX_BOOL CBC_QRCode::Encode(const CFX_WideStringC& contents,
                           FX_BOOL isDevice,
                           int32_t& e) {
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  uint8_t* data =
      ((CBC_QRCodeWriter*)m_pBCWriter)
          ->Encode(CFX_WideString(contents),
                   ((CBC_QRCodeWriter*)m_pBCWriter)->GetErrorCorrectionLevel(),
                   outWidth, outHeight, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderResult(data, outWidth, outHeight, e);
  FX_Free(data);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}

FX_BOOL CBC_QRCode::RenderDevice(CFX_RenderDevice* device,
                                 const CFX_Matrix* matrix,
                                 int32_t& e) {
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderDeviceResult(device, matrix);
  return TRUE;
}

FX_BOOL CBC_QRCode::RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) {
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderBitmapResult(pOutBitmap, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}

CFX_WideString CBC_QRCode::Decode(uint8_t* buf,
                                  int32_t width,
                                  int32_t height,
                                  int32_t& e) {
  CFX_WideString str;
  return str;
}

CFX_WideString CBC_QRCode::Decode(CFX_DIBitmap* pBitmap, int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString retStr = m_pBCReader->Decode(&bitmap, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, CFX_WideString());
  return CFX_WideString::FromUTF8(retStr.AsStringC());
}
