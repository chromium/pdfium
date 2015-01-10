// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DATAMATRIXVERSION_H_
#define _BC_DATAMATRIXVERSION_H_
class ECB;
class ECBlocks;
class CBC_DataMatrixVersion;
class ECB : public CFX_Object
{
public:
    ECB(FX_INT32 count, FX_INT32 dataCodewords)
    {
        m_count = count;
        m_dataCodewords = dataCodewords;
    }

    FX_INT32 GetCount()
    {
        return m_count;
    }

    FX_INT32 GetDataCodewords()
    {
        return m_dataCodewords;
    }
private:
    FX_INT32 m_count;
    FX_INT32 m_dataCodewords;
};
class ECBlocks : public CFX_Object
{
public:
    ECBlocks(FX_INT32 ecCodewords, ECB *ecBlocks)
    {
        m_ecCodewords = ecCodewords;
        m_ecBlocks.Add(ecBlocks);
    }

    ECBlocks(FX_INT32 ecCodewords, ECB *ecBlocks1, ECB *ecBlocks2)
    {
        m_ecCodewords = ecCodewords;
        m_ecBlocks.Add(ecBlocks1);
        m_ecBlocks.Add(ecBlocks2);
    }
    ~ECBlocks()
    {
        for(FX_INT32 i = 0; i < m_ecBlocks.GetSize(); i++) {
            delete (ECB*)m_ecBlocks[i];
        }
        m_ecBlocks.RemoveAll();
    }

    FX_INT32 GetECCodewords()
    {
        return m_ecCodewords;
    }

    const CFX_PtrArray &GetECBlocks()
    {
        return m_ecBlocks;
    }
private:
    FX_INT32 m_ecCodewords;
    CFX_PtrArray m_ecBlocks;
};
class CBC_DataMatrixVersion  : public CFX_Object
{
public:
    CBC_DataMatrixVersion(FX_INT32 versionNumber,
                          FX_INT32 symbolSizeRows,
                          FX_INT32 symbolSizeColumns,
                          FX_INT32 dataRegionSizeRows,
                          FX_INT32 dataRegionSizeColumns,
                          ECBlocks *ecBlocks);
    virtual ~CBC_DataMatrixVersion();
    static void Initialize();
    static void Finalize();
    FX_INT32 GetVersionNumber();
    FX_INT32 GetSymbolSizeRows();
    FX_INT32 GetSymbolSizeColumns();
    FX_INT32 GetDataRegionSizeRows();
    FX_INT32 GetDataRegionSizeColumns();
    FX_INT32 GetTotalCodewords();
    ECBlocks *GetECBlocks();
    static CBC_DataMatrixVersion *GetVersionForDimensions(FX_INT32 numRows, FX_INT32 numColumns, FX_INT32 &e);
    static void ReleaseAll();
private:
    FX_INT32 m_versionNumber;
    FX_INT32 m_symbolSizeRows;
    FX_INT32 m_symbolSizeColumns;
    FX_INT32 m_dataRegionSizeRows;
    FX_INT32 m_dataRegionSizeColumns;
    ECBlocks *m_ecBlocks;
    FX_INT32 m_totalCodewords;
    static CFX_PtrArray* VERSIONS;
};
#endif
