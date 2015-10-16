// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_HUFFMAN_DECODER_H_
#define _JBIG2_HUFFMAN_DECODER_H_
#include "JBig2_BitStream.h"
#include "JBig2_HuffmanTable.h"
class CJBig2_HuffmanDecoder {
 public:
  explicit CJBig2_HuffmanDecoder(CJBig2_BitStream* pStream);

  ~CJBig2_HuffmanDecoder();

  int decodeAValue(CJBig2_HuffmanTable* pTable, int* nResult);

 private:
  CJBig2_BitStream* const m_pStream;
};
#endif
