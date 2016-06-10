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

#include "xfa/fxbarcode/datamatrix/BC_DataMatrixDataBlock.h"

#include <memory>

#include "xfa/fxbarcode/datamatrix/BC_DataMatrixVersion.h"
#include "xfa/fxbarcode/utils.h"

CBC_DataMatrixDataBlock::~CBC_DataMatrixDataBlock() {}
CBC_DataMatrixDataBlock::CBC_DataMatrixDataBlock(int32_t numDataCodewords,
                                                 CFX_ByteArray* codewords) {
  m_codewords.Copy(*codewords);
  m_numDataCodewords = numDataCodewords;
}
CFX_ArrayTemplate<CBC_DataMatrixDataBlock*>*
CBC_DataMatrixDataBlock::GetDataBlocks(CFX_ByteArray* rawCodewords,
                                       CBC_DataMatrixVersion* version,
                                       int32_t& e) {
  ECBlocks* ecBlocks = version->GetECBlocks();
  int32_t totalBlocks = 0;
  const CFX_ArrayTemplate<ECB*>& ecBlockArray = ecBlocks->GetECBlocks();
  for (int32_t i = 0; i < ecBlockArray.GetSize(); i++) {
    totalBlocks += ecBlockArray[i]->GetCount();
  }
  std::unique_ptr<CFX_ArrayTemplate<CBC_DataMatrixDataBlock*>> result(
      new CFX_ArrayTemplate<CBC_DataMatrixDataBlock*>());
  result->SetSize(totalBlocks);
  int32_t numResultBlocks = 0;
  for (int32_t j = 0; j < ecBlockArray.GetSize(); j++) {
    for (int32_t i = 0; i < ecBlockArray[j]->GetCount(); i++) {
      int32_t numDataCodewords = ecBlockArray[j]->GetDataCodewords();
      int32_t numBlockCodewords = ecBlocks->GetECCodewords() + numDataCodewords;
      CFX_ByteArray codewords;
      codewords.SetSize(numBlockCodewords);
      (*result)[numResultBlocks++] =
          new CBC_DataMatrixDataBlock(numDataCodewords, &codewords);
      codewords.SetSize(0);
    }
  }
  int32_t longerBlocksNumDataCodewords =
      (*result)[0]->GetCodewords()->GetSize() - ecBlocks->GetECCodewords();
  int32_t rawCodewordsOffset = 0;
  for (int32_t i = 0; i < longerBlocksNumDataCodewords - 1; i++) {
    for (int32_t j = 0; j < numResultBlocks; j++) {
      if (rawCodewordsOffset < rawCodewords->GetSize()) {
        (*result)[j]->GetCodewords()->operator[](i) =
            (*rawCodewords)[rawCodewordsOffset++];
      }
    }
  }
  const bool specialVersion = version->GetVersionNumber() == 24;
  int32_t numLongerBlocks = specialVersion ? 8 : numResultBlocks;
  for (int32_t j = 0; j < numLongerBlocks; j++) {
    if (rawCodewordsOffset < rawCodewords->GetSize()) {
      (*result)[j]->GetCodewords()->operator[](longerBlocksNumDataCodewords -
                                               1) =
          (*rawCodewords)[rawCodewordsOffset++];
    }
  }
  for (int32_t i = longerBlocksNumDataCodewords;
       i < (*result)[0]->GetCodewords()->GetSize(); i++) {
    for (int32_t j = 0; j < numResultBlocks; j++) {
      int32_t iOffset = specialVersion && j > 7 ? i - 1 : i;
      if (rawCodewordsOffset < rawCodewords->GetSize()) {
        (*result)[j]->GetCodewords()->operator[](iOffset) =
            (*rawCodewords)[rawCodewordsOffset++];
      }
    }
  }
  if (rawCodewordsOffset != rawCodewords->GetSize()) {
    e = BCExceptionIllegalArgument;
    return nullptr;
  }
  return result.release();
}

int32_t CBC_DataMatrixDataBlock::GetNumDataCodewords() {
  return m_numDataCodewords;
}
CFX_ByteArray* CBC_DataMatrixDataBlock::GetCodewords() {
  return &m_codewords;
}
