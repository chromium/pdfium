// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_SYMBOLINFO_H_
#define _BC_SYMBOLINFO_H_
class CBC_SymbolShapeHint;
class CBC_Dimension;
class CBC_SymbolInfo;
class CBC_SymbolInfo : public CBC_SymbolShapeHint
{
public:
    CBC_SymbolInfo(FX_BOOL rectangular, FX_INT32 dataCapacity, FX_INT32 errorCodewords,
                   FX_INT32 matrixWidth, FX_INT32 matrixHeight, FX_INT32 dataRegions);
    virtual ~CBC_SymbolInfo();
    static void Initialize();
    static void Finalize();
    static void overrideSymbolSet(CBC_SymbolInfo* override);
    static CBC_SymbolInfo* lookup(FX_INT32 dataCodewords, FX_INT32 &e);
    static CBC_SymbolInfo* lookup(FX_INT32 dataCodewords, SymbolShapeHint shape, FX_INT32 &e);
    static CBC_SymbolInfo* lookup(FX_INT32 dataCodewords, FX_BOOL allowRectangular, FX_BOOL fail, FX_INT32 &e);
    static CBC_SymbolInfo* lookup(FX_INT32 dataCodewords, SymbolShapeHint shape, FX_BOOL fail, FX_INT32 &e);
    static CBC_SymbolInfo* lookup(FX_INT32 dataCodewords, SymbolShapeHint shape, CBC_Dimension* minSize, CBC_Dimension* maxSize, FX_BOOL fail, FX_INT32 &e);
    FX_INT32 getHorizontalDataRegions(FX_INT32 &e);
    FX_INT32 getVerticalDataRegions(FX_INT32 &e);
    FX_INT32 getSymbolDataWidth(FX_INT32 &e);
    FX_INT32 getSymbolDataHeight(FX_INT32 &e);
    FX_INT32 getSymbolWidth(FX_INT32 &e);
    FX_INT32 getSymbolHeight(FX_INT32 &e);
    FX_INT32 getCodewordCount();
    FX_INT32 getInterleavedBlockCount();
    FX_INT32 getDataLengthForInterleavedBlock(FX_INT32 index);
    FX_INT32 getErrorLengthForInterleavedBlock(FX_INT32 index);
    CFX_WideString toString(FX_INT32 &e);
public:
    FX_INT32 m_dataCapacity;
    FX_INT32 m_errorCodewords;
    FX_INT32 m_matrixWidth;
    FX_INT32 m_matrixHeight;
    FX_INT32 m_rsBlockData;
    FX_INT32 m_rsBlockError;
    static CBC_SymbolInfo* m_PROD_SYMBOLS[30];
private:
    static CBC_SymbolInfo* m_symbols[30];
    FX_BOOL m_rectangular;
    FX_INT32 m_dataRegions;
private:
    CBC_SymbolInfo(FX_BOOL rectangular, FX_INT32 dataCapacity, FX_INT32 errorCodewords, FX_INT32 matrixWidth, FX_INT32 matrixHeight, FX_INT32 dataRegions,
                   FX_INT32 rsBlockData, FX_INT32 rsBlockError);
};
#endif
