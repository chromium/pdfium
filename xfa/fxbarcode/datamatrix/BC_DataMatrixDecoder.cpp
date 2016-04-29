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

#include "xfa/fxbarcode/datamatrix/BC_DataMatrixDecoder.h"

#include <memory>

#include "xfa/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonDecoder.h"
#include "xfa/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256.h"
#include "xfa/fxbarcode/datamatrix/BC_DataMatrixBitMatrixParser.h"
#include "xfa/fxbarcode/datamatrix/BC_DataMatrixDataBlock.h"
#include "xfa/fxbarcode/datamatrix/BC_DataMatrixDecodedBitStreamParser.h"
#include "xfa/fxbarcode/datamatrix/BC_DataMatrixVersion.h"

CBC_DataMatrixDecoder::CBC_DataMatrixDecoder() {
  m_rsDecoder = NULL;
}
void CBC_DataMatrixDecoder::Init() {
  m_rsDecoder =
      new CBC_ReedSolomonDecoder(CBC_ReedSolomonGF256::DataMatrixField);
}
CBC_DataMatrixDecoder::~CBC_DataMatrixDecoder() {
  delete m_rsDecoder;
}

CBC_CommonDecoderResult* CBC_DataMatrixDecoder::Decode(
    CBC_CommonBitMatrix* bits,
    int32_t& e) {
  CBC_DataMatrixBitMatrixParser parser;
  parser.Init(bits, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  CBC_DataMatrixVersion* version = parser.GetVersion();
  std::unique_ptr<CFX_ByteArray> codewords(parser.ReadCodewords(e));
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  CFX_ArrayTemplate<CBC_DataMatrixDataBlock*>* dataBlocks =
      CBC_DataMatrixDataBlock::GetDataBlocks(codewords.get(), version, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  int32_t dataBlocksCount = dataBlocks->GetSize();
  int32_t totalBytes = 0;
  int32_t i, j;
  for (i = 0; i < dataBlocksCount; i++) {
    totalBytes += (*dataBlocks)[i]->GetNumDataCodewords();
  }
  CFX_ByteArray resultBytes;
  resultBytes.SetSize(totalBytes);
  for (j = 0; j < dataBlocksCount; j++) {
    CFX_ByteArray* codewordBytes = (*dataBlocks)[j]->GetCodewords();
    int32_t numDataCodewords = (*dataBlocks)[j]->GetNumDataCodewords();
    CorrectErrors(*codewordBytes, numDataCodewords, e);
    if (e != BCExceptionNO) {
      for (int32_t i = 0; i < dataBlocks->GetSize(); i++) {
        delete (*dataBlocks)[i];
      }
      delete dataBlocks;
      return nullptr;
    }
    int32_t i;
    for (i = 0; i < numDataCodewords; i++) {
      resultBytes[i * dataBlocksCount + j] = (*codewordBytes)[i];
    }
  }
  for (i = 0; i < (dataBlocks->GetSize()); i++) {
    delete (*dataBlocks)[i];
  }
  delete dataBlocks;
  CBC_CommonDecoderResult* resultR =
      CBC_DataMatrixDecodedBitStreamParser::Decode(resultBytes, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, nullptr);
  return resultR;
}

void CBC_DataMatrixDecoder::CorrectErrors(CFX_ByteArray& codewordBytes,
                                          int32_t numDataCodewords,
                                          int32_t& e) {
  int32_t numCodewords = codewordBytes.GetSize();
  CFX_Int32Array codewordsInts;
  codewordsInts.SetSize(numCodewords);
  int32_t i;
  for (i = 0; i < numCodewords; i++) {
    codewordsInts[i] = codewordBytes[i] & 0xFF;
  }
  int32_t numECCodewords = codewordBytes.GetSize() - numDataCodewords;
  m_rsDecoder->Decode(&codewordsInts, numECCodewords, e);
  if (e != BCExceptionNO) {
    e = BCExceptionChecksumException;
    return;
  }
  for (i = 0; i < numDataCodewords; i++) {
    codewordBytes[i] = (uint8_t)codewordsInts[i];
  }
}
