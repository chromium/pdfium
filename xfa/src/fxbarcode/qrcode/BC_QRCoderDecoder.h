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
class CBC_QRCoderDecoder {
 private:
  CBC_ReedSolomonDecoder* m_rsDecoder;

 public:
  CBC_QRCoderDecoder();
  virtual ~CBC_QRCoderDecoder();

  CBC_CommonDecoderResult* Decode(FX_BOOL* image,
                                  int32_t width,
                                  int32_t height,
                                  int32_t& e);
  CBC_CommonDecoderResult* Decode(CBC_CommonBitMatrix* bits,
                                  int32_t byteModeDecode,
                                  int32_t& e);
  void CorrectErrors(CFX_ByteArray* codewordBytes,
                     int32_t numDataCodewords,
                     int32_t& e);
  virtual void Init();
};
#endif
