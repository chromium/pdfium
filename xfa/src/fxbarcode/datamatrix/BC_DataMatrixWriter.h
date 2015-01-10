// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DATAMATRIXWRITER_H_
#define _BC_DATAMATRIXWRITER_H_
class CBC_CommonByteMatrix;
class CBC_CommonBitMatrix;
class CBC_DefaultPlacement;
class CBC_SymbolShapeHint;
class CBC_SymbolInfo;
class CBC_TwoDimWriter;
class CBC_DataMatrixWriter;
class CBC_DataMatrixWriter : public CBC_TwoDimWriter
{
public:
    CBC_DataMatrixWriter();
    virtual ~CBC_DataMatrixWriter();
    FX_BYTE*	Encode(const CFX_WideString &contents, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e);
    FX_BYTE *	Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e);
    FX_BYTE *	Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e);
    FX_BOOL		SetErrorCorrectionLevel (FX_INT32 level);

private:
    static CBC_CommonByteMatrix* encodeLowLevel(CBC_DefaultPlacement* placement, CBC_SymbolInfo* symbolInfo, FX_INT32 &e);
    FX_INT32	m_iCorrectLevel;
};
#endif
