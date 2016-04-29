// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2007 ZXing authors
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

#include "xfa/fxbarcode/qrcode/BC_QRCodeReader.h"

#include <memory>

#include "xfa/fxbarcode/BC_Binarizer.h"
#include "xfa/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/fxbarcode/BC_BufferedImageLuminanceSource.h"
#include "xfa/fxbarcode/BC_LuminanceSource.h"
#include "xfa/fxbarcode/BC_Reader.h"
#include "xfa/fxbarcode/BC_ResultPoint.h"
#include "xfa/fxbarcode/common/BC_CommonDecoderResult.h"
#include "xfa/fxbarcode/common/BC_GlobalHistogramBinarizer.h"
#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256.h"
#include "xfa/fxbarcode/qrcode/BC_QRCoderDecoder.h"
#include "xfa/fxbarcode/qrcode/BC_QRCoderErrorCorrectionLevel.h"
#include "xfa/fxbarcode/qrcode/BC_QRCoderMode.h"
#include "xfa/fxbarcode/qrcode/BC_QRCoderVersion.h"
#include "xfa/fxbarcode/qrcode/BC_QRDataMask.h"
#include "xfa/fxbarcode/qrcode/BC_QRDetector.h"
#include "xfa/fxbarcode/qrcode/BC_QRDetectorResult.h"

CBC_QRCodeReader::CBC_QRCodeReader() : m_decoder(NULL) {}
void CBC_QRCodeReader::Init() {
  m_decoder = new CBC_QRCoderDecoder;
  m_decoder->Init();
}
CBC_QRCodeReader::~CBC_QRCodeReader() {
  delete m_decoder;
}
CFX_ByteString CBC_QRCodeReader::Decode(CBC_BinaryBitmap* image,
                                        int32_t hints,
                                        int32_t& e) {
  CBC_CommonBitMatrix* matrix = image->GetMatrix(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CBC_QRDetector detector(matrix);
  std::unique_ptr<CBC_QRDetectorResult> detectorResult(
      detector.Detect(hints, e));
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  std::unique_ptr<CBC_CommonDecoderResult> decodeResult(
      m_decoder->Decode(detectorResult->GetBits(), 0, e));
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return (decodeResult->GetText());
}
CFX_ByteString CBC_QRCodeReader::Decode(const CFX_WideString& filename,
                                        int32_t hints,
                                        int32_t byteModeDecode,
                                        int32_t& e) {
  CBC_BufferedImageLuminanceSource source(filename);
  source.Init(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString bs = Decode(&bitmap, hints, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return bs;
}
CFX_ByteString CBC_QRCodeReader::Decode(CFX_DIBitmap* pBitmap,
                                        int32_t hints,
                                        int32_t byteModeDecode,
                                        int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString bs = Decode(&bitmap, hints, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return bs;
}
CFX_ByteString CBC_QRCodeReader::Decode(CBC_BinaryBitmap* image, int32_t& e) {
  CFX_ByteString bs = Decode(image, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return bs;
}
void CBC_QRCodeReader::ReleaseAll() {
  delete CBC_ReedSolomonGF256::QRCodeFild;
  CBC_ReedSolomonGF256::QRCodeFild = nullptr;
  delete CBC_ReedSolomonGF256::DataMatrixField;
  CBC_ReedSolomonGF256::DataMatrixField = nullptr;
  CBC_QRCoderMode::Destroy();
  CBC_QRCoderErrorCorrectionLevel::Destroy();
  CBC_QRDataMask::Destroy();
  CBC_QRCoderVersion::Destroy();
}
