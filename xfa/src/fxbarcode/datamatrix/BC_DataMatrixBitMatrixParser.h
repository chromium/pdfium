// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DATAMATRIXBITMATRIXPARSER_H_
#define _BC_DATAMATRIXBITMATRIXPARSER_H_
class CBC_CommonBitMatrix;
class CBC_DataMatrixVersion;
class CBC_DataMatrixBitMatrixParser;
class CBC_DataMatrixBitMatrixParser : public CFX_Object
{
public:
    CBC_DataMatrixBitMatrixParser();
    virtual ~CBC_DataMatrixBitMatrixParser();
    CBC_DataMatrixVersion *GetVersion();
    CFX_ByteArray *ReadCodewords(FX_INT32 &e);
    FX_BOOL ReadModule(FX_INT32 row, FX_INT32 column, FX_INT32 numRows, FX_INT32 numColumns);
    FX_INT32 ReadUtah(FX_INT32 row, FX_INT32 column, FX_INT32 numRows, FX_INT32 numColumns);
    FX_INT32 ReadCorner1(FX_INT32 numRows, FX_INT32 numColumns);
    FX_INT32 ReadCorner2(FX_INT32 numRows, FX_INT32 numColumns);
    FX_INT32 ReadCorner3(FX_INT32 numRows, FX_INT32 numColumns);
    FX_INT32 ReadCorner4(FX_INT32 numRows, FX_INT32 numColumns);
    CBC_CommonBitMatrix *ExtractDataRegion(CBC_CommonBitMatrix *bitMatrix, FX_INT32 &e);
    virtual void Init(CBC_CommonBitMatrix *bitMatrix, FX_INT32 &e);
private:
    static CBC_DataMatrixVersion* ReadVersion(CBC_CommonBitMatrix *bitMatrix, FX_INT32 &e);
    CBC_CommonBitMatrix *m_mappingBitMatrix;
    CBC_CommonBitMatrix *m_readMappingMatrix;
    CBC_DataMatrixVersion *m_version;
};
#endif
