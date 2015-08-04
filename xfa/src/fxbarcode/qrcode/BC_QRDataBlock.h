// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRDATABLOCK_H_
#define _BC_QRDATABLOCK_H_
class CBC_QRCoderVersion;
class CBC_QRCoderECBlocks;
class CBC_QRCoderECB;
class CBC_QRCoderErrorCorrectionLevel;
class CBC_QRDataBlock {
 private:
  int32_t m_numDataCodewords;
  CFX_ByteArray* m_codewords;
  CBC_QRDataBlock(int32_t numDataCodewords, CFX_ByteArray* codewords);

 public:
  virtual ~CBC_QRDataBlock();
  int32_t GetNumDataCodewords();
  CFX_ByteArray* GetCodewords();
  static CFX_PtrArray* GetDataBlocks(CFX_ByteArray* rawCodewords,
                                     CBC_QRCoderVersion* version,
                                     CBC_QRCoderErrorCorrectionLevel* ecLevel,
                                     int32_t& e);
};
#endif
