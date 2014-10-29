// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_ReedSolomonDecoder.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_CommonDecoderResult.h"
#include "include/BC_QRBitMatrixParser.h"
#include "include/BC_QRDataBlock.h"
#include "include/BC_QRDecodedBitStreamParser.h"
#include "include/BC_QRCoderVersion.h"
#include "include/BC_QRCoderFormatInformation.h"
#include "include/BC_ReedSolomonGF256.h"
#include "include/BC_QRCoderDecoder.h"
CBC_QRCoderDecoder::CBC_QRCoderDecoder()
{
    m_rsDecoder = NULL;
}

void CBC_QRCoderDecoder::Init()
{
    m_rsDecoder = FX_NEW CBC_ReedSolomonDecoder(CBC_ReedSolomonGF256::QRCodeFild);
}
CBC_QRCoderDecoder::~CBC_QRCoderDecoder()
{
    if(m_rsDecoder != NULL) {
        delete m_rsDecoder;
    }
    m_rsDecoder = NULL;
}
CBC_CommonDecoderResult* CBC_QRCoderDecoder::Decode(FX_BOOL* image, FX_INT32 width, FX_INT32 height, FX_INT32 &e)
{
    CBC_CommonBitMatrix  bits;
    bits.Init(width);
    for(FX_INT32 i = 0; i < width; i++) {
        for(FX_INT32 j = 0; j < height; j++) {
            if(image[i * width + j]) {
                bits.Set(j, i);
            }
        }
    }
    CBC_CommonDecoderResult* cdr = Decode(&bits, height, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return cdr;
}
CBC_CommonDecoderResult* CBC_QRCoderDecoder::Decode(CBC_CommonBitMatrix* bits, FX_INT32 byteModeDecode, FX_INT32 &e)
{
    CBC_QRBitMatrixParser parser;
    parser.Init(bits, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CBC_QRCoderVersion *version = parser.ReadVersion(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CBC_QRCoderFormatInformation* temp = parser.ReadFormatInformation(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CBC_QRCoderErrorCorrectionLevel* ecLevel = temp->GetErrorCorrectionLevel();
    CFX_ByteArray* ba = parser.ReadCodewords(e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    CBC_AutoPtr<CFX_ByteArray > codewords(ba);
    CFX_PtrArray *dataBlocks = CBC_QRDataBlock::GetDataBlocks(codewords.get(), version, ecLevel, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    FX_INT32 totalBytes = 0;
    for (FX_INT32 i = 0; i < dataBlocks->GetSize(); i++) {
        totalBytes += ((CBC_QRDataBlock*) ((*dataBlocks)[i]))->GetNumDataCodewords();
    }
    CFX_ByteArray resultBytes;
    FX_INT32 resultOffset = 0;
    for (FX_INT32 j = 0; j < dataBlocks->GetSize(); j++) {
        CBC_QRDataBlock *dataBlock = (CBC_QRDataBlock *)((*dataBlocks)[j]);
        CFX_ByteArray* codewordBytes = dataBlock->GetCodewords();
        FX_INT32 numDataCodewords = dataBlock->GetNumDataCodewords();
        CorrectErrors(codewordBytes, numDataCodewords, e);
        if (e != BCExceptionNO) {
            for(FX_INT32 k = 0; k < dataBlocks->GetSize(); k++) {
                delete (CBC_QRDataBlock*)(*dataBlocks)[k];
            }
            dataBlocks->RemoveAll();
            delete dataBlocks;
            dataBlocks = NULL;
            return NULL;
        }
        for(FX_INT32 i = 0; i < numDataCodewords; i++) {
            resultBytes.Add((*codewordBytes)[i]);
        }
    }
    for(FX_INT32 k = 0; k < dataBlocks->GetSize(); k++) {
        delete  (CBC_QRDataBlock*)(*dataBlocks)[k] ;
    }
    dataBlocks->RemoveAll();
    delete dataBlocks;
    dataBlocks = NULL;
    CBC_CommonDecoderResult* cdr = CBC_QRDecodedBitStreamParser::Decode(&resultBytes, version, ecLevel, byteModeDecode, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return cdr;
}
void CBC_QRCoderDecoder::CorrectErrors(CFX_ByteArray* codewordBytes, FX_INT32 numDataCodewords, FX_INT32 &e)
{
    FX_INT32 numCodewords = codewordBytes->GetSize();
    CFX_Int32Array codewordsInts;
    codewordsInts.SetSize(numCodewords);
    for(FX_INT32 i = 0; i < numCodewords; i++) {
        codewordsInts[i] = (FX_INT32)((*codewordBytes)[i] & 0xff);
    }
    FX_INT32 numECCodewords = codewordBytes->GetSize() - numDataCodewords;
    m_rsDecoder->Decode(&codewordsInts, numECCodewords, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    for(FX_INT32 k = 0; k < numDataCodewords; k++) {
        (*codewordBytes)[k] = (FX_BYTE) codewordsInts[k];
    }
}
