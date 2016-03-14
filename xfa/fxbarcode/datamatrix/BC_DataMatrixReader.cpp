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

#include "xfa/fxbarcode/datamatrix/BC_DataMatrixReader.h"

#include <memory>

#include "xfa/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/fxbarcode/BC_Reader.h"
#include "xfa/fxbarcode/common/BC_CommonDecoderResult.h"
#include "xfa/fxbarcode/datamatrix/BC_DataMatrixDecoder.h"
#include "xfa/fxbarcode/datamatrix/BC_DataMatrixDetector.h"
#include "xfa/fxbarcode/qrcode/BC_QRDetectorResult.h"
#include "xfa/fxbarcode/utils.h"

CBC_DataMatrixReader::CBC_DataMatrixReader() {
  m_decoder = NULL;
}
void CBC_DataMatrixReader::Init() {
  m_decoder = new CBC_DataMatrixDecoder;
  m_decoder->Init();
}
CBC_DataMatrixReader::~CBC_DataMatrixReader() {
  delete m_decoder;
}
CFX_ByteString CBC_DataMatrixReader::Decode(CBC_BinaryBitmap* image,
                                            int32_t hints,
                                            int32_t& e) {
  CBC_CommonBitMatrix* cdr = image->GetBlackMatrix(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  CBC_DataMatrixDetector detector(cdr);
  detector.Init(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  std::unique_ptr<CBC_QRDetectorResult> detectorResult(detector.Detect(e));
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  std::unique_ptr<CBC_CommonDecoderResult> decodeResult(
      m_decoder->Decode(detectorResult->GetBits(), e));
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return decodeResult->GetText();
}
CFX_ByteString CBC_DataMatrixReader::Decode(CBC_BinaryBitmap* image,
                                            int32_t& e) {
  CFX_ByteString bs = Decode(image, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, "");
  return bs;
}
