// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417_H_
#define _BC_PDF417_H_
class CBC_Compaction;
class CBC_BarcodeRow;
class CBC_BarcodeMatrix;
class CBC_PDF417;
class CBC_PDF417 : public CFX_Object
{
public:
    CBC_PDF417();
    CBC_PDF417(FX_BOOL compact);
    virtual ~CBC_PDF417();
    CBC_BarcodeMatrix* getBarcodeMatrix();
    void generateBarcodeLogic(CFX_WideString msg, FX_INT32 errorCorrectionLevel, FX_INT32 &e);
    void setDimensions(FX_INT32 maxCols, FX_INT32 minCols, FX_INT32 maxRows, FX_INT32 minRows);
    void setCompaction(Compaction compaction);
    void setCompact(FX_BOOL compact);
private:
    static FX_INT32 START_PATTERN;
    static FX_INT32 STOP_PATTERN;
    static FX_INT32 CODEWORD_TABLE[][1000];
    static FX_FLOAT PREFERRED_RATIO;
    static FX_FLOAT DEFAULT_MODULE_WIDTH;
    static FX_FLOAT HEIGHT;
    CBC_BarcodeMatrix* m_barcodeMatrix;
    FX_BOOL m_compact;
    Compaction m_compaction;
    FX_INT32 m_minCols;
    FX_INT32 m_maxCols;
    FX_INT32 m_maxRows;
    FX_INT32 m_minRows;
private:
    static FX_INT32 calculateNumberOfRows(FX_INT32 m, FX_INT32 k, FX_INT32 c);
    static FX_INT32 getNumberOfPadCodewords(FX_INT32 m, FX_INT32 k, FX_INT32 c, FX_INT32 r);
    static void encodeChar(FX_INT32 pattern, FX_INT32 len, CBC_BarcodeRow* logic);
    void encodeLowLevel(CFX_WideString fullCodewords, FX_INT32 c, FX_INT32 r, FX_INT32 errorCorrectionLevel, CBC_BarcodeMatrix* logic);
    CFX_Int32Array* determineDimensions(FX_INT32 sourceCodeWords, FX_INT32 errorCorrectionCodeWords, FX_INT32 &e);
};
#endif
