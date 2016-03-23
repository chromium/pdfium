// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXDECODER_H_
#define XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXDECODER_H_

#include "core/fxcrt/include/fx_basic.h"

class CBC_ReedSolomonDecoder;
class CBC_CommonDecoderResult;
class CBC_CommonBitMatrix;

class CBC_DataMatrixDecoder {
 public:
  CBC_DataMatrixDecoder();
  virtual ~CBC_DataMatrixDecoder();
  CBC_CommonDecoderResult* Decode(CBC_CommonBitMatrix* bits, int32_t& e);
  virtual void Init();

 private:
  void CorrectErrors(CFX_ByteArray& codewordBytes,
                     int32_t numDataCodewords,
                     int32_t& e);
  CBC_ReedSolomonDecoder* m_rsDecoder;
};

#endif  // XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXDECODER_H_
