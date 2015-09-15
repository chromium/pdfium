// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_GeneralDecoder.h"

#include "../../../../third_party/base/nonstd_unique_ptr.h"
#include "JBig2_ArithDecoder.h"
#include "JBig2_ArithIntDecoder.h"
#include "JBig2_HuffmanDecoder.h"
#include "JBig2_HuffmanTable.h"
#include "JBig2_PatternDict.h"

extern const JBig2ArithQe QeTable[] = {
    {0x5601, 1, 1, 1},   {0x3401, 2, 6, 0},   {0x1801, 3, 9, 0},
    {0x0AC1, 4, 12, 0},  {0x0521, 5, 29, 0},  {0x0221, 38, 33, 0},
    {0x5601, 7, 6, 1},   {0x5401, 8, 14, 0},  {0x4801, 9, 14, 0},
    {0x3801, 10, 14, 0}, {0x3001, 11, 17, 0}, {0x2401, 12, 18, 0},
    {0x1C01, 13, 20, 0}, {0x1601, 29, 21, 0}, {0x5601, 15, 14, 1},
    {0x5401, 16, 14, 0}, {0x5101, 17, 15, 0}, {0x4801, 18, 16, 0},
    {0x3801, 19, 17, 0}, {0x3401, 20, 18, 0}, {0x3001, 21, 19, 0},
    {0x2801, 22, 19, 0}, {0x2401, 23, 20, 0}, {0x2201, 24, 21, 0},
    {0x1C01, 25, 22, 0}, {0x1801, 26, 23, 0}, {0x1601, 27, 24, 0},
    {0x1401, 28, 25, 0}, {0x1201, 29, 26, 0}, {0x1101, 30, 27, 0},
    {0x0AC1, 31, 28, 0}, {0x09C1, 32, 29, 0}, {0x08A1, 33, 30, 0},
    {0x0521, 34, 31, 0}, {0x0441, 35, 32, 0}, {0x02A1, 36, 33, 0},
    {0x0221, 37, 34, 0}, {0x0141, 38, 35, 0}, {0x0111, 39, 36, 0},
    {0x0085, 40, 37, 0}, {0x0049, 41, 38, 0}, {0x0025, 42, 39, 0},
    {0x0015, 43, 40, 0}, {0x0009, 44, 41, 0}, {0x0005, 45, 42, 0},
    {0x0001, 45, 43, 0}, {0x5601, 46, 46, 0}};

extern const unsigned int JBIG2_QE_NUM = FX_ArraySize(QeTable);

bool CJBig2_GRDProc::UseTemplate0Opt3() const {
  return (GBAT[0] == 3) && (GBAT[1] == -1) && (GBAT[2] == -3) &&
         (GBAT[3] == -1) && (GBAT[4] == 2) && (GBAT[5] == -2) &&
         (GBAT[6] == -2) && (GBAT[7] == -2);
}

bool CJBig2_GRDProc::UseTemplate1Opt3() const {
  return (GBAT[0] == 3) && (GBAT[1] == -1);
}

bool CJBig2_GRDProc::UseTemplate23Opt3() const {
  return (GBAT[0] == 2) && (GBAT[1] == -1);
}

CJBig2_Image* CJBig2_GRDProc::decode_Arith(CJBig2_ArithDecoder* pArithDecoder,
                                           JBig2ArithCtx* gbContext) {
  if (GBW == 0 || GBH == 0)
    return new CJBig2_Image(GBW, GBH);

  if (GBTEMPLATE == 0) {
    if (UseTemplate0Opt3())
      return decode_Arith_Template0_opt3(pArithDecoder, gbContext);
    return decode_Arith_Template0_unopt(pArithDecoder, gbContext);
  } else if (GBTEMPLATE == 1) {
    if (UseTemplate1Opt3())
      return decode_Arith_Template1_opt3(pArithDecoder, gbContext);
    return decode_Arith_Template1_unopt(pArithDecoder, gbContext);
  } else if (GBTEMPLATE == 2) {
    if (UseTemplate23Opt3())
      return decode_Arith_Template2_opt3(pArithDecoder, gbContext);
    return decode_Arith_Template2_unopt(pArithDecoder, gbContext);
  } else {
    if (UseTemplate23Opt3())
      return decode_Arith_Template3_opt3(pArithDecoder, gbContext);
    return decode_Arith_Template3_unopt(pArithDecoder, gbContext);
  }
}
CJBig2_Image* CJBig2_GRDProc::decode_Arith_Template0_opt3(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext) {
  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2;
  uint8_t *pLine, *pLine1, *pLine2, cVal;
  int32_t nStride, nStride2, k;
  int32_t nLineBytes, nBitsLeft, cc;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GBREG(new CJBig2_Image(GBW, GBH));
  if (!GBREG->m_pData)
    return nullptr;

  pLine = GBREG->m_pData;
  nStride = GBREG->m_nStride;
  nStride2 = nStride << 1;
  nLineBytes = ((GBW + 7) >> 3) - 1;
  nBitsLeft = GBW - (nLineBytes << 3);
  FX_DWORD height = GBH & 0x7fffffff;
  for (FX_DWORD h = 0; h < height; h++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x9b25]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      GBREG->copyLine(h, h - 1);
    } else {
      if (h > 1) {
        pLine1 = pLine - nStride2;
        pLine2 = pLine - nStride;
        line1 = (*pLine1++) << 6;
        line2 = *pLine2++;
        CONTEXT = ((line1 & 0xf800) | (line2 & 0x07f0));
        for (cc = 0; cc < nLineBytes; cc++) {
          line1 = (line1 << 8) | ((*pLine1++) << 6);
          line2 = (line2 << 8) | (*pLine2++);
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal |
                       ((line1 >> k) & 0x0800) | ((line2 >> k) & 0x0010));
          }
          pLine[cc] = cVal;
        }
        line1 <<= 8;
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT =
              (((CONTEXT & 0x7bf7) << 1) | bVal |
               ((line1 >> (7 - k)) & 0x0800) | ((line2 >> (7 - k)) & 0x0010));
        }
        pLine[nLineBytes] = cVal;
      } else {
        pLine2 = pLine - nStride;
        line2 = (h & 1) ? (*pLine2++) : 0;
        CONTEXT = (line2 & 0x07f0);
        for (cc = 0; cc < nLineBytes; cc++) {
          if (h & 1) {
            line2 = (line2 << 8) | (*pLine2++);
          }
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT =
                (((CONTEXT & 0x7bf7) << 1) | bVal | ((line2 >> k) & 0x0010));
          }
          pLine[cc] = cVal;
        }
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal |
                     (((line2 >> (7 - k))) & 0x0010));
        }
        pLine[nLineBytes] = cVal;
      }
    }
    pLine += nStride;
  }
  return GBREG.release();
}

CJBig2_Image* CJBig2_GRDProc::decode_Arith_Template0_unopt(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext) {
  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2, line3;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GBREG(new CJBig2_Image(GBW, GBH));
  GBREG->fill(0);
  for (FX_DWORD h = 0; h < GBH; h++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x9b25]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      GBREG->copyLine(h, h - 1);
    } else {
      line1 = GBREG->getPixel(1, h - 2);
      line1 |= GBREG->getPixel(0, h - 2) << 1;
      line2 = GBREG->getPixel(2, h - 1);
      line2 |= GBREG->getPixel(1, h - 1) << 1;
      line2 |= GBREG->getPixel(0, h - 1) << 2;
      line3 = 0;
      for (FX_DWORD w = 0; w < GBW; w++) {
        if (USESKIP && SKIP->getPixel(w, h)) {
          bVal = 0;
        } else {
          CONTEXT = line3;
          CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 4;
          CONTEXT |= line2 << 5;
          CONTEXT |= GBREG->getPixel(w + GBAT[2], h + GBAT[3]) << 10;
          CONTEXT |= GBREG->getPixel(w + GBAT[4], h + GBAT[5]) << 11;
          CONTEXT |= line1 << 12;
          CONTEXT |= GBREG->getPixel(w + GBAT[6], h + GBAT[7]) << 15;
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
        }
        if (bVal) {
          GBREG->setPixel(w, h, bVal);
        }
        line1 = ((line1 << 1) | GBREG->getPixel(w + 2, h - 2)) & 0x07;
        line2 = ((line2 << 1) | GBREG->getPixel(w + 3, h - 1)) & 0x1f;
        line3 = ((line3 << 1) | bVal) & 0x0f;
      }
    }
  }
  return GBREG.release();
}

CJBig2_Image* CJBig2_GRDProc::decode_Arith_Template1_opt3(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext) {
  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2;
  uint8_t *pLine, *pLine1, *pLine2, cVal;
  int32_t nStride, nStride2, k;
  int32_t nLineBytes, nBitsLeft, cc;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GBREG(new CJBig2_Image(GBW, GBH));
  if (!GBREG->m_pData)
    return nullptr;

  pLine = GBREG->m_pData;
  nStride = GBREG->m_nStride;
  nStride2 = nStride << 1;
  nLineBytes = ((GBW + 7) >> 3) - 1;
  nBitsLeft = GBW - (nLineBytes << 3);
  for (FX_DWORD h = 0; h < GBH; h++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x0795]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      GBREG->copyLine(h, h - 1);
    } else {
      if (h > 1) {
        pLine1 = pLine - nStride2;
        pLine2 = pLine - nStride;
        line1 = (*pLine1++) << 4;
        line2 = *pLine2++;
        CONTEXT = (line1 & 0x1e00) | ((line2 >> 1) & 0x01f8);
        for (cc = 0; cc < nLineBytes; cc++) {
          line1 = (line1 << 8) | ((*pLine1++) << 4);
          line2 = (line2 << 8) | (*pLine2++);
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal |
                      ((line1 >> k) & 0x0200) | ((line2 >> (k + 1)) & 0x0008);
          }
          pLine[cc] = cVal;
        }
        line1 <<= 8;
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal |
                    ((line1 >> (7 - k)) & 0x0200) |
                    ((line2 >> (8 - k)) & 0x0008);
        }
        pLine[nLineBytes] = cVal;
      } else {
        pLine2 = pLine - nStride;
        line2 = (h & 1) ? (*pLine2++) : 0;
        CONTEXT = (line2 >> 1) & 0x01f8;
        for (cc = 0; cc < nLineBytes; cc++) {
          if (h & 1) {
            line2 = (line2 << 8) | (*pLine2++);
          }
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal |
                      ((line2 >> (k + 1)) & 0x0008);
          }
          pLine[cc] = cVal;
        }
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT =
              ((CONTEXT & 0x0efb) << 1) | bVal | ((line2 >> (8 - k)) & 0x0008);
        }
        pLine[nLineBytes] = cVal;
      }
    }
    pLine += nStride;
  }
  return GBREG.release();
}

CJBig2_Image* CJBig2_GRDProc::decode_Arith_Template1_unopt(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext) {
  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2, line3;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GBREG(new CJBig2_Image(GBW, GBH));
  GBREG->fill(0);
  for (FX_DWORD h = 0; h < GBH; h++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x0795]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      GBREG->copyLine(h, h - 1);
    } else {
      line1 = GBREG->getPixel(2, h - 2);
      line1 |= GBREG->getPixel(1, h - 2) << 1;
      line1 |= GBREG->getPixel(0, h - 2) << 2;
      line2 = GBREG->getPixel(2, h - 1);
      line2 |= GBREG->getPixel(1, h - 1) << 1;
      line2 |= GBREG->getPixel(0, h - 1) << 2;
      line3 = 0;
      for (FX_DWORD w = 0; w < GBW; w++) {
        if (USESKIP && SKIP->getPixel(w, h)) {
          bVal = 0;
        } else {
          CONTEXT = line3;
          CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 3;
          CONTEXT |= line2 << 4;
          CONTEXT |= line1 << 9;
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
        }
        if (bVal) {
          GBREG->setPixel(w, h, bVal);
        }
        line1 = ((line1 << 1) | GBREG->getPixel(w + 3, h - 2)) & 0x0f;
        line2 = ((line2 << 1) | GBREG->getPixel(w + 3, h - 1)) & 0x1f;
        line3 = ((line3 << 1) | bVal) & 0x07;
      }
    }
  }
  return GBREG.release();
}
CJBig2_Image* CJBig2_GRDProc::decode_Arith_Template2_opt3(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext) {
  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2;
  uint8_t *pLine, *pLine1, *pLine2, cVal;
  int32_t nStride, nStride2, k;
  int32_t nLineBytes, nBitsLeft, cc;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GBREG(new CJBig2_Image(GBW, GBH));
  if (!GBREG->m_pData)
    return nullptr;

  pLine = GBREG->m_pData;
  nStride = GBREG->m_nStride;
  nStride2 = nStride << 1;
  nLineBytes = ((GBW + 7) >> 3) - 1;
  nBitsLeft = GBW - (nLineBytes << 3);
  for (FX_DWORD h = 0; h < GBH; h++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x00e5]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      GBREG->copyLine(h, h - 1);
    } else {
      if (h > 1) {
        pLine1 = pLine - nStride2;
        pLine2 = pLine - nStride;
        line1 = (*pLine1++) << 1;
        line2 = *pLine2++;
        CONTEXT = (line1 & 0x0380) | ((line2 >> 3) & 0x007c);
        for (cc = 0; cc < nLineBytes; cc++) {
          line1 = (line1 << 8) | ((*pLine1++) << 1);
          line2 = (line2 << 8) | (*pLine2++);
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal |
                      ((line1 >> k) & 0x0080) | ((line2 >> (k + 3)) & 0x0004);
          }
          pLine[cc] = cVal;
        }
        line1 <<= 8;
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal |
                    ((line1 >> (7 - k)) & 0x0080) |
                    ((line2 >> (10 - k)) & 0x0004);
        }
        pLine[nLineBytes] = cVal;
      } else {
        pLine2 = pLine - nStride;
        line2 = (h & 1) ? (*pLine2++) : 0;
        CONTEXT = (line2 >> 3) & 0x007c;
        for (cc = 0; cc < nLineBytes; cc++) {
          if (h & 1) {
            line2 = (line2 << 8) | (*pLine2++);
          }
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal |
                      ((line2 >> (k + 3)) & 0x0004);
          }
          pLine[cc] = cVal;
        }
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal |
                    (((line2 >> (10 - k))) & 0x0004);
        }
        pLine[nLineBytes] = cVal;
      }
    }
    pLine += nStride;
  }
  return GBREG.release();
}

CJBig2_Image* CJBig2_GRDProc::decode_Arith_Template2_unopt(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext) {
  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2, line3;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GBREG(new CJBig2_Image(GBW, GBH));
  GBREG->fill(0);
  for (FX_DWORD h = 0; h < GBH; h++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x00e5]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      GBREG->copyLine(h, h - 1);
    } else {
      line1 = GBREG->getPixel(1, h - 2);
      line1 |= GBREG->getPixel(0, h - 2) << 1;
      line2 = GBREG->getPixel(1, h - 1);
      line2 |= GBREG->getPixel(0, h - 1) << 1;
      line3 = 0;
      for (FX_DWORD w = 0; w < GBW; w++) {
        if (USESKIP && SKIP->getPixel(w, h)) {
          bVal = 0;
        } else {
          CONTEXT = line3;
          CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 2;
          CONTEXT |= line2 << 3;
          CONTEXT |= line1 << 7;
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
        }
        if (bVal) {
          GBREG->setPixel(w, h, bVal);
        }
        line1 = ((line1 << 1) | GBREG->getPixel(w + 2, h - 2)) & 0x07;
        line2 = ((line2 << 1) | GBREG->getPixel(w + 2, h - 1)) & 0x0f;
        line3 = ((line3 << 1) | bVal) & 0x03;
      }
    }
  }
  return GBREG.release();
}

CJBig2_Image* CJBig2_GRDProc::decode_Arith_Template3_opt3(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext) {
  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1;
  uint8_t *pLine, *pLine1, cVal;
  int32_t nStride, k;
  int32_t nLineBytes, nBitsLeft, cc;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GBREG(new CJBig2_Image(GBW, GBH));
  if (!GBREG->m_pData)
    return nullptr;

  pLine = GBREG->m_pData;
  nStride = GBREG->m_nStride;
  nLineBytes = ((GBW + 7) >> 3) - 1;
  nBitsLeft = GBW - (nLineBytes << 3);
  for (FX_DWORD h = 0; h < GBH; h++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x0195]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      GBREG->copyLine(h, h - 1);
    } else {
      if (h > 0) {
        pLine1 = pLine - nStride;
        line1 = *pLine1++;
        CONTEXT = (line1 >> 1) & 0x03f0;
        for (cc = 0; cc < nLineBytes; cc++) {
          line1 = (line1 << 8) | (*pLine1++);
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal |
                      ((line1 >> (k + 1)) & 0x0010);
          }
          pLine[cc] = cVal;
        }
        line1 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT =
              ((CONTEXT & 0x01f7) << 1) | bVal | ((line1 >> (8 - k)) & 0x0010);
        }
        pLine[nLineBytes] = cVal;
      } else {
        CONTEXT = 0;
        for (cc = 0; cc < nLineBytes; cc++) {
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal;
          }
          pLine[cc] = cVal;
        }
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal;
        }
        pLine[nLineBytes] = cVal;
      }
    }
    pLine += nStride;
  }
  return GBREG.release();
}

CJBig2_Image* CJBig2_GRDProc::decode_Arith_Template3_unopt(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext) {
  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GBREG(new CJBig2_Image(GBW, GBH));
  GBREG->fill(0);
  for (FX_DWORD h = 0; h < GBH; h++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x0195]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      GBREG->copyLine(h, h - 1);
    } else {
      line1 = GBREG->getPixel(1, h - 1);
      line1 |= GBREG->getPixel(0, h - 1) << 1;
      line2 = 0;
      for (FX_DWORD w = 0; w < GBW; w++) {
        if (USESKIP && SKIP->getPixel(w, h)) {
          bVal = 0;
        } else {
          CONTEXT = line2;
          CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 4;
          CONTEXT |= line1 << 5;
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
        }
        if (bVal) {
          GBREG->setPixel(w, h, bVal);
        }
        line1 = ((line1 << 1) | GBREG->getPixel(w + 2, h - 1)) & 0x1f;
        line2 = ((line2 << 1) | bVal) & 0x0f;
      }
    }
  }
  return GBREG.release();
}

CJBig2_Image* CJBig2_GRRDProc::decode(CJBig2_ArithDecoder* pArithDecoder,
                                      JBig2ArithCtx* grContext) {
  if (GRW == 0 || GRH == 0)
    return new CJBig2_Image(GRW, GRH);

  if (GRTEMPLATE == 0) {
    if ((GRAT[0] == -1) && (GRAT[1] == -1) && (GRAT[2] == -1) &&
        (GRAT[3] == -1) && (GRREFERENCEDX == 0) &&
        (GRW == (FX_DWORD)GRREFERENCE->m_nWidth)) {
      return decode_Template0_opt(pArithDecoder, grContext);
    }
    return decode_Template0_unopt(pArithDecoder, grContext);
  }

  if ((GRREFERENCEDX == 0) && (GRW == (FX_DWORD)GRREFERENCE->m_nWidth))
    return decode_Template1_opt(pArithDecoder, grContext);
  return decode_Template1_unopt(pArithDecoder, grContext);
}

CJBig2_Image* CJBig2_GRRDProc::decode_Template0_unopt(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* grContext) {
  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2, line3, line4, line5;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GRREG(new CJBig2_Image(GRW, GRH));
  GRREG->fill(0);
  for (FX_DWORD h = 0; h < GRH; h++) {
    if (TPGRON) {
      SLTP = pArithDecoder->DECODE(&grContext[0x0010]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 0) {
      line1 = GRREG->getPixel(1, h - 1);
      line1 |= GRREG->getPixel(0, h - 1) << 1;
      line2 = 0;
      line3 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY - 1);
      line3 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY - 1)
               << 1;
      line4 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY);
      line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY) << 1;
      line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY)
               << 2;
      line5 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
      line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY + 1)
               << 1;
      line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY + 1)
               << 2;
      for (FX_DWORD w = 0; w < GRW; w++) {
        CONTEXT = line5;
        CONTEXT |= line4 << 3;
        CONTEXT |= line3 << 6;
        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + GRAT[2],
                                         h - GRREFERENCEDY + GRAT[3])
                   << 8;
        CONTEXT |= line2 << 9;
        CONTEXT |= line1 << 10;
        CONTEXT |= GRREG->getPixel(w + GRAT[0], h + GRAT[1]) << 12;
        bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
        GRREG->setPixel(w, h, bVal);
        line1 = ((line1 << 1) | GRREG->getPixel(w + 2, h - 1)) & 0x03;
        line2 = ((line2 << 1) | bVal) & 0x01;
        line3 = ((line3 << 1) |
                 GRREFERENCE->getPixel(w - GRREFERENCEDX + 2,
                                       h - GRREFERENCEDY - 1)) &
                0x03;
        line4 =
            ((line4 << 1) |
             GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY)) &
            0x07;
        line5 = ((line5 << 1) |
                 GRREFERENCE->getPixel(w - GRREFERENCEDX + 2,
                                       h - GRREFERENCEDY + 1)) &
                0x07;
      }
    } else {
      line1 = GRREG->getPixel(1, h - 1);
      line1 |= GRREG->getPixel(0, h - 1) << 1;
      line2 = 0;
      line3 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY - 1);
      line3 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY - 1)
               << 1;
      line4 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY);
      line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY) << 1;
      line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY)
               << 2;
      line5 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
      line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY + 1)
               << 1;
      line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY + 1)
               << 2;
      for (FX_DWORD w = 0; w < GRW; w++) {
        bVal = GRREFERENCE->getPixel(w, h);
        if (!(TPGRON && (bVal == GRREFERENCE->getPixel(w - 1, h - 1)) &&
              (bVal == GRREFERENCE->getPixel(w, h - 1)) &&
              (bVal == GRREFERENCE->getPixel(w + 1, h - 1)) &&
              (bVal == GRREFERENCE->getPixel(w - 1, h)) &&
              (bVal == GRREFERENCE->getPixel(w + 1, h)) &&
              (bVal == GRREFERENCE->getPixel(w - 1, h + 1)) &&
              (bVal == GRREFERENCE->getPixel(w, h + 1)) &&
              (bVal == GRREFERENCE->getPixel(w + 1, h + 1)))) {
          CONTEXT = line5;
          CONTEXT |= line4 << 3;
          CONTEXT |= line3 << 6;
          CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + GRAT[2],
                                           h - GRREFERENCEDY + GRAT[3])
                     << 8;
          CONTEXT |= line2 << 9;
          CONTEXT |= line1 << 10;
          CONTEXT |= GRREG->getPixel(w + GRAT[0], h + GRAT[1]) << 12;
          bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
        }
        GRREG->setPixel(w, h, bVal);
        line1 = ((line1 << 1) | GRREG->getPixel(w + 2, h - 1)) & 0x03;
        line2 = ((line2 << 1) | bVal) & 0x01;
        line3 = ((line3 << 1) |
                 GRREFERENCE->getPixel(w - GRREFERENCEDX + 2,
                                       h - GRREFERENCEDY - 1)) &
                0x03;
        line4 =
            ((line4 << 1) |
             GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY)) &
            0x07;
        line5 = ((line5 << 1) |
                 GRREFERENCE->getPixel(w - GRREFERENCEDX + 2,
                                       h - GRREFERENCEDY + 1)) &
                0x07;
      }
    }
  }
  return GRREG.release();
}

CJBig2_Image* CJBig2_GRRDProc::decode_Template0_opt(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* grContext) {
  if (!GRREFERENCE->m_pData)
    return nullptr;

  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line1_r, line2_r, line3_r;
  uint8_t *pLine, *pLineR, cVal;
  intptr_t nStride, nStrideR, nOffset;
  int32_t k, nBits;
  int32_t GRWR, GRHR;
  int32_t GRW, GRH;
  GRW = (int32_t)CJBig2_GRRDProc::GRW;
  GRH = (int32_t)CJBig2_GRRDProc::GRH;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GRREG(new CJBig2_Image(GRW, GRH));
  if (!GRREG->m_pData)
    return nullptr;

  pLine = GRREG->m_pData;
  pLineR = GRREFERENCE->m_pData;
  nStride = GRREG->m_nStride;
  nStrideR = GRREFERENCE->m_nStride;
  GRWR = (int32_t)GRREFERENCE->m_nWidth;
  GRHR = (int32_t)GRREFERENCE->m_nHeight;
  if (GRREFERENCEDY < -GRHR + 1 || GRREFERENCEDY > GRHR - 1) {
    GRREFERENCEDY = 0;
  }
  nOffset = -GRREFERENCEDY * nStrideR;
  for (int32_t h = 0; h < GRH; h++) {
    if (TPGRON) {
      SLTP = pArithDecoder->DECODE(&grContext[0x0010]);
      LTP = LTP ^ SLTP;
    }
    line1 = (h > 0) ? pLine[-nStride] << 4 : 0;
    int32_t reference_h = h - GRREFERENCEDY;
    FX_BOOL line1_r_ok = (reference_h > 0 && reference_h < GRHR + 1);
    FX_BOOL line2_r_ok = (reference_h > -1 && reference_h < GRHR);
    FX_BOOL line3_r_ok = (reference_h > -2 && reference_h < GRHR - 1);
    line1_r = line1_r_ok ? pLineR[nOffset - nStrideR] : 0;
    line2_r = line2_r_ok ? pLineR[nOffset] : 0;
    line3_r = line3_r_ok ? pLineR[nOffset + nStrideR] : 0;
    if (LTP == 0) {
      CONTEXT = (line1 & 0x1c00) | (line1_r & 0x01c0) |
                ((line2_r >> 3) & 0x0038) | ((line3_r >> 6) & 0x0007);
      for (int32_t w = 0; w < GRW; w += 8) {
        nBits = GRW - w > 8 ? 8 : GRW - w;
        if (h > 0)
          line1 = (line1 << 8) |
                  (w + 8 < GRW ? pLine[-nStride + (w >> 3) + 1] << 4 : 0);
        if (h > GRHR + GRREFERENCEDY + 1) {
          line1_r = 0;
          line2_r = 0;
          line3_r = 0;
        } else {
          if (line1_r_ok)
            line1_r =
                (line1_r << 8) |
                (w + 8 < GRWR ? pLineR[nOffset - nStrideR + (w >> 3) + 1] : 0);
          if (line2_r_ok)
            line2_r = (line2_r << 8) |
                      (w + 8 < GRWR ? pLineR[nOffset + (w >> 3) + 1] : 0);
          if (line3_r_ok) {
            line3_r =
                (line3_r << 8) |
                (w + 8 < GRWR ? pLineR[nOffset + nStrideR + (w >> 3) + 1] : 0);
          } else {
            line3_r = 0;
          }
        }
        cVal = 0;
        for (k = 0; k < nBits; k++) {
          bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x0cdb) << 1) | (bVal << 9) |
                    ((line1 >> (7 - k)) & 0x0400) |
                    ((line1_r >> (7 - k)) & 0x0040) |
                    ((line2_r >> (10 - k)) & 0x0008) |
                    ((line3_r >> (13 - k)) & 0x0001);
        }
        pLine[w >> 3] = cVal;
      }
    } else {
      CONTEXT = (line1 & 0x1c00) | (line1_r & 0x01c0) |
                ((line2_r >> 3) & 0x0038) | ((line3_r >> 6) & 0x0007);
      for (int32_t w = 0; w < GRW; w += 8) {
        nBits = GRW - w > 8 ? 8 : GRW - w;
        if (h > 0)
          line1 = (line1 << 8) |
                  (w + 8 < GRW ? pLine[-nStride + (w >> 3) + 1] << 4 : 0);
        if (line1_r_ok)
          line1_r =
              (line1_r << 8) |
              (w + 8 < GRWR ? pLineR[nOffset - nStrideR + (w >> 3) + 1] : 0);
        if (line2_r_ok)
          line2_r = (line2_r << 8) |
                    (w + 8 < GRWR ? pLineR[nOffset + (w >> 3) + 1] : 0);
        if (line3_r_ok) {
          line3_r =
              (line3_r << 8) |
              (w + 8 < GRWR ? pLineR[nOffset + nStrideR + (w >> 3) + 1] : 0);
        } else {
          line3_r = 0;
        }
        cVal = 0;
        for (k = 0; k < nBits; k++) {
          bVal = GRREFERENCE->getPixel(w + k, h);
          if (!(TPGRON && (bVal == GRREFERENCE->getPixel(w + k - 1, h - 1)) &&
                (bVal == GRREFERENCE->getPixel(w + k, h - 1)) &&
                (bVal == GRREFERENCE->getPixel(w + k + 1, h - 1)) &&
                (bVal == GRREFERENCE->getPixel(w + k - 1, h)) &&
                (bVal == GRREFERENCE->getPixel(w + k + 1, h)) &&
                (bVal == GRREFERENCE->getPixel(w + k - 1, h + 1)) &&
                (bVal == GRREFERENCE->getPixel(w + k, h + 1)) &&
                (bVal == GRREFERENCE->getPixel(w + k + 1, h + 1)))) {
            bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
          }
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x0cdb) << 1) | (bVal << 9) |
                    ((line1 >> (7 - k)) & 0x0400) |
                    ((line1_r >> (7 - k)) & 0x0040) |
                    ((line2_r >> (10 - k)) & 0x0008) |
                    ((line3_r >> (13 - k)) & 0x0001);
        }
        pLine[w >> 3] = cVal;
      }
    }
    pLine += nStride;
    if (h < GRHR + GRREFERENCEDY) {
      pLineR += nStrideR;
    }
  }
  return GRREG.release();
}

CJBig2_Image* CJBig2_GRRDProc::decode_Template1_unopt(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* grContext) {
  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2, line3, line4, line5;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GRREG(new CJBig2_Image(GRW, GRH));
  GRREG->fill(0);
  for (FX_DWORD h = 0; h < GRH; h++) {
    if (TPGRON) {
      SLTP = pArithDecoder->DECODE(&grContext[0x0008]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 0) {
      line1 = GRREG->getPixel(1, h - 1);
      line1 |= GRREG->getPixel(0, h - 1) << 1;
      line1 |= GRREG->getPixel(-1, h - 1) << 2;
      line2 = 0;
      line3 = GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY - 1);
      line4 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY);
      line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY) << 1;
      line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY)
               << 2;
      line5 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
      line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY + 1)
               << 1;
      for (FX_DWORD w = 0; w < GRW; w++) {
        CONTEXT = line5;
        CONTEXT |= line4 << 2;
        CONTEXT |= line3 << 5;
        CONTEXT |= line2 << 6;
        CONTEXT |= line1 << 7;
        bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
        GRREG->setPixel(w, h, bVal);
        line1 = ((line1 << 1) | GRREG->getPixel(w + 2, h - 1)) & 0x07;
        line2 = ((line2 << 1) | bVal) & 0x01;
        line3 = ((line3 << 1) |
                 GRREFERENCE->getPixel(w - GRREFERENCEDX + 1,
                                       h - GRREFERENCEDY - 1)) &
                0x01;
        line4 =
            ((line4 << 1) |
             GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY)) &
            0x07;
        line5 = ((line5 << 1) |
                 GRREFERENCE->getPixel(w - GRREFERENCEDX + 2,
                                       h - GRREFERENCEDY + 1)) &
                0x03;
      }
    } else {
      line1 = GRREG->getPixel(1, h - 1);
      line1 |= GRREG->getPixel(0, h - 1) << 1;
      line1 |= GRREG->getPixel(-1, h - 1) << 2;
      line2 = 0;
      line3 = GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY - 1);
      line4 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY);
      line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY) << 1;
      line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY)
               << 2;
      line5 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
      line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY + 1)
               << 1;
      for (FX_DWORD w = 0; w < GRW; w++) {
        bVal = GRREFERENCE->getPixel(w, h);
        if (!(TPGRON && (bVal == GRREFERENCE->getPixel(w - 1, h - 1)) &&
              (bVal == GRREFERENCE->getPixel(w, h - 1)) &&
              (bVal == GRREFERENCE->getPixel(w + 1, h - 1)) &&
              (bVal == GRREFERENCE->getPixel(w - 1, h)) &&
              (bVal == GRREFERENCE->getPixel(w + 1, h)) &&
              (bVal == GRREFERENCE->getPixel(w - 1, h + 1)) &&
              (bVal == GRREFERENCE->getPixel(w, h + 1)) &&
              (bVal == GRREFERENCE->getPixel(w + 1, h + 1)))) {
          CONTEXT = line5;
          CONTEXT |= line4 << 2;
          CONTEXT |= line3 << 5;
          CONTEXT |= line2 << 6;
          CONTEXT |= line1 << 7;
          bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
        }
        GRREG->setPixel(w, h, bVal);
        line1 = ((line1 << 1) | GRREG->getPixel(w + 2, h - 1)) & 0x07;
        line2 = ((line2 << 1) | bVal) & 0x01;
        line3 = ((line3 << 1) |
                 GRREFERENCE->getPixel(w - GRREFERENCEDX + 1,
                                       h - GRREFERENCEDY - 1)) &
                0x01;
        line4 =
            ((line4 << 1) |
             GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY)) &
            0x07;
        line5 = ((line5 << 1) |
                 GRREFERENCE->getPixel(w - GRREFERENCEDX + 2,
                                       h - GRREFERENCEDY + 1)) &
                0x03;
      }
    }
  }
  return GRREG.release();
}

CJBig2_Image* CJBig2_GRRDProc::decode_Template1_opt(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* grContext) {
  if (!GRREFERENCE->m_pData)
    return nullptr;

  FX_BOOL LTP, SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line1_r, line2_r, line3_r;
  uint8_t *pLine, *pLineR, cVal;
  intptr_t nStride, nStrideR, nOffset;
  int32_t k, nBits;
  int32_t GRWR, GRHR;
  int32_t GRW, GRH;
  GRW = (int32_t)CJBig2_GRRDProc::GRW;
  GRH = (int32_t)CJBig2_GRRDProc::GRH;
  LTP = 0;
  nonstd::unique_ptr<CJBig2_Image> GRREG(new CJBig2_Image(GRW, GRH));
  if (!GRREG->m_pData)
    return nullptr;

  pLine = GRREG->m_pData;
  pLineR = GRREFERENCE->m_pData;
  nStride = GRREG->m_nStride;
  nStrideR = GRREFERENCE->m_nStride;
  GRWR = (int32_t)GRREFERENCE->m_nWidth;
  GRHR = (int32_t)GRREFERENCE->m_nHeight;
  if (GRREFERENCEDY < -GRHR + 1 || GRREFERENCEDY > GRHR - 1) {
    GRREFERENCEDY = 0;
  }
  nOffset = -GRREFERENCEDY * nStrideR;
  for (int32_t h = 0; h < GRH; h++) {
    if (TPGRON) {
      SLTP = pArithDecoder->DECODE(&grContext[0x0008]);
      LTP = LTP ^ SLTP;
    }
    line1 = (h > 0) ? pLine[-nStride] << 1 : 0;
    int32_t reference_h = h - GRREFERENCEDY;
    FX_BOOL line1_r_ok = (reference_h > 0 && reference_h < GRHR + 1);
    FX_BOOL line2_r_ok = (reference_h > -1 && reference_h < GRHR);
    FX_BOOL line3_r_ok = (reference_h > -2 && reference_h < GRHR - 1);
    line1_r = line1_r_ok ? pLineR[nOffset - nStrideR] : 0;
    line2_r = line2_r_ok ? pLineR[nOffset] : 0;
    line3_r = line3_r_ok ? pLineR[nOffset + nStrideR] : 0;
    if (LTP == 0) {
      CONTEXT = (line1 & 0x0380) | ((line1_r >> 2) & 0x0020) |
                ((line2_r >> 4) & 0x001c) | ((line3_r >> 6) & 0x0003);
      for (int32_t w = 0; w < GRW; w += 8) {
        nBits = GRW - w > 8 ? 8 : GRW - w;
        if (h > 0)
          line1 = (line1 << 8) |
                  (w + 8 < GRW ? pLine[-nStride + (w >> 3) + 1] << 1 : 0);
        if (line1_r_ok)
          line1_r =
              (line1_r << 8) |
              (w + 8 < GRWR ? pLineR[nOffset - nStrideR + (w >> 3) + 1] : 0);
        if (line2_r_ok)
          line2_r = (line2_r << 8) |
                    (w + 8 < GRWR ? pLineR[nOffset + (w >> 3) + 1] : 0);
        if (line3_r_ok) {
          line3_r =
              (line3_r << 8) |
              (w + 8 < GRWR ? pLineR[nOffset + nStrideR + (w >> 3) + 1] : 0);
        } else {
          line3_r = 0;
        }
        cVal = 0;
        for (k = 0; k < nBits; k++) {
          bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x018d) << 1) | (bVal << 6) |
                    ((line1 >> (7 - k)) & 0x0080) |
                    ((line1_r >> (9 - k)) & 0x0020) |
                    ((line2_r >> (11 - k)) & 0x0004) |
                    ((line3_r >> (13 - k)) & 0x0001);
        }
        pLine[w >> 3] = cVal;
      }
    } else {
      CONTEXT = (line1 & 0x0380) | ((line1_r >> 2) & 0x0020) |
                ((line2_r >> 4) & 0x001c) | ((line3_r >> 6) & 0x0003);
      for (int32_t w = 0; w < GRW; w += 8) {
        nBits = GRW - w > 8 ? 8 : GRW - w;
        if (h > 0)
          line1 = (line1 << 8) |
                  (w + 8 < GRW ? pLine[-nStride + (w >> 3) + 1] << 1 : 0);
        if (line1_r_ok)
          line1_r =
              (line1_r << 8) |
              (w + 8 < GRWR ? pLineR[nOffset - nStrideR + (w >> 3) + 1] : 0);
        if (line2_r_ok)
          line2_r = (line2_r << 8) |
                    (w + 8 < GRWR ? pLineR[nOffset + (w >> 3) + 1] : 0);
        if (line3_r_ok) {
          line3_r =
              (line3_r << 8) |
              (w + 8 < GRWR ? pLineR[nOffset + nStrideR + (w >> 3) + 1] : 0);
        } else {
          line3_r = 0;
        }
        cVal = 0;
        for (k = 0; k < nBits; k++) {
          bVal = GRREFERENCE->getPixel(w + k, h);
          if (!(TPGRON && (bVal == GRREFERENCE->getPixel(w + k - 1, h - 1)) &&
                (bVal == GRREFERENCE->getPixel(w + k, h - 1)) &&
                (bVal == GRREFERENCE->getPixel(w + k + 1, h - 1)) &&
                (bVal == GRREFERENCE->getPixel(w + k - 1, h)) &&
                (bVal == GRREFERENCE->getPixel(w + k + 1, h)) &&
                (bVal == GRREFERENCE->getPixel(w + k - 1, h + 1)) &&
                (bVal == GRREFERENCE->getPixel(w + k, h + 1)) &&
                (bVal == GRREFERENCE->getPixel(w + k + 1, h + 1)))) {
            bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
          }
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x018d) << 1) | (bVal << 6) |
                    ((line1 >> (7 - k)) & 0x0080) |
                    ((line1_r >> (9 - k)) & 0x0020) |
                    ((line2_r >> (11 - k)) & 0x0004) |
                    ((line3_r >> (13 - k)) & 0x0001);
        }
        pLine[w >> 3] = cVal;
      }
    }
    pLine += nStride;
    if (h < GRHR + GRREFERENCEDY) {
      pLineR += nStrideR;
    }
  }
  return GRREG.release();
}

CJBig2_Image* CJBig2_TRDProc::decode_Huffman(CJBig2_BitStream* pStream,
                                             JBig2ArithCtx* grContext) {
  int32_t STRIPT, FIRSTS;
  FX_DWORD NINSTANCES;
  int32_t DT, DFS, CURS;
  uint8_t CURT;
  int32_t SI, TI;
  FX_DWORD IDI;
  CJBig2_Image* IBI;
  FX_DWORD WI, HI;
  int32_t IDS;
  FX_BOOL RI;
  int32_t RDWI, RDHI, RDXI, RDYI;
  CJBig2_Image* IBOI;
  FX_DWORD WOI, HOI;
  FX_BOOL bFirst;
  FX_DWORD nTmp;
  int32_t nVal, nBits;
  nonstd::unique_ptr<CJBig2_HuffmanDecoder> pHuffmanDecoder(
      new CJBig2_HuffmanDecoder(pStream));
  nonstd::unique_ptr<CJBig2_Image> SBREG(new CJBig2_Image(SBW, SBH));
  SBREG->fill(SBDEFPIXEL);
  if (pHuffmanDecoder->decodeAValue(SBHUFFDT, &STRIPT) != 0)
    return nullptr;

  STRIPT *= SBSTRIPS;
  STRIPT = -STRIPT;
  FIRSTS = 0;
  NINSTANCES = 0;
  while (NINSTANCES < SBNUMINSTANCES) {
    if (pHuffmanDecoder->decodeAValue(SBHUFFDT, &DT) != 0)
      return nullptr;

    DT *= SBSTRIPS;
    STRIPT = STRIPT + DT;
    bFirst = TRUE;
    for (;;) {
      if (bFirst) {
        if (pHuffmanDecoder->decodeAValue(SBHUFFFS, &DFS) != 0)
          return nullptr;

        FIRSTS = FIRSTS + DFS;
        CURS = FIRSTS;
        bFirst = FALSE;
      } else {
        nVal = pHuffmanDecoder->decodeAValue(SBHUFFDS, &IDS);
        if (nVal == JBIG2_OOB) {
          break;
        } else if (nVal != 0) {
          return nullptr;
        } else {
          CURS = CURS + IDS + SBDSOFFSET;
        }
      }
      if (SBSTRIPS == 1) {
        CURT = 0;
      } else {
        nTmp = 1;
        while ((FX_DWORD)(1 << nTmp) < SBSTRIPS) {
          nTmp++;
        }
        if (pStream->readNBits(nTmp, &nVal) != 0)
          return nullptr;

        CURT = nVal;
      }
      TI = STRIPT + CURT;
      nVal = 0;
      nBits = 0;
      for (;;) {
        if (pStream->read1Bit(&nTmp) != 0)
          return nullptr;

        nVal = (nVal << 1) | nTmp;
        nBits++;
        for (IDI = 0; IDI < SBNUMSYMS; IDI++) {
          if ((nBits == SBSYMCODES[IDI].codelen) &&
              (nVal == SBSYMCODES[IDI].code)) {
            break;
          }
        }
        if (IDI < SBNUMSYMS) {
          break;
        }
      }
      if (SBREFINE == 0) {
        RI = 0;
      } else {
        if (pStream->read1Bit(&RI) != 0) {
          return nullptr;
        }
      }
      if (RI == 0) {
        IBI = SBSYMS[IDI];
      } else {
        if ((pHuffmanDecoder->decodeAValue(SBHUFFRDW, &RDWI) != 0) ||
            (pHuffmanDecoder->decodeAValue(SBHUFFRDH, &RDHI) != 0) ||
            (pHuffmanDecoder->decodeAValue(SBHUFFRDX, &RDXI) != 0) ||
            (pHuffmanDecoder->decodeAValue(SBHUFFRDY, &RDYI) != 0) ||
            (pHuffmanDecoder->decodeAValue(SBHUFFRSIZE, &nVal) != 0)) {
          return nullptr;
        }
        pStream->alignByte();
        nTmp = pStream->getOffset();
        IBOI = SBSYMS[IDI];
        if (!IBOI)
          return nullptr;

        WOI = IBOI->m_nWidth;
        HOI = IBOI->m_nHeight;
        if ((int)(WOI + RDWI) < 0 || (int)(HOI + RDHI) < 0)
          return nullptr;

        nonstd::unique_ptr<CJBig2_GRRDProc> pGRRD(new CJBig2_GRRDProc());
        pGRRD->GRW = WOI + RDWI;
        pGRRD->GRH = HOI + RDHI;
        pGRRD->GRTEMPLATE = SBRTEMPLATE;
        pGRRD->GRREFERENCE = IBOI;
        pGRRD->GRREFERENCEDX = (RDWI >> 2) + RDXI;
        pGRRD->GRREFERENCEDY = (RDHI >> 2) + RDYI;
        pGRRD->TPGRON = 0;
        pGRRD->GRAT[0] = SBRAT[0];
        pGRRD->GRAT[1] = SBRAT[1];
        pGRRD->GRAT[2] = SBRAT[2];
        pGRRD->GRAT[3] = SBRAT[3];

        {
          nonstd::unique_ptr<CJBig2_ArithDecoder> pArithDecoder(
              new CJBig2_ArithDecoder(pStream));
          IBI = pGRRD->decode(pArithDecoder.get(), grContext);
          if (!IBI)
            return nullptr;
        }

        pStream->alignByte();
        pStream->offset(2);
        if ((FX_DWORD)nVal != (pStream->getOffset() - nTmp)) {
          delete IBI;
          return nullptr;
        }
      }
      if (!IBI) {
        continue;
      }
      WI = IBI->m_nWidth;
      HI = IBI->m_nHeight;
      if (TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPRIGHT) ||
                              (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
        CURS = CURS + WI - 1;
      } else if (TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_BOTTOMLEFT) ||
                                     (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
        CURS = CURS + HI - 1;
      }
      SI = CURS;
      if (TRANSPOSED == 0) {
        switch (REFCORNER) {
          case JBIG2_CORNER_TOPLEFT:
            SBREG->composeFrom(SI, TI, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_TOPRIGHT:
            SBREG->composeFrom(SI - WI + 1, TI, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMLEFT:
            SBREG->composeFrom(SI, TI - HI + 1, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMRIGHT:
            SBREG->composeFrom(SI - WI + 1, TI - HI + 1, IBI, SBCOMBOP);
            break;
        }
      } else {
        switch (REFCORNER) {
          case JBIG2_CORNER_TOPLEFT:
            SBREG->composeFrom(TI, SI, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_TOPRIGHT:
            SBREG->composeFrom(TI - WI + 1, SI, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMLEFT:
            SBREG->composeFrom(TI, SI - HI + 1, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMRIGHT:
            SBREG->composeFrom(TI - WI + 1, SI - HI + 1, IBI, SBCOMBOP);
            break;
        }
      }
      if (RI != 0) {
        delete IBI;
      }
      if (TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPLEFT) ||
                              (REFCORNER == JBIG2_CORNER_BOTTOMLEFT))) {
        CURS = CURS + WI - 1;
      } else if (TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_TOPLEFT) ||
                                     (REFCORNER == JBIG2_CORNER_TOPRIGHT))) {
        CURS = CURS + HI - 1;
      }
      NINSTANCES = NINSTANCES + 1;
    }
  }
  return SBREG.release();
}

CJBig2_Image* CJBig2_TRDProc::decode_Arith(CJBig2_ArithDecoder* pArithDecoder,
                                           JBig2ArithCtx* grContext,
                                           JBig2IntDecoderState* pIDS) {
  int32_t STRIPT, FIRSTS;
  FX_DWORD NINSTANCES;
  int32_t DT, DFS, CURS;
  int32_t CURT;
  int32_t SI, TI;
  FX_DWORD IDI;
  CJBig2_Image* IBI;
  FX_DWORD WI, HI;
  int32_t IDS;
  int RI;
  int32_t RDWI, RDHI, RDXI, RDYI;
  CJBig2_Image* IBOI;
  FX_DWORD WOI, HOI;
  FX_BOOL bFirst;
  int32_t nRet, nVal;
  int32_t bRetained;
  CJBig2_ArithIntDecoder *IADT, *IAFS, *IADS, *IAIT, *IARI, *IARDW, *IARDH,
      *IARDX, *IARDY;
  CJBig2_ArithIaidDecoder* IAID;
  if (pIDS) {
    IADT = pIDS->IADT;
    IAFS = pIDS->IAFS;
    IADS = pIDS->IADS;
    IAIT = pIDS->IAIT;
    IARI = pIDS->IARI;
    IARDW = pIDS->IARDW;
    IARDH = pIDS->IARDH;
    IARDX = pIDS->IARDX;
    IARDY = pIDS->IARDY;
    IAID = pIDS->IAID;
    bRetained = TRUE;
  } else {
    IADT = new CJBig2_ArithIntDecoder();
    IAFS = new CJBig2_ArithIntDecoder();
    IADS = new CJBig2_ArithIntDecoder();
    IAIT = new CJBig2_ArithIntDecoder();
    IARI = new CJBig2_ArithIntDecoder();
    IARDW = new CJBig2_ArithIntDecoder();
    IARDH = new CJBig2_ArithIntDecoder();
    IARDX = new CJBig2_ArithIntDecoder();
    IARDY = new CJBig2_ArithIntDecoder();
    IAID = new CJBig2_ArithIaidDecoder(SBSYMCODELEN);
    bRetained = FALSE;
  }
  nonstd::unique_ptr<CJBig2_Image> SBREG(new CJBig2_Image(SBW, SBH));
  SBREG->fill(SBDEFPIXEL);
  if (IADT->decode(pArithDecoder, &STRIPT) == -1) {
    goto failed;
  }
  STRIPT *= SBSTRIPS;
  STRIPT = -STRIPT;
  FIRSTS = 0;
  NINSTANCES = 0;
  while (NINSTANCES < SBNUMINSTANCES) {
    if (IADT->decode(pArithDecoder, &DT) == -1) {
      goto failed;
    }
    DT *= SBSTRIPS;
    STRIPT = STRIPT + DT;
    bFirst = TRUE;
    for (;;) {
      if (bFirst) {
        if (IAFS->decode(pArithDecoder, &DFS) == -1) {
          goto failed;
        }
        FIRSTS = FIRSTS + DFS;
        CURS = FIRSTS;
        bFirst = FALSE;
      } else {
        nRet = IADS->decode(pArithDecoder, &IDS);
        if (nRet == JBIG2_OOB) {
          break;
        } else if (nRet != 0) {
          goto failed;
        } else {
          CURS = CURS + IDS + SBDSOFFSET;
        }
      }
      if (NINSTANCES >= SBNUMINSTANCES) {
        break;
      }
      if (SBSTRIPS == 1) {
        CURT = 0;
      } else {
        if (IAIT->decode(pArithDecoder, &nVal) == -1) {
          goto failed;
        }
        CURT = nVal;
      }
      TI = STRIPT + CURT;
      if (IAID->decode(pArithDecoder, &nVal) == -1) {
        goto failed;
      }
      IDI = nVal;
      if (IDI >= SBNUMSYMS) {
        goto failed;
      }
      if (SBREFINE == 0) {
        RI = 0;
      } else {
        if (IARI->decode(pArithDecoder, &RI) == -1) {
          goto failed;
        }
      }
      if (!SBSYMS[IDI]) {
        goto failed;
      }
      if (RI == 0) {
        IBI = SBSYMS[IDI];
      } else {
        if ((IARDW->decode(pArithDecoder, &RDWI) == -1) ||
            (IARDH->decode(pArithDecoder, &RDHI) == -1) ||
            (IARDX->decode(pArithDecoder, &RDXI) == -1) ||
            (IARDY->decode(pArithDecoder, &RDYI) == -1)) {
          goto failed;
        }
        IBOI = SBSYMS[IDI];
        WOI = IBOI->m_nWidth;
        HOI = IBOI->m_nHeight;
        if ((int)(WOI + RDWI) < 0 || (int)(HOI + RDHI) < 0) {
          goto failed;
        }
        nonstd::unique_ptr<CJBig2_GRRDProc> pGRRD(new CJBig2_GRRDProc());
        pGRRD->GRW = WOI + RDWI;
        pGRRD->GRH = HOI + RDHI;
        pGRRD->GRTEMPLATE = SBRTEMPLATE;
        pGRRD->GRREFERENCE = IBOI;
        pGRRD->GRREFERENCEDX = (RDWI >> 1) + RDXI;
        pGRRD->GRREFERENCEDY = (RDHI >> 1) + RDYI;
        pGRRD->TPGRON = 0;
        pGRRD->GRAT[0] = SBRAT[0];
        pGRRD->GRAT[1] = SBRAT[1];
        pGRRD->GRAT[2] = SBRAT[2];
        pGRRD->GRAT[3] = SBRAT[3];
        IBI = pGRRD->decode(pArithDecoder, grContext);
        if (!IBI)
          goto failed;
      }
      WI = IBI->m_nWidth;
      HI = IBI->m_nHeight;
      if (TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPRIGHT) ||
                              (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
        CURS = CURS + WI - 1;
      } else if (TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_BOTTOMLEFT) ||
                                     (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
        CURS = CURS + HI - 1;
      }
      SI = CURS;
      if (TRANSPOSED == 0) {
        switch (REFCORNER) {
          case JBIG2_CORNER_TOPLEFT:
            SBREG->composeFrom(SI, TI, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_TOPRIGHT:
            SBREG->composeFrom(SI - WI + 1, TI, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMLEFT:
            SBREG->composeFrom(SI, TI - HI + 1, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMRIGHT:
            SBREG->composeFrom(SI - WI + 1, TI - HI + 1, IBI, SBCOMBOP);
            break;
        }
      } else {
        switch (REFCORNER) {
          case JBIG2_CORNER_TOPLEFT:
            SBREG->composeFrom(TI, SI, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_TOPRIGHT:
            SBREG->composeFrom(TI - WI + 1, SI, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMLEFT:
            SBREG->composeFrom(TI, SI - HI + 1, IBI, SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMRIGHT:
            SBREG->composeFrom(TI - WI + 1, SI - HI + 1, IBI, SBCOMBOP);
            break;
        }
      }
      if (RI != 0) {
        delete IBI;
      }
      if (TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPLEFT) ||
                              (REFCORNER == JBIG2_CORNER_BOTTOMLEFT))) {
        CURS = CURS + WI - 1;
      } else if (TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_TOPLEFT) ||
                                     (REFCORNER == JBIG2_CORNER_TOPRIGHT))) {
        CURS = CURS + HI - 1;
      }
      NINSTANCES = NINSTANCES + 1;
    }
  }
  if (bRetained == FALSE) {
    delete IADT;
    delete IAFS;
    delete IADS;
    delete IAIT;
    delete IARI;
    delete IARDW;
    delete IARDH;
    delete IARDX;
    delete IARDY;
    delete IAID;
  }
  return SBREG.release();
failed:
  if (bRetained == FALSE) {
    delete IADT;
    delete IAFS;
    delete IADS;
    delete IAIT;
    delete IARI;
    delete IARDW;
    delete IARDH;
    delete IARDX;
    delete IARDY;
    delete IAID;
  }
  return nullptr;
}

CJBig2_SymbolDict* CJBig2_SDDProc::decode_Arith(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    JBig2ArithCtx* grContext) {
  CJBig2_Image** SDNEWSYMS;
  FX_DWORD HCHEIGHT, NSYMSDECODED;
  int32_t HCDH;
  FX_DWORD SYMWIDTH, TOTWIDTH;
  int32_t DW;
  CJBig2_Image* BS;
  FX_DWORD I, J, REFAGGNINST;
  FX_BOOL* EXFLAGS;
  FX_DWORD EXINDEX;
  FX_BOOL CUREXFLAG;
  FX_DWORD EXRUNLENGTH;
  int32_t nVal;
  FX_DWORD nTmp;
  FX_DWORD SBNUMSYMS;
  uint8_t SBSYMCODELEN;
  FX_DWORD IDI;
  int32_t RDXI, RDYI;
  CJBig2_Image** SBSYMS;
  nonstd::unique_ptr<CJBig2_ArithIaidDecoder> IAID;
  nonstd::unique_ptr<CJBig2_SymbolDict> pDict;
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IADH(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IADW(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IAAI(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IARDX(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IARDY(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IAEX(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IADT(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IAFS(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IADS(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IAIT(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IARI(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IARDW(new CJBig2_ArithIntDecoder);
  nonstd::unique_ptr<CJBig2_ArithIntDecoder> IARDH(new CJBig2_ArithIntDecoder);
  nTmp = 0;
  while ((FX_DWORD)(1 << nTmp) < (SDNUMINSYMS + SDNUMNEWSYMS)) {
    nTmp++;
  }
  IAID.reset(new CJBig2_ArithIaidDecoder((uint8_t)nTmp));
  SDNEWSYMS = FX_Alloc(CJBig2_Image*, SDNUMNEWSYMS);
  FXSYS_memset(SDNEWSYMS, 0, SDNUMNEWSYMS * sizeof(CJBig2_Image*));

  HCHEIGHT = 0;
  NSYMSDECODED = 0;
  while (NSYMSDECODED < SDNUMNEWSYMS) {
    BS = nullptr;
    if (IADH->decode(pArithDecoder, &HCDH) == -1) {
      goto failed;
    }
    HCHEIGHT = HCHEIGHT + HCDH;
    if ((int)HCHEIGHT < 0 || (int)HCHEIGHT > JBIG2_MAX_IMAGE_SIZE) {
      goto failed;
    }
    SYMWIDTH = 0;
    TOTWIDTH = 0;
    for (;;) {
      nVal = IADW->decode(pArithDecoder, &DW);
      if (nVal == JBIG2_OOB) {
        break;
      } else if (nVal != 0) {
        goto failed;
      } else {
        if (NSYMSDECODED >= SDNUMNEWSYMS) {
          goto failed;
        }
        SYMWIDTH = SYMWIDTH + DW;
        if ((int)SYMWIDTH < 0 || (int)SYMWIDTH > JBIG2_MAX_IMAGE_SIZE) {
          goto failed;
        } else if (HCHEIGHT == 0 || SYMWIDTH == 0) {
          TOTWIDTH = TOTWIDTH + SYMWIDTH;
          SDNEWSYMS[NSYMSDECODED] = nullptr;
          NSYMSDECODED = NSYMSDECODED + 1;
          continue;
        }
        TOTWIDTH = TOTWIDTH + SYMWIDTH;
      }
      if (SDREFAGG == 0) {
        nonstd::unique_ptr<CJBig2_GRDProc> pGRD(new CJBig2_GRDProc());
        pGRD->MMR = 0;
        pGRD->GBW = SYMWIDTH;
        pGRD->GBH = HCHEIGHT;
        pGRD->GBTEMPLATE = SDTEMPLATE;
        pGRD->TPGDON = 0;
        pGRD->USESKIP = 0;
        pGRD->GBAT[0] = SDAT[0];
        pGRD->GBAT[1] = SDAT[1];
        pGRD->GBAT[2] = SDAT[2];
        pGRD->GBAT[3] = SDAT[3];
        pGRD->GBAT[4] = SDAT[4];
        pGRD->GBAT[5] = SDAT[5];
        pGRD->GBAT[6] = SDAT[6];
        pGRD->GBAT[7] = SDAT[7];
        BS = pGRD->decode_Arith(pArithDecoder, gbContext);
        if (!BS) {
          goto failed;
        }
      } else {
        if (IAAI->decode(pArithDecoder, (int*)&REFAGGNINST) == -1) {
          goto failed;
        }
        if (REFAGGNINST > 1) {
          nonstd::unique_ptr<CJBig2_TRDProc> pDecoder(new CJBig2_TRDProc());
          pDecoder->SBHUFF = SDHUFF;
          pDecoder->SBREFINE = 1;
          pDecoder->SBW = SYMWIDTH;
          pDecoder->SBH = HCHEIGHT;
          pDecoder->SBNUMINSTANCES = REFAGGNINST;
          pDecoder->SBSTRIPS = 1;
          pDecoder->SBNUMSYMS = SDNUMINSYMS + NSYMSDECODED;
          SBNUMSYMS = pDecoder->SBNUMSYMS;
          nTmp = 0;
          while ((FX_DWORD)(1 << nTmp) < SBNUMSYMS) {
            nTmp++;
          }
          SBSYMCODELEN = (uint8_t)nTmp;
          pDecoder->SBSYMCODELEN = SBSYMCODELEN;
          SBSYMS = FX_Alloc(CJBig2_Image*, SBNUMSYMS);
          JBIG2_memcpy(SBSYMS, SDINSYMS, SDNUMINSYMS * sizeof(CJBig2_Image*));
          JBIG2_memcpy(SBSYMS + SDNUMINSYMS, SDNEWSYMS,
                       NSYMSDECODED * sizeof(CJBig2_Image*));
          pDecoder->SBSYMS = SBSYMS;
          pDecoder->SBDEFPIXEL = 0;
          pDecoder->SBCOMBOP = JBIG2_COMPOSE_OR;
          pDecoder->TRANSPOSED = 0;
          pDecoder->REFCORNER = JBIG2_CORNER_TOPLEFT;
          pDecoder->SBDSOFFSET = 0;
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFFS(
              new CJBig2_HuffmanTable(HuffmanTable_B6,
                                      FX_ArraySize(HuffmanTable_B6),
                                      HuffmanTable_HTOOB_B6));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFDS(
              new CJBig2_HuffmanTable(HuffmanTable_B8,
                                      FX_ArraySize(HuffmanTable_B8),
                                      HuffmanTable_HTOOB_B8));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFDT(
              new CJBig2_HuffmanTable(HuffmanTable_B11,
                                      FX_ArraySize(HuffmanTable_B11),
                                      HuffmanTable_HTOOB_B11));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRDW(
              new CJBig2_HuffmanTable(HuffmanTable_B15,
                                      FX_ArraySize(HuffmanTable_B15),
                                      HuffmanTable_HTOOB_B15));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRDH(
              new CJBig2_HuffmanTable(HuffmanTable_B15,
                                      FX_ArraySize(HuffmanTable_B15),
                                      HuffmanTable_HTOOB_B15));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRDX(
              new CJBig2_HuffmanTable(HuffmanTable_B15,
                                      FX_ArraySize(HuffmanTable_B15),
                                      HuffmanTable_HTOOB_B15));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRDY(
              new CJBig2_HuffmanTable(HuffmanTable_B15,
                                      FX_ArraySize(HuffmanTable_B15),
                                      HuffmanTable_HTOOB_B15));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRSIZE(
              new CJBig2_HuffmanTable(HuffmanTable_B1,
                                      FX_ArraySize(HuffmanTable_B1),
                                      HuffmanTable_HTOOB_B1));
          pDecoder->SBHUFFFS = SBHUFFFS.get();
          pDecoder->SBHUFFDS = SBHUFFDS.get();
          pDecoder->SBHUFFDT = SBHUFFDT.get();
          pDecoder->SBHUFFRDW = SBHUFFRDW.get();
          pDecoder->SBHUFFRDH = SBHUFFRDH.get();
          pDecoder->SBHUFFRDX = SBHUFFRDX.get();
          pDecoder->SBHUFFRDY = SBHUFFRDY.get();
          pDecoder->SBHUFFRSIZE = SBHUFFRSIZE.get();
          pDecoder->SBRTEMPLATE = SDRTEMPLATE;
          pDecoder->SBRAT[0] = SDRAT[0];
          pDecoder->SBRAT[1] = SDRAT[1];
          pDecoder->SBRAT[2] = SDRAT[2];
          pDecoder->SBRAT[3] = SDRAT[3];
          JBig2IntDecoderState ids;
          ids.IADT = IADT.get();
          ids.IAFS = IAFS.get();
          ids.IADS = IADS.get();
          ids.IAIT = IAIT.get();
          ids.IARI = IARI.get();
          ids.IARDW = IARDW.get();
          ids.IARDH = IARDH.get();
          ids.IARDX = IARDX.get();
          ids.IARDY = IARDY.get();
          ids.IAID = IAID.get();
          BS = pDecoder->decode_Arith(pArithDecoder, grContext, &ids);
          if (!BS) {
            FX_Free(SBSYMS);
            goto failed;
          }
          FX_Free(SBSYMS);
        } else if (REFAGGNINST == 1) {
          SBNUMSYMS = SDNUMINSYMS + NSYMSDECODED;
          if (IAID->decode(pArithDecoder, (int*)&IDI) == -1) {
            goto failed;
          }
          if ((IARDX->decode(pArithDecoder, &RDXI) == -1) ||
              (IARDY->decode(pArithDecoder, &RDYI) == -1)) {
            goto failed;
          }
          if (IDI >= SBNUMSYMS) {
            goto failed;
          }
          SBSYMS = FX_Alloc(CJBig2_Image*, SBNUMSYMS);
          JBIG2_memcpy(SBSYMS, SDINSYMS, SDNUMINSYMS * sizeof(CJBig2_Image*));
          JBIG2_memcpy(SBSYMS + SDNUMINSYMS, SDNEWSYMS,
                       NSYMSDECODED * sizeof(CJBig2_Image*));
          if (!SBSYMS[IDI]) {
            FX_Free(SBSYMS);
            goto failed;
          }
          nonstd::unique_ptr<CJBig2_GRRDProc> pGRRD(new CJBig2_GRRDProc());
          pGRRD->GRW = SYMWIDTH;
          pGRRD->GRH = HCHEIGHT;
          pGRRD->GRTEMPLATE = SDRTEMPLATE;
          pGRRD->GRREFERENCE = SBSYMS[IDI];
          pGRRD->GRREFERENCEDX = RDXI;
          pGRRD->GRREFERENCEDY = RDYI;
          pGRRD->TPGRON = 0;
          pGRRD->GRAT[0] = SDRAT[0];
          pGRRD->GRAT[1] = SDRAT[1];
          pGRRD->GRAT[2] = SDRAT[2];
          pGRRD->GRAT[3] = SDRAT[3];
          BS = pGRRD->decode(pArithDecoder, grContext);
          if (!BS) {
            FX_Free(SBSYMS);
            goto failed;
          }
          FX_Free(SBSYMS);
        }
      }
      SDNEWSYMS[NSYMSDECODED] = BS;
      BS = nullptr;
      NSYMSDECODED = NSYMSDECODED + 1;
    }
  }
  EXINDEX = 0;
  CUREXFLAG = 0;
  EXFLAGS = FX_Alloc(FX_BOOL, SDNUMINSYMS + SDNUMNEWSYMS);
  while (EXINDEX < SDNUMINSYMS + SDNUMNEWSYMS) {
    if (IAEX->decode(pArithDecoder, (int*)&EXRUNLENGTH) == -1) {
      FX_Free(EXFLAGS);
      goto failed;
    }
    if (EXINDEX + EXRUNLENGTH > SDNUMINSYMS + SDNUMNEWSYMS) {
      FX_Free(EXFLAGS);
      goto failed;
    }
    if (EXRUNLENGTH != 0) {
      for (I = EXINDEX; I < EXINDEX + EXRUNLENGTH; I++) {
        EXFLAGS[I] = CUREXFLAG;
      }
    }
    EXINDEX = EXINDEX + EXRUNLENGTH;
    CUREXFLAG = !CUREXFLAG;
  }
  pDict.reset(new CJBig2_SymbolDict);
  pDict->SDNUMEXSYMS = SDNUMEXSYMS;
  pDict->SDEXSYMS = FX_Alloc(CJBig2_Image*, SDNUMEXSYMS);
  I = J = 0;
  for (I = 0; I < SDNUMINSYMS + SDNUMNEWSYMS; I++) {
    if (EXFLAGS[I] && J < SDNUMEXSYMS) {
      if (I < SDNUMINSYMS) {
        pDict->SDEXSYMS[J] = new CJBig2_Image(*SDINSYMS[I]);
      } else {
        pDict->SDEXSYMS[J] = SDNEWSYMS[I - SDNUMINSYMS];
      }
      J = J + 1;
    } else if (!EXFLAGS[I] && I >= SDNUMINSYMS) {
      delete SDNEWSYMS[I - SDNUMINSYMS];
    }
  }
  if (J < SDNUMEXSYMS) {
    pDict->SDNUMEXSYMS = J;
  }
  FX_Free(EXFLAGS);
  FX_Free(SDNEWSYMS);
  return pDict.release();
failed:
  for (I = 0; I < NSYMSDECODED; I++) {
    if (SDNEWSYMS[I]) {
      delete SDNEWSYMS[I];
      SDNEWSYMS[I] = nullptr;
    }
  }
  FX_Free(SDNEWSYMS);
  return nullptr;
}

CJBig2_SymbolDict* CJBig2_SDDProc::decode_Huffman(CJBig2_BitStream* pStream,
                                                  JBig2ArithCtx* gbContext,
                                                  JBig2ArithCtx* grContext,
                                                  IFX_Pause* pPause) {
  CJBig2_Image** SDNEWSYMS;
  FX_DWORD* SDNEWSYMWIDTHS;
  FX_DWORD HCHEIGHT, NSYMSDECODED;
  int32_t HCDH;
  FX_DWORD SYMWIDTH, TOTWIDTH, HCFIRSTSYM;
  int32_t DW;
  CJBig2_Image *BS, *BHC;
  FX_DWORD I, J, REFAGGNINST;
  FX_BOOL* EXFLAGS;
  FX_DWORD EXINDEX;
  FX_BOOL CUREXFLAG;
  FX_DWORD EXRUNLENGTH;
  int32_t nVal, nBits;
  FX_DWORD nTmp;
  FX_DWORD SBNUMSYMS;
  uint8_t SBSYMCODELEN;
  JBig2HuffmanCode* SBSYMCODES;
  FX_DWORD IDI;
  int32_t RDXI, RDYI;
  FX_DWORD BMSIZE;
  FX_DWORD stride;
  CJBig2_Image** SBSYMS;
  nonstd::unique_ptr<CJBig2_HuffmanDecoder> pHuffmanDecoder(
      new CJBig2_HuffmanDecoder(pStream));
  SDNEWSYMS = FX_Alloc(CJBig2_Image*, SDNUMNEWSYMS);
  FXSYS_memset(SDNEWSYMS, 0, SDNUMNEWSYMS * sizeof(CJBig2_Image*));
  SDNEWSYMWIDTHS = nullptr;
  BHC = nullptr;
  if (SDREFAGG == 0) {
    SDNEWSYMWIDTHS = FX_Alloc(FX_DWORD, SDNUMNEWSYMS);
    FXSYS_memset(SDNEWSYMWIDTHS, 0, SDNUMNEWSYMS * sizeof(FX_DWORD));
  }
  nonstd::unique_ptr<CJBig2_SymbolDict> pDict(new CJBig2_SymbolDict());
  nonstd::unique_ptr<CJBig2_HuffmanTable> pTable;

  HCHEIGHT = 0;
  NSYMSDECODED = 0;
  BS = nullptr;
  while (NSYMSDECODED < SDNUMNEWSYMS) {
    if (pHuffmanDecoder->decodeAValue(SDHUFFDH, &HCDH) != 0) {
      goto failed;
    }
    HCHEIGHT = HCHEIGHT + HCDH;
    if ((int)HCHEIGHT < 0 || (int)HCHEIGHT > JBIG2_MAX_IMAGE_SIZE) {
      goto failed;
    }
    SYMWIDTH = 0;
    TOTWIDTH = 0;
    HCFIRSTSYM = NSYMSDECODED;
    for (;;) {
      nVal = pHuffmanDecoder->decodeAValue(SDHUFFDW, &DW);
      if (nVal == JBIG2_OOB) {
        break;
      } else if (nVal != 0) {
        goto failed;
      } else {
        if (NSYMSDECODED >= SDNUMNEWSYMS) {
          goto failed;
        }
        SYMWIDTH = SYMWIDTH + DW;
        if ((int)SYMWIDTH < 0 || (int)SYMWIDTH > JBIG2_MAX_IMAGE_SIZE) {
          goto failed;
        } else if (HCHEIGHT == 0 || SYMWIDTH == 0) {
          TOTWIDTH = TOTWIDTH + SYMWIDTH;
          SDNEWSYMS[NSYMSDECODED] = nullptr;
          NSYMSDECODED = NSYMSDECODED + 1;
          continue;
        }
        TOTWIDTH = TOTWIDTH + SYMWIDTH;
      }
      if (SDREFAGG == 1) {
        if (pHuffmanDecoder->decodeAValue(SDHUFFAGGINST, (int*)&REFAGGNINST) !=
            0) {
          goto failed;
        }
        BS = nullptr;
        if (REFAGGNINST > 1) {
          nonstd::unique_ptr<CJBig2_TRDProc> pDecoder(new CJBig2_TRDProc());
          pDecoder->SBHUFF = SDHUFF;
          pDecoder->SBREFINE = 1;
          pDecoder->SBW = SYMWIDTH;
          pDecoder->SBH = HCHEIGHT;
          pDecoder->SBNUMINSTANCES = REFAGGNINST;
          pDecoder->SBSTRIPS = 1;
          pDecoder->SBNUMSYMS = SDNUMINSYMS + NSYMSDECODED;
          SBNUMSYMS = pDecoder->SBNUMSYMS;
          SBSYMCODES = FX_Alloc(JBig2HuffmanCode, SBNUMSYMS);
          nTmp = 1;
          while ((FX_DWORD)(1 << nTmp) < SBNUMSYMS) {
            nTmp++;
          }
          for (I = 0; I < SBNUMSYMS; I++) {
            SBSYMCODES[I].codelen = nTmp;
            SBSYMCODES[I].code = I;
          }
          pDecoder->SBSYMCODES = SBSYMCODES;
          SBSYMS = FX_Alloc(CJBig2_Image*, SBNUMSYMS);
          JBIG2_memcpy(SBSYMS, SDINSYMS, SDNUMINSYMS * sizeof(CJBig2_Image*));
          JBIG2_memcpy(SBSYMS + SDNUMINSYMS, SDNEWSYMS,
                       NSYMSDECODED * sizeof(CJBig2_Image*));
          pDecoder->SBSYMS = SBSYMS;
          pDecoder->SBDEFPIXEL = 0;
          pDecoder->SBCOMBOP = JBIG2_COMPOSE_OR;
          pDecoder->TRANSPOSED = 0;
          pDecoder->REFCORNER = JBIG2_CORNER_TOPLEFT;
          pDecoder->SBDSOFFSET = 0;
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFFS(
              new CJBig2_HuffmanTable(HuffmanTable_B6,
                                      FX_ArraySize(HuffmanTable_B6),
                                      HuffmanTable_HTOOB_B6));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFDS(
              new CJBig2_HuffmanTable(HuffmanTable_B8,
                                      FX_ArraySize(HuffmanTable_B8),
                                      HuffmanTable_HTOOB_B8));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFDT(
              new CJBig2_HuffmanTable(HuffmanTable_B11,
                                      FX_ArraySize(HuffmanTable_B11),
                                      HuffmanTable_HTOOB_B11));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRDW(
              new CJBig2_HuffmanTable(HuffmanTable_B15,
                                      FX_ArraySize(HuffmanTable_B15),
                                      HuffmanTable_HTOOB_B15));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRDH(
              new CJBig2_HuffmanTable(HuffmanTable_B15,
                                      FX_ArraySize(HuffmanTable_B15),
                                      HuffmanTable_HTOOB_B15));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRDX(
              new CJBig2_HuffmanTable(HuffmanTable_B15,
                                      FX_ArraySize(HuffmanTable_B15),
                                      HuffmanTable_HTOOB_B15));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRDY(
              new CJBig2_HuffmanTable(HuffmanTable_B15,
                                      FX_ArraySize(HuffmanTable_B15),
                                      HuffmanTable_HTOOB_B15));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRSIZE(
              new CJBig2_HuffmanTable(HuffmanTable_B1,
                                      FX_ArraySize(HuffmanTable_B1),
                                      HuffmanTable_HTOOB_B1));
          pDecoder->SBHUFFFS = SBHUFFFS.get();
          pDecoder->SBHUFFDS = SBHUFFDS.get();
          pDecoder->SBHUFFDT = SBHUFFDT.get();
          pDecoder->SBHUFFRDW = SBHUFFRDW.get();
          pDecoder->SBHUFFRDH = SBHUFFRDH.get();
          pDecoder->SBHUFFRDX = SBHUFFRDX.get();
          pDecoder->SBHUFFRDY = SBHUFFRDY.get();
          pDecoder->SBHUFFRSIZE = SBHUFFRSIZE.get();
          pDecoder->SBRTEMPLATE = SDRTEMPLATE;
          pDecoder->SBRAT[0] = SDRAT[0];
          pDecoder->SBRAT[1] = SDRAT[1];
          pDecoder->SBRAT[2] = SDRAT[2];
          pDecoder->SBRAT[3] = SDRAT[3];
          BS = pDecoder->decode_Huffman(pStream, grContext);
          if (!BS) {
            FX_Free(SBSYMCODES);
            FX_Free(SBSYMS);
            goto failed;
          }
          FX_Free(SBSYMCODES);
          FX_Free(SBSYMS);
        } else if (REFAGGNINST == 1) {
          SBNUMSYMS = SDNUMINSYMS + SDNUMNEWSYMS;
          nTmp = 1;
          while ((FX_DWORD)(1 << nTmp) < SBNUMSYMS) {
            nTmp++;
          }
          SBSYMCODELEN = (uint8_t)nTmp;
          SBSYMCODES = FX_Alloc(JBig2HuffmanCode, SBNUMSYMS);
          for (I = 0; I < SBNUMSYMS; I++) {
            SBSYMCODES[I].codelen = SBSYMCODELEN;
            SBSYMCODES[I].code = I;
          }
          nVal = 0;
          nBits = 0;
          for (;;) {
            if (pStream->read1Bit(&nTmp) != 0) {
              FX_Free(SBSYMCODES);
              goto failed;
            }
            nVal = (nVal << 1) | nTmp;
            for (IDI = 0; IDI < SBNUMSYMS; IDI++) {
              if ((nVal == SBSYMCODES[IDI].code) &&
                  (nBits == SBSYMCODES[IDI].codelen)) {
                break;
              }
            }
            if (IDI < SBNUMSYMS) {
              break;
            }
          }
          FX_Free(SBSYMCODES);
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRDX(
              new CJBig2_HuffmanTable(HuffmanTable_B15,
                                      FX_ArraySize(HuffmanTable_B15),
                                      HuffmanTable_HTOOB_B15));
          nonstd::unique_ptr<CJBig2_HuffmanTable> SBHUFFRSIZE(
              new CJBig2_HuffmanTable(HuffmanTable_B1,
                                      FX_ArraySize(HuffmanTable_B1),
                                      HuffmanTable_HTOOB_B1));
          if ((pHuffmanDecoder->decodeAValue(SBHUFFRDX.get(), &RDXI) != 0) ||
              (pHuffmanDecoder->decodeAValue(SBHUFFRDX.get(), &RDYI) != 0) ||
              (pHuffmanDecoder->decodeAValue(SBHUFFRSIZE.get(), &nVal) != 0)) {
            goto failed;
          }
          pStream->alignByte();
          nTmp = pStream->getOffset();
          SBSYMS = FX_Alloc(CJBig2_Image*, SBNUMSYMS);
          JBIG2_memcpy(SBSYMS, SDINSYMS, SDNUMINSYMS * sizeof(CJBig2_Image*));
          JBIG2_memcpy(SBSYMS + SDNUMINSYMS, SDNEWSYMS,
                       NSYMSDECODED * sizeof(CJBig2_Image*));
          nonstd::unique_ptr<CJBig2_GRRDProc> pGRRD(new CJBig2_GRRDProc());
          pGRRD->GRW = SYMWIDTH;
          pGRRD->GRH = HCHEIGHT;
          pGRRD->GRTEMPLATE = SDRTEMPLATE;
          pGRRD->GRREFERENCE = SBSYMS[IDI];
          pGRRD->GRREFERENCEDX = RDXI;
          pGRRD->GRREFERENCEDY = RDYI;
          pGRRD->TPGRON = 0;
          pGRRD->GRAT[0] = SDRAT[0];
          pGRRD->GRAT[1] = SDRAT[1];
          pGRRD->GRAT[2] = SDRAT[2];
          pGRRD->GRAT[3] = SDRAT[3];
          nonstd::unique_ptr<CJBig2_ArithDecoder> pArithDecoder(
              new CJBig2_ArithDecoder(pStream));
          BS = pGRRD->decode(pArithDecoder.get(), grContext);
          if (!BS) {
            FX_Free(SBSYMS);
            goto failed;
          }
          pStream->alignByte();
          pStream->offset(2);
          if ((FX_DWORD)nVal != (pStream->getOffset() - nTmp)) {
            delete BS;
            FX_Free(SBSYMS);
            goto failed;
          }
          FX_Free(SBSYMS);
        }
        SDNEWSYMS[NSYMSDECODED] = BS;
      }
      if (SDREFAGG == 0) {
        SDNEWSYMWIDTHS[NSYMSDECODED] = SYMWIDTH;
      }
      NSYMSDECODED = NSYMSDECODED + 1;
    }
    if (SDREFAGG == 0) {
      if (pHuffmanDecoder->decodeAValue(SDHUFFBMSIZE, (int32_t*)&BMSIZE) != 0) {
        goto failed;
      }
      pStream->alignByte();
      if (BMSIZE == 0) {
        stride = (TOTWIDTH + 7) >> 3;
        if (pStream->getByteLeft() >= stride * HCHEIGHT) {
          BHC = new CJBig2_Image(TOTWIDTH, HCHEIGHT);
          for (I = 0; I < HCHEIGHT; I++) {
            JBIG2_memcpy(BHC->m_pData + I * BHC->m_nStride,
                         pStream->getPointer(), stride);
            pStream->offset(stride);
          }
        } else {
          goto failed;
        }
      } else {
        nonstd::unique_ptr<CJBig2_GRDProc> pGRD(new CJBig2_GRDProc());
        pGRD->MMR = 1;
        pGRD->GBW = TOTWIDTH;
        pGRD->GBH = HCHEIGHT;
        FXCODEC_STATUS status = pGRD->Start_decode_MMR(&BHC, pStream);
        while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
          pGRD->Continue_decode(pPause);
        }
        pStream->alignByte();
      }
      nTmp = 0;
      if (!BHC) {
        continue;
      }
      for (I = HCFIRSTSYM; I < NSYMSDECODED; I++) {
        SDNEWSYMS[I] = BHC->subImage(nTmp, 0, SDNEWSYMWIDTHS[I], HCHEIGHT);
        nTmp += SDNEWSYMWIDTHS[I];
      }
      delete BHC;
      BHC = nullptr;
    }
  }
  EXINDEX = 0;
  CUREXFLAG = 0;
  pTable.reset(new CJBig2_HuffmanTable(
      HuffmanTable_B1, FX_ArraySize(HuffmanTable_B1), HuffmanTable_HTOOB_B1));
  EXFLAGS = FX_Alloc(FX_BOOL, SDNUMINSYMS + SDNUMNEWSYMS);
  while (EXINDEX < SDNUMINSYMS + SDNUMNEWSYMS) {
    if (pHuffmanDecoder->decodeAValue(pTable.get(), (int*)&EXRUNLENGTH) != 0) {
      FX_Free(EXFLAGS);
      goto failed;
    }
    if (EXINDEX + EXRUNLENGTH > SDNUMINSYMS + SDNUMNEWSYMS) {
      FX_Free(EXFLAGS);
      goto failed;
    }
    if (EXRUNLENGTH != 0) {
      for (I = EXINDEX; I < EXINDEX + EXRUNLENGTH; I++) {
        EXFLAGS[I] = CUREXFLAG;
      }
    }
    EXINDEX = EXINDEX + EXRUNLENGTH;
    CUREXFLAG = !CUREXFLAG;
  }
  pDict->SDNUMEXSYMS = SDNUMEXSYMS;
  pDict->SDEXSYMS = FX_Alloc(CJBig2_Image*, SDNUMEXSYMS);
  I = J = 0;
  for (I = 0; I < SDNUMINSYMS + SDNUMNEWSYMS; I++) {
    if (EXFLAGS[I] && J < SDNUMEXSYMS) {
      if (I < SDNUMINSYMS) {
        pDict->SDEXSYMS[J] = new CJBig2_Image(*SDINSYMS[I]);
      } else {
        pDict->SDEXSYMS[J] = SDNEWSYMS[I - SDNUMINSYMS];
      }
      J = J + 1;
    } else if (!EXFLAGS[I] && I >= SDNUMINSYMS) {
      delete SDNEWSYMS[I - SDNUMINSYMS];
    }
  }
  if (J < SDNUMEXSYMS) {
    pDict->SDNUMEXSYMS = J;
  }
  FX_Free(EXFLAGS);
  FX_Free(SDNEWSYMS);
  if (SDREFAGG == 0) {
    FX_Free(SDNEWSYMWIDTHS);
  }
  return pDict.release();
failed:
  for (I = 0; I < NSYMSDECODED; I++) {
    delete SDNEWSYMS[I];
  }
  FX_Free(SDNEWSYMS);
  if (SDREFAGG == 0) {
    FX_Free(SDNEWSYMWIDTHS);
  }
  return nullptr;
}

CJBig2_Image* CJBig2_HTRDProc::decode_Arith(CJBig2_ArithDecoder* pArithDecoder,
                                            JBig2ArithCtx* gbContext,
                                            IFX_Pause* pPause) {
  FX_DWORD ng, mg;
  int32_t x, y;
  FX_DWORD HBPP;
  FX_DWORD* GI;
  nonstd::unique_ptr<CJBig2_Image> HSKIP;
  nonstd::unique_ptr<CJBig2_Image> HTREG(new CJBig2_Image(HBW, HBH));
  HTREG->fill(HDEFPIXEL);
  if (HENABLESKIP == 1) {
    HSKIP.reset(new CJBig2_Image(HGW, HGH));
    for (mg = 0; mg < HGH; mg++) {
      for (ng = 0; ng < HGW; ng++) {
        x = (HGX + mg * HRY + ng * HRX) >> 8;
        y = (HGY + mg * HRX - ng * HRY) >> 8;
        if ((x + HPW <= 0) | (x >= (int32_t)HBW) | (y + HPH <= 0) |
            (y >= (int32_t)HPH)) {
          HSKIP->setPixel(ng, mg, 1);
        } else {
          HSKIP->setPixel(ng, mg, 0);
        }
      }
    }
  }
  HBPP = 1;
  while ((FX_DWORD)(1 << HBPP) < HNUMPATS) {
    HBPP++;
  }
  nonstd::unique_ptr<CJBig2_GSIDProc> pGID(new CJBig2_GSIDProc());
  pGID->GSMMR = HMMR;
  pGID->GSW = HGW;
  pGID->GSH = HGH;
  pGID->GSBPP = (uint8_t)HBPP;
  pGID->GSUSESKIP = HENABLESKIP;
  pGID->GSKIP = HSKIP.get();
  pGID->GSTEMPLATE = HTEMPLATE;
  GI = pGID->decode_Arith(pArithDecoder, gbContext, pPause);
  if (!GI)
    return nullptr;

  for (mg = 0; mg < HGH; mg++) {
    for (ng = 0; ng < HGW; ng++) {
      x = (HGX + mg * HRY + ng * HRX) >> 8;
      y = (HGY + mg * HRX - ng * HRY) >> 8;
      FX_DWORD pat_index = GI[mg * HGW + ng];
      if (pat_index >= HNUMPATS) {
        pat_index = HNUMPATS - 1;
      }
      HTREG->composeFrom(x, y, HPATS[pat_index], HCOMBOP);
    }
  }
  FX_Free(GI);
  return HTREG.release();
}

CJBig2_Image* CJBig2_HTRDProc::decode_MMR(CJBig2_BitStream* pStream,
                                          IFX_Pause* pPause) {
  FX_DWORD ng, mg;
  int32_t x, y;
  FX_DWORD* GI;
  nonstd::unique_ptr<CJBig2_Image> HTREG(new CJBig2_Image(HBW, HBH));
  HTREG->fill(HDEFPIXEL);
  FX_DWORD HBPP = 1;
  while ((FX_DWORD)(1 << HBPP) < HNUMPATS) {
    HBPP++;
  }
  nonstd::unique_ptr<CJBig2_GSIDProc> pGID(new CJBig2_GSIDProc());
  pGID->GSMMR = HMMR;
  pGID->GSW = HGW;
  pGID->GSH = HGH;
  pGID->GSBPP = (uint8_t)HBPP;
  pGID->GSUSESKIP = 0;
  GI = pGID->decode_MMR(pStream, pPause);
  if (!GI)
    return nullptr;

  for (mg = 0; mg < HGH; mg++) {
    for (ng = 0; ng < HGW; ng++) {
      x = (HGX + mg * HRY + ng * HRX) >> 8;
      y = (HGY + mg * HRX - ng * HRY) >> 8;
      FX_DWORD pat_index = GI[mg * HGW + ng];
      if (pat_index >= HNUMPATS) {
        pat_index = HNUMPATS - 1;
      }
      HTREG->composeFrom(x, y, HPATS[pat_index], HCOMBOP);
    }
  }
  FX_Free(GI);
  return HTREG.release();
}

CJBig2_PatternDict* CJBig2_PDDProc::decode_Arith(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    IFX_Pause* pPause) {
  FX_DWORD GRAY;
  CJBig2_Image* BHDC = nullptr;
  nonstd::unique_ptr<CJBig2_PatternDict> pDict(new CJBig2_PatternDict());
  pDict->NUMPATS = GRAYMAX + 1;
  pDict->HDPATS = FX_Alloc(CJBig2_Image*, pDict->NUMPATS);
  JBIG2_memset(pDict->HDPATS, 0, sizeof(CJBig2_Image*) * pDict->NUMPATS);

  nonstd::unique_ptr<CJBig2_GRDProc> pGRD(new CJBig2_GRDProc());
  pGRD->MMR = HDMMR;
  pGRD->GBW = (GRAYMAX + 1) * HDPW;
  pGRD->GBH = HDPH;
  pGRD->GBTEMPLATE = HDTEMPLATE;
  pGRD->TPGDON = 0;
  pGRD->USESKIP = 0;
  pGRD->GBAT[0] = -(int32_t)HDPW;
  pGRD->GBAT[1] = 0;
  if (pGRD->GBTEMPLATE == 0) {
    pGRD->GBAT[2] = -3;
    pGRD->GBAT[3] = -1;
    pGRD->GBAT[4] = 2;
    pGRD->GBAT[5] = -2;
    pGRD->GBAT[6] = -2;
    pGRD->GBAT[7] = -2;
  }
  FXCODEC_STATUS status =
      pGRD->Start_decode_Arith(&BHDC, pArithDecoder, gbContext);
  while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
    pGRD->Continue_decode(pPause);
  }
  if (!BHDC)
    return nullptr;

  GRAY = 0;
  while (GRAY <= GRAYMAX) {
    pDict->HDPATS[GRAY] = BHDC->subImage(HDPW * GRAY, 0, HDPW, HDPH);
    GRAY = GRAY + 1;
  }
  delete BHDC;
  return pDict.release();
}

CJBig2_PatternDict* CJBig2_PDDProc::decode_MMR(CJBig2_BitStream* pStream,
                                               IFX_Pause* pPause) {
  FX_DWORD GRAY;
  CJBig2_Image* BHDC = nullptr;
  nonstd::unique_ptr<CJBig2_PatternDict> pDict(new CJBig2_PatternDict());
  pDict->NUMPATS = GRAYMAX + 1;
  pDict->HDPATS = FX_Alloc(CJBig2_Image*, pDict->NUMPATS);
  JBIG2_memset(pDict->HDPATS, 0, sizeof(CJBig2_Image*) * pDict->NUMPATS);

  nonstd::unique_ptr<CJBig2_GRDProc> pGRD(new CJBig2_GRDProc());
  pGRD->MMR = HDMMR;
  pGRD->GBW = (GRAYMAX + 1) * HDPW;
  pGRD->GBH = HDPH;
  FXCODEC_STATUS status = pGRD->Start_decode_MMR(&BHDC, pStream);
  while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
    pGRD->Continue_decode(pPause);
  }
  if (!BHDC)
    return nullptr;

  GRAY = 0;
  while (GRAY <= GRAYMAX) {
    pDict->HDPATS[GRAY] = BHDC->subImage(HDPW * GRAY, 0, HDPW, HDPH);
    GRAY = GRAY + 1;
  }
  delete BHDC;
  return pDict.release();
}

FX_DWORD* CJBig2_GSIDProc::decode_Arith(CJBig2_ArithDecoder* pArithDecoder,
                                        JBig2ArithCtx* gbContext,
                                        IFX_Pause* pPause) {
  CJBig2_Image** GSPLANES;
  int32_t J, K;
  FX_DWORD x, y;
  FX_DWORD* GSVALS;
  GSPLANES = FX_Alloc(CJBig2_Image*, GSBPP);
  GSVALS = FX_Alloc2D(FX_DWORD, GSW, GSH);
  JBIG2_memset(GSPLANES, 0, sizeof(CJBig2_Image*) * GSBPP);
  JBIG2_memset(GSVALS, 0, sizeof(FX_DWORD) * GSW * GSH);

  nonstd::unique_ptr<CJBig2_GRDProc> pGRD(new CJBig2_GRDProc());
  pGRD->MMR = GSMMR;
  pGRD->GBW = GSW;
  pGRD->GBH = GSH;
  pGRD->GBTEMPLATE = GSTEMPLATE;
  pGRD->TPGDON = 0;
  pGRD->USESKIP = GSUSESKIP;
  pGRD->SKIP = GSKIP;
  if (GSTEMPLATE <= 1) {
    pGRD->GBAT[0] = 3;
  } else {
    pGRD->GBAT[0] = 2;
  }
  pGRD->GBAT[1] = -1;
  if (pGRD->GBTEMPLATE == 0) {
    pGRD->GBAT[2] = -3;
    pGRD->GBAT[3] = -1;
    pGRD->GBAT[4] = 2;
    pGRD->GBAT[5] = -2;
    pGRD->GBAT[6] = -2;
    pGRD->GBAT[7] = -2;
  }
  FXCODEC_STATUS status =
      pGRD->Start_decode_Arith(&GSPLANES[GSBPP - 1], pArithDecoder, gbContext);
  while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
    pGRD->Continue_decode(pPause);
  }
  if (!GSPLANES[GSBPP - 1]) {
    goto failed;
  }
  J = GSBPP - 2;
  while (J >= 0) {
    FXCODEC_STATUS status =
        pGRD->Start_decode_Arith(&GSPLANES[J], pArithDecoder, gbContext);
    while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
      pGRD->Continue_decode(pPause);
    }
    if (!GSPLANES[J]) {
      for (K = GSBPP - 1; K > J; K--) {
        delete GSPLANES[K];
        goto failed;
      }
    }
    GSPLANES[J]->composeFrom(0, 0, GSPLANES[J + 1], JBIG2_COMPOSE_XOR);
    J = J - 1;
  }
  for (y = 0; y < GSH; y++) {
    for (x = 0; x < GSW; x++) {
      for (J = 0; J < GSBPP; J++) {
        GSVALS[y * GSW + x] |= GSPLANES[J]->getPixel(x, y) << J;
      }
    }
  }
  for (J = 0; J < GSBPP; J++) {
    delete GSPLANES[J];
  }
  FX_Free(GSPLANES);
  return GSVALS;
failed:
  FX_Free(GSPLANES);
  FX_Free(GSVALS);
  return nullptr;
}

FX_DWORD* CJBig2_GSIDProc::decode_MMR(CJBig2_BitStream* pStream,
                                      IFX_Pause* pPause) {
  CJBig2_Image** GSPLANES;
  int32_t J, K;
  FX_DWORD x, y;
  FX_DWORD* GSVALS;
  GSPLANES = FX_Alloc(CJBig2_Image*, GSBPP);
  GSVALS = FX_Alloc2D(FX_DWORD, GSW, GSH);
  JBIG2_memset(GSPLANES, 0, sizeof(CJBig2_Image*) * GSBPP);
  JBIG2_memset(GSVALS, 0, sizeof(FX_DWORD) * GSW * GSH);

  nonstd::unique_ptr<CJBig2_GRDProc> pGRD(new CJBig2_GRDProc());
  pGRD->MMR = GSMMR;
  pGRD->GBW = GSW;
  pGRD->GBH = GSH;
  FXCODEC_STATUS status = pGRD->Start_decode_MMR(&GSPLANES[GSBPP - 1], pStream);
  while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
    pGRD->Continue_decode(pPause);
  }
  if (!GSPLANES[GSBPP - 1]) {
    goto failed;
  }
  pStream->alignByte();
  pStream->offset(3);
  J = GSBPP - 2;
  while (J >= 0) {
    FXCODEC_STATUS status = pGRD->Start_decode_MMR(&GSPLANES[J], pStream);
    while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
      pGRD->Continue_decode(pPause);
    }
    if (!GSPLANES[J]) {
      for (K = GSBPP - 1; K > J; K--) {
        delete GSPLANES[K];
        goto failed;
      }
    }
    pStream->alignByte();
    pStream->offset(3);
    GSPLANES[J]->composeFrom(0, 0, GSPLANES[J + 1], JBIG2_COMPOSE_XOR);
    J = J - 1;
  }
  for (y = 0; y < GSH; y++) {
    for (x = 0; x < GSW; x++) {
      for (J = 0; J < GSBPP; J++) {
        GSVALS[y * GSW + x] |= GSPLANES[J]->getPixel(x, y) << J;
      }
    }
  }
  for (J = 0; J < GSBPP; J++) {
    delete GSPLANES[J];
  }
  FX_Free(GSPLANES);
  return GSVALS;
failed:
  FX_Free(GSPLANES);
  FX_Free(GSVALS);
  return nullptr;
}

FXCODEC_STATUS CJBig2_GRDProc::Start_decode_Arith(
    CJBig2_Image** pImage,
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    IFX_Pause* pPause) {
  if (GBW == 0 || GBH == 0) {
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return FXCODEC_STATUS_DECODE_FINISH;
  }
  m_ProssiveStatus = FXCODEC_STATUS_DECODE_READY;
  m_pPause = pPause;
  if (!*pImage)
    *pImage = new CJBig2_Image(GBW, GBH);
  if (!(*pImage)->m_pData) {
    delete *pImage;
    *pImage = nullptr;
    m_ProssiveStatus = FXCODEC_STATUS_ERROR;
    return FXCODEC_STATUS_ERROR;
  }
  m_DecodeType = 1;
  m_pImage = pImage;
  (*m_pImage)->fill(0);
  m_pArithDecoder = pArithDecoder;
  m_gbContext = gbContext;
  LTP = 0;
  m_pLine = nullptr;
  m_loopIndex = 0;
  return decode_Arith(pPause);
}

FXCODEC_STATUS CJBig2_GRDProc::decode_Arith(IFX_Pause* pPause) {
  int iline = m_loopIndex;
  CJBig2_Image* pImage = *m_pImage;
  if (GBTEMPLATE == 0) {
    if (UseTemplate0Opt3()) {
      m_ProssiveStatus = decode_Arith_Template0_opt3(pImage, m_pArithDecoder,
                                                     m_gbContext, pPause);
    } else {
      m_ProssiveStatus = decode_Arith_Template0_unopt(pImage, m_pArithDecoder,
                                                      m_gbContext, pPause);
    }
  } else if (GBTEMPLATE == 1) {
    if (UseTemplate1Opt3()) {
      m_ProssiveStatus = decode_Arith_Template1_opt3(pImage, m_pArithDecoder,
                                                     m_gbContext, pPause);
    } else {
      m_ProssiveStatus = decode_Arith_Template1_unopt(pImage, m_pArithDecoder,
                                                      m_gbContext, pPause);
    }
  } else if (GBTEMPLATE == 2) {
    if (UseTemplate23Opt3()) {
      m_ProssiveStatus = decode_Arith_Template2_opt3(pImage, m_pArithDecoder,
                                                     m_gbContext, pPause);
    } else {
      m_ProssiveStatus = decode_Arith_Template2_unopt(pImage, m_pArithDecoder,
                                                      m_gbContext, pPause);
    }
  } else {
    if (UseTemplate23Opt3()) {
      m_ProssiveStatus = decode_Arith_Template3_opt3(pImage, m_pArithDecoder,
                                                     m_gbContext, pPause);
    } else {
      m_ProssiveStatus = decode_Arith_Template3_unopt(pImage, m_pArithDecoder,
                                                      m_gbContext, pPause);
    }
  }
  m_ReplaceRect.left = 0;
  m_ReplaceRect.right = pImage->m_nWidth;
  m_ReplaceRect.top = iline;
  m_ReplaceRect.bottom = m_loopIndex;
  if (m_ProssiveStatus == FXCODEC_STATUS_DECODE_FINISH) {
    m_loopIndex = 0;
  }
  return m_ProssiveStatus;
}

FXCODEC_STATUS CJBig2_GRDProc::Start_decode_MMR(CJBig2_Image** pImage,
                                                CJBig2_BitStream* pStream,
                                                IFX_Pause* pPause) {
  int bitpos, i;
  *pImage = new CJBig2_Image(GBW, GBH);
  if (!(*pImage)->m_pData) {
    delete (*pImage);
    (*pImage) = nullptr;
    m_ProssiveStatus = FXCODEC_STATUS_ERROR;
    return m_ProssiveStatus;
  }
  bitpos = (int)pStream->getBitPos();
  _FaxG4Decode(pStream->getBuf(), pStream->getLength(), &bitpos,
               (*pImage)->m_pData, GBW, GBH, (*pImage)->m_nStride);
  pStream->setBitPos(bitpos);
  for (i = 0; (FX_DWORD)i < (*pImage)->m_nStride * GBH; i++) {
    (*pImage)->m_pData[i] = ~(*pImage)->m_pData[i];
  }
  m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
  return m_ProssiveStatus;
}

FXCODEC_STATUS CJBig2_GRDProc::Continue_decode(IFX_Pause* pPause) {
  if (m_ProssiveStatus != FXCODEC_STATUS_DECODE_TOBECONTINUE)
    return m_ProssiveStatus;

  if (m_DecodeType != 1) {
    m_ProssiveStatus = FXCODEC_STATUS_ERROR;
    return m_ProssiveStatus;
  }

  return decode_Arith(pPause);
}

FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template0_opt3(
    CJBig2_Image* pImage,
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    IFX_Pause* pPause) {
  FX_BOOL SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2;
  uint8_t *pLine1, *pLine2, cVal;
  int32_t nStride, nStride2, k;
  int32_t nLineBytes, nBitsLeft, cc;
  if (!m_pLine) {
    m_pLine = pImage->m_pData;
  }
  nStride = pImage->m_nStride;
  nStride2 = nStride << 1;
  nLineBytes = ((GBW + 7) >> 3) - 1;
  nBitsLeft = GBW - (nLineBytes << 3);
  FX_DWORD height = GBH & 0x7fffffff;
  for (; m_loopIndex < height; m_loopIndex++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x9b25]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      pImage->copyLine(m_loopIndex, m_loopIndex - 1);
    } else {
      if (m_loopIndex > 1) {
        pLine1 = m_pLine - nStride2;
        pLine2 = m_pLine - nStride;
        line1 = (*pLine1++) << 6;
        line2 = *pLine2++;
        CONTEXT = ((line1 & 0xf800) | (line2 & 0x07f0));
        for (cc = 0; cc < nLineBytes; cc++) {
          line1 = (line1 << 8) | ((*pLine1++) << 6);
          line2 = (line2 << 8) | (*pLine2++);
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal |
                       ((line1 >> k) & 0x0800) | ((line2 >> k) & 0x0010));
          }
          m_pLine[cc] = cVal;
        }
        line1 <<= 8;
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT =
              (((CONTEXT & 0x7bf7) << 1) | bVal |
               ((line1 >> (7 - k)) & 0x0800) | ((line2 >> (7 - k)) & 0x0010));
        }
        m_pLine[nLineBytes] = cVal;
      } else {
        pLine2 = m_pLine - nStride;
        line2 = (m_loopIndex & 1) ? (*pLine2++) : 0;
        CONTEXT = (line2 & 0x07f0);
        for (cc = 0; cc < nLineBytes; cc++) {
          if (m_loopIndex & 1) {
            line2 = (line2 << 8) | (*pLine2++);
          }
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT =
                (((CONTEXT & 0x7bf7) << 1) | bVal | ((line2 >> k) & 0x0010));
          }
          m_pLine[cc] = cVal;
        }
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal |
                     ((line2 >> (7 - k)) & 0x0010));
        }
        m_pLine[nLineBytes] = cVal;
      }
    }
    m_pLine += nStride;
    if (pPause && pPause->NeedToPauseNow()) {
      m_loopIndex++;
      m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      return FXCODEC_STATUS_DECODE_TOBECONTINUE;
    }
  }
  m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
  return FXCODEC_STATUS_DECODE_FINISH;
}

FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template0_unopt(
    CJBig2_Image* pImage,
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    IFX_Pause* pPause) {
  FX_BOOL SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2, line3;
  for (; m_loopIndex < GBH; m_loopIndex++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x9b25]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      pImage->copyLine(m_loopIndex, m_loopIndex - 1);
    } else {
      line1 = pImage->getPixel(1, m_loopIndex - 2);
      line1 |= pImage->getPixel(0, m_loopIndex - 2) << 1;
      line2 = pImage->getPixel(2, m_loopIndex - 1);
      line2 |= pImage->getPixel(1, m_loopIndex - 1) << 1;
      line2 |= pImage->getPixel(0, m_loopIndex - 1) << 2;
      line3 = 0;
      for (FX_DWORD w = 0; w < GBW; w++) {
        if (USESKIP && SKIP->getPixel(w, m_loopIndex)) {
          bVal = 0;
        } else {
          CONTEXT = line3;
          CONTEXT |= pImage->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 4;
          CONTEXT |= line2 << 5;
          CONTEXT |= pImage->getPixel(w + GBAT[2], m_loopIndex + GBAT[3]) << 10;
          CONTEXT |= pImage->getPixel(w + GBAT[4], m_loopIndex + GBAT[5]) << 11;
          CONTEXT |= line1 << 12;
          CONTEXT |= pImage->getPixel(w + GBAT[6], m_loopIndex + GBAT[7]) << 15;
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
        }
        if (bVal) {
          pImage->setPixel(w, m_loopIndex, bVal);
        }
        line1 =
            ((line1 << 1) | pImage->getPixel(w + 2, m_loopIndex - 2)) & 0x07;
        line2 =
            ((line2 << 1) | pImage->getPixel(w + 3, m_loopIndex - 1)) & 0x1f;
        line3 = ((line3 << 1) | bVal) & 0x0f;
      }
    }
    if (pPause && pPause->NeedToPauseNow()) {
      m_loopIndex++;
      m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      return FXCODEC_STATUS_DECODE_TOBECONTINUE;
    }
  }
  m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
  return FXCODEC_STATUS_DECODE_FINISH;
}

FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template1_opt3(
    CJBig2_Image* pImage,
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    IFX_Pause* pPause) {
  FX_BOOL SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2;
  uint8_t *pLine1, *pLine2, cVal;
  int32_t nStride, nStride2, k;
  int32_t nLineBytes, nBitsLeft, cc;
  if (!m_pLine) {
    m_pLine = pImage->m_pData;
  }
  nStride = pImage->m_nStride;
  nStride2 = nStride << 1;
  nLineBytes = ((GBW + 7) >> 3) - 1;
  nBitsLeft = GBW - (nLineBytes << 3);
  for (; m_loopIndex < GBH; m_loopIndex++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x0795]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      pImage->copyLine(m_loopIndex, m_loopIndex - 1);
    } else {
      if (m_loopIndex > 1) {
        pLine1 = m_pLine - nStride2;
        pLine2 = m_pLine - nStride;
        line1 = (*pLine1++) << 4;
        line2 = *pLine2++;
        CONTEXT = (line1 & 0x1e00) | ((line2 >> 1) & 0x01f8);
        for (cc = 0; cc < nLineBytes; cc++) {
          line1 = (line1 << 8) | ((*pLine1++) << 4);
          line2 = (line2 << 8) | (*pLine2++);
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal |
                      ((line1 >> k) & 0x0200) | ((line2 >> (k + 1)) & 0x0008);
          }
          m_pLine[cc] = cVal;
        }
        line1 <<= 8;
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal |
                    ((line1 >> (7 - k)) & 0x0200) |
                    ((line2 >> (8 - k)) & 0x0008);
        }
        m_pLine[nLineBytes] = cVal;
      } else {
        pLine2 = m_pLine - nStride;
        line2 = (m_loopIndex & 1) ? (*pLine2++) : 0;
        CONTEXT = (line2 >> 1) & 0x01f8;
        for (cc = 0; cc < nLineBytes; cc++) {
          if (m_loopIndex & 1) {
            line2 = (line2 << 8) | (*pLine2++);
          }
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal |
                      ((line2 >> (k + 1)) & 0x0008);
          }
          m_pLine[cc] = cVal;
        }
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT =
              ((CONTEXT & 0x0efb) << 1) | bVal | ((line2 >> (8 - k)) & 0x0008);
        }
        m_pLine[nLineBytes] = cVal;
      }
    }
    m_pLine += nStride;
    if (pPause && pPause->NeedToPauseNow()) {
      m_loopIndex++;
      m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      return FXCODEC_STATUS_DECODE_TOBECONTINUE;
    }
  }
  m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
  return FXCODEC_STATUS_DECODE_FINISH;
}

FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template1_unopt(
    CJBig2_Image* pImage,
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    IFX_Pause* pPause) {
  FX_BOOL SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2, line3;
  for (FX_DWORD h = 0; h < GBH; h++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x0795]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      pImage->copyLine(h, h - 1);
    } else {
      line1 = pImage->getPixel(2, h - 2);
      line1 |= pImage->getPixel(1, h - 2) << 1;
      line1 |= pImage->getPixel(0, h - 2) << 2;
      line2 = pImage->getPixel(2, h - 1);
      line2 |= pImage->getPixel(1, h - 1) << 1;
      line2 |= pImage->getPixel(0, h - 1) << 2;
      line3 = 0;
      for (FX_DWORD w = 0; w < GBW; w++) {
        if (USESKIP && SKIP->getPixel(w, h)) {
          bVal = 0;
        } else {
          CONTEXT = line3;
          CONTEXT |= pImage->getPixel(w + GBAT[0], h + GBAT[1]) << 3;
          CONTEXT |= line2 << 4;
          CONTEXT |= line1 << 9;
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
        }
        if (bVal) {
          pImage->setPixel(w, h, bVal);
        }
        line1 = ((line1 << 1) | pImage->getPixel(w + 3, h - 2)) & 0x0f;
        line2 = ((line2 << 1) | pImage->getPixel(w + 3, h - 1)) & 0x1f;
        line3 = ((line3 << 1) | bVal) & 0x07;
      }
    }
    if (pPause && pPause->NeedToPauseNow()) {
      m_loopIndex++;
      m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      return FXCODEC_STATUS_DECODE_TOBECONTINUE;
    }
  }
  m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
  return FXCODEC_STATUS_DECODE_FINISH;
}

FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template2_opt3(
    CJBig2_Image* pImage,
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    IFX_Pause* pPause) {
  FX_BOOL SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2;
  uint8_t *pLine1, *pLine2, cVal;
  int32_t nStride, nStride2, k;
  int32_t nLineBytes, nBitsLeft, cc;
  if (!m_pLine) {
    m_pLine = pImage->m_pData;
  }
  nStride = pImage->m_nStride;
  nStride2 = nStride << 1;
  nLineBytes = ((GBW + 7) >> 3) - 1;
  nBitsLeft = GBW - (nLineBytes << 3);
  for (; m_loopIndex < GBH; m_loopIndex++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x00e5]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      pImage->copyLine(m_loopIndex, m_loopIndex - 1);
    } else {
      if (m_loopIndex > 1) {
        pLine1 = m_pLine - nStride2;
        pLine2 = m_pLine - nStride;
        line1 = (*pLine1++) << 1;
        line2 = *pLine2++;
        CONTEXT = (line1 & 0x0380) | ((line2 >> 3) & 0x007c);
        for (cc = 0; cc < nLineBytes; cc++) {
          line1 = (line1 << 8) | ((*pLine1++) << 1);
          line2 = (line2 << 8) | (*pLine2++);
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal |
                      ((line1 >> k) & 0x0080) | ((line2 >> (k + 3)) & 0x0004);
          }
          m_pLine[cc] = cVal;
        }
        line1 <<= 8;
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal |
                    ((line1 >> (7 - k)) & 0x0080) |
                    ((line2 >> (10 - k)) & 0x0004);
        }
        m_pLine[nLineBytes] = cVal;
      } else {
        pLine2 = m_pLine - nStride;
        line2 = (m_loopIndex & 1) ? (*pLine2++) : 0;
        CONTEXT = (line2 >> 3) & 0x007c;
        for (cc = 0; cc < nLineBytes; cc++) {
          if (m_loopIndex & 1) {
            line2 = (line2 << 8) | (*pLine2++);
          }
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal |
                      ((line2 >> (k + 3)) & 0x0004);
          }
          m_pLine[cc] = cVal;
        }
        line2 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal |
                    (((line2 >> (10 - k))) & 0x0004);
        }
        m_pLine[nLineBytes] = cVal;
      }
    }
    m_pLine += nStride;
    if (pPause && m_loopIndex % 50 == 0 && pPause->NeedToPauseNow()) {
      m_loopIndex++;
      m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      return FXCODEC_STATUS_DECODE_TOBECONTINUE;
    }
  }
  m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
  return FXCODEC_STATUS_DECODE_FINISH;
}

FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template2_unopt(
    CJBig2_Image* pImage,
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    IFX_Pause* pPause) {
  FX_BOOL SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2, line3;
  for (; m_loopIndex < GBH; m_loopIndex++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x00e5]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      pImage->copyLine(m_loopIndex, m_loopIndex - 1);
    } else {
      line1 = pImage->getPixel(1, m_loopIndex - 2);
      line1 |= pImage->getPixel(0, m_loopIndex - 2) << 1;
      line2 = pImage->getPixel(1, m_loopIndex - 1);
      line2 |= pImage->getPixel(0, m_loopIndex - 1) << 1;
      line3 = 0;
      for (FX_DWORD w = 0; w < GBW; w++) {
        if (USESKIP && SKIP->getPixel(w, m_loopIndex)) {
          bVal = 0;
        } else {
          CONTEXT = line3;
          CONTEXT |= pImage->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 2;
          CONTEXT |= line2 << 3;
          CONTEXT |= line1 << 7;
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
        }
        if (bVal) {
          pImage->setPixel(w, m_loopIndex, bVal);
        }
        line1 =
            ((line1 << 1) | pImage->getPixel(w + 2, m_loopIndex - 2)) & 0x07;
        line2 =
            ((line2 << 1) | pImage->getPixel(w + 2, m_loopIndex - 1)) & 0x0f;
        line3 = ((line3 << 1) | bVal) & 0x03;
      }
    }
    if (pPause && pPause->NeedToPauseNow()) {
      m_loopIndex++;
      m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      return FXCODEC_STATUS_DECODE_TOBECONTINUE;
    }
  }
  m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
  return FXCODEC_STATUS_DECODE_FINISH;
}

FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template3_opt3(
    CJBig2_Image* pImage,
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    IFX_Pause* pPause) {
  FX_BOOL SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1;
  uint8_t *pLine1, cVal;
  int32_t nStride, k;
  int32_t nLineBytes, nBitsLeft, cc;
  if (!m_pLine) {
    m_pLine = pImage->m_pData;
  }
  nStride = pImage->m_nStride;
  nLineBytes = ((GBW + 7) >> 3) - 1;
  nBitsLeft = GBW - (nLineBytes << 3);
  for (; m_loopIndex < GBH; m_loopIndex++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x0195]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      pImage->copyLine(m_loopIndex, m_loopIndex - 1);
    } else {
      if (m_loopIndex > 0) {
        pLine1 = m_pLine - nStride;
        line1 = *pLine1++;
        CONTEXT = (line1 >> 1) & 0x03f0;
        for (cc = 0; cc < nLineBytes; cc++) {
          line1 = (line1 << 8) | (*pLine1++);
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal |
                      ((line1 >> (k + 1)) & 0x0010);
          }
          m_pLine[cc] = cVal;
        }
        line1 <<= 8;
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT =
              ((CONTEXT & 0x01f7) << 1) | bVal | ((line1 >> (8 - k)) & 0x0010);
        }
        m_pLine[nLineBytes] = cVal;
      } else {
        CONTEXT = 0;
        for (cc = 0; cc < nLineBytes; cc++) {
          cVal = 0;
          for (k = 7; k >= 0; k--) {
            bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            cVal |= bVal << k;
            CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal;
          }
          m_pLine[cc] = cVal;
        }
        cVal = 0;
        for (k = 0; k < nBitsLeft; k++) {
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
          cVal |= bVal << (7 - k);
          CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal;
        }
        m_pLine[nLineBytes] = cVal;
      }
    }
    m_pLine += nStride;
    if (pPause && pPause->NeedToPauseNow()) {
      m_loopIndex++;
      m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      return FXCODEC_STATUS_DECODE_TOBECONTINUE;
    }
  }
  m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
  return FXCODEC_STATUS_DECODE_FINISH;
}

FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template3_unopt(
    CJBig2_Image* pImage,
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    IFX_Pause* pPause) {
  FX_BOOL SLTP, bVal;
  FX_DWORD CONTEXT;
  FX_DWORD line1, line2;
  for (; m_loopIndex < GBH; m_loopIndex++) {
    if (TPGDON) {
      SLTP = pArithDecoder->DECODE(&gbContext[0x0195]);
      LTP = LTP ^ SLTP;
    }
    if (LTP == 1) {
      pImage->copyLine(m_loopIndex, m_loopIndex - 1);
    } else {
      line1 = pImage->getPixel(1, m_loopIndex - 1);
      line1 |= pImage->getPixel(0, m_loopIndex - 1) << 1;
      line2 = 0;
      for (FX_DWORD w = 0; w < GBW; w++) {
        if (USESKIP && SKIP->getPixel(w, m_loopIndex)) {
          bVal = 0;
        } else {
          CONTEXT = line2;
          CONTEXT |= pImage->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 4;
          CONTEXT |= line1 << 5;
          bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
        }
        if (bVal) {
          pImage->setPixel(w, m_loopIndex, bVal);
        }
        line1 =
            ((line1 << 1) | pImage->getPixel(w + 2, m_loopIndex - 1)) & 0x1f;
        line2 = ((line2 << 1) | bVal) & 0x0f;
      }
    }
    if (pPause && pPause->NeedToPauseNow()) {
      m_loopIndex++;
      m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
      return FXCODEC_STATUS_DECODE_TOBECONTINUE;
    }
  }
  m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
  return FXCODEC_STATUS_DECODE_FINISH;
}
