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

#include "barcode.h"
#include "include/BC_DataMatrixVersion.h"
#include "include/BC_DataMatrixDataBlock.h"
CBC_DataMatrixDataBlock::~CBC_DataMatrixDataBlock()
{
}
CBC_DataMatrixDataBlock::CBC_DataMatrixDataBlock(FX_INT32 numDataCodewords, CFX_ByteArray *codewords)
{
    m_codewords.Copy(*codewords);
    m_numDataCodewords = numDataCodewords;
}
CFX_PtrArray *CBC_DataMatrixDataBlock::GetDataBlocks(CFX_ByteArray* rawCodewords, CBC_DataMatrixVersion *version, FX_INT32 &e)
{
    ECBlocks *ecBlocks = version->GetECBlocks();
    FX_INT32 totalBlocks = 0;
    const CFX_PtrArray &ecBlockArray = ecBlocks->GetECBlocks();
    FX_INT32 i;
    for (i = 0; i < ecBlockArray.GetSize(); i++) {
        totalBlocks += ((ECB*)ecBlockArray[i])->GetCount();
    }
    CBC_AutoPtr<CFX_PtrArray>result(FX_NEW CFX_PtrArray());
    result->SetSize(totalBlocks);
    FX_INT32 numResultBlocks = 0;
    FX_INT32 j;
    for (j = 0; j < ecBlockArray.GetSize(); j++) {
        for (i = 0; i < ((ECB*)ecBlockArray[j])->GetCount(); i++) {
            FX_INT32 numDataCodewords = ((ECB*)ecBlockArray[j])->GetDataCodewords();
            FX_INT32 numBlockCodewords = ecBlocks->GetECCodewords() + numDataCodewords;
            CFX_ByteArray codewords;
            codewords.SetSize(numBlockCodewords);
            (*result)[numResultBlocks++] = FX_NEW CBC_DataMatrixDataBlock(numDataCodewords, &codewords);
            codewords.SetSize(0);
        }
    }
    FX_INT32 longerBlocksTotalCodewords = ((CBC_DataMatrixDataBlock*)(*result)[0])->GetCodewords()->GetSize();
    FX_INT32 longerBlocksNumDataCodewords = longerBlocksTotalCodewords - ecBlocks->GetECCodewords();
    FX_INT32 shorterBlocksNumDataCodewords = longerBlocksNumDataCodewords - 1;
    FX_INT32 rawCodewordsOffset = 0;
    for (i = 0; i < shorterBlocksNumDataCodewords; i++) {
        FX_INT32 j;
        for (j = 0; j < numResultBlocks; j++) {
            if (rawCodewordsOffset < rawCodewords->GetSize()) {
                ((CBC_DataMatrixDataBlock*)(*result)[j])->GetCodewords()->operator [](i) = (*rawCodewords)[rawCodewordsOffset++];
            }
        }
    }
    FX_BOOL specialVersion = version->GetVersionNumber() == 24;
    FX_INT32 numLongerBlocks = specialVersion ? 8 : numResultBlocks;
    for (j = 0; j < numLongerBlocks; j++) {
        if (rawCodewordsOffset < rawCodewords->GetSize()) {
            ((CBC_DataMatrixDataBlock*)(*result)[j])->GetCodewords()->operator [](longerBlocksNumDataCodewords - 1) = (*rawCodewords)[rawCodewordsOffset++];
        }
    }
    FX_INT32 max = ((CBC_DataMatrixDataBlock*)(*result)[0])->GetCodewords()->GetSize();
    for (i = longerBlocksNumDataCodewords; i < max; i++) {
        FX_INT32 j;
        for (j = 0; j < numResultBlocks; j++) {
            FX_INT32 iOffset = specialVersion && j > 7 ? i - 1 : i;
            if (rawCodewordsOffset < rawCodewords->GetSize()) {
                ((CBC_DataMatrixDataBlock*)(*result)[j])->GetCodewords()->operator [](iOffset) = (*rawCodewords)[rawCodewordsOffset++];
            }
        }
    }
    if (rawCodewordsOffset != rawCodewords->GetSize()) {
        e = BCExceptionIllegalArgument;
        return NULL;
    }
    return result.release();
}
FX_INT32 CBC_DataMatrixDataBlock::GetNumDataCodewords()
{
    return m_numDataCodewords;
}
CFX_ByteArray *CBC_DataMatrixDataBlock::GetCodewords()
{
    return &m_codewords;
}
