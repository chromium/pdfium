// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DATAMATRIXDATABLOCK_H_
#define _BC_DATAMATRIXDATABLOCK_H_
class CBC_DataMatrixVersion;
class CBC_DataMatrixDataBlock;
class CBC_DataMatrixDataBlock : public CFX_Object
{
public:
    virtual ~CBC_DataMatrixDataBlock();

    FX_INT32 GetNumDataCodewords();
    CFX_ByteArray* GetCodewords();

    static CFX_PtrArray *GetDataBlocks(CFX_ByteArray* rawCodewords, CBC_DataMatrixVersion *version, FX_INT32 &e);
private:
    FX_INT32 m_numDataCodewords;
    CFX_ByteArray m_codewords;

    CBC_DataMatrixDataBlock(FX_INT32 numDataCodewords, CFX_ByteArray *codewords);
};
#endif
