// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERDECODER_H_
#define _BC_QRCODERDECODER_H_
class CBC_CommonBitMatrix;
class CBC_ReedSolomonDecoder;
class CBC_CommonDecoderResult;
class CBC_QRBitMatrixParser;
class CBC_QRCoderVersion;
class CBC_QRDataBlock;
class CBC_QRCoderDecoder;
class CBC_QRCoderDecoder  : public CFX_Object
{
private:
    CBC_ReedSolomonDecoder *m_rsDecoder;
public:
    CBC_QRCoderDecoder();
    virtual ~CBC_QRCoderDecoder();

    CBC_CommonDecoderResult* Decode(FX_BOOL* image, FX_INT32 width, FX_INT32 height, FX_INT32 &e);
    CBC_CommonDecoderResult* Decode(CBC_CommonBitMatrix* bits, FX_INT32 byteModeDecode, FX_INT32 &e);
    void CorrectErrors(CFX_ByteArray* codewordBytes, FX_INT32 numDataCodewords, FX_INT32 &e);
    virtual void Init();
};
#endif
