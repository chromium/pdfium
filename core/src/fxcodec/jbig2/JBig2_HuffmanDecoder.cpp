// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_HuffmanDecoder.h"
CJBig2_HuffmanDecoder::CJBig2_HuffmanDecoder(CJBig2_BitStream* pStream) {
  m_pStream = pStream;
}
CJBig2_HuffmanDecoder::~CJBig2_HuffmanDecoder() {}
int CJBig2_HuffmanDecoder::decodeAValue(CJBig2_HuffmanTable* pTable,
                                        int* nResult) {
  int i;
  int nVal = 0;
  int nBits = 0;
  FX_DWORD nTmp;
  while (1) {
    if (m_pStream->read1Bit(&nTmp) == -1) {
      return -1;
    }
    nVal = (nVal << 1) | nTmp;
    nBits++;
    for (i = 0; i < pTable->NTEMP; i++) {
      if ((pTable->PREFLEN[i] == nBits) && (pTable->CODES[i] == nVal)) {
        if ((pTable->HTOOB == 1) && (i == pTable->NTEMP - 1)) {
          return JBIG2_OOB;
        }
        if (m_pStream->readNBits(pTable->RANGELEN[i], &nTmp) == -1) {
          return -1;
        }
        if (pTable->HTOOB) {
          if (i == pTable->NTEMP - 3) {
            *nResult = pTable->RANGELOW[i] - nTmp;
            return 0;
          } else {
            *nResult = pTable->RANGELOW[i] + nTmp;
            return 0;
          }
        } else {
          if (i == pTable->NTEMP - 2) {
            *nResult = pTable->RANGELOW[i] - nTmp;
            return 0;
          } else {
            *nResult = pTable->RANGELOW[i] + nTmp;
            return 0;
          }
        }
      }
    }
  }
  return -2;
}
