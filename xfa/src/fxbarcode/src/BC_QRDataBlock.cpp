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

#include "barcode.h"
#include "include/BC_QRCoderECBlocks.h"
#include "include/BC_QRCoderECB.h"
#include "include/BC_QRDataBlock.h"
#include "include/BC_QRCoderVersion.h"
CBC_QRDataBlock::CBC_QRDataBlock(FX_INT32 numDataCodewords, CFX_ByteArray *codewords)
    : m_numDataCodewords(numDataCodewords)
    , m_codewords(codewords)
{
}
CBC_QRDataBlock::~CBC_QRDataBlock()
{
    if(m_codewords != NULL) {
        delete m_codewords;
        m_codewords = NULL;
    }
}
FX_INT32 CBC_QRDataBlock::GetNumDataCodewords()
{
    return m_numDataCodewords;
}
CFX_ByteArray *CBC_QRDataBlock::GetCodewords()
{
    return m_codewords;
}
CFX_PtrArray *CBC_QRDataBlock::GetDataBlocks(CFX_ByteArray* rawCodewords, CBC_QRCoderVersion *version, CBC_QRCoderErrorCorrectionLevel* ecLevel, FX_INT32 &e)
{
    if(rawCodewords->GetSize() != version->GetTotalCodeWords()) {
        e = BCExceptionIllegalArgument;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    CBC_QRCoderECBlocks *ecBlocks = version->GetECBlocksForLevel(ecLevel);
    FX_INT32 totalBlocks = 0;
    CFX_PtrArray* ecBlockArray = ecBlocks->GetECBlocks();
    FX_INT32 i = 0;
    for(i = 0; i < ecBlockArray->GetSize(); i++) {
        totalBlocks += ((CBC_QRCoderECB*)(*ecBlockArray)[i])->GetCount();
    }
    CFX_PtrArray *datablock = FX_NEW CFX_PtrArray();
    datablock->SetSize(totalBlocks);
    CBC_AutoPtr<CFX_PtrArray > result(datablock);
    FX_INT32 numResultBlocks = 0;
    for(FX_INT32 j = 0; j < ecBlockArray->GetSize(); j++) {
        CBC_QRCoderECB *ecBlock = (CBC_QRCoderECB*)(*ecBlockArray)[j];
        for(FX_INT32 k = 0; k < ecBlock->GetCount(); k++) {
            FX_INT32 numDataCodewords = ecBlock->GetDataCodeWords();
            FX_INT32 numBlockCodewords = ecBlocks->GetECCodeWordsPerBlock() + numDataCodewords;
            CFX_ByteArray *bytearray = FX_NEW CFX_ByteArray();
            bytearray->SetSize(numBlockCodewords);
            (*result)[numResultBlocks++] = FX_NEW CBC_QRDataBlock(numDataCodewords, bytearray);
        }
    }
    FX_INT32 shorterBlocksTotalCodewords = ((CBC_QRDataBlock*)(*result)[0])->m_codewords->GetSize();
    FX_INT32 longerBlocksStartAt = result->GetSize() - 1;
    while(longerBlocksStartAt >= 0) {
        FX_INT32 numCodewords = ((CBC_QRDataBlock*)(*result)[longerBlocksStartAt])->m_codewords->GetSize();
        if(numCodewords == shorterBlocksTotalCodewords) {
            break;
        }
        longerBlocksStartAt--;
    }
    longerBlocksStartAt++;
    FX_INT32 shorterBlocksNumDataCodewords = shorterBlocksTotalCodewords - ecBlocks->GetECCodeWordsPerBlock();
    FX_INT32 rawCodewordsOffset = 0;
    FX_INT32 x = 0;
    for(FX_INT32 k = 0; k < shorterBlocksNumDataCodewords; k++) {
        for(x = 0; x < numResultBlocks; x++) {
            (*(((CBC_QRDataBlock*)(*result)[x])->m_codewords))[k] = (*rawCodewords)[rawCodewordsOffset++];
        }
    }
    for(x = longerBlocksStartAt; x < numResultBlocks; x++) {
        (*(((CBC_QRDataBlock*)(*result)[x])->m_codewords))[shorterBlocksNumDataCodewords] = (*rawCodewords)[rawCodewordsOffset++];
    }
    FX_INT32 max = ((CBC_QRDataBlock*)(*result)[0])->m_codewords->GetSize();
    for(i = shorterBlocksNumDataCodewords; i < max; i++) {
        for(FX_INT32 y = 0; y < numResultBlocks; y++) {
            FX_INT32 iOffset = y < longerBlocksStartAt ? i : i + 1;
            (*(((CBC_QRDataBlock*)(*result)[y])->m_codewords))[iOffset] = (*rawCodewords)[rawCodewordsOffset++];
        }
    }
    return result.release();
}
