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

#include "xfa/fxbarcode/cbc_pdf417i.h"

#include "xfa/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/fxbarcode/BC_BufferedImageLuminanceSource.h"
#include "xfa/fxbarcode/common/BC_GlobalHistogramBinarizer.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417Reader.h"
#include "xfa/fxbarcode/pdf417/BC_PDF417Writer.h"

CBC_PDF417I::CBC_PDF417I() {
  m_pBCReader = (CBC_Reader*)new (CBC_PDF417Reader);
  m_pBCWriter = (CBC_Writer*)new (CBC_PDF417Writer);
}

CBC_PDF417I::~CBC_PDF417I() {
  delete (m_pBCReader);
  delete (m_pBCWriter);
}

FX_BOOL CBC_PDF417I::SetErrorCorrectionLevel(int32_t level) {
  ((CBC_PDF417Writer*)m_pBCWriter)->SetErrorCorrectionLevel(level);
  return TRUE;
}

void CBC_PDF417I::SetTruncated(FX_BOOL truncated) {
  ((CBC_PDF417Writer*)m_pBCWriter)->SetTruncated(truncated);
}

FX_BOOL CBC_PDF417I::Encode(const CFX_WideStringC& contents,
                            FX_BOOL isDevice,
                            int32_t& e) {
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  uint8_t* data = ((CBC_PDF417Writer*)m_pBCWriter)
                      ->Encode(contents, outWidth, outHeight, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderResult(data, outWidth, outHeight, e);
  FX_Free(data);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}

FX_BOOL CBC_PDF417I::RenderDevice(CFX_RenderDevice* device,
                                  const CFX_Matrix* matirx,
                                  int32_t& e) {
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderDeviceResult(device, matirx);
  return TRUE;
}

FX_BOOL CBC_PDF417I::RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) {
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderBitmapResult(pOutBitmap, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}

CFX_WideString CBC_PDF417I::Decode(uint8_t* buf,
                                   int32_t width,
                                   int32_t hight,
                                   int32_t& e) {
  CFX_WideString str;
  return str;
}

CFX_WideString CBC_PDF417I::Decode(CFX_DIBitmap* pBitmap, int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString bytestring = m_pBCReader->Decode(&bitmap, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FX_WSTRC(L""));
  return CFX_WideString::FromUTF8(bytestring, bytestring.GetLength());
}
