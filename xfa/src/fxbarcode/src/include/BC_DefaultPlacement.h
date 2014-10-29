// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DEFAULTPLACEMENT_H_
#define _BC_DEFAULTPLACEMENT_H_
class CBC_DefaultPlacement;
class CBC_DefaultPlacement : public CFX_Object
{
public:
    CBC_DefaultPlacement(CFX_WideString codewords, FX_INT32 numcols, FX_INT32 numrows);
    virtual ~CBC_DefaultPlacement();

    FX_INT32 getNumrows();
    FX_INT32 getNumcols();
    CFX_ByteArray& getBits();
    FX_BOOL getBit(FX_INT32 col, FX_INT32 row);
    void setBit(FX_INT32 col, FX_INT32 row, FX_BOOL bit);
    FX_BOOL hasBit(FX_INT32 col, FX_INT32 row);
    void place();
private:
    CFX_WideString m_codewords;
    FX_INT32 m_numrows;
    FX_INT32 m_numcols;
    CFX_ByteArray m_bits;
    void module(FX_INT32 row, FX_INT32 col, FX_INT32 pos, FX_INT32 bit);
    void utah(FX_INT32 row, FX_INT32 col, FX_INT32 pos);
    void corner1(FX_INT32 pos);
    void corner2(FX_INT32 pos);
    void corner3(FX_INT32 pos);
    void corner4(FX_INT32 pos);
};
#endif
