// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRABITMATRIXPARSER_H_
#define _BC_QRABITMATRIXPARSER_H_
class CBC_CommonBitMatrix ;
class CBC_QRCoderVersion;
class CBC_QRCoderFormatInformation;
class CBC_QRDataMask;
class CBC_QRBitMatrixParser;
class CBC_QRBitMatrixParser  : public CFX_Object
{
private:
    CBC_CommonBitMatrix *m_bitMatrix;
    CBC_CommonBitMatrix *m_tempBitMatrix;
    CBC_QRCoderVersion   *m_version;
    CBC_QRCoderFormatInformation *m_parsedFormatInfo;
    FX_INT32 m_dimension;
public:
    CBC_QRBitMatrixParser();
    virtual ~CBC_QRBitMatrixParser();
    CBC_QRCoderFormatInformation *ReadFormatInformation(FX_INT32 &e);
    CBC_QRCoderVersion			  *ReadVersion(FX_INT32 &e);
    FX_INT32				   CopyBit(FX_INT32 i, FX_INT32 j, FX_INT32 versionBits);
    CFX_ByteArray	  *ReadCodewords(FX_INT32 &e);
    virtual void Init(CBC_CommonBitMatrix *bitMatrix, FX_INT32 &e);
};
#endif
