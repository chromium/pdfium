// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_QRCODE_BC_QRDATABLOCK_H_
#define XFA_FXBARCODE_QRCODE_BC_QRDATABLOCK_H_

#include "core/fxcrt/include/fx_basic.h"

class CBC_QRCoderErrorCorrectionLevel;
class CBC_QRCoderVersion;

class CBC_QRDataBlock final {
 public:
  ~CBC_QRDataBlock();
  int32_t GetNumDataCodewords();
  CFX_ByteArray* GetCodewords();
  static CFX_ArrayTemplate<CBC_QRDataBlock*>* GetDataBlocks(
      CFX_ByteArray* rawCodewords,
      CBC_QRCoderVersion* version,
      CBC_QRCoderErrorCorrectionLevel* ecLevel,
      int32_t& e);

 private:
  CBC_QRDataBlock(int32_t numDataCodewords, CFX_ByteArray* codewords);

  int32_t m_numDataCodewords;
  CFX_ByteArray* m_codewords;
};

#endif  // XFA_FXBARCODE_QRCODE_BC_QRDATABLOCK_H_
