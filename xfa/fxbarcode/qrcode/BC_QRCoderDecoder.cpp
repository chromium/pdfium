// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2008 ZXing authors
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

#include "xfa/fxbarcode/qrcode/BC_QRCoderDecoder.h"

#include <memory>

#include "xfa/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/fxbarcode/common/BC_CommonDecoderResult.h"
#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonDecoder.h"
#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256.h"
#include "xfa/fxbarcode/qrcode/BC_QRBitMatrixParser.h"
#include "xfa/fxbarcode/qrcode/BC_QRCoderFormatInformation.h"
#include "xfa/fxbarcode/qrcode/BC_QRCoderVersion.h"
#include "xfa/fxbarcode/qrcode/BC_QRDataBlock.h"
#include "xfa/fxbarcode/qrcode/BC_QRDecodedBitStreamParser.h"

CBC_QRCoderDecoder::CBC_QRCoderDecoder() {
  m_rsDecoder = NULL;
}

void CBC_QRCoderDecoder::Init() {
  m_rsDecoder = new CBC_ReedSolomonDecoder(CBC_ReedSolomonGF256::QRCodeFild);
}
CBC_QRCoderDecoder::~CBC_QRCoderDecoder() {
  delete m_rsDecoder;
}
CBC_CommonDecoderResult* CBC_QRCoderDecoder::Decode(FX_BOOL* image,
                                                    int32_t width,
                                                    int32_t height,
                                                    int32_t& e) {
  CBC_CommonBitMatrix bits;
  bits.Init(width);
  for (int32_t i = 0; i < width; i++) {
    for (int32_t j = 0; j < height; j++) {
      if (image[i * width + j]) {
        bits.Set(j, i);
      }
    }
  }
  CBC_CommonDecoderResult* cdr = Decode(&bits, height, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return cdr;
}
CBC_CommonDecoderResult* CBC_QRCoderDecoder::Decode(CBC_CommonBitMatrix* bits,
                                                    int32_t byteModeDecode,
                                                    int32_t& e) {
  CBC_QRBitMatrixParser parser;
  parser.Init(bits, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_QRCoderVersion* version = parser.ReadVersion(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_QRCoderFormatInformation* temp = parser.ReadFormatInformation(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_QRCoderErrorCorrectionLevel* ecLevel = temp->GetErrorCorrectionLevel();
  std::unique_ptr<CFX_ByteArray> codewords(parser.ReadCodewords(e));
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CFX_ArrayTemplate<CBC_QRDataBlock*>* dataBlocks =
      CBC_QRDataBlock::GetDataBlocks(codewords.get(), version, ecLevel, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  int32_t totalBytes = 0;
  for (int32_t i = 0; i < dataBlocks->GetSize(); i++) {
    totalBytes += (*dataBlocks)[i]->GetNumDataCodewords();
  }
  CFX_ByteArray resultBytes;
  for (int32_t j = 0; j < dataBlocks->GetSize(); j++) {
    CBC_QRDataBlock* dataBlock = (*dataBlocks)[j];
    CFX_ByteArray* codewordBytes = dataBlock->GetCodewords();
    int32_t numDataCodewords = dataBlock->GetNumDataCodewords();
    CorrectErrors(codewordBytes, numDataCodewords, e);
    if (e != BCExceptionNO) {
      for (int32_t k = 0; k < dataBlocks->GetSize(); k++) {
        delete (*dataBlocks)[k];
      }
      delete dataBlocks;
      return NULL;
    }
    for (int32_t i = 0; i < numDataCodewords; i++) {
      resultBytes.Add((*codewordBytes)[i]);
    }
  }
  for (int32_t k = 0; k < dataBlocks->GetSize(); k++) {
    delete (*dataBlocks)[k];
  }
  delete dataBlocks;
  CBC_CommonDecoderResult* cdr = CBC_QRDecodedBitStreamParser::Decode(
      &resultBytes, version, ecLevel, byteModeDecode, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return cdr;
}
void CBC_QRCoderDecoder::CorrectErrors(CFX_ByteArray* codewordBytes,
                                       int32_t numDataCodewords,
                                       int32_t& e) {
  int32_t numCodewords = codewordBytes->GetSize();
  CFX_Int32Array codewordsInts;
  codewordsInts.SetSize(numCodewords);
  for (int32_t i = 0; i < numCodewords; i++) {
    codewordsInts[i] = (int32_t)((*codewordBytes)[i] & 0xff);
  }
  int32_t numECCodewords = codewordBytes->GetSize() - numDataCodewords;
  m_rsDecoder->Decode(&codewordsInts, numECCodewords, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  for (int32_t k = 0; k < numDataCodewords; k++) {
    (*codewordBytes)[k] = (uint8_t)codewordsInts[k];
  }
}
