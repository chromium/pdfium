// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_HUFFMAN_TABLE_H_
#define _JBIG2_HUFFMAN_TABLE_H_
#include "JBig2_Module.h"
#include "JBig2_HuffmanTable_Standard.h"
#include "JBig2_BitStream.h"
class CJBig2_HuffmanTable : public CJBig2_Object
{
public:

    CJBig2_HuffmanTable(const JBig2TableLine *pTable, int nLines, FX_BOOL bHTOOB);

    CJBig2_HuffmanTable(CJBig2_BitStream *pStream);

    ~CJBig2_HuffmanTable();

    void init();

    int parseFromStandardTable(const JBig2TableLine *pTable, int nLines, FX_BOOL bHTOOB);

    int parseFromCodedBuffer(CJBig2_BitStream *pStream);

    FX_BOOL isOK()
    {
        return m_bOK;
    }
private:
    FX_BOOL HTOOB;
    int NTEMP;
    int *CODES;
    int *PREFLEN;
    int *RANGELEN;
    int *RANGELOW;
    FX_BOOL m_bOK;
    friend class CJBig2_HuffmanDecoder;
};
#endif
