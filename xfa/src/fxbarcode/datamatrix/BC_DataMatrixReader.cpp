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

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/src/fxbarcode/BC_Reader.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRDetectorResult.h"
#include "xfa/src/fxbarcode/common/BC_CommonDecoderResult.h"
#include "BC_DataMatrixDecoder.h"
#include "BC_DataMatrixDetector.h"
#include "BC_DataMatrixReader.h"
CBC_DataMatrixReader::CBC_DataMatrixReader() {
  m_decoder = NULL;
}
void CBC_DataMatrixReader::Init() {
  m_decoder = new CBC_DataMatrixDecoder;
  m_decoder->Init();
}
CBC_DataMatrixReader::~CBC_DataMatrixReader() {
  if (m_decoder != NULL) {
    delete m_decoder;
  }
  m_decoder = NULL;
}
CFX_ByteString CBC_DataMatrixReader::Decode(CBC_BinaryBitmap* image,
                                            int32_t hints,
                                            int32_t& e) {
  CBC_CommonBitMatrix* cdr = image->GetBlackMatrix(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CBC_DataMatrixDetector detector(cdr);
  detector.Init(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CBC_QRDetectorResult* ddr = detector.Detect(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CBC_AutoPtr<CBC_QRDetectorResult> detectorResult(ddr);
  CBC_CommonDecoderResult* ResultTemp =
      m_decoder->Decode(detectorResult->GetBits(), e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CBC_AutoPtr<CBC_CommonDecoderResult> decodeResult(ResultTemp);
  return decodeResult->GetText();
}
CFX_ByteString CBC_DataMatrixReader::Decode(CBC_BinaryBitmap* image,
                                            int32_t& e) {
  CFX_ByteString bs = Decode(image, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return bs;
}
