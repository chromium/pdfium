// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_HuffmanDecoder.h"

#include "JBig2_Define.h"

CJBig2_HuffmanDecoder::CJBig2_HuffmanDecoder(CJBig2_BitStream* pStream)
    : m_pStream(pStream) {
}

CJBig2_HuffmanDecoder::~CJBig2_HuffmanDecoder() {}

int CJBig2_HuffmanDecoder::decodeAValue(CJBig2_HuffmanTable* pTable,
                                        int* nResult) {
  int nVal = 0;
  int nBits = 0;
  while (1) {
    FX_DWORD nTmp;
    if (m_pStream->read1Bit(&nTmp) == -1)
      break;

    nVal = (nVal << 1) | nTmp;
    ++nBits;
    for (FX_DWORD i = 0; i < pTable->Size(); ++i) {
      if (pTable->GetPREFLEN()[i] == nBits && pTable->GetCODES()[i] == nVal) {
        if (pTable->IsHTOOB() && i == pTable->Size() - 1)
          return JBIG2_OOB;

        if (m_pStream->readNBits(pTable->GetRANGELEN()[i], &nTmp) == -1)
          return -1;

        FX_DWORD offset = pTable->IsHTOOB() ? 3 : 2;
        if (i == pTable->Size() - offset)
          *nResult = pTable->GetRANGELOW()[i] - nTmp;
        else
          *nResult = pTable->GetRANGELOW()[i] + nTmp;
        return 0;
      }
    }
  }
  return -1;
}
