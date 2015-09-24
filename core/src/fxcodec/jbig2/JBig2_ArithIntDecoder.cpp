// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_ArithIntDecoder.h"

#include "../../../include/fxcrt/fx_memory.h"
#include "JBig2_Define.h"

CJBig2_ArithIntDecoder::CJBig2_ArithIntDecoder() {
  IAx = FX_Alloc(JBig2ArithCtx, 512);
  JBIG2_memset(IAx, 0, sizeof(JBig2ArithCtx) * 512);
}
CJBig2_ArithIntDecoder::~CJBig2_ArithIntDecoder() {
  FX_Free(IAx);
}
int CJBig2_ArithIntDecoder::decode(CJBig2_ArithDecoder* pArithDecoder,
                                   int* nResult) {
  int PREV, V;
  int S, D;
  int nNeedBits, nTemp, i;
  PREV = 1;
  S = pArithDecoder->DECODE(IAx + PREV);
  PREV = (PREV << 1) | S;
  D = pArithDecoder->DECODE(IAx + PREV);
  PREV = (PREV << 1) | D;
  if (D) {
    D = pArithDecoder->DECODE(IAx + PREV);
    PREV = (PREV << 1) | D;
    if (D) {
      D = pArithDecoder->DECODE(IAx + PREV);
      PREV = (PREV << 1) | D;
      if (D) {
        D = pArithDecoder->DECODE(IAx + PREV);
        PREV = (PREV << 1) | D;
        if (D) {
          D = pArithDecoder->DECODE(IAx + PREV);
          PREV = (PREV << 1) | D;
          if (D) {
            nNeedBits = 32;
            V = 4436;
          } else {
            nNeedBits = 12;
            V = 340;
          }
        } else {
          nNeedBits = 8;
          V = 84;
        }
      } else {
        nNeedBits = 6;
        V = 20;
      }
    } else {
      nNeedBits = 4;
      V = 4;
    }
  } else {
    nNeedBits = 2;
    V = 0;
  }
  nTemp = 0;
  for (i = 0; i < nNeedBits; i++) {
    D = pArithDecoder->DECODE(IAx + PREV);
    if (PREV < 256) {
      PREV = (PREV << 1) | D;
    } else {
      PREV = (((PREV << 1) | D) & 511) | 256;
    }
    nTemp = (nTemp << 1) | D;
  }
  V += nTemp;
  if (S == 1 && V > 0) {
    V = -V;
  }
  *nResult = V;
  if (S == 1 && V == 0) {
    return JBIG2_OOB;
  }
  return 0;
}
CJBig2_ArithIaidDecoder::CJBig2_ArithIaidDecoder(unsigned char SBSYMCODELENA) {
  SBSYMCODELEN = SBSYMCODELENA;
  IAID = FX_Alloc(JBig2ArithCtx, 1 << SBSYMCODELEN);
  JBIG2_memset(IAID, 0, sizeof(JBig2ArithCtx) * (int)(1 << SBSYMCODELEN));
}
CJBig2_ArithIaidDecoder::~CJBig2_ArithIaidDecoder() {
  FX_Free(IAID);
}
int CJBig2_ArithIaidDecoder::decode(CJBig2_ArithDecoder* pArithDecoder,
                                    int* nResult) {
  int PREV;
  int D;
  int i;
  PREV = 1;
  for (i = 0; i < SBSYMCODELEN; i++) {
    D = pArithDecoder->DECODE(IAID + PREV);
    PREV = (PREV << 1) | D;
  }
  PREV = PREV - (1 << SBSYMCODELEN);
  *nResult = PREV;
  return 0;
}
