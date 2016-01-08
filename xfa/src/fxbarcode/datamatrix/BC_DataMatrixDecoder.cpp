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
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/src/fxbarcode/common/reedsolomon/BC_ReedSolomonDecoder.h"
#include "xfa/src/fxbarcode/common/reedsolomon/BC_ReedSolomonGF256.h"
#include "BC_DataMatrixDecoder.h"
#include "BC_DataMatrixBitMatrixParser.h"
#include "BC_DataMatrixVersion.h"
#include "BC_DataMatrixDataBlock.h"
#include "BC_DataMatrixDecodedBitStreamParser.h"
CBC_DataMatrixDecoder::CBC_DataMatrixDecoder() {
  m_rsDecoder = NULL;
}
void CBC_DataMatrixDecoder::Init() {
  m_rsDecoder =
      new CBC_ReedSolomonDecoder(CBC_ReedSolomonGF256::DataMatrixField);
}
CBC_DataMatrixDecoder::~CBC_DataMatrixDecoder() {
  if (m_rsDecoder != NULL) {
    delete m_rsDecoder;
  }
  m_rsDecoder = NULL;
}
CBC_CommonDecoderResult* CBC_DataMatrixDecoder::Decode(
    CBC_CommonBitMatrix* bits,
    int32_t& e) {
  CBC_DataMatrixBitMatrixParser parser;
  parser.Init(bits, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_DataMatrixVersion* version = parser.GetVersion();
  CFX_ByteArray* byteTemp = parser.ReadCodewords(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_AutoPtr<CFX_ByteArray> codewords(byteTemp);
  CFX_PtrArray* dataBlocks =
      CBC_DataMatrixDataBlock::GetDataBlocks(codewords.get(), version, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  int32_t dataBlocksCount = dataBlocks->GetSize();
  int32_t totalBytes = 0;
  int32_t i, j;
  for (i = 0; i < dataBlocksCount; i++) {
    totalBytes +=
        ((CBC_DataMatrixDataBlock*)(*dataBlocks)[i])->GetNumDataCodewords();
  }
  CFX_ByteArray resultBytes;
  resultBytes.SetSize(totalBytes);
  for (j = 0; j < dataBlocksCount; j++) {
    CFX_ByteArray* codewordBytes =
        ((CBC_DataMatrixDataBlock*)(*dataBlocks)[j])->GetCodewords();
    int32_t numDataCodewords =
        ((CBC_DataMatrixDataBlock*)(*dataBlocks)[j])->GetNumDataCodewords();
    CorrectErrors(*codewordBytes, numDataCodewords, e);
    if (e != BCExceptionNO) {
      for (int32_t i = 0; i < dataBlocks->GetSize(); i++) {
        delete (CBC_DataMatrixDataBlock*)(*dataBlocks)[i];
      }
      delete dataBlocks;
      dataBlocks = NULL;
      return NULL;
    }
    int32_t i;
    for (i = 0; i < numDataCodewords; i++) {
      resultBytes[i * dataBlocksCount + j] = (*codewordBytes)[i];
    }
  }
  for (i = 0; i < (dataBlocks->GetSize()); i++) {
    delete (CBC_DataMatrixDataBlock*)(*dataBlocks)[i];
  }
  delete dataBlocks;
  dataBlocks = NULL;
  CBC_CommonDecoderResult* resultR =
      CBC_DataMatrixDecodedBitStreamParser::Decode(resultBytes, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
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
