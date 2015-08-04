// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERECBLOCKS_H_
#define _BC_QRCODERECBLOCKS_H_
class CBC_QRCoderECB;
class CBC_QRCoderECBlocks {
 private:
  int32_t m_ecCodeWordsPerBlock;
  CFX_PtrArray m_ecBlocks;

 public:
  CBC_QRCoderECBlocks(int32_t ecCodeWordsPerBlock, CBC_QRCoderECB* ecBlocks);
  CBC_QRCoderECBlocks(int32_t ecCodeWordsPerBlock,
                      CBC_QRCoderECB* ecBlocks1,
                      CBC_QRCoderECB* ecBlocks2);
  virtual ~CBC_QRCoderECBlocks();
  int32_t GetECCodeWordsPerBlock();
  int32_t GetNumBlocks();
  int32_t GetTotalECCodeWords();
  CFX_PtrArray* GetECBlocks();
};
#endif
