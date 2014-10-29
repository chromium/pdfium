// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/utils.h"
#include "include/BC_QRCoderECB.h"
#include "include/BC_CommonBitMatrix.h"
#include "include/BC_QRCoderFormatInformation.h"
#include "include/BC_QRCoderErrorCorrectionLevel.h"
#include "include/BC_QRCoderBitVector.h"
#include "include/BC_QRCoderECBlocks.h"
#include "include/BC_QRCoderVersion.h"
const FX_INT32 CBC_QRCoderVersion::VERSION_DECODE_INFO[] = {
    0x07C94, 0x085BC, 0x09A99, 0x0A4D3, 0x0BBF6,
    0x0C762, 0x0D847, 0x0E60D, 0x0F928, 0x10B78,
    0x1145D, 0x12A17, 0x13532, 0x149A6, 0x15683,
    0x168C9, 0x177EC, 0x18EC4, 0x191E1, 0x1AFAB,
    0x1B08E, 0x1CC1A, 0x1D33F, 0x1ED75, 0x1F250,
    0x209D5, 0x216F0, 0x228BA, 0x2379F, 0x24B0B,
    0x2542E, 0x26A64, 0x27541, 0x28C69
};
CFX_PtrArray *CBC_QRCoderVersion::VERSION = NULL;
void CBC_QRCoderVersion::Initialize()
{
    VERSION = FX_NEW CFX_PtrArray();
}
void CBC_QRCoderVersion::Finalize()
{
    for(FX_INT32 i = 0 ; i < VERSION->GetSize(); i++) {
        CBC_QRCoderVersion* v = (CBC_QRCoderVersion*)(VERSION->GetAt(i));
        delete v;
    }
    delete VERSION;
}
CBC_QRCoderVersion::CBC_QRCoderVersion(FX_INT32 versionNumber,
                                       CBC_QRCoderECBlocks* ecBlocks1,
                                       CBC_QRCoderECBlocks* ecBlocks2,
                                       CBC_QRCoderECBlocks* ecBlocks3,
                                       CBC_QRCoderECBlocks* ecBlocks4)
{
    m_versionNumber = versionNumber;
    m_ecBlocks.Add(ecBlocks1);
    m_ecBlocks.Add(ecBlocks2);
    m_ecBlocks.Add(ecBlocks3);
    m_ecBlocks.Add(ecBlocks4);
    FX_INT32 total = 0;
    FX_INT32 ecCodeWords = ecBlocks1->GetECCodeWordsPerBlock();
    CFX_PtrArray* ecbArray = ecBlocks1->GetECBlocks();
    for(FX_INT32 i = 0; i < ecbArray->GetSize(); i++) {
        CBC_QRCoderECB* ecBlock = (CBC_QRCoderECB*)((*ecbArray)[i]);
        total += ecBlock->GetCount() * (ecBlock->GetDataCodeWords() + ecCodeWords);
    }
    m_totalCodeWords = total;
    switch(versionNumber) {
        case 1:
            break;
        case 2:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(18);
            break;
        case 3:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(22);
            break;
        case 4:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(26);
            break;
        case 5:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            break;
        case 6:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(34);
            break;
        case 7:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(22);
            m_alignmentPatternCenters.Add(38);
            break;
        case 8:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(24);
            m_alignmentPatternCenters.Add(42);
            break;
        case 9:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(26);
            m_alignmentPatternCenters.Add(46);
            break;
        case 10:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(28);
            m_alignmentPatternCenters.Add(50);
            break;
        case 11:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            m_alignmentPatternCenters.Add(54);
            break;
        case 12:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(32);
            m_alignmentPatternCenters.Add(58);
            break;
        case 13:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(34);
            m_alignmentPatternCenters.Add(62);
            break;
        case 14:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(26);
            m_alignmentPatternCenters.Add(46);
            m_alignmentPatternCenters.Add(66);
            break;
        case 15:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(26);
            m_alignmentPatternCenters.Add(48);
            m_alignmentPatternCenters.Add(70);
            break;
        case 16:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(26);
            m_alignmentPatternCenters.Add(50);
            m_alignmentPatternCenters.Add(74);
            break;
        case 17:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            m_alignmentPatternCenters.Add(54);
            m_alignmentPatternCenters.Add(78);
            break;
        case 18:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            m_alignmentPatternCenters.Add(56);
            m_alignmentPatternCenters.Add(82);
            break;
        case 19:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            m_alignmentPatternCenters.Add(58);
            m_alignmentPatternCenters.Add(86);
            break;
        case 20:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(34);
            m_alignmentPatternCenters.Add(62);
            m_alignmentPatternCenters.Add(90);
            break;
        case 21:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(28);
            m_alignmentPatternCenters.Add(50);
            m_alignmentPatternCenters.Add(72);
            m_alignmentPatternCenters.Add(94);
            break;
        case 22:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(26);
            m_alignmentPatternCenters.Add(50);
            m_alignmentPatternCenters.Add(74);
            m_alignmentPatternCenters.Add(98);
            break;
        case 23:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            m_alignmentPatternCenters.Add(54);
            m_alignmentPatternCenters.Add(74);
            m_alignmentPatternCenters.Add(102);
            break;
        case 24:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(28);
            m_alignmentPatternCenters.Add(54);
            m_alignmentPatternCenters.Add(80);
            m_alignmentPatternCenters.Add(106);
            break;
        case 25:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(32);
            m_alignmentPatternCenters.Add(58);
            m_alignmentPatternCenters.Add(84);
            m_alignmentPatternCenters.Add(110);
            break;
        case 26:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            m_alignmentPatternCenters.Add(58);
            m_alignmentPatternCenters.Add(86);
            m_alignmentPatternCenters.Add(114);
            break;
        case 27:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(34);
            m_alignmentPatternCenters.Add(62);
            m_alignmentPatternCenters.Add(90);
            m_alignmentPatternCenters.Add(118);
            break;
        case 28:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(26);
            m_alignmentPatternCenters.Add(50);
            m_alignmentPatternCenters.Add(74);
            m_alignmentPatternCenters.Add(98);
            m_alignmentPatternCenters.Add(122);
            break;
        case 29:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            m_alignmentPatternCenters.Add(54);
            m_alignmentPatternCenters.Add(78);
            m_alignmentPatternCenters.Add(102);
            m_alignmentPatternCenters.Add(126);
            break;
        case 30:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(26);
            m_alignmentPatternCenters.Add(52);
            m_alignmentPatternCenters.Add(78);
            m_alignmentPatternCenters.Add(104);
            m_alignmentPatternCenters.Add(130);
            break;
        case 31:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            m_alignmentPatternCenters.Add(56);
            m_alignmentPatternCenters.Add(82);
            m_alignmentPatternCenters.Add(108);
            m_alignmentPatternCenters.Add(134);
            break;
        case 32:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(34);
            m_alignmentPatternCenters.Add(60);
            m_alignmentPatternCenters.Add(86);
            m_alignmentPatternCenters.Add(112);
            m_alignmentPatternCenters.Add(138);
            break;
        case 33:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            m_alignmentPatternCenters.Add(58);
            m_alignmentPatternCenters.Add(86);
            m_alignmentPatternCenters.Add(114);
            m_alignmentPatternCenters.Add(142);
            break;
        case 34:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(34);
            m_alignmentPatternCenters.Add(62);
            m_alignmentPatternCenters.Add(90);
            m_alignmentPatternCenters.Add(118);
            m_alignmentPatternCenters.Add(146);
            break;
        case 35:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            m_alignmentPatternCenters.Add(54);
            m_alignmentPatternCenters.Add(78);
            m_alignmentPatternCenters.Add(102);
            m_alignmentPatternCenters.Add(126);
            m_alignmentPatternCenters.Add(150);
            break;
        case 36:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(24);
            m_alignmentPatternCenters.Add(50);
            m_alignmentPatternCenters.Add(76);
            m_alignmentPatternCenters.Add(102);
            m_alignmentPatternCenters.Add(128);
            m_alignmentPatternCenters.Add(154);
            break;
        case 37:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(28);
            m_alignmentPatternCenters.Add(54);
            m_alignmentPatternCenters.Add(80);
            m_alignmentPatternCenters.Add(106);
            m_alignmentPatternCenters.Add(132);
            m_alignmentPatternCenters.Add(158);
            break;
        case 38:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(32);
            m_alignmentPatternCenters.Add(58);
            m_alignmentPatternCenters.Add(84);
            m_alignmentPatternCenters.Add(110);
            m_alignmentPatternCenters.Add(136);
            m_alignmentPatternCenters.Add(162);
            break;
        case 39:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(26);
            m_alignmentPatternCenters.Add(54);
            m_alignmentPatternCenters.Add(82);
            m_alignmentPatternCenters.Add(110);
            m_alignmentPatternCenters.Add(138);
            m_alignmentPatternCenters.Add(166);
            break;
        case 40:
            m_alignmentPatternCenters.Add(6);
            m_alignmentPatternCenters.Add(30);
            m_alignmentPatternCenters.Add(58);
            m_alignmentPatternCenters.Add(86);
            m_alignmentPatternCenters.Add(114);
            m_alignmentPatternCenters.Add(142);
            m_alignmentPatternCenters.Add(170);
            break;
    }
}
CBC_QRCoderVersion::~CBC_QRCoderVersion()
{
    if(m_ecBlocks.GetSize() != 0) {
        FX_INT32 itBeg = 0;
        FX_INT32 itEnd = m_ecBlocks.GetSize();
        while(itBeg != itEnd) {
            delete ( (CBC_QRCoderECBlocks*)(m_ecBlocks[itBeg]) );
            itBeg++;
        }
    }
}
FX_INT32 CBC_QRCoderVersion::GetVersionNumber()
{
    return m_versionNumber;
}
CFX_Int32Array* CBC_QRCoderVersion::GetAlignmentPatternCenters()
{
    return &m_alignmentPatternCenters;
}
FX_INT32 CBC_QRCoderVersion::GetTotalCodeWords()
{
    return m_totalCodeWords;
}
FX_INT32 CBC_QRCoderVersion::GetDimensionForVersion()
{
    return 17 + 4 * m_versionNumber;
}
CBC_QRCoderECBlocks* CBC_QRCoderVersion::GetECBlocksForLevel(CBC_QRCoderErrorCorrectionLevel *ecLevel)
{
    return (CBC_QRCoderECBlocks*)m_ecBlocks[ecLevel->Ordinal()];
}
CBC_QRCoderVersion* CBC_QRCoderVersion::GetProvisionalVersionForDimension(FX_INT32 dimension, FX_INT32 &e)
{
    if((dimension % 4) != 1) {
        e = BCExceptionRead;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    CBC_QRCoderVersion* qcv = GetVersionForNumber((dimension - 17) >> 2, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return qcv;
}
CBC_QRCoderVersion* CBC_QRCoderVersion::DecodeVersionInformation(FX_INT32 versionBits, FX_INT32 &e)
{
    FX_INT32 bestDifference = FXSYS_IntMax;
    FX_INT32 bestVersion = 0;
    for (FX_INT32 i = 0; i < 34; i++) {
        FX_INT32 targetVersion = VERSION_DECODE_INFO[i];
        if(targetVersion == versionBits) {
            CBC_QRCoderVersion* qcv = GetVersionForNumber(i + 7, e);
            BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
            return qcv;
        }
        FX_INT32 bitsDifference = CBC_QRCoderFormatInformation::NumBitsDiffering(versionBits, targetVersion);
        if(bitsDifference < bestDifference) {
            bestVersion = i + 7;
            bestDifference = bitsDifference;
        }
    }
    if(bestDifference <= 3) {
        CBC_QRCoderVersion* qcv = GetVersionForNumber(bestVersion, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        return qcv;
    }
    return NULL;
}
CBC_CommonBitMatrix* CBC_QRCoderVersion::BuildFunctionPattern(FX_INT32 &e)
{
    FX_INT32 dimension = GetDimensionForVersion();
    CBC_CommonBitMatrix* bitMatrix = FX_NEW CBC_CommonBitMatrix();
    bitMatrix->Init(dimension);
    bitMatrix->SetRegion(0, 0 , 9, 9, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    bitMatrix->SetRegion(dimension - 8, 0, 8, 9, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    bitMatrix->SetRegion(0, dimension - 8, 9, 8, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    FX_INT32 max = m_alignmentPatternCenters.GetSize();
    for (FX_INT32 x = 0; x < max; x++) {
        FX_INT32 i = m_alignmentPatternCenters[x] - 2;
        for (FX_INT32 y = 0; y < max; y++) {
            if ((x == 0 && (y == 0 || y == max - 1)) || (x == max - 1 && y == 0)) {
                continue;
            }
            bitMatrix->SetRegion(m_alignmentPatternCenters[y] - 2, i, 5, 5, e);
            BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        }
    }
    bitMatrix->SetRegion(6, 9, 1, dimension - 17, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    bitMatrix->SetRegion(9, 6, dimension - 17, 1, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    if (m_versionNumber > 6) {
        bitMatrix->SetRegion(dimension - 11, 0, 3, 6, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        bitMatrix->SetRegion(0, dimension - 11, 6, 3, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    return bitMatrix;
}
CBC_QRCoderVersion* CBC_QRCoderVersion::GetVersionForNumber(FX_INT32 versionNumber, FX_INT32 &e)
{
    if(VERSION->GetSize() == 0) {
        VERSION->Add(FX_NEW CBC_QRCoderVersion(1,
                                               FX_NEW CBC_QRCoderECBlocks(7, FX_NEW CBC_QRCoderECB(1, 19)),
                                               FX_NEW CBC_QRCoderECBlocks(10, FX_NEW CBC_QRCoderECB(1, 16)),
                                               FX_NEW CBC_QRCoderECBlocks(13, FX_NEW CBC_QRCoderECB(1, 13)),
                                               FX_NEW CBC_QRCoderECBlocks(17, FX_NEW CBC_QRCoderECB(1, 9))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(2,
                                               FX_NEW CBC_QRCoderECBlocks(10, FX_NEW CBC_QRCoderECB(1, 34)),
                                               FX_NEW CBC_QRCoderECBlocks(16, FX_NEW CBC_QRCoderECB(1, 28)),
                                               FX_NEW CBC_QRCoderECBlocks(22, FX_NEW CBC_QRCoderECB(1, 22)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(1, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(3,
                                               FX_NEW CBC_QRCoderECBlocks(15, FX_NEW CBC_QRCoderECB(1, 55)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(1, 44)),
                                               FX_NEW CBC_QRCoderECBlocks(18, FX_NEW CBC_QRCoderECB(2, 17)),
                                               FX_NEW CBC_QRCoderECBlocks(22, FX_NEW CBC_QRCoderECB(2, 13))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(4,
                                               FX_NEW CBC_QRCoderECBlocks(20, FX_NEW CBC_QRCoderECB(1, 80)),
                                               FX_NEW CBC_QRCoderECBlocks(18, FX_NEW CBC_QRCoderECB(2, 32)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(2, 24)),
                                               FX_NEW CBC_QRCoderECBlocks(16, FX_NEW CBC_QRCoderECB(4, 9))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(5,
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(1, 108)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(2, 43)),
                                               FX_NEW CBC_QRCoderECBlocks(18, FX_NEW CBC_QRCoderECB(2, 15),
                                                       FX_NEW CBC_QRCoderECB(2, 16)),
                                               FX_NEW CBC_QRCoderECBlocks(22, FX_NEW CBC_QRCoderECB(2, 11),
                                                       FX_NEW CBC_QRCoderECB(2, 12))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(6,
                                               FX_NEW CBC_QRCoderECBlocks(18, FX_NEW CBC_QRCoderECB(2, 68)),
                                               FX_NEW CBC_QRCoderECBlocks(16, FX_NEW CBC_QRCoderECB(4, 27)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(4, 19)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(4, 15))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(7,
                                               FX_NEW CBC_QRCoderECBlocks(20, FX_NEW CBC_QRCoderECB(2, 78)),
                                               FX_NEW CBC_QRCoderECBlocks(18, FX_NEW CBC_QRCoderECB(4, 31)),
                                               FX_NEW CBC_QRCoderECBlocks(18, FX_NEW CBC_QRCoderECB(2, 14),
                                                       FX_NEW CBC_QRCoderECB(4, 15)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(4, 13),
                                                       FX_NEW CBC_QRCoderECB(1, 14))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(8,
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(2, 97)),
                                               FX_NEW CBC_QRCoderECBlocks(22, FX_NEW CBC_QRCoderECB(2, 38),
                                                       FX_NEW CBC_QRCoderECB(2, 39)),
                                               FX_NEW CBC_QRCoderECBlocks(22, FX_NEW CBC_QRCoderECB(4, 18),
                                                       FX_NEW CBC_QRCoderECB(2, 19)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(4, 14),
                                                       FX_NEW CBC_QRCoderECB(2, 15))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(9,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(2, 116)),
                                               FX_NEW CBC_QRCoderECBlocks(22, FX_NEW CBC_QRCoderECB(3, 36),
                                                       FX_NEW CBC_QRCoderECB(2, 37)),
                                               FX_NEW CBC_QRCoderECBlocks(20, FX_NEW CBC_QRCoderECB(4, 16),
                                                       FX_NEW CBC_QRCoderECB(4, 17)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(4, 12),
                                                       FX_NEW CBC_QRCoderECB(4, 13))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(10,
                                               FX_NEW CBC_QRCoderECBlocks(18, FX_NEW CBC_QRCoderECB(2, 68),
                                                       FX_NEW CBC_QRCoderECB(2, 69)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(4, 43),
                                                       FX_NEW CBC_QRCoderECB(1, 44)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(6, 19),
                                                       FX_NEW CBC_QRCoderECB(2, 20)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(6, 15),
                                                       FX_NEW CBC_QRCoderECB(2, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(11,
                                               FX_NEW CBC_QRCoderECBlocks(20, FX_NEW CBC_QRCoderECB(4, 81)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(1, 50),
                                                       FX_NEW CBC_QRCoderECB(4, 51)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(4, 22),
                                                       FX_NEW CBC_QRCoderECB(4, 23)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(3, 12),
                                                       FX_NEW CBC_QRCoderECB(8, 13))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(12,
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(2, 92),
                                                       FX_NEW CBC_QRCoderECB(2, 93)),
                                               FX_NEW CBC_QRCoderECBlocks(22, FX_NEW CBC_QRCoderECB(6, 36),
                                                       FX_NEW CBC_QRCoderECB(2, 37)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(4, 20),
                                                       FX_NEW CBC_QRCoderECB(6, 21)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(7, 14),
                                                       FX_NEW CBC_QRCoderECB(4, 15))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(13,
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(4, 107)),
                                               FX_NEW CBC_QRCoderECBlocks(22, FX_NEW CBC_QRCoderECB(8, 37),
                                                       FX_NEW CBC_QRCoderECB(1, 38)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(8, 20),
                                                       FX_NEW CBC_QRCoderECB(4, 21)),
                                               FX_NEW CBC_QRCoderECBlocks(22, FX_NEW CBC_QRCoderECB(12, 11),
                                                       FX_NEW CBC_QRCoderECB(4, 12))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(14,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(3, 115),
                                                       FX_NEW CBC_QRCoderECB(1, 116)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(4, 40),
                                                       FX_NEW CBC_QRCoderECB(5, 41)),
                                               FX_NEW CBC_QRCoderECBlocks(20, FX_NEW CBC_QRCoderECB(11, 16),
                                                       FX_NEW CBC_QRCoderECB(5, 17)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(11, 12),
                                                       FX_NEW CBC_QRCoderECB(5, 13))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(15,
                                               FX_NEW CBC_QRCoderECBlocks(22, FX_NEW CBC_QRCoderECB(5, 87),
                                                       FX_NEW CBC_QRCoderECB(1, 88)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(5, 41),
                                                       FX_NEW CBC_QRCoderECB(5, 42)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(5, 24),
                                                       FX_NEW CBC_QRCoderECB(7, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(11, 12),
                                                       FX_NEW CBC_QRCoderECB(7, 13))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(16,
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(5, 98),
                                                       FX_NEW CBC_QRCoderECB(1, 99)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(7, 45),
                                                       FX_NEW CBC_QRCoderECB(3, 46)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(15, 19),
                                                       FX_NEW CBC_QRCoderECB(2, 20)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(3, 15),
                                                       FX_NEW CBC_QRCoderECB(13, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(17,
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(1, 107),
                                                       FX_NEW CBC_QRCoderECB(5, 108)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(10, 46),
                                                       FX_NEW CBC_QRCoderECB(1, 47)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(1, 22),
                                                       FX_NEW CBC_QRCoderECB(15, 23)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(2, 14),
                                                       FX_NEW CBC_QRCoderECB(17, 15))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(18,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(5, 120),
                                                       FX_NEW CBC_QRCoderECB(1, 121)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(9, 43),
                                                       FX_NEW CBC_QRCoderECB(4, 44)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(17, 22),
                                                       FX_NEW CBC_QRCoderECB(1, 23)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(2, 14),
                                                       FX_NEW CBC_QRCoderECB(19, 15))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(19,
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(3, 113),
                                                       FX_NEW CBC_QRCoderECB(4, 114)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(3, 44),
                                                       FX_NEW CBC_QRCoderECB(11, 45)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(17, 21),
                                                       FX_NEW CBC_QRCoderECB(4, 22)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(9, 13),
                                                       FX_NEW CBC_QRCoderECB(16, 14))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(20,
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(3, 107),
                                                       FX_NEW CBC_QRCoderECB(5, 108)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(3, 41),
                                                       FX_NEW CBC_QRCoderECB(13, 42)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(15, 24),
                                                       FX_NEW CBC_QRCoderECB(5, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(15, 15),
                                                       FX_NEW CBC_QRCoderECB(10, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(21,
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(4, 116),
                                                       FX_NEW CBC_QRCoderECB(4, 117)),
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(17, 42)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(17, 22),
                                                       FX_NEW CBC_QRCoderECB(6, 23)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(19, 16),
                                                       FX_NEW CBC_QRCoderECB(6, 17))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(22,
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(2, 111),
                                                       FX_NEW CBC_QRCoderECB(7, 112)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(17, 46)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(7, 24),
                                                       FX_NEW CBC_QRCoderECB(16, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(24, FX_NEW CBC_QRCoderECB(34, 13))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(23,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(4, 121),
                                                       FX_NEW CBC_QRCoderECB(5, 122)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(4, 47),
                                                       FX_NEW CBC_QRCoderECB(14, 48)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(11, 24),
                                                       FX_NEW CBC_QRCoderECB(14, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(16, 15),
                                                       FX_NEW CBC_QRCoderECB(14, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(24,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(6, 117),
                                                       FX_NEW CBC_QRCoderECB(4, 118)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(6, 45),
                                                       FX_NEW CBC_QRCoderECB(14, 46)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(11, 24),
                                                       FX_NEW CBC_QRCoderECB(16, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(30, 16),
                                                       FX_NEW CBC_QRCoderECB(2, 17))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(25,
                                               FX_NEW CBC_QRCoderECBlocks(26, FX_NEW CBC_QRCoderECB(8, 106),
                                                       FX_NEW CBC_QRCoderECB(4, 107)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(8, 47),
                                                       FX_NEW CBC_QRCoderECB(13, 48)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(7, 24),
                                                       FX_NEW CBC_QRCoderECB(22, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(22, 15),
                                                       FX_NEW CBC_QRCoderECB(13, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(26,
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(10, 114),
                                                       FX_NEW CBC_QRCoderECB(2, 115)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(19, 46),
                                                       FX_NEW CBC_QRCoderECB(4, 47)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(28, 22),
                                                       FX_NEW CBC_QRCoderECB(6, 23)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(33, 16),
                                                       FX_NEW CBC_QRCoderECB(4, 17))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(27,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(8, 122),
                                                       FX_NEW CBC_QRCoderECB(4, 123)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(22, 45),
                                                       FX_NEW CBC_QRCoderECB(3, 46)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(8, 23),
                                                       FX_NEW CBC_QRCoderECB(26, 24)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(12, 15),
                                                       FX_NEW CBC_QRCoderECB(28, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(28,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(3, 117),
                                                       FX_NEW CBC_QRCoderECB(10, 118)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(3, 45),
                                                       FX_NEW CBC_QRCoderECB(23, 46)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(4, 24),
                                                       FX_NEW CBC_QRCoderECB(31, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(11, 15),
                                                       FX_NEW CBC_QRCoderECB(31, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(29,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(7, 116),
                                                       FX_NEW CBC_QRCoderECB(7, 117)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(21, 45),
                                                       FX_NEW CBC_QRCoderECB(7, 46)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(1, 23),
                                                       FX_NEW CBC_QRCoderECB(37, 24)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(19, 15),
                                                       FX_NEW CBC_QRCoderECB(26, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(30,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(5, 115),
                                                       FX_NEW CBC_QRCoderECB(10, 116)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(19, 47),
                                                       FX_NEW CBC_QRCoderECB(10, 48)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(15, 24),
                                                       FX_NEW CBC_QRCoderECB(25, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(23, 15),
                                                       FX_NEW CBC_QRCoderECB(25, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(31,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(13, 115),
                                                       FX_NEW CBC_QRCoderECB(3, 116)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(2, 46),
                                                       FX_NEW CBC_QRCoderECB(29, 47)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(42, 24),
                                                       FX_NEW CBC_QRCoderECB(1, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(23, 15),
                                                       FX_NEW CBC_QRCoderECB(28, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(32,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(17, 115)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(10, 46),
                                                       FX_NEW CBC_QRCoderECB(23, 47)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(10, 24),
                                                       FX_NEW CBC_QRCoderECB(35, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(19, 15),
                                                       FX_NEW CBC_QRCoderECB(35, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(33,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(17, 115),
                                                       FX_NEW CBC_QRCoderECB(1, 116)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(14, 46),
                                                       FX_NEW CBC_QRCoderECB(21, 47)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(29, 24),
                                                       FX_NEW CBC_QRCoderECB(19, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(11, 15),
                                                       FX_NEW CBC_QRCoderECB(46, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(34,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(13, 115),
                                                       FX_NEW CBC_QRCoderECB(6, 116)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(14, 46),
                                                       FX_NEW CBC_QRCoderECB(23, 47)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(44, 24),
                                                       FX_NEW CBC_QRCoderECB(7, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(59, 16),
                                                       FX_NEW CBC_QRCoderECB(1, 17))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(35,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(12, 121),
                                                       FX_NEW CBC_QRCoderECB(7, 122)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(12, 47),
                                                       FX_NEW CBC_QRCoderECB(26, 48)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(39, 24),
                                                       FX_NEW CBC_QRCoderECB(14, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(22, 15),
                                                       FX_NEW CBC_QRCoderECB(41, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(36,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(6, 121),
                                                       FX_NEW CBC_QRCoderECB(14, 122)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(6, 47),
                                                       FX_NEW CBC_QRCoderECB(34, 48)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(46, 24),
                                                       FX_NEW CBC_QRCoderECB(10, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(2, 15),
                                                       FX_NEW CBC_QRCoderECB(64, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(37,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(17, 122),
                                                       FX_NEW CBC_QRCoderECB(4, 123)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(29, 46),
                                                       FX_NEW CBC_QRCoderECB(14, 47)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(49, 24),
                                                       FX_NEW CBC_QRCoderECB(10, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(24, 15),
                                                       FX_NEW CBC_QRCoderECB(46, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(38,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(4, 122),
                                                       FX_NEW CBC_QRCoderECB(18, 123)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(13, 46),
                                                       FX_NEW CBC_QRCoderECB(32, 47)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(48, 24),
                                                       FX_NEW CBC_QRCoderECB(14, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(42, 15),
                                                       FX_NEW CBC_QRCoderECB(32, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(39,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(20, 117),
                                                       FX_NEW CBC_QRCoderECB(4, 118)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(40, 47),
                                                       FX_NEW CBC_QRCoderECB(7, 48)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(43, 24),
                                                       FX_NEW CBC_QRCoderECB(22, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(10, 15),
                                                       FX_NEW CBC_QRCoderECB(67, 16))));
        VERSION->Add(FX_NEW CBC_QRCoderVersion(40,
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(19, 118),
                                                       FX_NEW CBC_QRCoderECB(6, 119)),
                                               FX_NEW CBC_QRCoderECBlocks(28, FX_NEW CBC_QRCoderECB(18, 47),
                                                       FX_NEW CBC_QRCoderECB(31, 48)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(34, 24),
                                                       FX_NEW CBC_QRCoderECB(34, 25)),
                                               FX_NEW CBC_QRCoderECBlocks(30, FX_NEW CBC_QRCoderECB(20, 15),
                                                       FX_NEW CBC_QRCoderECB(61, 16))));
    }
    if(versionNumber < 1 || versionNumber > 40) {
        e = BCExceptionIllegalArgument;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    }
    return  (CBC_QRCoderVersion*)(*VERSION)[versionNumber - 1];
}
void CBC_QRCoderVersion::Destroy()
{
    FX_INT32 i;
    for(i = 0; i < VERSION->GetSize(); i++) {
        delete ( (CBC_QRCoderVersion*)(*VERSION)[i] );
    }
}
