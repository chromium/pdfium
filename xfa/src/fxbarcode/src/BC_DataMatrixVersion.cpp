// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_DataMatrixVersion.h"
CFX_PtrArray* CBC_DataMatrixVersion::VERSIONS = NULL;
void CBC_DataMatrixVersion::Initialize()
{
    VERSIONS = FX_NEW CFX_PtrArray();
}
void CBC_DataMatrixVersion::Finalize()
{
    for (FX_INT32 i = 0; i < VERSIONS->GetSize(); i++) {
        delete ( (CBC_DataMatrixVersion*)(VERSIONS->GetAt(i)) );
    }
    VERSIONS->RemoveAll();
    delete VERSIONS;
}
CBC_DataMatrixVersion::CBC_DataMatrixVersion(FX_INT32 versionNumber,
        FX_INT32 symbolSizeRows,
        FX_INT32 symbolSizeColumns,
        FX_INT32 dataRegionSizeRows,
        FX_INT32 dataRegionSizeColumns,
        ECBlocks *ecBlocks)
{
    m_versionNumber = versionNumber;
    m_symbolSizeRows = symbolSizeRows;
    m_symbolSizeColumns = symbolSizeColumns;
    m_dataRegionSizeRows = dataRegionSizeRows;
    m_dataRegionSizeColumns = dataRegionSizeColumns;
    m_ecBlocks = ecBlocks;
    FX_INT32 total = 0;
    FX_INT32 ecCodewords = ecBlocks->GetECCodewords();
    const CFX_PtrArray &ecbArray = ecBlocks->GetECBlocks();
    for (FX_INT32 i = 0; i < ecbArray.GetSize(); i++) {
        total += ((ECB*)ecbArray[i])->GetCount() * (((ECB*)ecbArray[i])->GetDataCodewords() + ecCodewords);
    }
    m_totalCodewords = total;
}
CBC_DataMatrixVersion::~CBC_DataMatrixVersion()
{
    if(m_ecBlocks != NULL) {
        delete m_ecBlocks;
    }
    m_ecBlocks = NULL;
}
FX_INT32 CBC_DataMatrixVersion::GetVersionNumber()
{
    return m_versionNumber;
}
FX_INT32 CBC_DataMatrixVersion::GetSymbolSizeRows()
{
    return m_symbolSizeRows;
}
FX_INT32 CBC_DataMatrixVersion::GetSymbolSizeColumns()
{
    return m_symbolSizeColumns;
}
FX_INT32 CBC_DataMatrixVersion::GetDataRegionSizeRows()
{
    return m_dataRegionSizeRows;
}
FX_INT32 CBC_DataMatrixVersion::GetDataRegionSizeColumns()
{
    return m_dataRegionSizeColumns;
}
FX_INT32 CBC_DataMatrixVersion::GetTotalCodewords()
{
    return m_totalCodewords;
}
ECBlocks *CBC_DataMatrixVersion::GetECBlocks()
{
    return m_ecBlocks;
}
void CBC_DataMatrixVersion::ReleaseAll()
{
    for (FX_INT32 i = 0; i < VERSIONS->GetSize(); i++) {
        delete (CBC_DataMatrixVersion*)VERSIONS->GetAt(i);
    }
    VERSIONS->RemoveAll();
}
CBC_DataMatrixVersion *CBC_DataMatrixVersion::GetVersionForDimensions(FX_INT32 numRows, FX_INT32 numColumns, FX_INT32 &e)
{
    if ((numRows & 0x01) != 0 || (numColumns & 0x01) != 0) {
        e = BCExceptionNotFound;
        return NULL;
    }
    if(VERSIONS->GetSize() == 0) {
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(1, 10, 10, 8, 8, FX_NEW ECBlocks(5, FX_NEW ECB(1, 3))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(2, 12, 12, 10, 10, FX_NEW ECBlocks(7, FX_NEW ECB(1, 5))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(3, 14, 14, 12, 12, FX_NEW ECBlocks(10, FX_NEW ECB(1, 8))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(4, 16, 16, 14, 14, FX_NEW ECBlocks(12, FX_NEW ECB(1, 12))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(5, 18, 18, 16, 16, FX_NEW ECBlocks(14, FX_NEW ECB(1, 18))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(6, 20, 20, 18, 18, FX_NEW ECBlocks(18, FX_NEW ECB(1, 22))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(7, 22, 22, 20, 20, FX_NEW ECBlocks(20, FX_NEW ECB(1, 30))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(8, 24, 24, 22, 22, FX_NEW ECBlocks(24, FX_NEW ECB(1, 36))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(9, 26, 26, 24, 24, FX_NEW ECBlocks(28, FX_NEW ECB(1, 44))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(10, 32, 32, 14, 14, FX_NEW ECBlocks(36, FX_NEW ECB(1, 62))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(11, 36, 36, 16, 16, FX_NEW ECBlocks(42, FX_NEW ECB(1, 86))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(12, 40, 40, 18, 18, FX_NEW ECBlocks(48, FX_NEW ECB(1, 114))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(13, 44, 44, 20, 20, FX_NEW ECBlocks(56, FX_NEW ECB(1, 144))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(14, 48, 48, 22, 22, FX_NEW ECBlocks(68, FX_NEW ECB(1, 174))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(15, 52, 52, 24, 24, FX_NEW ECBlocks(42, FX_NEW ECB(2, 102))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(16, 64, 64, 14, 14, FX_NEW ECBlocks(56, FX_NEW ECB(2, 140))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(17, 72, 72, 16, 16, FX_NEW ECBlocks(36, FX_NEW ECB(4, 92))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(18, 80, 80, 18, 18, FX_NEW ECBlocks(48, FX_NEW ECB(4, 114))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(19, 88, 88, 20, 20, FX_NEW ECBlocks(56, FX_NEW ECB(4, 144))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(20, 96, 96, 22, 22, FX_NEW ECBlocks(68, FX_NEW ECB(4, 174))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(21, 104, 104, 24, 24, FX_NEW ECBlocks(56, FX_NEW ECB(6, 136))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(22, 120, 120, 18, 18, FX_NEW ECBlocks(68, FX_NEW ECB(6, 175))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(23, 132, 132, 20, 20, FX_NEW ECBlocks(62, FX_NEW ECB(8, 163))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(24, 144, 144, 22, 22, FX_NEW ECBlocks(62, FX_NEW ECB(8, 156), FX_NEW ECB(2, 155))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(25, 8, 18, 6, 16, FX_NEW ECBlocks(7, FX_NEW ECB(1, 5))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(26, 8, 32, 6, 14, FX_NEW ECBlocks(11, FX_NEW ECB(1, 10))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(27, 12, 26, 10, 24, FX_NEW ECBlocks(14, FX_NEW ECB(1, 16))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(28, 12, 36, 10, 16, FX_NEW ECBlocks(18, FX_NEW ECB(1, 22))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(29, 16, 36, 14, 16, FX_NEW ECBlocks(24, FX_NEW ECB(1, 32))));
        VERSIONS->Add(FX_NEW CBC_DataMatrixVersion(30, 16, 48, 14, 22, FX_NEW ECBlocks(28, FX_NEW ECB(1, 49))));
    }
    FX_INT32 numVersions = VERSIONS->GetSize();
    for (FX_INT32 i = 0; i < numVersions; ++i) {
        if (((CBC_DataMatrixVersion*)((*VERSIONS)[i]))->m_symbolSizeRows == numRows && ((CBC_DataMatrixVersion*)((*VERSIONS)[i]))->m_symbolSizeColumns == numColumns) {
            return (CBC_DataMatrixVersion*)(*VERSIONS)[i];
        }
    }
    e = BCExceptionNotFound;
    return NULL;
}
