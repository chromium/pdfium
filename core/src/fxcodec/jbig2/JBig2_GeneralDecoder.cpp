// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_GeneralDecoder.h"
#include "JBig2_ArithDecoder.h"
#include "JBig2_ArithIntDecoder.h"
#include "JBig2_HuffmanDecoder.h"
#include "JBig2_HuffmanTable.h"
#include "JBig2_PatternDict.h"

extern const JBig2ArithQe QeTable[] = {
    { 0x5601,  1,  1, 1 },
    { 0x3401,  2,  6, 0 },
    { 0x1801,  3,  9, 0 },
    { 0x0AC1,  4, 12, 0 },
    { 0x0521,  5, 29, 0 },
    { 0x0221, 38, 33, 0 },
    { 0x5601,  7,  6, 1 },
    { 0x5401,  8, 14, 0 },
    { 0x4801,  9, 14, 0 },
    { 0x3801, 10, 14, 0 },
    { 0x3001, 11, 17, 0 },
    { 0x2401, 12, 18, 0 },
    { 0x1C01, 13, 20, 0 },
    { 0x1601, 29, 21, 0 },
    { 0x5601, 15, 14, 1 },
    { 0x5401, 16, 14, 0 },
    { 0x5101, 17, 15, 0 },
    { 0x4801, 18, 16, 0 },
    { 0x3801, 19, 17, 0 },
    { 0x3401, 20, 18, 0 },
    { 0x3001, 21, 19, 0 },
    { 0x2801, 22, 19, 0 },
    { 0x2401, 23, 20, 0 },
    { 0x2201, 24, 21, 0 },
    { 0x1C01, 25, 22, 0 },
    { 0x1801, 26, 23, 0 },
    { 0x1601, 27, 24, 0 },
    { 0x1401, 28, 25, 0 },
    { 0x1201, 29, 26, 0 },
    { 0x1101, 30, 27, 0 },
    { 0x0AC1, 31, 28, 0 },
    { 0x09C1, 32, 29, 0 },
    { 0x08A1, 33, 30, 0 },
    { 0x0521, 34, 31, 0 },
    { 0x0441, 35, 32, 0 },
    { 0x02A1, 36, 33, 0 },
    { 0x0221, 37, 34, 0 },
    { 0x0141, 38, 35, 0 },
    { 0x0111, 39, 36, 0 },
    { 0x0085, 40, 37, 0 },
    { 0x0049, 41, 38, 0 },
    { 0x0025, 42, 39, 0 },
    { 0x0015, 43, 40, 0 },
    { 0x0009, 44, 41, 0 },
    { 0x0005, 45, 42, 0 },
    { 0x0001, 45, 43, 0 },
    { 0x5601, 46, 46, 0 }
};

extern const unsigned int JBIG2_QE_NUM = sizeof(QeTable) / sizeof(JBig2ArithQe);

CJBig2_Image *CJBig2_GRDProc::decode_Arith(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    if (GBW == 0 || GBH == 0) {
        CJBig2_Image* pImage;
        JBIG2_ALLOC(pImage, CJBig2_Image(GBW, GBH));
        return pImage;
    }
    if(GBTEMPLATE == 0) {
        if((GBAT[0] == 3) && (GBAT[1] == (signed char) - 1)
                && (GBAT[2] == (signed char) - 3) && (GBAT[3] == (signed char) - 1)
                && (GBAT[4] == 2) && (GBAT[5] == (signed char) - 2)
                && (GBAT[6] == (signed char) - 2) && (GBAT[7] == (signed char) - 2)) {
            return decode_Arith_Template0_opt3(pArithDecoder, gbContext);
        } else {
            return decode_Arith_Template0_unopt(pArithDecoder, gbContext);
        }
    } else if(GBTEMPLATE == 1) {
        if((GBAT[0] == 3) && (GBAT[1] == (signed char) - 1)) {
            return decode_Arith_Template1_opt3(pArithDecoder, gbContext);
        } else {
            return decode_Arith_Template1_unopt(pArithDecoder, gbContext);
        }
    } else if(GBTEMPLATE == 2) {
        if((GBAT[0] == 2) && (GBAT[1] == (signed char) - 1)) {
            return decode_Arith_Template2_opt3(pArithDecoder, gbContext);
        } else {
            return decode_Arith_Template2_unopt(pArithDecoder, gbContext);
        }
    } else {
        if((GBAT[0] == 2) && (GBAT[1] == (signed char) - 1)) {
            return decode_Arith_Template3_opt3(pArithDecoder, gbContext);
        } else {
            return decode_Arith_Template3_unopt(pArithDecoder, gbContext);
        }
    }
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template0_opt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2, line3;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    GBREG->fill(0);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x9b25]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = GBREG->getPixel(2, h - 2);
            line1 |= GBREG->getPixel(1, h - 2) << 1;
            line1 |= GBREG->getPixel(0, h - 2) << 2;
            line2 = GBREG->getPixel(3, h - 1);
            line2 |= GBREG->getPixel(2, h - 1) << 1;
            line2 |= GBREG->getPixel(1, h - 1) << 2;
            line2 |= GBREG->getPixel(0, h - 1) << 3;
            line3 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, h)) {
                    bVal = 0;
                } else {
                    CONTEXT = line3;
                    CONTEXT |= line2 << 4;
                    CONTEXT |= line1 << 11;
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                }
                if(bVal) {
                    GBREG->setPixel(w, h, bVal);
                }
                line1 = ((line1 << 1) | GBREG->getPixel(w + 3, h - 2)) & 0x1f;
                line2 = ((line2 << 1) | GBREG->getPixel(w + 4, h - 1)) & 0x7f;
                line3 = ((line3 << 1) | bVal) & 0x0f;
            }
        }
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template0_opt2(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2;
    FX_BYTE *pLine, cVal;
    FX_INTPTR nStride, nStride2;
    FX_INT32 nBits, k;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    if (GBREG->m_pData == NULL) {
        delete GBREG;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        return NULL;
    }
    pLine = GBREG->m_pData;
    nStride = GBREG->m_nStride;
    nStride2 = nStride << 1;
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x9b25]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = (h > 1) ? pLine[-nStride2] << 6 : 0;
            line2 = (h > 0) ? pLine[-nStride] : 0;
            CONTEXT = (line1 & 0xf800) | (line2 & 0x07f0);
            for(FX_DWORD w = 0; w < GBW; w += 8) {
                if(w + 8 < GBW) {
                    nBits = 8;
                    if(h > 1) {
                        line1 = (line1 << 8) | (pLine[-nStride2 + (w >> 3) + 1] << 6);
                    }
                    if(h > 0) {
                        line2 = (line2 << 8) | (pLine[-nStride + (w >> 3) + 1]);
                    }
                } else {
                    nBits = GBW - w;
                    if(h > 1) {
                        line1 <<= 8;
                    }
                    if(h > 0) {
                        line2 <<= 8;
                    }
                }
                cVal = 0;
                for(k = 0; k < nBits; k++) {
                    if(USESKIP && SKIP->getPixel(w, h)) {
                        bVal = 0;
                    } else {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    }
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x7bf7) << 1) | bVal
                              | ((line1 >> (7 - k)) & 0x0800)
                              | ((line2 >> (7 - k)) & 0x0010);
                }
                pLine[w >> 3] = cVal;
            }
        }
        pLine += nStride;
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template0_opt3(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2;
    FX_BYTE *pLine, *pLine1, *pLine2, cVal;
    FX_INT32 nStride, nStride2, k;
    FX_INT32 nLineBytes, nBitsLeft, cc;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    if (GBREG->m_pData == NULL) {
        delete GBREG;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        return NULL;
    }
    pLine = GBREG->m_pData;
    nStride = GBREG->m_nStride;
    nStride2 = nStride << 1;
    nLineBytes = ((GBW + 7) >> 3) - 1;
    nBitsLeft = GBW - (nLineBytes << 3);
    FX_DWORD height = GBH & 0x7fffffff;
    for(FX_DWORD h = 0; h < height; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x9b25]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            if(h > 1) {
                pLine1 = pLine - nStride2;
                pLine2 = pLine - nStride;
                line1 = (*pLine1++) << 6;
                line2 = *pLine2++;
                CONTEXT = ((line1 & 0xf800) | (line2 & 0x07f0));
                for(cc = 0; cc < nLineBytes; cc++) {
                    line1 = (line1 << 8) | ((*pLine1++) << 6);
                    line2 = (line2 << 8) | (*pLine2++);
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal
                                   | ((line1 >> k) & 0x0800)
                                   | ((line2 >> k) & 0x0010));
                    }
                    pLine[cc] = cVal;
                }
                line1 <<= 8;
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal
                               | ((line1 >> (7 - k)) & 0x0800)
                               | ((line2 >> (7 - k)) & 0x0010));
                }
                pLine[nLineBytes] = cVal;
            } else {
                pLine2 = pLine - nStride;
                line2 = (h & 1) ? (*pLine2++) : 0;
                CONTEXT = (line2 & 0x07f0);
                for(cc = 0; cc < nLineBytes; cc++) {
                    if(h & 1) {
                        line2 = (line2 << 8) | (*pLine2++);
                    }
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal
                                   | ((line2 >> k) & 0x0010));
                    }
                    pLine[cc] = cVal;
                }
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal
                               | (((line2 >> (7 - k))) & 0x0010));
                }
                pLine[nLineBytes] = cVal;
            }
        }
        pLine += nStride;
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template0_unopt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2, line3;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    GBREG->fill(0);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x9b25]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = GBREG->getPixel(1, h - 2);
            line1 |= GBREG->getPixel(0, h - 2) << 1;
            line2 = GBREG->getPixel(2, h - 1);
            line2 |= GBREG->getPixel(1, h - 1) << 1;
            line2 |= GBREG->getPixel(0, h - 1) << 2;
            line3 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, h)) {
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
                if(bVal) {
                    GBREG->setPixel(w, h, bVal);
                }
                line1 = ((line1 << 1) | GBREG->getPixel(w + 2, h - 2)) & 0x07;
                line2 = ((line2 << 1) | GBREG->getPixel(w + 3, h - 1)) & 0x1f;
                line3 = ((line3 << 1) | bVal) & 0x0f;
            }
        }
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template1_opt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2, line3;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    GBREG->fill(0);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0795]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = GBREG->getPixel(2, h - 2);
            line1 |= GBREG->getPixel(1, h - 2) << 1;
            line1 |= GBREG->getPixel(0, h - 2) << 2;
            line2 = GBREG->getPixel(3, h - 1);
            line2 |= GBREG->getPixel(2, h - 1) << 1;
            line2 |= GBREG->getPixel(1, h - 1) << 2;
            line2 |= GBREG->getPixel(0, h - 1) << 3;
            line3 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, h)) {
                    bVal = 0;
                } else {
                    CONTEXT = line3;
                    CONTEXT |= line2 << 3;
                    CONTEXT |= line1 << 9;
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                }
                if(bVal) {
                    GBREG->setPixel(w, h, bVal);
                }
                line1 = ((line1 << 1) | GBREG->getPixel(w + 3, h - 2)) & 0x0f;
                line2 = ((line2 << 1) | GBREG->getPixel(w + 4, h - 1)) & 0x3f;
                line3 = ((line3 << 1) | bVal) & 0x07;
            }
        }
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template1_opt2(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2;
    FX_BYTE *pLine, cVal;
    FX_INTPTR nStride, nStride2;
    FX_INT32 nBits, k;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    if (GBREG->m_pData == NULL) {
        delete GBREG;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        return NULL;
    }
    pLine = GBREG->m_pData;
    nStride = GBREG->m_nStride;
    nStride2 = nStride << 1;
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0795]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = (h > 1) ? pLine[-nStride2] << 4 : 0;
            line2 = (h > 0) ? pLine[-nStride] : 0;
            CONTEXT = (line1 & 0x1e00) | ((line2 >> 1) & 0x01f8);
            for(FX_DWORD w = 0; w < GBW; w += 8) {
                if(w + 8 < GBW) {
                    nBits = 8;
                    if(h > 1) {
                        line1 = (line1 << 8) | (pLine[-nStride2 + (w >> 3) + 1] << 4);
                    }
                    if(h > 0) {
                        line2 = (line2 << 8) | (pLine[-nStride + (w >> 3) + 1]);
                    }
                } else {
                    nBits = GBW - w;
                    if(h > 1) {
                        line1 <<= 8;
                    }
                    if(h > 0) {
                        line2 <<= 8;
                    }
                }
                cVal = 0;
                for(k = 0; k < nBits; k++) {
                    if(USESKIP && SKIP->getPixel(w, h)) {
                        bVal = 0;
                    } else {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    }
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal
                              | ((line1 >> (7 - k)) & 0x0200)
                              | ((line2 >> (8 - k)) & 0x0008);
                }
                pLine[w >> 3] = cVal;
            }
        }
        pLine += nStride;
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template1_opt3(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2;
    FX_BYTE *pLine, *pLine1, *pLine2, cVal;
    FX_INT32 nStride, nStride2, k;
    FX_INT32 nLineBytes, nBitsLeft, cc;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    if (GBREG->m_pData == NULL) {
        delete GBREG;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        return NULL;
    }
    pLine = GBREG->m_pData;
    nStride = GBREG->m_nStride;
    nStride2 = nStride << 1;
    nLineBytes = ((GBW + 7) >> 3) - 1;
    nBitsLeft = GBW - (nLineBytes << 3);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0795]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            if(h > 1) {
                pLine1 = pLine - nStride2;
                pLine2 = pLine - nStride;
                line1 = (*pLine1++) << 4;
                line2 = *pLine2++;
                CONTEXT = (line1 & 0x1e00) | ((line2 >> 1) & 0x01f8);
                for(cc = 0; cc < nLineBytes; cc++) {
                    line1 = (line1 << 8) | ((*pLine1++) << 4);
                    line2 = (line2 << 8) | (*pLine2++);
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal
                                  | ((line1 >> k) & 0x0200)
                                  | ((line2 >> (k + 1)) & 0x0008);
                    }
                    pLine[cc] = cVal;
                }
                line1 <<= 8;
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal
                              | ((line1 >> (7 - k)) & 0x0200)
                              | ((line2 >> (8 - k)) & 0x0008);
                }
                pLine[nLineBytes] = cVal;
            } else {
                pLine2 = pLine - nStride;
                line2 = (h & 1) ? (*pLine2++) : 0;
                CONTEXT = (line2 >> 1) & 0x01f8;
                for(cc = 0; cc < nLineBytes; cc++) {
                    if(h & 1) {
                        line2 = (line2 << 8) | (*pLine2++);
                    }
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal
                                  | ((line2 >> (k + 1)) & 0x0008);
                    }
                    pLine[cc] = cVal;
                }
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal
                              | ((line2 >> (8 - k)) & 0x0008);
                }
                pLine[nLineBytes] = cVal;
            }
        }
        pLine += nStride;
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template1_unopt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2, line3;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    GBREG->fill(0);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0795]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = GBREG->getPixel(2, h - 2);
            line1 |= GBREG->getPixel(1, h - 2) << 1;
            line1 |= GBREG->getPixel(0, h - 2) << 2;
            line2 = GBREG->getPixel(2, h - 1);
            line2 |= GBREG->getPixel(1, h - 1) << 1;
            line2 |= GBREG->getPixel(0, h - 1) << 2;
            line3 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, h)) {
                    bVal = 0;
                } else {
                    CONTEXT = line3;
                    CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 3;
                    CONTEXT |= line2 << 4;
                    CONTEXT |= line1 << 9;
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                }
                if(bVal) {
                    GBREG->setPixel(w, h, bVal);
                }
                line1 = ((line1 << 1) | GBREG->getPixel(w + 3, h - 2)) & 0x0f;
                line2 = ((line2 << 1) | GBREG->getPixel(w + 3, h - 1)) & 0x1f;
                line3 = ((line3 << 1) | bVal) & 0x07;
            }
        }
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template2_opt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2, line3;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    GBREG->fill(0);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x00e5]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = GBREG->getPixel(1, h - 2);
            line1 |= GBREG->getPixel(0, h - 2) << 1;
            line2 = GBREG->getPixel(2, h - 1);
            line2 |= GBREG->getPixel(1, h - 1) << 1;
            line2 |= GBREG->getPixel(0, h - 1) << 2;
            line3 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, h)) {
                    bVal = 0;
                } else {
                    CONTEXT = line3;
                    CONTEXT |= line2 << 2;
                    CONTEXT |= line1 << 7;
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                }
                if(bVal) {
                    GBREG->setPixel(w, h, bVal);
                }
                line1 = ((line1 << 1) | GBREG->getPixel(w + 2, h - 2)) & 0x07;
                line2 = ((line2 << 1) | GBREG->getPixel(w + 3, h - 1)) & 0x1f;
                line3 = ((line3 << 1) | bVal) & 0x03;
            }
        }
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template2_opt2(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2;
    FX_BYTE *pLine, cVal;
    FX_INTPTR nStride, nStride2;
    FX_INT32 nBits, k;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    if (GBREG->m_pData == NULL) {
        delete GBREG;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        return NULL;
    }
    pLine = GBREG->m_pData;
    nStride = GBREG->m_nStride;
    nStride2 = nStride << 1;
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x00e5]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = (h > 1) ? pLine[-nStride2] << 1 : 0;
            line2 = (h > 0) ? pLine[-nStride] : 0;
            CONTEXT = (line1 & 0x0380) | ((line2 >> 3) & 0x007c);
            for(FX_DWORD w = 0; w < GBW; w += 8) {
                if(w + 8 < GBW) {
                    nBits = 8;
                    if(h > 1) {
                        line1 = (line1 << 8) | (pLine[-nStride2 + (w >> 3) + 1] << 1);
                    }
                    if(h > 0) {
                        line2 = (line2 << 8) | (pLine[-nStride + (w >> 3) + 1]);
                    }
                } else {
                    nBits = GBW - w;
                    if(h > 1) {
                        line1 <<= 8;
                    }
                    if(h > 0) {
                        line2 <<= 8;
                    }
                }
                cVal = 0;
                for(k = 0; k < nBits; k++) {
                    if(USESKIP && SKIP->getPixel(w, h)) {
                        bVal = 0;
                    } else {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    }
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal
                              | ((line1 >> (7 - k)) & 0x0080)
                              | ((line2 >> (10 - k)) & 0x0004);
                }
                pLine[w >> 3] = cVal;
            }
        }
        pLine += nStride;
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template2_opt3(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2;
    FX_BYTE *pLine, *pLine1, *pLine2, cVal;
    FX_INT32 nStride, nStride2, k;
    FX_INT32 nLineBytes, nBitsLeft, cc;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    if (GBREG->m_pData == NULL) {
        delete GBREG;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        return NULL;
    }
    pLine = GBREG->m_pData;
    nStride = GBREG->m_nStride;
    nStride2 = nStride << 1;
    nLineBytes = ((GBW + 7) >> 3) - 1;
    nBitsLeft = GBW - (nLineBytes << 3);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x00e5]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            if(h > 1) {
                pLine1 = pLine - nStride2;
                pLine2 = pLine - nStride;
                line1 = (*pLine1++) << 1;
                line2 = *pLine2++;
                CONTEXT = (line1 & 0x0380) | ((line2 >> 3) & 0x007c);
                for(cc = 0; cc < nLineBytes; cc++) {
                    line1 = (line1 << 8) | ((*pLine1++) << 1);
                    line2 = (line2 << 8) | (*pLine2++);
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal
                                  | ((line1 >> k) & 0x0080)
                                  | ((line2 >> (k + 3)) & 0x0004);
                    }
                    pLine[cc] = cVal;
                }
                line1 <<= 8;
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal
                              | ((line1 >> (7 - k)) & 0x0080)
                              | ((line2 >> (10 - k)) & 0x0004);
                }
                pLine[nLineBytes] = cVal;
            } else {
                pLine2 = pLine - nStride;
                line2 = (h & 1) ? (*pLine2++) : 0;
                CONTEXT = (line2 >> 3) & 0x007c;
                for(cc = 0; cc < nLineBytes; cc++) {
                    if(h & 1) {
                        line2 = (line2 << 8) | (*pLine2++);
                    }
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal
                                  | ((line2 >> (k + 3)) & 0x0004);
                    }
                    pLine[cc] = cVal;
                }
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal
                              | (((line2 >> (10 - k))) & 0x0004);
                }
                pLine[nLineBytes] = cVal;
            }
        }
        pLine += nStride;
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template2_unopt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2, line3;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    GBREG->fill(0);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x00e5]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = GBREG->getPixel(1, h - 2);
            line1 |= GBREG->getPixel(0, h - 2) << 1;
            line2 = GBREG->getPixel(1, h - 1);
            line2 |= GBREG->getPixel(0, h - 1) << 1;
            line3 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, h)) {
                    bVal = 0;
                } else {
                    CONTEXT = line3;
                    CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 2;
                    CONTEXT |= line2 << 3;
                    CONTEXT |= line1 << 7;
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                }
                if(bVal) {
                    GBREG->setPixel(w, h, bVal);
                }
                line1 = ((line1 << 1) | GBREG->getPixel(w + 2, h - 2)) & 0x07;
                line2 = ((line2 << 1) | GBREG->getPixel(w + 2, h - 1)) & 0x0f;
                line3 = ((line3 << 1) | bVal) & 0x03;
            }
        }
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template3_opt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    GBREG->fill(0);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0195]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = GBREG->getPixel(2, h - 1);
            line1 |= GBREG->getPixel(1, h - 1) << 1;
            line1 |= GBREG->getPixel(0, h - 1) << 2;
            line2 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, h)) {
                    bVal = 0;
                } else {
                    CONTEXT = line2;
                    CONTEXT |= line1 << 4;
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                }
                if(bVal) {
                    GBREG->setPixel(w, h, bVal);
                }
                line1 = ((line1 << 1) | GBREG->getPixel(w + 3, h - 1)) & 0x3f;
                line2 = ((line2 << 1) | bVal) & 0x0f;
            }
        }
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template3_opt2(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1;
    FX_BYTE *pLine, cVal;
    FX_INTPTR nStride;
    FX_INT32 nBits, k;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    if (GBREG->m_pData == NULL) {
        delete GBREG;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        return NULL;
    }
    pLine = GBREG->m_pData;
    nStride = GBREG->m_nStride;
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0195]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = (h > 0) ? pLine[-nStride] : 0;
            CONTEXT = (line1 >> 1) & 0x03f0;
            for(FX_DWORD w = 0; w < GBW; w += 8) {
                if(w + 8 < GBW) {
                    nBits = 8;
                    if(h > 0) {
                        line1 = (line1 << 8) | (pLine[-nStride + (w >> 3) + 1]);
                    }
                } else {
                    nBits = GBW - w;
                    if(h > 0) {
                        line1 <<= 8;
                    }
                }
                cVal = 0;
                for(k = 0; k < nBits; k++) {
                    if(USESKIP && SKIP->getPixel(w, h)) {
                        bVal = 0;
                    } else {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    }
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal
                              | ((line1 >> (8 - k)) & 0x0010);
                }
                pLine[w >> 3] = cVal;
            }
        }
        pLine += nStride;
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template3_opt3(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1;
    FX_BYTE *pLine, *pLine1, cVal;
    FX_INT32 nStride, k;
    FX_INT32 nLineBytes, nBitsLeft, cc;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    if (GBREG->m_pData == NULL) {
        delete GBREG;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        return NULL;
    }
    pLine = GBREG->m_pData;
    nStride = GBREG->m_nStride;
    nLineBytes = ((GBW + 7) >> 3) - 1;
    nBitsLeft = GBW - (nLineBytes << 3);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0195]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            if(h > 0) {
                pLine1 = pLine - nStride;
                line1 = *pLine1++;
                CONTEXT = (line1 >> 1) & 0x03f0;
                for(cc = 0; cc < nLineBytes; cc++) {
                    line1 = (line1 << 8) | (*pLine1++);
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal
                                  | ((line1 >> (k + 1)) & 0x0010);
                    }
                    pLine[cc] = cVal;
                }
                line1 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal
                              | ((line1 >> (8 - k)) & 0x0010);
                }
                pLine[nLineBytes] = cVal;
            } else {
                CONTEXT = 0;
                for(cc = 0; cc < nLineBytes; cc++) {
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal;
                    }
                    pLine[cc] = cVal;
                }
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal;
                }
                pLine[nLineBytes] = cVal;
            }
        }
        pLine += nStride;
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_Template3_unopt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    GBREG->fill(0);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0195]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            line1 = GBREG->getPixel(1, h - 1);
            line1 |= GBREG->getPixel(0, h - 1) << 1;
            line2 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, h)) {
                    bVal = 0;
                } else {
                    CONTEXT = line2;
                    CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 4;
                    CONTEXT |= line1 << 5;
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                }
                if(bVal) {
                    GBREG->setPixel(w, h, bVal);
                }
                line1 = ((line1 << 1) | GBREG->getPixel(w + 2, h - 1)) & 0x1f;
                line2 = ((line2 << 1) | bVal) & 0x0f;
            }
        }
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_V2(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG;
    FX_DWORD line1, line2, line3;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    GBREG->fill(0);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            switch(GBTEMPLATE) {
                case 0:
                    CONTEXT = 0x9b25;
                    break;
                case 1:
                    CONTEXT = 0x0795;
                    break;
                case 2:
                    CONTEXT = 0x00e5;
                    break;
                case 3:
                    CONTEXT = 0x0195;
                    break;
            }
            SLTP = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(h, h - 1);
        } else {
            switch(GBTEMPLATE) {
                case 0: {
                        line1 = GBREG->getPixel(1, h - 2);
                        line1 |= GBREG->getPixel(0, h - 2) << 1;
                        line2 = GBREG->getPixel(2, h - 1);
                        line2 |= GBREG->getPixel(1, h - 1) << 1;
                        line2 |= GBREG->getPixel(0, h - 1) << 2;
                        line3 = 0;
                        for(FX_DWORD w = 0; w < GBW; w++) {
                            if(USESKIP && SKIP->getPixel(w, h)) {
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
                            if(bVal) {
                                GBREG->setPixel(w, h, bVal);
                            }
                            line1 = ((line1 << 1) | GBREG->getPixel(w + 2, h - 2)) & 0x07;
                            line2 = ((line2 << 1) | GBREG->getPixel(w + 3, h - 1)) & 0x1f;
                            line3 = ((line3 << 1) | bVal) & 0x0f;
                        }
                    }
                    break;
                case 1: {
                        line1 = GBREG->getPixel(2, h - 2);
                        line1 |= GBREG->getPixel(1, h - 2) << 1;
                        line1 |= GBREG->getPixel(0, h - 2) << 2;
                        line2 = GBREG->getPixel(2, h - 1);
                        line2 |= GBREG->getPixel(1, h - 1) << 1;
                        line2 |= GBREG->getPixel(0, h - 1) << 2;
                        line3 = 0;
                        for(FX_DWORD w = 0; w < GBW; w++) {
                            if(USESKIP && SKIP->getPixel(w, h)) {
                                bVal = 0;
                            } else {
                                CONTEXT = line3;
                                CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 3;
                                CONTEXT |= line2 << 4;
                                CONTEXT |= line1 << 9;
                                bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                            }
                            if(bVal) {
                                GBREG->setPixel(w, h, bVal);
                            }
                            line1 = ((line1 << 1) | GBREG->getPixel(w + 3, h - 2)) & 0x0f;
                            line2 = ((line2 << 1) | GBREG->getPixel(w + 3, h - 1)) & 0x1f;
                            line3 = ((line3 << 1) | bVal) & 0x07;
                        }
                    }
                    break;
                case 2: {
                        line1 = GBREG->getPixel(1, h - 2);
                        line1 |= GBREG->getPixel(0, h - 2) << 1;
                        line2 = GBREG->getPixel(1, h - 1);
                        line2 |= GBREG->getPixel(0, h - 1) << 1;
                        line3 = 0;
                        for(FX_DWORD w = 0; w < GBW; w++) {
                            if(USESKIP && SKIP->getPixel(w, h)) {
                                bVal = 0;
                            } else {
                                CONTEXT = line3;
                                CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 2;
                                CONTEXT |= line2 << 3;
                                CONTEXT |= line1 << 7;
                                bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                            }
                            if(bVal) {
                                GBREG->setPixel(w, h, bVal);
                            }
                            line1 = ((line1 << 1) | GBREG->getPixel(w + 2, h - 2)) & 0x07;
                            line2 = ((line2 << 1) | GBREG->getPixel(w + 2, h - 1)) & 0x0f;
                            line3 = ((line3 << 1) | bVal) & 0x03;
                        }
                    }
                    break;
                case 3: {
                        line1 = GBREG->getPixel(1, h - 1);
                        line1 |= GBREG->getPixel(0, h - 1) << 1;
                        line2 = 0;
                        for(FX_DWORD w = 0; w < GBW; w++) {
                            if(USESKIP && SKIP->getPixel(w, h)) {
                                bVal = 0;
                            } else {
                                CONTEXT = line2;
                                CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 4;
                                CONTEXT |= line1 << 5;
                                bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                            }
                            if(bVal) {
                                GBREG->setPixel(w, h, bVal);
                            }
                            line1 = ((line1 << 1) | GBREG->getPixel(w + 2, h - 1)) & 0x1f;
                            line2 = ((line2 << 1) | bVal) & 0x0f;
                        }
                    }
                    break;
            }
        }
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_Arith_V1(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT = 0;
    CJBig2_Image *GBREG;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    GBREG->fill(0);
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            switch(GBTEMPLATE) {
                case 0:
                    CONTEXT = 0x9b25;
                    break;
                case 1:
                    CONTEXT = 0x0795;
                    break;
                case 2:
                    CONTEXT = 0x00e5;
                    break;
                case 3:
                    CONTEXT = 0x0195;
                    break;
            }
            SLTP = pArithDecoder->DECODE(&gbContext[CONTEXT]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            for(FX_DWORD w = 0; w < GBW; w++) {
                GBREG->setPixel(w, h, GBREG->getPixel(w, h - 1));
            }
        } else {
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, h)) {
                    GBREG->setPixel(w, h, 0);
                } else {
                    CONTEXT = 0;
                    switch(GBTEMPLATE) {
                        case 0:
                            CONTEXT |= GBREG->getPixel(w - 1, h);
                            CONTEXT |= GBREG->getPixel(w - 2, h) << 1;
                            CONTEXT |= GBREG->getPixel(w - 3, h) << 2;
                            CONTEXT |= GBREG->getPixel(w - 4, h) << 3;
                            CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 4;
                            CONTEXT |= GBREG->getPixel(w + 2, h - 1) << 5;
                            CONTEXT |= GBREG->getPixel(w + 1, h - 1) << 6;
                            CONTEXT |= GBREG->getPixel(w, h - 1) << 7;
                            CONTEXT |= GBREG->getPixel(w - 1, h - 1) << 8;
                            CONTEXT |= GBREG->getPixel(w - 2, h - 1) << 9;
                            CONTEXT |= GBREG->getPixel(w + GBAT[2], h + GBAT[3]) << 10;
                            CONTEXT |= GBREG->getPixel(w + GBAT[4], h + GBAT[5]) << 11;
                            CONTEXT |= GBREG->getPixel(w + 1, h - 2) << 12;
                            CONTEXT |= GBREG->getPixel(w, h - 2) << 13;
                            CONTEXT |= GBREG->getPixel(w - 1, h - 2) << 14;
                            CONTEXT |= GBREG->getPixel(w + GBAT[6], h + GBAT[7]) << 15;
                            break;
                        case 1:
                            CONTEXT |= GBREG->getPixel(w - 1, h);
                            CONTEXT |= GBREG->getPixel(w - 2, h) << 1;
                            CONTEXT |= GBREG->getPixel(w - 3, h) << 2;
                            CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 3;
                            CONTEXT |= GBREG->getPixel(w + 2, h - 1) << 4;
                            CONTEXT |= GBREG->getPixel(w + 1, h - 1) << 5;
                            CONTEXT |= GBREG->getPixel(w, h - 1) << 6;
                            CONTEXT |= GBREG->getPixel(w - 1, h - 1) << 7;
                            CONTEXT |= GBREG->getPixel(w - 2, h - 1) << 8;
                            CONTEXT |= GBREG->getPixel(w + 2, h - 2) << 9;
                            CONTEXT |= GBREG->getPixel(w + 1, h - 2) << 10;
                            CONTEXT |= GBREG->getPixel(w, h - 2) << 11;
                            CONTEXT |= GBREG->getPixel(w - 1, h - 2) << 12;
                            break;
                        case 2:
                            CONTEXT |= GBREG->getPixel(w - 1, h);
                            CONTEXT |= GBREG->getPixel(w - 2, h) << 1;
                            CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 2;
                            CONTEXT |= GBREG->getPixel(w + 1, h - 1) << 3;
                            CONTEXT |= GBREG->getPixel(w, h - 1) << 4;
                            CONTEXT |= GBREG->getPixel(w - 1, h - 1) << 5;
                            CONTEXT |= GBREG->getPixel(w - 2, h - 1) << 6;
                            CONTEXT |= GBREG->getPixel(w + 1, h - 2) << 7;
                            CONTEXT |= GBREG->getPixel(w, h - 2) << 8;
                            CONTEXT |= GBREG->getPixel(w - 1, h - 2) << 9;
                            break;
                        case 3:
                            CONTEXT |= GBREG->getPixel(w - 1, h);
                            CONTEXT |= GBREG->getPixel(w - 2, h) << 1;
                            CONTEXT |= GBREG->getPixel(w - 3, h) << 2;
                            CONTEXT |= GBREG->getPixel(w - 4, h) << 3;
                            CONTEXT |= GBREG->getPixel(w + GBAT[0], h + GBAT[1]) << 4;
                            CONTEXT |= GBREG->getPixel(w + 1, h - 1) << 5;
                            CONTEXT |= GBREG->getPixel(w, h - 1) << 6;
                            CONTEXT |= GBREG->getPixel(w - 1, h - 1) << 7;
                            CONTEXT |= GBREG->getPixel(w - 2, h - 1) << 8;
                            CONTEXT |= GBREG->getPixel(w - 3, h - 1) << 9;
                            break;
                    }
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    GBREG->setPixel(w, h, bVal);
                }
            }
        }
    }
    return GBREG;
}
CJBig2_Image *CJBig2_GRDProc::decode_MMR(CJBig2_BitStream *pStream)
{
    int bitpos, i;
    CJBig2_Image *pImage;
    JBIG2_ALLOC(pImage, CJBig2_Image(GBW, GBH));
    if (pImage->m_pData == NULL) {
        delete pImage;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        return NULL;
    }
    bitpos = (int)pStream->getBitPos();
    _FaxG4Decode(m_pModule, pStream->getBuf(), pStream->getLength(), &bitpos, pImage->m_pData, GBW, GBH, pImage->m_nStride);
    pStream->setBitPos(bitpos);
    for(i = 0; (FX_DWORD)i < pImage->m_nStride * GBH; i++) {
        pImage->m_pData[i] = ~pImage->m_pData[i];
    }
    return pImage;
}
CJBig2_Image *CJBig2_GRRDProc::decode(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *grContext)
{
    if (GRW == 0 || GRH == 0) {
        CJBig2_Image* pImage;
        JBIG2_ALLOC(pImage, CJBig2_Image(GRW, GRH));
        return pImage;
    }
    if(GRTEMPLATE == 0) {
        if((GRAT[0] == (signed char) - 1) && (GRAT[1] == (signed char) - 1)
                && (GRAT[2] == (signed char) - 1) && (GRAT[3] == (signed char) - 1)
                && (GRREFERENCEDX == 0) && (GRW == (FX_DWORD)GRREFERENCE->m_nWidth)) {
            return decode_Template0_opt(pArithDecoder, grContext);
        } else {
            return decode_Template0_unopt(pArithDecoder, grContext);
        }
    } else {
        if((GRREFERENCEDX == 0) && (GRW == (FX_DWORD)GRREFERENCE->m_nWidth)) {
            return decode_Template1_opt(pArithDecoder, grContext);
        } else {
            return decode_Template1_unopt(pArithDecoder, grContext);
        }
    }
}
CJBig2_Image *CJBig2_GRRDProc::decode_Template0_unopt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *grContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GRREG;
    FX_DWORD line1, line2, line3, line4, line5;
    LTP = 0;
    JBIG2_ALLOC(GRREG, CJBig2_Image(GRW, GRH));
    GRREG->fill(0);
    for(FX_DWORD h = 0; h < GRH; h++) {
        if(TPGRON) {
            SLTP = pArithDecoder->DECODE(&grContext[0x0010]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 0) {
            line1 = GRREG->getPixel(1, h - 1);
            line1 |= GRREG->getPixel(0, h - 1) << 1;
            line2 = 0;
            line3 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY - 1);
            line3 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY - 1) << 1;
            line4 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY);
            line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY) << 1;
            line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY) << 2;
            line5 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
            line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY + 1) << 1;
            line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY + 1) << 2;
            for(FX_DWORD w = 0; w < GRW; w++) {
                CONTEXT = line5;
                CONTEXT |= line4 << 3;
                CONTEXT |= line3 << 6;
                CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + GRAT[2], h - GRREFERENCEDY + GRAT[3]) << 8;
                CONTEXT |= line2 << 9;
                CONTEXT |= line1 << 10;
                CONTEXT |= GRREG->getPixel(w + GRAT[0], h + GRAT[1]) << 12;
                bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
                GRREG->setPixel(w, h, bVal);
                line1 = ((line1 << 1) | GRREG->getPixel(w + 2, h - 1)) & 0x03;
                line2 = ((line2 << 1) | bVal) & 0x01;
                line3 = ((line3 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY - 1)) & 0x03;
                line4 = ((line4 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY)) & 0x07;
                line5 = ((line5 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY + 1)) & 0x07;
            }
        } else {
            line1 = GRREG->getPixel(1, h - 1);
            line1 |= GRREG->getPixel(0, h - 1) << 1;
            line2 = 0;
            line3 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY - 1);
            line3 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY - 1) << 1;
            line4 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY);
            line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY) << 1;
            line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY) << 2;
            line5 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
            line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY + 1) << 1;
            line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY + 1) << 2;
            for(FX_DWORD w = 0; w < GRW; w++) {
                bVal = GRREFERENCE->getPixel(w, h);
                if(!(TPGRON && (bVal == GRREFERENCE->getPixel(w - 1, h - 1))
                        && (bVal == GRREFERENCE->getPixel(w, h - 1))
                        && (bVal == GRREFERENCE->getPixel(w + 1, h - 1))
                        && (bVal == GRREFERENCE->getPixel(w - 1, h))
                        && (bVal == GRREFERENCE->getPixel(w + 1, h))
                        && (bVal == GRREFERENCE->getPixel(w - 1, h + 1))
                        && (bVal == GRREFERENCE->getPixel(w, h + 1))
                        && (bVal == GRREFERENCE->getPixel(w + 1, h + 1)))) {
                    CONTEXT = line5;
                    CONTEXT |= line4 << 3;
                    CONTEXT |= line3 << 6;
                    CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + GRAT[2], h - GRREFERENCEDY + GRAT[3]) << 8;
                    CONTEXT |= line2 << 9;
                    CONTEXT |= line1 << 10;
                    CONTEXT |= GRREG->getPixel(w + GRAT[0], h + GRAT[1]) << 12;
                    bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
                }
                GRREG->setPixel(w, h, bVal);
                line1 = ((line1 << 1) | GRREG->getPixel(w + 2, h - 1)) & 0x03;
                line2 = ((line2 << 1) | bVal) & 0x01;
                line3 = ((line3 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY - 1)) & 0x03;
                line4 = ((line4 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY)) & 0x07;
                line5 = ((line5 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY + 1)) & 0x07;
            }
        }
    }
    return GRREG;
}
CJBig2_Image *CJBig2_GRRDProc::decode_Template0_opt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *grContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GRREG;
    FX_DWORD line1, line1_r, line2_r, line3_r;
    FX_BYTE *pLine, *pLineR, cVal;
    FX_INTPTR nStride, nStrideR, nOffset;
    FX_INT32 k, nBits;
    FX_INT32 GRWR, GRHR;
    FX_INT32 GRW, GRH;
    GRW = (FX_INT32)CJBig2_GRRDProc::GRW;
    GRH = (FX_INT32)CJBig2_GRRDProc::GRH;
    LTP = 0;
    JBIG2_ALLOC(GRREG, CJBig2_Image(GRW, GRH));
    if (GRREG->m_pData == NULL) {
        delete GRREG;
        m_pModule->JBig2_Error("Generic refinement region decoding procedure: Create Image Failed with width = %d, height = %d\n", GRW, GRH);
        return NULL;
    }
    pLine = GRREG->m_pData;
    pLineR = GRREFERENCE->m_pData;
    nStride = GRREG->m_nStride;
    nStrideR = GRREFERENCE->m_nStride;
    GRWR = (FX_INT32)GRREFERENCE->m_nWidth;
    GRHR = (FX_INT32)GRREFERENCE->m_nHeight;
    if (GRREFERENCEDY < -GRHR + 1 || GRREFERENCEDY > GRHR - 1) {
        GRREFERENCEDY = 0;
    }
    nOffset = -GRREFERENCEDY * nStrideR;
    for (FX_INT32 h = 0; h < GRH; h++) {
        if(TPGRON) {
            SLTP = pArithDecoder->DECODE(&grContext[0x0010]);
            LTP = LTP ^ SLTP;
        }
        line1 = (h > 0) ? pLine[-nStride] << 4 : 0;
        FX_INT32 reference_h = h - GRREFERENCEDY;
        FX_BOOL line1_r_ok = (reference_h > 0 && reference_h < GRHR + 1);
        FX_BOOL line2_r_ok = (reference_h > -1 && reference_h < GRHR);
        FX_BOOL line3_r_ok = (reference_h > -2 && reference_h < GRHR - 1);
        line1_r = line1_r_ok ? pLineR[nOffset - nStrideR] : 0;
        line2_r = line2_r_ok ? pLineR[nOffset] : 0;
        line3_r = line3_r_ok ? pLineR[nOffset + nStrideR] : 0;
        if(LTP == 0) {
            CONTEXT = (line1 & 0x1c00) | (line1_r & 0x01c0)
                      | ((line2_r >> 3) & 0x0038) | ((line3_r >> 6) & 0x0007);
            for (FX_INT32 w = 0; w < GRW; w += 8) {
                nBits = GRW - w > 8 ? 8 : GRW - w;
                if (h > 0)
                    line1 = (line1 << 8) |
                            (w + 8 < GRW ? pLine[-nStride + (w >> 3) + 1] << 4 : 0);
                if (h > GRHR + GRREFERENCEDY + 1) {
                    line1_r = 0;
                    line2_r  = 0;
                    line3_r = 0;
                } else {
                    if(line1_r_ok)
                        line1_r = (line1_r << 8) |
                                  (w + 8 < GRWR ? pLineR[nOffset - nStrideR + (w >> 3) + 1] : 0);
                    if(line2_r_ok)
                        line2_r = (line2_r << 8) |
                                  (w + 8 < GRWR ? pLineR[nOffset + (w >> 3) + 1] : 0);
                    if(line3_r_ok)
                        line3_r = (line3_r << 8) |
                                  (w + 8 < GRWR ? pLineR[nOffset + nStrideR + (w >> 3) + 1] : 0);
                    else {
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
            CONTEXT = (line1 & 0x1c00) | (line1_r & 0x01c0)
                      | ((line2_r >> 3) & 0x0038) | ((line3_r >> 6) & 0x0007);
            for (FX_INT32 w = 0; w < GRW; w += 8) {
                nBits = GRW - w > 8 ? 8 : GRW - w;
                if (h > 0)
                    line1 = (line1 << 8) |
                            (w + 8 < GRW ? pLine[-nStride + (w >> 3) + 1] << 4 : 0);
                if(line1_r_ok)
                    line1_r = (line1_r << 8) |
                              (w + 8 < GRWR ? pLineR[nOffset - nStrideR + (w >> 3) + 1] : 0);
                if(line2_r_ok)
                    line2_r = (line2_r << 8) |
                              (w + 8 < GRWR ? pLineR[nOffset + (w >> 3) + 1] : 0);
                if(line3_r_ok)
                    line3_r = (line3_r << 8) |
                              (w + 8 < GRWR ? pLineR[nOffset + nStrideR + (w >> 3) + 1] : 0);
                else {
                    line3_r = 0;
                }
                cVal = 0;
                for (k = 0; k < nBits; k++) {
                    bVal = GRREFERENCE->getPixel(w + k, h);
                    if(!(TPGRON && (bVal == GRREFERENCE->getPixel(w + k - 1, h - 1))
                            && (bVal == GRREFERENCE->getPixel(w + k, h - 1))
                            && (bVal == GRREFERENCE->getPixel(w + k + 1, h - 1))
                            && (bVal == GRREFERENCE->getPixel(w + k - 1, h))
                            && (bVal == GRREFERENCE->getPixel(w + k + 1, h))
                            && (bVal == GRREFERENCE->getPixel(w + k - 1, h + 1))
                            && (bVal == GRREFERENCE->getPixel(w + k, h + 1))
                            && (bVal == GRREFERENCE->getPixel(w + k + 1, h + 1)))) {
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
    return GRREG;
}
CJBig2_Image *CJBig2_GRRDProc::decode_Template1_unopt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *grContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GRREG;
    FX_DWORD line1, line2, line3, line4, line5;
    LTP = 0;
    JBIG2_ALLOC(GRREG, CJBig2_Image(GRW, GRH));
    GRREG->fill(0);
    for(FX_DWORD h = 0; h < GRH; h++) {
        if(TPGRON) {
            SLTP = pArithDecoder->DECODE(&grContext[0x0008]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 0) {
            line1 = GRREG->getPixel(1, h - 1);
            line1 |= GRREG->getPixel(0, h - 1) << 1;
            line1 |= GRREG->getPixel(-1, h - 1) << 2;
            line2 = 0;
            line3 = GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY - 1);
            line4 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY);
            line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY) << 1;
            line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY) << 2;
            line5 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
            line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY + 1) << 1;
            for(FX_DWORD w = 0; w < GRW; w++) {
                CONTEXT = line5;
                CONTEXT |= line4 << 2;
                CONTEXT |= line3 << 5;
                CONTEXT |= line2 << 6;
                CONTEXT |= line1 << 7;
                bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
                GRREG->setPixel(w, h, bVal);
                line1 = ((line1 << 1) | GRREG->getPixel(w + 2, h - 1)) & 0x07;
                line2 = ((line2 << 1) | bVal) & 0x01;
                line3 = ((line3 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY - 1)) & 0x01;
                line4 = ((line4 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY)) & 0x07;
                line5 = ((line5 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY + 1)) & 0x03;
            }
        } else {
            line1 = GRREG->getPixel(1, h - 1);
            line1 |= GRREG->getPixel(0, h - 1) << 1;
            line1 |= GRREG->getPixel(-1, h - 1) << 2;
            line2 = 0;
            line3 = GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY - 1);
            line4 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY);
            line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY) << 1;
            line4 |= GRREFERENCE->getPixel(-GRREFERENCEDX - 1, h - GRREFERENCEDY) << 2;
            line5 = GRREFERENCE->getPixel(-GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
            line5 |= GRREFERENCE->getPixel(-GRREFERENCEDX, h - GRREFERENCEDY + 1) << 1;
            for(FX_DWORD w = 0; w < GRW; w++) {
                bVal = GRREFERENCE->getPixel(w, h);
                if(!(TPGRON && (bVal == GRREFERENCE->getPixel(w - 1, h - 1))
                        && (bVal == GRREFERENCE->getPixel(w, h - 1))
                        && (bVal == GRREFERENCE->getPixel(w + 1, h - 1))
                        && (bVal == GRREFERENCE->getPixel(w - 1, h))
                        && (bVal == GRREFERENCE->getPixel(w + 1, h))
                        && (bVal == GRREFERENCE->getPixel(w - 1, h + 1))
                        && (bVal == GRREFERENCE->getPixel(w, h + 1))
                        && (bVal == GRREFERENCE->getPixel(w + 1, h + 1)))) {
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
                line3 = ((line3 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY - 1)) & 0x01;
                line4 = ((line4 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY)) & 0x07;
                line5 = ((line5 << 1) | GRREFERENCE->getPixel(w - GRREFERENCEDX + 2, h - GRREFERENCEDY + 1)) & 0x03;
            }
        }
    }
    return GRREG;
}
CJBig2_Image *CJBig2_GRRDProc::decode_Template1_opt(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *grContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GRREG;
    FX_DWORD line1, line1_r, line2_r, line3_r;
    FX_BYTE *pLine, *pLineR, cVal;
    FX_INTPTR nStride, nStrideR, nOffset;
    FX_INT32 k, nBits;
    FX_INT32 GRWR, GRHR;
    FX_INT32 GRW, GRH;
    GRW = (FX_INT32)CJBig2_GRRDProc::GRW;
    GRH = (FX_INT32)CJBig2_GRRDProc::GRH;
    LTP = 0;
    JBIG2_ALLOC(GRREG, CJBig2_Image(GRW, GRH));
    if (GRREG->m_pData == NULL) {
        delete GRREG;
        m_pModule->JBig2_Error("Generic refinement region decoding procedure: Create Image Failed with width = %d, height = %d\n", GRW, GRH);
        return NULL;
    }
    pLine = GRREG->m_pData;
    pLineR = GRREFERENCE->m_pData;
    nStride = GRREG->m_nStride;
    nStrideR = GRREFERENCE->m_nStride;
    GRWR = (FX_INT32)GRREFERENCE->m_nWidth;
    GRHR = (FX_INT32)GRREFERENCE->m_nHeight;
    if (GRREFERENCEDY < -GRHR + 1 || GRREFERENCEDY > GRHR - 1) {
        GRREFERENCEDY = 0;
    }
    nOffset = -GRREFERENCEDY * nStrideR;
    for (FX_INT32 h = 0; h < GRH; h++) {
        if(TPGRON) {
            SLTP = pArithDecoder->DECODE(&grContext[0x0008]);
            LTP = LTP ^ SLTP;
        }
        line1 = (h > 0) ? pLine[-nStride] << 1 : 0;
        FX_INT32 reference_h = h - GRREFERENCEDY;
        FX_BOOL line1_r_ok = (reference_h > 0 && reference_h < GRHR + 1);
        FX_BOOL line2_r_ok = (reference_h > -1 && reference_h < GRHR);
        FX_BOOL line3_r_ok = (reference_h > -2 && reference_h < GRHR - 1);
        line1_r = line1_r_ok ? pLineR[nOffset - nStrideR] : 0;
        line2_r = line2_r_ok ? pLineR[nOffset] : 0;
        line3_r = line3_r_ok ? pLineR[nOffset + nStrideR] : 0;
        if(LTP == 0) {
            CONTEXT = (line1 & 0x0380) | ((line1_r >> 2) & 0x0020)
                      | ((line2_r >> 4) & 0x001c) | ((line3_r >> 6) & 0x0003);
            for (FX_INT32 w = 0; w < GRW; w += 8) {
                nBits = GRW - w > 8 ? 8 : GRW - w;
                if (h > 0)
                    line1 = (line1 << 8) |
                            (w + 8 < GRW ? pLine[-nStride + (w >> 3) + 1] << 1 : 0);
                if(line1_r_ok)
                    line1_r = (line1_r << 8) |
                              (w + 8 < GRWR ? pLineR[nOffset - nStrideR + (w >> 3) + 1] : 0);
                if(line2_r_ok)
                    line2_r = (line2_r << 8) |
                              (w + 8 < GRWR ? pLineR[nOffset + (w >> 3) + 1] : 0);
                if(line3_r_ok)
                    line3_r = (line3_r << 8) |
                              (w + 8 < GRWR ? pLineR[nOffset + nStrideR + (w >> 3) + 1] : 0);
                else {
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
            CONTEXT = (line1 & 0x0380) | ((line1_r >> 2) & 0x0020)
                      | ((line2_r >> 4) & 0x001c) | ((line3_r >> 6) & 0x0003);
            for (FX_INT32 w = 0; w < GRW; w += 8) {
                nBits = GRW - w > 8 ? 8 : GRW - w;
                if (h > 0)
                    line1 = (line1 << 8) |
                            (w + 8 < GRW ? pLine[-nStride + (w >> 3) + 1] << 1 : 0);
                if(line1_r_ok)
                    line1_r = (line1_r << 8) |
                              (w + 8 < GRWR ? pLineR[nOffset - nStrideR + (w >> 3) + 1] : 0);
                if(line2_r_ok)
                    line2_r = (line2_r << 8) |
                              (w + 8 < GRWR ? pLineR[nOffset + (w >> 3) + 1] : 0);
                if(line3_r_ok)
                    line3_r = (line3_r << 8) |
                              (w + 8 < GRWR ? pLineR[nOffset + nStrideR + (w >> 3) + 1] : 0);
                else {
                    line3_r = 0;
                }
                cVal = 0;
                for (k = 0; k < nBits; k++) {
                    bVal = GRREFERENCE->getPixel(w + k, h);
                    if(!(TPGRON && (bVal == GRREFERENCE->getPixel(w + k - 1, h - 1))
                            && (bVal == GRREFERENCE->getPixel(w + k, h - 1))
                            && (bVal == GRREFERENCE->getPixel(w + k + 1, h - 1))
                            && (bVal == GRREFERENCE->getPixel(w + k - 1, h))
                            && (bVal == GRREFERENCE->getPixel(w + k + 1, h))
                            && (bVal == GRREFERENCE->getPixel(w + k - 1, h + 1))
                            && (bVal == GRREFERENCE->getPixel(w + k, h + 1))
                            && (bVal == GRREFERENCE->getPixel(w + k + 1, h + 1)))) {
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
    return GRREG;
}
CJBig2_Image *CJBig2_GRRDProc::decode_V1(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *grContext)
{
    FX_BOOL LTP, SLTP, bVal;
    FX_BOOL TPGRPIX, TPGRVAL;
    FX_DWORD CONTEXT;
    CJBig2_Image *GRREG;
    LTP = 0;
    JBIG2_ALLOC(GRREG, CJBig2_Image(GRW, GRH));
    GRREG->fill(0);
    for(FX_DWORD h = 0; h < GRH; h++) {
        if(TPGRON) {
            switch(GRTEMPLATE) {
                case 0:
                    CONTEXT = 0x0010;
                    break;
                case 1:
                    CONTEXT = 0x0008;
                    break;
            }
            SLTP = pArithDecoder->DECODE(&grContext[CONTEXT]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 0) {
            for(FX_DWORD w = 0; w < GRW; w++) {
                CONTEXT = 0;
                switch(GRTEMPLATE) {
                    case 0:
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY + 1) << 1;
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX - 1, h - GRREFERENCEDY + 1) << 2;
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY) << 3;
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY) << 4;
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX - 1, h - GRREFERENCEDY) << 5;
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY - 1) << 6;
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY - 1) << 7;
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + GRAT[2], h - GRREFERENCEDY + GRAT[3]) << 8;
                        CONTEXT |= GRREG->getPixel(w - 1, h) << 9;
                        CONTEXT |= GRREG->getPixel(w + 1, h - 1) << 10;
                        CONTEXT |= GRREG->getPixel(w, h - 1) << 11;
                        CONTEXT |= GRREG->getPixel(w + GRAT[0], h + GRAT[1]) << 12;
                        break;
                    case 1:
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY + 1) << 1;
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY) << 2;
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY) << 3;
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX - 1, h - GRREFERENCEDY) << 4;
                        CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY - 1) << 5;
                        CONTEXT |= GRREG->getPixel(w - 1, h) << 6;
                        CONTEXT |= GRREG->getPixel(w + 1, h - 1) << 7;
                        CONTEXT |= GRREG->getPixel(w, h - 1) << 8;
                        CONTEXT |= GRREG->getPixel(w - 1, h - 1) << 9;
                        break;
                }
                bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
                GRREG->setPixel(w, h, bVal);
            }
        } else {
            for(FX_DWORD w = 0; w < GRW; w++) {
                bVal = GRREFERENCE->getPixel(w, h);
                if(TPGRON && (bVal == GRREFERENCE->getPixel(w - 1, h - 1))
                        && (bVal == GRREFERENCE->getPixel(w, h - 1))
                        && (bVal == GRREFERENCE->getPixel(w + 1, h - 1))
                        && (bVal == GRREFERENCE->getPixel(w - 1, h))
                        && (bVal == GRREFERENCE->getPixel(w + 1, h))
                        && (bVal == GRREFERENCE->getPixel(w - 1, h + 1))
                        && (bVal == GRREFERENCE->getPixel(w, h + 1))
                        && (bVal == GRREFERENCE->getPixel(w + 1, h + 1))) {
                    TPGRPIX = 1;
                    TPGRVAL = bVal;
                } else {
                    TPGRPIX = 0;
                }
                if(TPGRPIX) {
                    GRREG->setPixel(w, h, TPGRVAL);
                } else {
                    CONTEXT = 0;
                    switch(GRTEMPLATE) {
                        case 0:
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY + 1) << 1;
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX - 1, h - GRREFERENCEDY + 1) << 2;
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY) << 3;
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY) << 4;
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX - 1, h - GRREFERENCEDY) << 5;
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY - 1) << 6;
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY - 1) << 7;
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + GRAT[2], h - GRREFERENCEDY + GRAT[3]) << 8;
                            CONTEXT |= GRREG->getPixel(w - 1, h) << 9;
                            CONTEXT |= GRREG->getPixel(w + 1, h - 1) << 10;
                            CONTEXT |= GRREG->getPixel(w, h - 1) << 11;
                            CONTEXT |= GRREG->getPixel(w + GRAT[0], h + GRAT[1]) << 12;
                            break;
                        case 1:
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY + 1);
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY + 1) << 1;
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX + 1, h - GRREFERENCEDY) << 2;
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY) << 3;
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX - 1, h - GRREFERENCEDY) << 4;
                            CONTEXT |= GRREFERENCE->getPixel(w - GRREFERENCEDX, h - GRREFERENCEDY - 1) << 5;
                            CONTEXT |= GRREG->getPixel(w - 1, h) << 6;
                            CONTEXT |= GRREG->getPixel(w + 1, h - 1) << 7;
                            CONTEXT |= GRREG->getPixel(w, h - 1) << 8;
                            CONTEXT |= GRREG->getPixel(w - 1, h - 1) << 9;
                            break;
                    }
                    bVal = pArithDecoder->DECODE(&grContext[CONTEXT]);
                    GRREG->setPixel(w, h, bVal);
                }
            }
        }
    }
    return GRREG;
}
CJBig2_Image *CJBig2_TRDProc::decode_Huffman(CJBig2_BitStream *pStream, JBig2ArithCtx *grContext)
{
    FX_INT32 STRIPT, FIRSTS;
    FX_DWORD NINSTANCES;
    FX_INT32 DT, DFS, CURS;
    FX_BYTE CURT;
    FX_INT32 SI, TI;
    FX_DWORD IDI;
    CJBig2_Image *IBI;
    FX_DWORD WI, HI;
    FX_INT32 IDS;
    FX_BOOL RI;
    FX_INT32 RDWI, RDHI, RDXI, RDYI;
    CJBig2_Image *IBOI;
    FX_DWORD WOI, HOI;
    CJBig2_Image *SBREG;
    FX_BOOL bFirst;
    FX_DWORD nTmp;
    FX_INT32 nVal, nBits;
    CJBig2_HuffmanDecoder *pHuffmanDecoder;
    CJBig2_GRRDProc *pGRRD;
    CJBig2_ArithDecoder *pArithDecoder;
    JBIG2_ALLOC(pHuffmanDecoder, CJBig2_HuffmanDecoder(pStream));
    JBIG2_ALLOC(SBREG, CJBig2_Image(SBW, SBH));
    SBREG->fill(SBDEFPIXEL);
    if(pHuffmanDecoder->decodeAValue(SBHUFFDT, &STRIPT) != 0) {
        m_pModule->JBig2_Error("text region decoding procedure (huffman): too short.");
        goto failed;
    }
    STRIPT *= SBSTRIPS;
    STRIPT = -STRIPT;
    FIRSTS = 0;
    NINSTANCES = 0;
    while(NINSTANCES < SBNUMINSTANCES) {
        if(pHuffmanDecoder->decodeAValue(SBHUFFDT, &DT) != 0) {
            m_pModule->JBig2_Error("text region decoding procedure (huffman): too short.");
            goto failed;
        }
        DT *= SBSTRIPS;
        STRIPT = STRIPT + DT;
        bFirst = TRUE;
        for(;;) {
            if(bFirst) {
                if(pHuffmanDecoder->decodeAValue(SBHUFFFS, &DFS) != 0) {
                    m_pModule->JBig2_Error("text region decoding procedure (huffman): too short.");
                    goto failed;
                }
                FIRSTS = FIRSTS + DFS;
                CURS = FIRSTS;
                bFirst = FALSE;
            } else {
                nVal = pHuffmanDecoder->decodeAValue(SBHUFFDS, &IDS);
                if(nVal == JBIG2_OOB) {
                    break;
                } else if(nVal != 0) {
                    m_pModule->JBig2_Error("text region decoding procedure (huffman): too short.");
                    goto failed;
                } else {
                    CURS = CURS + IDS + SBDSOFFSET;
                }
            }
            if(SBSTRIPS == 1) {
                CURT = 0;
            } else {
                nTmp = 1;
                while((FX_DWORD)(1 << nTmp) < SBSTRIPS) {
                    nTmp ++;
                }
                if(pStream->readNBits(nTmp, &nVal) != 0) {
                    m_pModule->JBig2_Error("text region decoding procedure (huffman): too short.");
                    goto failed;
                }
                CURT = nVal;
            }
            TI = STRIPT + CURT;
            nVal = 0;
            nBits = 0;
            for(;;) {
                if(pStream->read1Bit(&nTmp) != 0) {
                    m_pModule->JBig2_Error("text region decoding procedure (huffman): too short.");
                    goto failed;
                }
                nVal = (nVal << 1) | nTmp;
                nBits ++;
                for(IDI = 0; IDI < SBNUMSYMS; IDI++) {
                    if((nBits == SBSYMCODES[IDI].codelen) && (nVal == SBSYMCODES[IDI].code)) {
                        break;
                    }
                }
                if(IDI < SBNUMSYMS) {
                    break;
                }
            }
            if(SBREFINE == 0) {
                RI = 0;
            } else {
                if(pStream->read1Bit(&RI) != 0) {
                    m_pModule->JBig2_Error("text region decoding procedure (huffman): too short.");
                    goto failed;
                }
            }
            if(RI == 0) {
                IBI = SBSYMS[IDI];
            } else {
                if((pHuffmanDecoder->decodeAValue(SBHUFFRDW, &RDWI) != 0)
                        || (pHuffmanDecoder->decodeAValue(SBHUFFRDH, &RDHI) != 0)
                        || (pHuffmanDecoder->decodeAValue(SBHUFFRDX, &RDXI) != 0)
                        || (pHuffmanDecoder->decodeAValue(SBHUFFRDY, &RDYI) != 0)
                        || (pHuffmanDecoder->decodeAValue(SBHUFFRSIZE, &nVal) != 0)) {
                    m_pModule->JBig2_Error("text region decoding procedure (huffman): too short.");
                    goto failed;
                }
                pStream->alignByte();
                nTmp = pStream->getOffset();
                IBOI = SBSYMS[IDI];
                if (!IBOI) {
                    goto failed;
                }
                WOI = IBOI->m_nWidth;
                HOI = IBOI->m_nHeight;
                if ((int)(WOI + RDWI) < 0 || (int)(HOI + RDHI) < 0) {
                    m_pModule->JBig2_Error("text region decoding procedure (huffman): Invalid RDWI or RDHI value.");
                    goto failed;
                }
                JBIG2_ALLOC(pGRRD, CJBig2_GRRDProc());
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
                JBIG2_ALLOC(pArithDecoder, CJBig2_ArithDecoder(pStream));
                IBI = pGRRD->decode(pArithDecoder, grContext);
                if(IBI == NULL) {
                    delete pGRRD;
                    delete pArithDecoder;
                    goto failed;
                }
                delete pArithDecoder;
                pStream->alignByte();
                pStream->offset(2);
                if((FX_DWORD)nVal != (pStream->getOffset() - nTmp)) {
                    delete IBI;
                    delete pGRRD;
                    m_pModule->JBig2_Error("text region decoding procedure (huffman):"
                                           "bytes processed by generic refinement region decoding procedure doesn't equal SBHUFFRSIZE.");
                    goto failed;
                }
                delete pGRRD;
            }
            if (!IBI) {
                continue;
            }
            WI = IBI->m_nWidth;
            HI = IBI->m_nHeight;
            if(TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPRIGHT)
                                   || (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
                CURS = CURS + WI - 1;
            } else if(TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_BOTTOMLEFT)
                                          || (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
                CURS = CURS + HI - 1;
            }
            SI = CURS;
            if(TRANSPOSED == 0) {
                switch(REFCORNER) {
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
                switch(REFCORNER) {
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
            if(RI != 0) {
                delete IBI;
            }
            if(TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPLEFT)
                                   || (REFCORNER == JBIG2_CORNER_BOTTOMLEFT))) {
                CURS = CURS + WI - 1;
            } else if(TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_TOPLEFT)
                                          || (REFCORNER == JBIG2_CORNER_TOPRIGHT))) {
                CURS = CURS + HI - 1;
            }
            NINSTANCES = NINSTANCES + 1;
        }
    }
    delete pHuffmanDecoder;
    return SBREG;
failed:
    delete pHuffmanDecoder;
    delete SBREG;
    return NULL;
}
CJBig2_Image *CJBig2_TRDProc::decode_Arith(CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *grContext,
        JBig2IntDecoderState *pIDS)
{
    FX_INT32 STRIPT, FIRSTS;
    FX_DWORD NINSTANCES;
    FX_INT32 DT, DFS, CURS;
    FX_INT32 CURT;
    FX_INT32 SI, TI;
    FX_DWORD IDI;
    CJBig2_Image *IBI;
    FX_DWORD WI, HI;
    FX_INT32 IDS;
    FX_BOOL RI;
    FX_INT32 RDWI, RDHI, RDXI, RDYI;
    CJBig2_Image *IBOI;
    FX_DWORD WOI, HOI;
    CJBig2_Image *SBREG;
    FX_BOOL bFirst;
    FX_INT32 nRet, nVal;
    FX_INT32 bRetained;
    CJBig2_ArithIntDecoder *IADT, *IAFS, *IADS, *IAIT, *IARI, *IARDW, *IARDH, *IARDX, *IARDY;
    CJBig2_ArithIaidDecoder *IAID;
    CJBig2_GRRDProc *pGRRD;
    if(pIDS) {
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
        JBIG2_ALLOC(IADT, CJBig2_ArithIntDecoder());
        JBIG2_ALLOC(IAFS, CJBig2_ArithIntDecoder());
        JBIG2_ALLOC(IADS, CJBig2_ArithIntDecoder());
        JBIG2_ALLOC(IAIT, CJBig2_ArithIntDecoder());
        JBIG2_ALLOC(IARI, CJBig2_ArithIntDecoder());
        JBIG2_ALLOC(IARDW, CJBig2_ArithIntDecoder());
        JBIG2_ALLOC(IARDH, CJBig2_ArithIntDecoder());
        JBIG2_ALLOC(IARDX, CJBig2_ArithIntDecoder());
        JBIG2_ALLOC(IARDY, CJBig2_ArithIntDecoder());
        JBIG2_ALLOC(IAID , CJBig2_ArithIaidDecoder(SBSYMCODELEN));
        bRetained = FALSE;
    }
    JBIG2_ALLOC(SBREG, CJBig2_Image(SBW, SBH));
    SBREG->fill(SBDEFPIXEL);
    if(IADT->decode(pArithDecoder, &STRIPT) == -1) {
        m_pModule->JBig2_Error("text region decoding procedure (arith): too short.");
        goto failed;
    }
    STRIPT *= SBSTRIPS;
    STRIPT = -STRIPT;
    FIRSTS = 0;
    NINSTANCES = 0;
    while(NINSTANCES < SBNUMINSTANCES) {
        if(IADT->decode(pArithDecoder, &DT) == -1) {
            m_pModule->JBig2_Error("text region decoding procedure (arith): too short.");
            goto failed;
        }
        DT *= SBSTRIPS;
        STRIPT = STRIPT + DT;
        bFirst = TRUE;
        for(;;) {
            if(bFirst) {
                if(IAFS->decode(pArithDecoder, &DFS) == -1) {
                    m_pModule->JBig2_Error("text region decoding procedure (arith): too short.");
                    goto failed;
                }
                FIRSTS = FIRSTS + DFS;
                CURS = FIRSTS;
                bFirst = FALSE;
            } else {
                nRet = IADS->decode(pArithDecoder, &IDS);
                if(nRet == JBIG2_OOB) {
                    break;
                } else if(nRet != 0) {
                    m_pModule->JBig2_Error("text region decoding procedure (arith): too short.");
                    goto failed;
                } else {
                    CURS = CURS + IDS + SBDSOFFSET;
                }
            }
            if (NINSTANCES >= SBNUMINSTANCES) {
                break;
            }
            if(SBSTRIPS == 1) {
                CURT = 0;
            } else {
                if(IAIT->decode(pArithDecoder, &nVal) == -1) {
                    m_pModule->JBig2_Error("text region decoding procedure (arith): too short.");
                    goto failed;
                }
                CURT = nVal;
            }
            TI = STRIPT + CURT;
            if(IAID->decode(pArithDecoder, &nVal) == -1) {
                m_pModule->JBig2_Error("text region decoding procedure (arith): too short.");
                goto failed;
            }
            IDI = nVal;
            if(IDI >= SBNUMSYMS) {
                m_pModule->JBig2_Error("text region decoding procedure (arith): symbol id out of range.(%d/%d)",
                                       IDI, SBNUMSYMS);
                goto failed;
            }
            if(SBREFINE == 0) {
                RI = 0;
            } else {
                if(IARI->decode(pArithDecoder, &RI) == -1) {
                    m_pModule->JBig2_Error("text region decoding procedure (arith): too short.");
                    goto failed;
                }
            }
            if (!SBSYMS[IDI]) {
                goto failed;
            }
            if(RI == 0) {
                IBI = SBSYMS[IDI];
            } else {
                if((IARDW->decode(pArithDecoder, &RDWI) == -1)
                        || (IARDH->decode(pArithDecoder, &RDHI) == -1)
                        || (IARDX->decode(pArithDecoder, &RDXI) == -1)
                        || (IARDY->decode(pArithDecoder, &RDYI) == -1)) {
                    m_pModule->JBig2_Error("text region decoding procedure (arith): too short.");
                    goto failed;
                }
                IBOI = SBSYMS[IDI];
                WOI = IBOI->m_nWidth;
                HOI = IBOI->m_nHeight;
                if ((int)(WOI + RDWI) < 0 || (int)(HOI + RDHI) < 0) {
                    m_pModule->JBig2_Error("text region decoding procedure (arith): Invalid RDWI or RDHI value.");
                    goto failed;
                }
                JBIG2_ALLOC(pGRRD, CJBig2_GRRDProc());
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
                if(IBI == NULL) {
                    delete pGRRD;
                    goto failed;
                }
                delete pGRRD;
            }
            WI = IBI->m_nWidth;
            HI = IBI->m_nHeight;
            if(TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPRIGHT)
                                   || (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
                CURS = CURS + WI - 1;
            } else if(TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_BOTTOMLEFT)
                                          || (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
                CURS = CURS + HI - 1;
            }
            SI = CURS;
            if(TRANSPOSED == 0) {
                switch(REFCORNER) {
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
                switch(REFCORNER) {
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
            if(RI != 0) {
                delete IBI;
            }
            if(TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPLEFT)
                                   || (REFCORNER == JBIG2_CORNER_BOTTOMLEFT))) {
                CURS = CURS + WI - 1;
            } else if(TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_TOPLEFT)
                                          || (REFCORNER == JBIG2_CORNER_TOPRIGHT))) {
                CURS = CURS + HI - 1;
            }
            NINSTANCES = NINSTANCES + 1;
        }
    }
    if(bRetained == FALSE) {
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
    return SBREG;
failed:
    if(bRetained == FALSE) {
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
    delete SBREG;
    return NULL;
}
CJBig2_SymbolDict *CJBig2_SDDProc::decode_Arith(CJBig2_ArithDecoder *pArithDecoder,
        JBig2ArithCtx *gbContext, JBig2ArithCtx *grContext)
{
    CJBig2_Image **SDNEWSYMS;
    FX_DWORD HCHEIGHT, NSYMSDECODED;
    FX_INT32 HCDH;
    FX_DWORD SYMWIDTH, TOTWIDTH;
    FX_INT32 DW;
    CJBig2_Image *BS;
    FX_DWORD I, J, REFAGGNINST;
    FX_BOOL *EXFLAGS;
    FX_DWORD EXINDEX;
    FX_BOOL CUREXFLAG;
    FX_DWORD EXRUNLENGTH;
    FX_INT32 nVal;
    FX_DWORD nTmp;
    FX_DWORD SBNUMSYMS;
    FX_BYTE SBSYMCODELEN;
    FX_DWORD IDI;
    FX_INT32 RDXI, RDYI;
    CJBig2_Image **SBSYMS;
    CJBig2_HuffmanTable *SBHUFFFS, *SBHUFFDS, *SBHUFFDT, *SBHUFFRDW, *SBHUFFRDH, *SBHUFFRDX, *SBHUFFRDY,
                        *SBHUFFRSIZE;
    CJBig2_GRRDProc *pGRRD;
    CJBig2_GRDProc *pGRD;
    CJBig2_ArithIntDecoder *IADH, *IADW, *IAAI, *IARDX, *IARDY, *IAEX,
                           *IADT, *IAFS, *IADS, *IAIT, *IARI, *IARDW, *IARDH;
    CJBig2_ArithIaidDecoder *IAID;
    CJBig2_SymbolDict *pDict;
    JBIG2_ALLOC(IADH, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IADW, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IAAI, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IARDX, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IARDY, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IAEX, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IADT, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IAFS, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IADS, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IAIT, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IARI, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IARDW, CJBig2_ArithIntDecoder());
    JBIG2_ALLOC(IARDH, CJBig2_ArithIntDecoder());
    nTmp = 0;
    while((FX_DWORD)(1 << nTmp) < (SDNUMINSYMS + SDNUMNEWSYMS)) {
        nTmp ++;
    }
    JBIG2_ALLOC(IAID, CJBig2_ArithIaidDecoder((FX_BYTE)nTmp));
    SDNEWSYMS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(SDNUMNEWSYMS, sizeof(CJBig2_Image*));
    FXSYS_memset32(SDNEWSYMS, 0 , SDNUMNEWSYMS * sizeof(CJBig2_Image*));
    HCHEIGHT = 0;
    NSYMSDECODED = 0;
    while(NSYMSDECODED < SDNUMNEWSYMS) {
        BS = NULL;
        if(IADH->decode(pArithDecoder, &HCDH) == -1) {
            m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith): too short.");
            goto failed;
        }
        HCHEIGHT = HCHEIGHT + HCDH;
        if ((int)HCHEIGHT < 0 || (int)HCHEIGHT > JBIG2_MAX_IMAGE_SIZE) {
            m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith): invalid HCHEIGHT value.");
            goto failed;
        }
        SYMWIDTH = 0;
        TOTWIDTH = 0;
        for(;;) {
            nVal = IADW->decode(pArithDecoder, &DW);
            if(nVal == JBIG2_OOB) {
                break;
            } else if(nVal != 0) {
                m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith): too short.");
                goto failed;
            } else {
                if (NSYMSDECODED >= SDNUMNEWSYMS) {
                    m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith): NSYMSDECODED >= SDNUMNEWSYMS.");
                    goto failed;
                }
                SYMWIDTH = SYMWIDTH + DW;
                if ((int)SYMWIDTH < 0 || (int)SYMWIDTH > JBIG2_MAX_IMAGE_SIZE) {
                    m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith): invalid SYMWIDTH value.");
                    goto failed;
                } else if (HCHEIGHT == 0 || SYMWIDTH == 0) {
                    TOTWIDTH = TOTWIDTH + SYMWIDTH;
                    SDNEWSYMS[NSYMSDECODED] = NULL;
                    NSYMSDECODED = NSYMSDECODED + 1;
                    continue;
                }
                TOTWIDTH = TOTWIDTH + SYMWIDTH;
            }
            if(SDREFAGG == 0) {
                JBIG2_ALLOC(pGRD, CJBig2_GRDProc());
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
                if(BS == NULL) {
                    delete pGRD;
                    goto failed;
                }
                delete pGRD;
            } else {
                if(IAAI->decode(pArithDecoder, (int*)&REFAGGNINST) == -1) {
                    m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith): too short.");
                    goto failed;
                }
                if(REFAGGNINST > 1) {
                    CJBig2_TRDProc *pDecoder;
                    JBIG2_ALLOC(pDecoder, CJBig2_TRDProc());
                    pDecoder->SBHUFF = SDHUFF;
                    pDecoder->SBREFINE = 1;
                    pDecoder->SBW = SYMWIDTH;
                    pDecoder->SBH = HCHEIGHT;
                    pDecoder->SBNUMINSTANCES = REFAGGNINST;
                    pDecoder->SBSTRIPS = 1;
                    pDecoder->SBNUMSYMS = SDNUMINSYMS + NSYMSDECODED;
                    SBNUMSYMS = pDecoder->SBNUMSYMS;
                    nTmp = 0;
                    while((FX_DWORD)(1 << nTmp) < SBNUMSYMS) {
                        nTmp ++;
                    }
                    SBSYMCODELEN = (FX_BYTE)nTmp;
                    pDecoder->SBSYMCODELEN = SBSYMCODELEN;
                    SBSYMS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(SBNUMSYMS, sizeof(CJBig2_Image*));
                    JBIG2_memcpy(SBSYMS, SDINSYMS, SDNUMINSYMS * sizeof(CJBig2_Image*));
                    JBIG2_memcpy(SBSYMS + SDNUMINSYMS, SDNEWSYMS, NSYMSDECODED * sizeof(CJBig2_Image*));
                    pDecoder->SBSYMS = SBSYMS;
                    pDecoder->SBDEFPIXEL = 0;
                    pDecoder->SBCOMBOP = JBIG2_COMPOSE_OR;
                    pDecoder->TRANSPOSED = 0;
                    pDecoder->REFCORNER = JBIG2_CORNER_TOPLEFT;
                    pDecoder->SBDSOFFSET = 0;
                    JBIG2_ALLOC(SBHUFFFS, CJBig2_HuffmanTable(HuffmanTable_B6,
                                sizeof(HuffmanTable_B6) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B6));
                    JBIG2_ALLOC(SBHUFFDS, CJBig2_HuffmanTable(HuffmanTable_B8,
                                sizeof(HuffmanTable_B8) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B8));
                    JBIG2_ALLOC(SBHUFFDT, CJBig2_HuffmanTable(HuffmanTable_B11,
                                sizeof(HuffmanTable_B11) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B11));
                    JBIG2_ALLOC(SBHUFFRDW, CJBig2_HuffmanTable(HuffmanTable_B15,
                                sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
                    JBIG2_ALLOC(SBHUFFRDH, CJBig2_HuffmanTable(HuffmanTable_B15,
                                sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
                    JBIG2_ALLOC(SBHUFFRDX, CJBig2_HuffmanTable(HuffmanTable_B15,
                                sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
                    JBIG2_ALLOC(SBHUFFRDY, CJBig2_HuffmanTable(HuffmanTable_B15,
                                sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
                    JBIG2_ALLOC(SBHUFFRSIZE, CJBig2_HuffmanTable(HuffmanTable_B1,
                                sizeof(HuffmanTable_B1) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B1));
                    pDecoder->SBHUFFFS = SBHUFFFS;
                    pDecoder->SBHUFFDS = SBHUFFDS;
                    pDecoder->SBHUFFDT = SBHUFFDT;
                    pDecoder->SBHUFFRDW = SBHUFFRDW;
                    pDecoder->SBHUFFRDH = SBHUFFRDH;
                    pDecoder->SBHUFFRDX = SBHUFFRDX;
                    pDecoder->SBHUFFRDY = SBHUFFRDY;
                    pDecoder->SBHUFFRSIZE = SBHUFFRSIZE;
                    pDecoder->SBRTEMPLATE = SDRTEMPLATE;
                    pDecoder->SBRAT[0] = SDRAT[0];
                    pDecoder->SBRAT[1] = SDRAT[1];
                    pDecoder->SBRAT[2] = SDRAT[2];
                    pDecoder->SBRAT[3] = SDRAT[3];
                    JBig2IntDecoderState ids;
                    ids.IADT = IADT;
                    ids.IAFS = IAFS;
                    ids.IADS = IADS;
                    ids.IAIT = IAIT;
                    ids.IARI = IARI;
                    ids.IARDW = IARDW;
                    ids.IARDH = IARDH;
                    ids.IARDX = IARDX;
                    ids.IARDY = IARDY;
                    ids.IAID = IAID;
                    BS = pDecoder->decode_Arith(pArithDecoder, grContext, &ids);
                    if(BS == NULL) {
                        m_pModule->JBig2_Free(SBSYMS);
                        delete SBHUFFFS;
                        delete SBHUFFDS;
                        delete SBHUFFDT;
                        delete SBHUFFRDW;
                        delete SBHUFFRDH;
                        delete SBHUFFRDX;
                        delete SBHUFFRDY;
                        delete SBHUFFRSIZE;
                        delete pDecoder;
                        goto failed;
                    }
                    m_pModule->JBig2_Free(SBSYMS);
                    delete SBHUFFFS;
                    delete SBHUFFDS;
                    delete SBHUFFDT;
                    delete SBHUFFRDW;
                    delete SBHUFFRDH;
                    delete SBHUFFRDX;
                    delete SBHUFFRDY;
                    delete SBHUFFRSIZE;
                    delete pDecoder;
                } else if(REFAGGNINST == 1) {
                    SBNUMSYMS = SDNUMINSYMS + NSYMSDECODED;
                    if(IAID->decode(pArithDecoder, (int*)&IDI) == -1) {
                        m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith): too short.");
                        goto failed;
                    }
                    if((IARDX->decode(pArithDecoder, &RDXI) == -1)
                            || (IARDY->decode(pArithDecoder, &RDYI) == -1)) {
                        m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith): too short.");
                        goto failed;
                    }
                    if (IDI >= SBNUMSYMS) {
                        m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith):"
                                               " refinement references unknown symbol %d", IDI);
                        goto failed;
                    }
                    SBSYMS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(SBNUMSYMS, sizeof(CJBig2_Image*));
                    JBIG2_memcpy(SBSYMS, SDINSYMS, SDNUMINSYMS * sizeof(CJBig2_Image*));
                    JBIG2_memcpy(SBSYMS + SDNUMINSYMS, SDNEWSYMS, NSYMSDECODED * sizeof(CJBig2_Image*));
                    if (!SBSYMS[IDI]) {
                        m_pModule->JBig2_Free(SBSYMS);
                        goto failed;
                    }
                    JBIG2_ALLOC(pGRRD, CJBig2_GRRDProc());
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
                    if(BS == NULL) {
                        m_pModule->JBig2_Free(SBSYMS);
                        delete pGRRD;
                        goto failed;
                    }
                    m_pModule->JBig2_Free(SBSYMS);
                    delete pGRRD;
                }
            }
            SDNEWSYMS[NSYMSDECODED] = BS;
            BS = NULL;
            NSYMSDECODED = NSYMSDECODED + 1;
        }
    }
    EXINDEX = 0;
    CUREXFLAG = 0;
    EXFLAGS = (FX_BOOL*)m_pModule->JBig2_Malloc2(sizeof(FX_BOOL), (SDNUMINSYMS + SDNUMNEWSYMS));
    while(EXINDEX < SDNUMINSYMS + SDNUMNEWSYMS) {
        if(IAEX->decode(pArithDecoder, (int*)&EXRUNLENGTH) == -1) {
            m_pModule->JBig2_Free(EXFLAGS);
            m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith): too short.");
            goto failed;
        }
        if (EXINDEX + EXRUNLENGTH > SDNUMINSYMS + SDNUMNEWSYMS) {
            m_pModule->JBig2_Free(EXFLAGS);
            m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith): Invalid EXRUNLENGTH value.");
            goto failed;
        }
        if(EXRUNLENGTH != 0) {
            for(I = EXINDEX; I < EXINDEX + EXRUNLENGTH; I++) {
                EXFLAGS[I] = CUREXFLAG;
            }
        }
        EXINDEX = EXINDEX + EXRUNLENGTH;
        CUREXFLAG = !CUREXFLAG;
    }
    JBIG2_ALLOC(pDict, CJBig2_SymbolDict());
    pDict->SDNUMEXSYMS = SDNUMEXSYMS;
    pDict->SDEXSYMS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(sizeof(CJBig2_Image*), SDNUMEXSYMS);
    I = J = 0;
    for(I = 0; I < SDNUMINSYMS + SDNUMNEWSYMS; I++) {
        if(EXFLAGS[I] && J < SDNUMEXSYMS) {
            if(I < SDNUMINSYMS) {
                JBIG2_ALLOC(pDict->SDEXSYMS[J], CJBig2_Image(*SDINSYMS[I]));
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
    m_pModule->JBig2_Free(EXFLAGS);
    m_pModule->JBig2_Free(SDNEWSYMS);
    delete IADH;
    delete IADW;
    delete IAAI;
    delete IARDX;
    delete IARDY;
    delete IAEX;
    delete IAID;
    delete IADT;
    delete IAFS;
    delete IADS;
    delete IAIT;
    delete IARI;
    delete IARDW;
    delete IARDH;
    return pDict;
failed:
    for(I = 0; I < NSYMSDECODED; I++) {
        if (SDNEWSYMS[I]) {
            delete SDNEWSYMS[I];
            SDNEWSYMS[I] = NULL;
        }
    }
    m_pModule->JBig2_Free(SDNEWSYMS);
    delete IADH;
    delete IADW;
    delete IAAI;
    delete IARDX;
    delete IARDY;
    delete IAEX;
    delete IAID;
    delete IADT;
    delete IAFS;
    delete IADS;
    delete IAIT;
    delete IARI;
    delete IARDW;
    delete IARDH;
    return NULL;
}
CJBig2_SymbolDict *CJBig2_SDDProc::decode_Huffman(CJBig2_BitStream *pStream,
        JBig2ArithCtx *gbContext, JBig2ArithCtx *grContext, IFX_Pause* pPause)
{
    CJBig2_Image **SDNEWSYMS;
    FX_DWORD *SDNEWSYMWIDTHS;
    FX_DWORD HCHEIGHT, NSYMSDECODED;
    FX_INT32 HCDH;
    FX_DWORD SYMWIDTH, TOTWIDTH, HCFIRSTSYM;
    FX_INT32 DW;
    CJBig2_Image *BS, *BHC;
    FX_DWORD I, J, REFAGGNINST;
    FX_BOOL *EXFLAGS;
    FX_DWORD EXINDEX;
    FX_BOOL CUREXFLAG;
    FX_DWORD EXRUNLENGTH;
    FX_INT32 nVal, nBits;
    FX_DWORD nTmp;
    FX_DWORD SBNUMSYMS;
    FX_BYTE SBSYMCODELEN;
    JBig2HuffmanCode *SBSYMCODES;
    FX_DWORD IDI;
    FX_INT32 RDXI, RDYI;
    FX_DWORD BMSIZE;
    FX_DWORD stride;
    CJBig2_Image **SBSYMS;
    CJBig2_HuffmanTable *SBHUFFFS, *SBHUFFDS, *SBHUFFDT, *SBHUFFRDW, *SBHUFFRDH, *SBHUFFRDX, *SBHUFFRDY,
                        *SBHUFFRSIZE, *pTable;
    CJBig2_HuffmanDecoder *pHuffmanDecoder;
    CJBig2_GRRDProc *pGRRD;
    CJBig2_ArithDecoder *pArithDecoder;
    CJBig2_GRDProc *pGRD;
    CJBig2_SymbolDict *pDict;
    JBIG2_ALLOC(pHuffmanDecoder, CJBig2_HuffmanDecoder(pStream));
    SDNEWSYMS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(SDNUMNEWSYMS, sizeof(CJBig2_Image*));
    FXSYS_memset32(SDNEWSYMS, 0 , SDNUMNEWSYMS * sizeof(CJBig2_Image*));
    SDNEWSYMWIDTHS = NULL;
    BHC = NULL;
    if(SDREFAGG == 0) {
        SDNEWSYMWIDTHS = (FX_DWORD *)m_pModule->JBig2_Malloc2(SDNUMNEWSYMS, sizeof(FX_DWORD));
        FXSYS_memset32(SDNEWSYMWIDTHS, 0 , SDNUMNEWSYMS * sizeof(FX_DWORD));
    }
    HCHEIGHT = 0;
    NSYMSDECODED = 0;
    BS = NULL;
    while(NSYMSDECODED < SDNUMNEWSYMS) {
        if(pHuffmanDecoder->decodeAValue(SDHUFFDH, &HCDH) != 0) {
            m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman): too short.");
            goto failed;
        }
        HCHEIGHT = HCHEIGHT + HCDH;
        if ((int)HCHEIGHT < 0 || (int)HCHEIGHT > JBIG2_MAX_IMAGE_SIZE) {
            m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman): invalid HCHEIGHT value.");
            goto failed;
        }
        SYMWIDTH = 0;
        TOTWIDTH = 0;
        HCFIRSTSYM = NSYMSDECODED;
        for(;;) {
            nVal = pHuffmanDecoder->decodeAValue(SDHUFFDW, &DW);
            if(nVal == JBIG2_OOB) {
                break;
            } else if(nVal != 0) {
                m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman): too short.");
                goto failed;
            } else {
                if (NSYMSDECODED >= SDNUMNEWSYMS) {
                    m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman): NSYMSDECODED >= SDNUMNEWSYMS.");
                    goto failed;
                }
                SYMWIDTH = SYMWIDTH + DW;
                if ((int)SYMWIDTH < 0 || (int)SYMWIDTH > JBIG2_MAX_IMAGE_SIZE) {
                    m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman): invalid SYMWIDTH value.");
                    goto failed;
                } else if (HCHEIGHT == 0 || SYMWIDTH == 0) {
                    TOTWIDTH = TOTWIDTH + SYMWIDTH;
                    SDNEWSYMS[NSYMSDECODED] = NULL;
                    NSYMSDECODED = NSYMSDECODED + 1;
                    continue;
                }
                TOTWIDTH = TOTWIDTH + SYMWIDTH;
            }
            if(SDREFAGG == 1) {
                if(pHuffmanDecoder->decodeAValue(SDHUFFAGGINST, (int*)&REFAGGNINST) != 0) {
                    m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman): too short.");
                    goto failed;
                }
                BS = NULL;
                if(REFAGGNINST > 1) {
                    CJBig2_TRDProc *pDecoder;
                    JBIG2_ALLOC(pDecoder, CJBig2_TRDProc());
                    pDecoder->SBHUFF = SDHUFF;
                    pDecoder->SBREFINE = 1;
                    pDecoder->SBW = SYMWIDTH;
                    pDecoder->SBH = HCHEIGHT;
                    pDecoder->SBNUMINSTANCES = REFAGGNINST;
                    pDecoder->SBSTRIPS = 1;
                    pDecoder->SBNUMSYMS = SDNUMINSYMS + NSYMSDECODED;
                    SBNUMSYMS = pDecoder->SBNUMSYMS;
                    SBSYMCODES = (JBig2HuffmanCode*)m_pModule->JBig2_Malloc2(SBNUMSYMS, sizeof(JBig2HuffmanCode));
                    nTmp = 1;
                    while((FX_DWORD)(1 << nTmp) < SBNUMSYMS) {
                        nTmp ++;
                    }
                    for(I = 0; I < SBNUMSYMS; I++) {
                        SBSYMCODES[I].codelen = nTmp;
                        SBSYMCODES[I].code = I;
                    }
                    pDecoder->SBSYMCODES = SBSYMCODES;
                    SBSYMS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(SBNUMSYMS, sizeof(CJBig2_Image*));
                    JBIG2_memcpy(SBSYMS, SDINSYMS, SDNUMINSYMS * sizeof(CJBig2_Image*));
                    JBIG2_memcpy(SBSYMS + SDNUMINSYMS, SDNEWSYMS, NSYMSDECODED * sizeof(CJBig2_Image*));
                    pDecoder->SBSYMS = SBSYMS;
                    pDecoder->SBDEFPIXEL = 0;
                    pDecoder->SBCOMBOP = JBIG2_COMPOSE_OR;
                    pDecoder->TRANSPOSED = 0;
                    pDecoder->REFCORNER = JBIG2_CORNER_TOPLEFT;
                    pDecoder->SBDSOFFSET = 0;
                    JBIG2_ALLOC(SBHUFFFS, CJBig2_HuffmanTable(HuffmanTable_B6,
                                sizeof(HuffmanTable_B6) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B6));
                    JBIG2_ALLOC(SBHUFFDS, CJBig2_HuffmanTable(HuffmanTable_B8,
                                sizeof(HuffmanTable_B8) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B8));
                    JBIG2_ALLOC(SBHUFFDT, CJBig2_HuffmanTable(HuffmanTable_B11,
                                sizeof(HuffmanTable_B11) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B11));
                    JBIG2_ALLOC(SBHUFFRDW, CJBig2_HuffmanTable(HuffmanTable_B15,
                                sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
                    JBIG2_ALLOC(SBHUFFRDH, CJBig2_HuffmanTable(HuffmanTable_B15,
                                sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
                    JBIG2_ALLOC(SBHUFFRDX, CJBig2_HuffmanTable(HuffmanTable_B15,
                                sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
                    JBIG2_ALLOC(SBHUFFRDY, CJBig2_HuffmanTable(HuffmanTable_B15,
                                sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
                    JBIG2_ALLOC(SBHUFFRSIZE, CJBig2_HuffmanTable(HuffmanTable_B1,
                                sizeof(HuffmanTable_B1) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B1));
                    pDecoder->SBHUFFFS = SBHUFFFS;
                    pDecoder->SBHUFFDS = SBHUFFDS;
                    pDecoder->SBHUFFDT = SBHUFFDT;
                    pDecoder->SBHUFFRDW = SBHUFFRDW;
                    pDecoder->SBHUFFRDH = SBHUFFRDH;
                    pDecoder->SBHUFFRDX = SBHUFFRDX;
                    pDecoder->SBHUFFRDY = SBHUFFRDY;
                    pDecoder->SBHUFFRSIZE = SBHUFFRSIZE;
                    pDecoder->SBRTEMPLATE = SDRTEMPLATE;
                    pDecoder->SBRAT[0] = SDRAT[0];
                    pDecoder->SBRAT[1] = SDRAT[1];
                    pDecoder->SBRAT[2] = SDRAT[2];
                    pDecoder->SBRAT[3] = SDRAT[3];
                    BS = pDecoder->decode_Huffman(pStream, grContext);
                    if(BS == NULL) {
                        m_pModule->JBig2_Free(SBSYMCODES);
                        m_pModule->JBig2_Free(SBSYMS);
                        delete SBHUFFFS;
                        delete SBHUFFDS;
                        delete SBHUFFDT;
                        delete SBHUFFRDW;
                        delete SBHUFFRDH;
                        delete SBHUFFRDX;
                        delete SBHUFFRDY;
                        delete SBHUFFRSIZE;
                        delete pDecoder;
                        goto failed;
                    }
                    m_pModule->JBig2_Free(SBSYMCODES);
                    m_pModule->JBig2_Free(SBSYMS);
                    delete SBHUFFFS;
                    delete SBHUFFDS;
                    delete SBHUFFDT;
                    delete SBHUFFRDW;
                    delete SBHUFFRDH;
                    delete SBHUFFRDX;
                    delete SBHUFFRDY;
                    delete SBHUFFRSIZE;
                    delete pDecoder;
                } else if(REFAGGNINST == 1) {
                    SBNUMSYMS = SDNUMINSYMS + SDNUMNEWSYMS;
                    nTmp = 1;
                    while((FX_DWORD)(1 << nTmp) < SBNUMSYMS) {
                        nTmp ++;
                    }
                    SBSYMCODELEN = (FX_BYTE)nTmp;
                    SBSYMCODES = (JBig2HuffmanCode*)m_pModule->JBig2_Malloc2(SBNUMSYMS, sizeof(JBig2HuffmanCode));
                    for(I = 0; I < SBNUMSYMS; I++) {
                        SBSYMCODES[I].codelen = SBSYMCODELEN;
                        SBSYMCODES[I].code = I;
                    }
                    nVal = 0;
                    nBits = 0;
                    for(;;) {
                        if(pStream->read1Bit(&nTmp) != 0) {
                            m_pModule->JBig2_Free(SBSYMCODES);
                            m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman): too short.");
                            goto failed;
                        }
                        nVal = (nVal << 1) | nTmp;
                        for(IDI = 0; IDI < SBNUMSYMS; IDI++) {
                            if((nVal == SBSYMCODES[IDI].code)
                                    && (nBits == SBSYMCODES[IDI].codelen)) {
                                break;
                            }
                        }
                        if(IDI < SBNUMSYMS) {
                            break;
                        }
                    }
                    m_pModule->JBig2_Free(SBSYMCODES);
                    JBIG2_ALLOC(SBHUFFRDX, CJBig2_HuffmanTable(HuffmanTable_B15,
                                sizeof(HuffmanTable_B15) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B15));
                    JBIG2_ALLOC(SBHUFFRSIZE, CJBig2_HuffmanTable(HuffmanTable_B1,
                                sizeof(HuffmanTable_B1) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B1));
                    if((pHuffmanDecoder->decodeAValue(SBHUFFRDX, &RDXI) != 0)
                            || (pHuffmanDecoder->decodeAValue(SBHUFFRDX, &RDYI) != 0)
                            || (pHuffmanDecoder->decodeAValue(SBHUFFRSIZE, &nVal) != 0)) {
                        delete SBHUFFRDX;
                        delete SBHUFFRSIZE;
                        m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman): too short.");
                        goto failed;
                    }
                    delete SBHUFFRDX;
                    delete SBHUFFRSIZE;
                    pStream->alignByte();
                    nTmp = pStream->getOffset();
                    SBSYMS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(SBNUMSYMS, sizeof(CJBig2_Image*));
                    JBIG2_memcpy(SBSYMS, SDINSYMS, SDNUMINSYMS * sizeof(CJBig2_Image*));
                    JBIG2_memcpy(SBSYMS + SDNUMINSYMS, SDNEWSYMS, NSYMSDECODED * sizeof(CJBig2_Image*));
                    JBIG2_ALLOC(pGRRD, CJBig2_GRRDProc());
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
                    JBIG2_ALLOC(pArithDecoder, CJBig2_ArithDecoder(pStream));
                    BS = pGRRD->decode(pArithDecoder, grContext);
                    if(BS == NULL) {
                        m_pModule->JBig2_Free(SBSYMS);
                        delete pGRRD;
                        delete pArithDecoder;
                        goto failed;
                    }
                    pStream->alignByte();
                    pStream->offset(2);
                    if((FX_DWORD)nVal != (pStream->getOffset() - nTmp)) {
                        delete BS;
                        m_pModule->JBig2_Free(SBSYMS);
                        delete pGRRD;
                        delete pArithDecoder;
                        m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman):"
                                               "bytes processed by generic refinement region decoding procedure doesn't equal SBHUFFRSIZE.");
                        goto failed;
                    }
                    m_pModule->JBig2_Free(SBSYMS);
                    delete pGRRD;
                    delete pArithDecoder;
                }
                SDNEWSYMS[NSYMSDECODED] = BS;
            }
            if(SDREFAGG == 0) {
                SDNEWSYMWIDTHS[NSYMSDECODED] = SYMWIDTH;
            }
            NSYMSDECODED = NSYMSDECODED + 1;
        }
        if(SDREFAGG == 0) {
            if(pHuffmanDecoder->decodeAValue(SDHUFFBMSIZE, (FX_INT32*)&BMSIZE) != 0) {
                m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman): too short.");
                goto failed;
            }
            pStream->alignByte();
            if(BMSIZE == 0) {
                stride = (TOTWIDTH + 7) >> 3;
                if(pStream->getByteLeft() >= stride * HCHEIGHT) {
                    JBIG2_ALLOC(BHC, CJBig2_Image(TOTWIDTH, HCHEIGHT));
                    for(I = 0; I < HCHEIGHT; I ++) {
                        JBIG2_memcpy(BHC->m_pData + I * BHC->m_nStride, pStream->getPointer(), stride);
                        pStream->offset(stride);
                    }
                } else {
                    m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman): too short.");
                    goto failed;
                }
            } else {
                JBIG2_ALLOC(pGRD, CJBig2_GRDProc());
                pGRD->MMR = 1;
                pGRD->GBW = TOTWIDTH;
                pGRD->GBH = HCHEIGHT;
                FXCODEC_STATUS status = pGRD->Start_decode_MMR(&BHC, pStream);
                while(status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
                    pGRD->Continue_decode(pPause);
                }
                delete pGRD;
                pStream->alignByte();
            }
            nTmp = 0;
            if (!BHC) {
                continue;
            }
            for(I = HCFIRSTSYM; I < NSYMSDECODED; I++) {
                SDNEWSYMS[I] = BHC->subImage(nTmp, 0, SDNEWSYMWIDTHS[I], HCHEIGHT);
                nTmp += SDNEWSYMWIDTHS[I];
            }
            delete BHC;
            BHC = NULL;
        }
    }
    EXINDEX = 0;
    CUREXFLAG = 0;
    JBIG2_ALLOC(pTable, CJBig2_HuffmanTable(HuffmanTable_B1,
                                            sizeof(HuffmanTable_B1) / sizeof(JBig2TableLine), HuffmanTable_HTOOB_B1));
    EXFLAGS = (FX_BOOL*)m_pModule->JBig2_Malloc2(sizeof(FX_BOOL), (SDNUMINSYMS + SDNUMNEWSYMS));
    while(EXINDEX < SDNUMINSYMS + SDNUMNEWSYMS) {
        if(pHuffmanDecoder->decodeAValue(pTable, (int*)&EXRUNLENGTH) != 0) {
            delete pTable;
            m_pModule->JBig2_Free(EXFLAGS);
            m_pModule->JBig2_Error("symbol dictionary decoding procedure (huffman): too short.");
            goto failed;
        }
        if (EXINDEX + EXRUNLENGTH > SDNUMINSYMS + SDNUMNEWSYMS) {
            delete pTable;
            m_pModule->JBig2_Free(EXFLAGS);
            m_pModule->JBig2_Error("symbol dictionary decoding procedure (arith): Invalid EXRUNLENGTH value.");
            goto failed;
        }
        if(EXRUNLENGTH != 0) {
            for(I = EXINDEX; I < EXINDEX + EXRUNLENGTH; I++) {
                EXFLAGS[I] = CUREXFLAG;
            }
        }
        EXINDEX = EXINDEX + EXRUNLENGTH;
        CUREXFLAG = !CUREXFLAG;
    }
    delete pTable;
    JBIG2_ALLOC(pDict, CJBig2_SymbolDict());
    pDict->SDNUMEXSYMS = SDNUMEXSYMS;
    pDict->SDEXSYMS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(sizeof(CJBig2_Image*), SDNUMEXSYMS);
    I = J = 0;
    for(I = 0; I < SDNUMINSYMS + SDNUMNEWSYMS; I++) {
        if(EXFLAGS[I] && J < SDNUMEXSYMS) {
            if(I < SDNUMINSYMS) {
                JBIG2_ALLOC(pDict->SDEXSYMS[J], CJBig2_Image(*SDINSYMS[I]));
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
    m_pModule->JBig2_Free(EXFLAGS);
    m_pModule->JBig2_Free(SDNEWSYMS);
    if(SDREFAGG == 0) {
        m_pModule->JBig2_Free(SDNEWSYMWIDTHS);
    }
    delete pHuffmanDecoder;
    return pDict;
failed:
    for(I = 0; I < NSYMSDECODED; I++) {
        if (SDNEWSYMS[I]) {
            delete SDNEWSYMS[I];
        }
    }
    m_pModule->JBig2_Free(SDNEWSYMS);
    if(SDREFAGG == 0) {
        m_pModule->JBig2_Free(SDNEWSYMWIDTHS);
    }
    delete pHuffmanDecoder;
    return NULL;
}
CJBig2_Image *CJBig2_HTRDProc::decode_Arith(CJBig2_ArithDecoder *pArithDecoder,
        JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    FX_DWORD ng, mg;
    FX_INT32 x, y;
    CJBig2_Image *HSKIP;
    FX_DWORD HBPP;
    FX_DWORD *GI;
    CJBig2_Image *HTREG;
    CJBig2_GSIDProc *pGID;
    JBIG2_ALLOC(HTREG, CJBig2_Image(HBW, HBH));
    HTREG->fill(HDEFPIXEL);
    HSKIP = NULL;
    if(HENABLESKIP == 1) {
        JBIG2_ALLOC(HSKIP, CJBig2_Image(HGW, HGH));
        for(mg = 0; mg < HGH; mg++) {
            for(ng = 0; ng < HGW; ng++) {
                x = (HGX + mg * HRY + ng * HRX) >> 8;
                y = (HGY + mg * HRX - ng * HRY) >> 8;
                if((x + HPW <= 0) | (x >= (FX_INT32)HBW)
                        | (y + HPH <= 0) | (y >= (FX_INT32)HPH)) {
                    HSKIP->setPixel(ng, mg, 1);
                } else {
                    HSKIP->setPixel(ng, mg, 0);
                }
            }
        }
    }
    HBPP = 1;
    while((FX_DWORD)(1 << HBPP) < HNUMPATS) {
        HBPP ++;
    }
    JBIG2_ALLOC(pGID, CJBig2_GSIDProc());
    pGID->GSMMR = HMMR;
    pGID->GSW = HGW;
    pGID->GSH = HGH;
    pGID->GSBPP = (FX_BYTE)HBPP;
    pGID->GSUSESKIP = HENABLESKIP;
    pGID->GSKIP = HSKIP;
    pGID->GSTEMPLATE = HTEMPLATE;
    GI = pGID->decode_Arith(pArithDecoder, gbContext, pPause);
    if(GI == NULL) {
        goto failed;
    }
    for(mg = 0; mg < HGH; mg++) {
        for(ng = 0; ng < HGW; ng++) {
            x = (HGX + mg * HRY + ng * HRX) >> 8;
            y = (HGY + mg * HRX - ng * HRY) >> 8;
            FX_DWORD pat_index = GI[mg * HGW + ng];
            if (pat_index >= HNUMPATS) {
                pat_index = HNUMPATS - 1;
            }
            HTREG->composeFrom(x, y, HPATS[pat_index], HCOMBOP);
        }
    }
    m_pModule->JBig2_Free(GI);
    if(HSKIP) {
        delete HSKIP;
    }
    delete pGID;
    return HTREG;
failed:
    if(HSKIP) {
        delete HSKIP;
    }
    delete pGID;
    delete HTREG;
    return NULL;
}
CJBig2_Image *CJBig2_HTRDProc::decode_MMR(CJBig2_BitStream *pStream, IFX_Pause* pPause)
{
    FX_DWORD ng, mg;
    FX_INT32 x, y;
    FX_DWORD HBPP;
    FX_DWORD *GI;
    CJBig2_Image *HTREG;
    CJBig2_GSIDProc *pGID;
    JBIG2_ALLOC(HTREG, CJBig2_Image(HBW, HBH));
    HTREG->fill(HDEFPIXEL);
    HBPP = 1;
    while((FX_DWORD)(1 << HBPP) < HNUMPATS) {
        HBPP ++;
    }
    JBIG2_ALLOC(pGID, CJBig2_GSIDProc());
    pGID->GSMMR = HMMR;
    pGID->GSW = HGW;
    pGID->GSH = HGH;
    pGID->GSBPP = (FX_BYTE)HBPP;
    pGID->GSUSESKIP = 0;
    GI = pGID->decode_MMR(pStream, pPause);
    if(GI == NULL) {
        goto failed;
    }
    for(mg = 0; mg < HGH; mg++) {
        for(ng = 0; ng < HGW; ng++) {
            x = (HGX + mg * HRY + ng * HRX) >> 8;
            y = (HGY + mg * HRX - ng * HRY) >> 8;
            FX_DWORD pat_index = GI[mg * HGW + ng];
            if (pat_index >= HNUMPATS) {
                pat_index = HNUMPATS - 1;
            }
            HTREG->composeFrom(x, y, HPATS[pat_index], HCOMBOP);
        }
    }
    m_pModule->JBig2_Free(GI);
    delete pGID;
    return HTREG;
failed:
    delete pGID;
    delete HTREG;
    return NULL;
}
CJBig2_PatternDict *CJBig2_PDDProc::decode_Arith(CJBig2_ArithDecoder *pArithDecoder,
        JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    FX_DWORD GRAY;
    CJBig2_Image *BHDC = NULL;
    CJBig2_PatternDict *pDict;
    CJBig2_GRDProc *pGRD;
    JBIG2_ALLOC(pDict, CJBig2_PatternDict());
    pDict->NUMPATS = GRAYMAX + 1;
    pDict->HDPATS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(sizeof(CJBig2_Image*), pDict->NUMPATS);
    JBIG2_memset(pDict->HDPATS, 0, sizeof(CJBig2_Image*)*pDict->NUMPATS);
    JBIG2_ALLOC(pGRD, CJBig2_GRDProc());
    pGRD->MMR = HDMMR;
    pGRD->GBW = (GRAYMAX + 1) * HDPW;
    pGRD->GBH = HDPH;
    pGRD->GBTEMPLATE = HDTEMPLATE;
    pGRD->TPGDON = 0;
    pGRD->USESKIP = 0;
    pGRD->GBAT[0] = -(FX_INT32)HDPW;
    pGRD->GBAT[1] = 0;
    if(pGRD->GBTEMPLATE == 0) {
        pGRD->GBAT[2] = -3;
        pGRD->GBAT[3] = -1;
        pGRD->GBAT[4] = 2;
        pGRD->GBAT[5] = -2;
        pGRD->GBAT[6] = -2;
        pGRD->GBAT[7] = -2;
    }
    FXCODEC_STATUS status = pGRD->Start_decode_Arith(&BHDC, pArithDecoder, gbContext);
    while(status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
        pGRD->Continue_decode(pPause);
    }
    if(BHDC == NULL) {
        delete pGRD;
        goto failed;
    }
    delete pGRD;
    GRAY = 0;
    while(GRAY <= GRAYMAX) {
        pDict->HDPATS[GRAY] = BHDC->subImage(HDPW * GRAY, 0, HDPW, HDPH);
        GRAY = GRAY + 1;
    }
    delete BHDC;
    return pDict;
failed:
    delete pDict;
    return NULL;
}

CJBig2_PatternDict *CJBig2_PDDProc::decode_MMR(CJBig2_BitStream *pStream, IFX_Pause* pPause)
{
    FX_DWORD GRAY;
    CJBig2_Image *BHDC = NULL;
    CJBig2_PatternDict *pDict;
    CJBig2_GRDProc *pGRD;
    JBIG2_ALLOC(pDict, CJBig2_PatternDict());
    pDict->NUMPATS = GRAYMAX + 1;
    pDict->HDPATS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(sizeof(CJBig2_Image*), pDict->NUMPATS);
    JBIG2_memset(pDict->HDPATS, 0, sizeof(CJBig2_Image*)*pDict->NUMPATS);
    JBIG2_ALLOC(pGRD, CJBig2_GRDProc());
    pGRD->MMR = HDMMR;
    pGRD->GBW = (GRAYMAX + 1) * HDPW;
    pGRD->GBH = HDPH;
    FXCODEC_STATUS status = pGRD->Start_decode_MMR(&BHDC, pStream);
    while(status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
        pGRD->Continue_decode(pPause);
    }
    if(BHDC == NULL) {
        delete pGRD;
        goto failed;
    }
    delete pGRD;
    GRAY = 0;
    while(GRAY <= GRAYMAX) {
        pDict->HDPATS[GRAY] = BHDC->subImage(HDPW * GRAY, 0, HDPW, HDPH);
        GRAY = GRAY + 1;
    }
    delete BHDC;
    return pDict;
failed:
    delete pDict;
    return NULL;
}
FX_DWORD *CJBig2_GSIDProc::decode_Arith(CJBig2_ArithDecoder *pArithDecoder,
                                        JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    CJBig2_Image **GSPLANES;
    FX_INT32 J, K;
    FX_DWORD x, y;
    FX_DWORD *GSVALS;
    CJBig2_GRDProc *pGRD;
    GSPLANES = (CJBig2_Image **)m_pModule->JBig2_Malloc2(sizeof(CJBig2_Image*), GSBPP);
    if (!GSPLANES) {
        return NULL;
    }
    GSVALS = (FX_DWORD*)m_pModule->JBig2_Malloc3(sizeof(FX_DWORD), GSW, GSH);
    if (!GSVALS) {
        m_pModule->JBig2_Free(GSPLANES);
        return NULL;
    }
    JBIG2_memset(GSPLANES, 0, sizeof(CJBig2_Image*)*GSBPP);
    JBIG2_memset(GSVALS, 0, sizeof(FX_DWORD)*GSW * GSH);
    JBIG2_ALLOC(pGRD, CJBig2_GRDProc());
    pGRD->MMR = GSMMR;
    pGRD->GBW = GSW;
    pGRD->GBH = GSH;
    pGRD->GBTEMPLATE = GSTEMPLATE;
    pGRD->TPGDON = 0;
    pGRD->USESKIP = GSUSESKIP;
    pGRD->SKIP = GSKIP;
    if(GSTEMPLATE <= 1) {
        pGRD->GBAT[0] = 3;
    } else {
        pGRD->GBAT[0] = 2;
    }
    pGRD->GBAT[1] = -1;
    if(pGRD->GBTEMPLATE == 0) {
        pGRD->GBAT[2] = -3;
        pGRD->GBAT[3] = -1;
        pGRD->GBAT[4] = 2;
        pGRD->GBAT[5] = -2;
        pGRD->GBAT[6] = -2;
        pGRD->GBAT[7] = -2;
    }
    FXCODEC_STATUS status = pGRD->Start_decode_Arith(&GSPLANES[GSBPP - 1], pArithDecoder, gbContext);
    while(status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
        pGRD->Continue_decode(pPause);
    }
    if(GSPLANES[GSBPP - 1] == NULL) {
        goto failed;
    }
    J = GSBPP - 2;
    while(J >= 0) {
        FXCODEC_STATUS status = pGRD->Start_decode_Arith(&GSPLANES[J], pArithDecoder, gbContext);
        while(status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
            pGRD->Continue_decode(pPause);
        }
        if(GSPLANES[J] == NULL) {
            for(K = GSBPP - 1; K > J; K--) {
                delete GSPLANES[K];
                goto failed;
            }
        }
        GSPLANES[J]->composeFrom(0, 0, GSPLANES[J + 1], JBIG2_COMPOSE_XOR);
        J = J - 1;
    }
    for(y = 0; y < GSH; y++) {
        for(x = 0; x < GSW; x++) {
            for(J = 0; J < GSBPP; J++) {
                GSVALS[y * GSW + x] |= GSPLANES[J]->getPixel(x, y) << J;
            }
        }
    }
    for(J = 0; J < GSBPP; J++) {
        delete GSPLANES[J];
    }
    m_pModule->JBig2_Free(GSPLANES);
    delete pGRD;
    return GSVALS;
failed:
    m_pModule->JBig2_Free(GSPLANES);
    delete pGRD;
    m_pModule->JBig2_Free(GSVALS);
    return NULL;
}
FX_DWORD *CJBig2_GSIDProc::decode_MMR(CJBig2_BitStream *pStream, IFX_Pause* pPause)
{
    CJBig2_Image **GSPLANES;
    FX_INT32 J, K;
    FX_DWORD x, y;
    FX_DWORD *GSVALS;
    CJBig2_GRDProc *pGRD;
    GSPLANES = (CJBig2_Image **)m_pModule->JBig2_Malloc2(sizeof(CJBig2_Image*), GSBPP);
    if (!GSPLANES) {
        return NULL;
    }
    GSVALS = (FX_DWORD*)m_pModule->JBig2_Malloc3(sizeof(FX_DWORD), GSW, GSH);
    if (!GSVALS) {
        if (GSPLANES) {
            m_pModule->JBig2_Free(GSPLANES);
        }
        return NULL;
    }
    JBIG2_memset(GSPLANES, 0, sizeof(CJBig2_Image*)*GSBPP);
    JBIG2_memset(GSVALS, 0, sizeof(FX_DWORD)*GSW * GSH);
    JBIG2_ALLOC(pGRD, CJBig2_GRDProc());
    pGRD->MMR = GSMMR;
    pGRD->GBW = GSW;
    pGRD->GBH = GSH;
    FXCODEC_STATUS status = pGRD->Start_decode_MMR(&GSPLANES[GSBPP - 1], pStream);
    while(status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
        pGRD->Continue_decode(pPause);
    }
    if(GSPLANES[GSBPP - 1] == NULL) {
        goto failed;
    }
    pStream->alignByte();
    pStream->offset(3);
    J = GSBPP - 2;
    while(J >= 0) {
        FXCODEC_STATUS status = pGRD->Start_decode_MMR(&GSPLANES[J], pStream);
        while(status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
            pGRD->Continue_decode(pPause);
        }
        if(GSPLANES[J] == NULL) {
            for(K = GSBPP - 1; K > J; K--) {
                delete GSPLANES[K];
                goto failed;
            }
        }
        pStream->alignByte();
        pStream->offset(3);
        GSPLANES[J]->composeFrom(0, 0, GSPLANES[J + 1], JBIG2_COMPOSE_XOR);
        J = J - 1;
    }
    for(y = 0; y < GSH; y++) {
        for(x = 0; x < GSW; x++) {
            for(J = 0; J < GSBPP; J++) {
                GSVALS[y * GSW + x] |= GSPLANES[J]->getPixel(x, y) << J;
            }
        }
    }
    for(J = 0; J < GSBPP; J++) {
        delete GSPLANES[J];
    }
    m_pModule->JBig2_Free(GSPLANES);
    delete pGRD;
    return GSVALS;
failed:
    m_pModule->JBig2_Free(GSPLANES);
    delete pGRD;
    m_pModule->JBig2_Free(GSVALS);
    return NULL;
}
FXCODEC_STATUS CJBig2_GRDProc::Start_decode_Arith(CJBig2_Image** pImage, CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    if (GBW == 0 || GBH == 0) {
        m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
        return FXCODEC_STATUS_DECODE_FINISH;
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_READY;
    m_pPause = pPause;
    if(*pImage == NULL) {
        JBIG2_ALLOC((*pImage), CJBig2_Image(GBW, GBH));
    }
    if ((*pImage)->m_pData == NULL) {
        delete *pImage;
        *pImage = NULL;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        m_ProssiveStatus = FXCODEC_STATUS_ERROR;
        return FXCODEC_STATUS_ERROR;
    }
    m_DecodeType = 1;
    m_pImage = pImage;
    (*m_pImage)->fill(0);
    m_pArithDecoder = pArithDecoder;
    m_gbContext = gbContext;
    LTP = 0;
    m_pLine = NULL;
    m_loopIndex = 0;
    return decode_Arith(pPause);
}
FXCODEC_STATUS CJBig2_GRDProc::decode_Arith(IFX_Pause* pPause)
{
    int iline = m_loopIndex;
    CJBig2_Image* pImage = *m_pImage;
    if(GBTEMPLATE == 0) {
        if((GBAT[0] == 3) && (GBAT[1] == (signed char) - 1)
                && (GBAT[2] == (signed char) - 3) && (GBAT[3] == (signed char) - 1)
                && (GBAT[4] == 2) && (GBAT[5] == (signed char) - 2)
                && (GBAT[6] == (signed char) - 2) && (GBAT[7] == (signed char) - 2)) {
            m_ProssiveStatus = decode_Arith_Template0_opt3(pImage, m_pArithDecoder, m_gbContext, pPause);
        } else {
            m_ProssiveStatus = decode_Arith_Template0_unopt(pImage, m_pArithDecoder, m_gbContext, pPause);
        }
    } else if(GBTEMPLATE == 1) {
        if((GBAT[0] == 3) && (GBAT[1] == (signed char) - 1)) {
            m_ProssiveStatus = decode_Arith_Template1_opt3(pImage, m_pArithDecoder, m_gbContext, pPause);
        } else {
            m_ProssiveStatus = decode_Arith_Template1_unopt(pImage, m_pArithDecoder, m_gbContext, pPause);
        }
    } else if(GBTEMPLATE == 2) {
        if((GBAT[0] == 2) && (GBAT[1] == (signed char) - 1)) {
            m_ProssiveStatus =  decode_Arith_Template2_opt3(pImage, m_pArithDecoder, m_gbContext, pPause);
        } else {
            m_ProssiveStatus =  decode_Arith_Template2_unopt(pImage, m_pArithDecoder, m_gbContext, pPause);
        }
    } else {
        if((GBAT[0] == 2) && (GBAT[1] == (signed char) - 1)) {
            m_ProssiveStatus = decode_Arith_Template3_opt3(pImage, m_pArithDecoder, m_gbContext, pPause);
        } else {
            m_ProssiveStatus = decode_Arith_Template3_unopt(pImage, m_pArithDecoder, m_gbContext, pPause);
        }
    }
    m_ReplaceRect.left = 0;
    m_ReplaceRect.right = pImage->m_nWidth;
    m_ReplaceRect.top = iline;
    m_ReplaceRect.bottom = m_loopIndex;
    if(m_ProssiveStatus == FXCODEC_STATUS_DECODE_FINISH) {
        m_loopIndex = 0;
    }
    return m_ProssiveStatus;
}
FXCODEC_STATUS CJBig2_GRDProc::Start_decode_Arith_V2(CJBig2_Image** pImage, CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    if(GBW == 0 || GBH == 0) {
        * pImage = NULL;
        m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
        return FXCODEC_STATUS_DECODE_FINISH;
    }
    if(*pImage == NULL) {
        JBIG2_ALLOC((*pImage), CJBig2_Image(GBW, GBH));
    }
    if ((*pImage)->m_pData == NULL) {
        delete *pImage;
        *pImage = NULL;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        m_ProssiveStatus = FXCODEC_STATUS_ERROR;
        return FXCODEC_STATUS_ERROR;
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_READY;
    m_DecodeType = 2;
    m_pPause = pPause;
    m_pImage = pImage;
    (*m_pImage)->fill(0);
    LTP = 0;
    m_loopIndex = 0;
    m_pArithDecoder = pArithDecoder;
    m_gbContext = gbContext;
    return decode_Arith_V2(pPause);
}
FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_V2(IFX_Pause* pPause)
{
    FX_BOOL SLTP, bVal;
    FX_DWORD CONTEXT;
    CJBig2_Image *GBREG = *m_pImage;
    FX_DWORD line1, line2, line3;
    LTP = 0;
    JBIG2_ALLOC(GBREG, CJBig2_Image(GBW, GBH));
    GBREG->fill(0);
    for(; m_loopIndex < GBH; m_loopIndex++) {
        if(TPGDON) {
            switch(GBTEMPLATE) {
                case 0:
                    CONTEXT = 0x9b25;
                    break;
                case 1:
                    CONTEXT = 0x0795;
                    break;
                case 2:
                    CONTEXT = 0x00e5;
                    break;
                case 3:
                    CONTEXT = 0x0195;
                    break;
            }
            SLTP = m_pArithDecoder->DECODE(&m_gbContext[CONTEXT]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            GBREG->copyLine(m_loopIndex, m_loopIndex - 1);
        } else {
            switch(GBTEMPLATE) {
                case 0: {
                        line1 = GBREG->getPixel(1, m_loopIndex - 2);
                        line1 |= GBREG->getPixel(0, m_loopIndex - 2) << 1;
                        line2 = GBREG->getPixel(2, m_loopIndex - 1);
                        line2 |= GBREG->getPixel(1, m_loopIndex - 1) << 1;
                        line2 |= GBREG->getPixel(0, m_loopIndex - 1) << 2;
                        line3 = 0;
                        for(FX_DWORD w = 0; w < GBW; w++) {
                            if(USESKIP && SKIP->getPixel(w, m_loopIndex)) {
                                bVal = 0;
                            } else {
                                CONTEXT = line3;
                                CONTEXT |= GBREG->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 4;
                                CONTEXT |= line2 << 5;
                                CONTEXT |= GBREG->getPixel(w + GBAT[2], m_loopIndex + GBAT[3]) << 10;
                                CONTEXT |= GBREG->getPixel(w + GBAT[4], m_loopIndex + GBAT[5]) << 11;
                                CONTEXT |= line1 << 12;
                                CONTEXT |= GBREG->getPixel(w + GBAT[6], m_loopIndex + GBAT[7]) << 15;
                                bVal = m_pArithDecoder->DECODE(&m_gbContext[CONTEXT]);
                            }
                            if(bVal) {
                                GBREG->setPixel(w, m_loopIndex, bVal);
                            }
                            line1 = ((line1 << 1) | GBREG->getPixel(w + 2, m_loopIndex - 2)) & 0x07;
                            line2 = ((line2 << 1) | GBREG->getPixel(w + 3, m_loopIndex - 1)) & 0x1f;
                            line3 = ((line3 << 1) | bVal) & 0x0f;
                        }
                    }
                    break;
                case 1: {
                        line1 = GBREG->getPixel(2, m_loopIndex - 2);
                        line1 |= GBREG->getPixel(1, m_loopIndex - 2) << 1;
                        line1 |= GBREG->getPixel(0, m_loopIndex - 2) << 2;
                        line2 = GBREG->getPixel(2, m_loopIndex - 1);
                        line2 |= GBREG->getPixel(1, m_loopIndex - 1) << 1;
                        line2 |= GBREG->getPixel(0, m_loopIndex - 1) << 2;
                        line3 = 0;
                        for(FX_DWORD w = 0; w < GBW; w++) {
                            if(USESKIP && SKIP->getPixel(w, m_loopIndex)) {
                                bVal = 0;
                            } else {
                                CONTEXT = line3;
                                CONTEXT |= GBREG->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 3;
                                CONTEXT |= line2 << 4;
                                CONTEXT |= line1 << 9;
                                bVal = m_pArithDecoder->DECODE(&m_gbContext[CONTEXT]);
                            }
                            if(bVal) {
                                GBREG->setPixel(w, m_loopIndex, bVal);
                            }
                            line1 = ((line1 << 1) | GBREG->getPixel(w + 3, m_loopIndex - 2)) & 0x0f;
                            line2 = ((line2 << 1) | GBREG->getPixel(w + 3, m_loopIndex - 1)) & 0x1f;
                            line3 = ((line3 << 1) | bVal) & 0x07;
                        }
                    }
                    break;
                case 2: {
                        line1 = GBREG->getPixel(1, m_loopIndex - 2);
                        line1 |= GBREG->getPixel(0, m_loopIndex - 2) << 1;
                        line2 = GBREG->getPixel(1, m_loopIndex - 1);
                        line2 |= GBREG->getPixel(0, m_loopIndex - 1) << 1;
                        line3 = 0;
                        for(FX_DWORD w = 0; w < GBW; w++) {
                            if(USESKIP && SKIP->getPixel(w, m_loopIndex)) {
                                bVal = 0;
                            } else {
                                CONTEXT = line3;
                                CONTEXT |= GBREG->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 2;
                                CONTEXT |= line2 << 3;
                                CONTEXT |= line1 << 7;
                                bVal = m_pArithDecoder->DECODE(&m_gbContext[CONTEXT]);
                            }
                            if(bVal) {
                                GBREG->setPixel(w, m_loopIndex, bVal);
                            }
                            line1 = ((line1 << 1) | GBREG->getPixel(w + 2, m_loopIndex - 2)) & 0x07;
                            line2 = ((line2 << 1) | GBREG->getPixel(w + 2, m_loopIndex - 1)) & 0x0f;
                            line3 = ((line3 << 1) | bVal) & 0x03;
                        }
                    }
                    break;
                case 3: {
                        line1 = GBREG->getPixel(1, m_loopIndex - 1);
                        line1 |= GBREG->getPixel(0, m_loopIndex - 1) << 1;
                        line2 = 0;
                        for(FX_DWORD w = 0; w < GBW; w++) {
                            if(USESKIP && SKIP->getPixel(w, m_loopIndex)) {
                                bVal = 0;
                            } else {
                                CONTEXT = line2;
                                CONTEXT |= GBREG->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 4;
                                CONTEXT |= line1 << 5;
                                bVal = m_pArithDecoder->DECODE(&m_gbContext[CONTEXT]);
                            }
                            if(bVal) {
                                GBREG->setPixel(w, m_loopIndex, bVal);
                            }
                            line1 = ((line1 << 1) | GBREG->getPixel(w + 2, m_loopIndex - 1)) & 0x1f;
                            line2 = ((line2 << 1) | bVal) & 0x0f;
                        }
                    }
                    break;
            }
        }
        if(pPause && pPause->NeedToPauseNow()) {
            m_loopIndex ++;
            m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return FXCODEC_STATUS_DECODE_FINISH;
}
FXCODEC_STATUS CJBig2_GRDProc::Start_decode_Arith_V1(CJBig2_Image** pImage, CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    if(GBW == 0 || GBH == 0) {
        * pImage = NULL;
        m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
        return FXCODEC_STATUS_DECODE_FINISH;
    }
    if(*pImage == NULL) {
        JBIG2_ALLOC((*pImage), CJBig2_Image(GBW, GBH));
    }
    if ((*pImage)->m_pData == NULL) {
        delete *pImage;
        *pImage = NULL;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        m_ProssiveStatus = FXCODEC_STATUS_ERROR;
        return FXCODEC_STATUS_ERROR;
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_READY;
    m_pPause = pPause;
    m_pImage = pImage;
    m_DecodeType = 3;
    (*m_pImage)->fill(0);
    LTP = 0;
    m_loopIndex = 0;
    m_pArithDecoder = pArithDecoder;
    m_gbContext = gbContext;
    return decode_Arith_V1(pPause);
}
FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_V1(IFX_Pause* pPause)
{
    FX_BOOL SLTP, bVal;
    FX_DWORD CONTEXT = 0;
    CJBig2_Image *GBREG = (*m_pImage);
    for(; m_loopIndex < GBH; m_loopIndex++) {
        if(TPGDON) {
            switch(GBTEMPLATE) {
                case 0:
                    CONTEXT = 0x9b25;
                    break;
                case 1:
                    CONTEXT = 0x0795;
                    break;
                case 2:
                    CONTEXT = 0x00e5;
                    break;
                case 3:
                    CONTEXT = 0x0195;
                    break;
            }
            SLTP = m_pArithDecoder->DECODE(&m_gbContext[CONTEXT]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            for(FX_DWORD w = 0; w < GBW; w++) {
                GBREG->setPixel(w, m_loopIndex, GBREG->getPixel(w, m_loopIndex - 1));
            }
        } else {
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, m_loopIndex)) {
                    GBREG->setPixel(w, m_loopIndex, 0);
                } else {
                    CONTEXT = 0;
                    switch(GBTEMPLATE) {
                        case 0:
                            CONTEXT |= GBREG->getPixel(w - 1, m_loopIndex);
                            CONTEXT |= GBREG->getPixel(w - 2, m_loopIndex) << 1;
                            CONTEXT |= GBREG->getPixel(w - 3, m_loopIndex) << 2;
                            CONTEXT |= GBREG->getPixel(w - 4, m_loopIndex) << 3;
                            CONTEXT |= GBREG->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 4;
                            CONTEXT |= GBREG->getPixel(w + 2, m_loopIndex - 1) << 5;
                            CONTEXT |= GBREG->getPixel(w + 1, m_loopIndex - 1) << 6;
                            CONTEXT |= GBREG->getPixel(w, m_loopIndex - 1) << 7;
                            CONTEXT |= GBREG->getPixel(w - 1, m_loopIndex - 1) << 8;
                            CONTEXT |= GBREG->getPixel(w - 2, m_loopIndex - 1) << 9;
                            CONTEXT |= GBREG->getPixel(w + GBAT[2], m_loopIndex + GBAT[3]) << 10;
                            CONTEXT |= GBREG->getPixel(w + GBAT[4], m_loopIndex + GBAT[5]) << 11;
                            CONTEXT |= GBREG->getPixel(w + 1, m_loopIndex - 2) << 12;
                            CONTEXT |= GBREG->getPixel(w, m_loopIndex - 2) << 13;
                            CONTEXT |= GBREG->getPixel(w - 1, m_loopIndex - 2) << 14;
                            CONTEXT |= GBREG->getPixel(w + GBAT[6], m_loopIndex + GBAT[7]) << 15;
                            break;
                        case 1:
                            CONTEXT |= GBREG->getPixel(w - 1, m_loopIndex);
                            CONTEXT |= GBREG->getPixel(w - 2, m_loopIndex) << 1;
                            CONTEXT |= GBREG->getPixel(w - 3, m_loopIndex) << 2;
                            CONTEXT |= GBREG->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 3;
                            CONTEXT |= GBREG->getPixel(w + 2, m_loopIndex - 1) << 4;
                            CONTEXT |= GBREG->getPixel(w + 1, m_loopIndex - 1) << 5;
                            CONTEXT |= GBREG->getPixel(w, m_loopIndex - 1) << 6;
                            CONTEXT |= GBREG->getPixel(w - 1, m_loopIndex - 1) << 7;
                            CONTEXT |= GBREG->getPixel(w - 2, m_loopIndex - 1) << 8;
                            CONTEXT |= GBREG->getPixel(w + 2, m_loopIndex - 2) << 9;
                            CONTEXT |= GBREG->getPixel(w + 1, m_loopIndex - 2) << 10;
                            CONTEXT |= GBREG->getPixel(w, m_loopIndex - 2) << 11;
                            CONTEXT |= GBREG->getPixel(w - 1, m_loopIndex - 2) << 12;
                            break;
                        case 2:
                            CONTEXT |= GBREG->getPixel(w - 1, m_loopIndex);
                            CONTEXT |= GBREG->getPixel(w - 2, m_loopIndex) << 1;
                            CONTEXT |= GBREG->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 2;
                            CONTEXT |= GBREG->getPixel(w + 1, m_loopIndex - 1) << 3;
                            CONTEXT |= GBREG->getPixel(w, m_loopIndex - 1) << 4;
                            CONTEXT |= GBREG->getPixel(w - 1, m_loopIndex - 1) << 5;
                            CONTEXT |= GBREG->getPixel(w - 2, m_loopIndex - 1) << 6;
                            CONTEXT |= GBREG->getPixel(w + 1, m_loopIndex - 2) << 7;
                            CONTEXT |= GBREG->getPixel(w, m_loopIndex - 2) << 8;
                            CONTEXT |= GBREG->getPixel(w - 1, m_loopIndex - 2) << 9;
                            break;
                        case 3:
                            CONTEXT |= GBREG->getPixel(w - 1, m_loopIndex);
                            CONTEXT |= GBREG->getPixel(w - 2, m_loopIndex) << 1;
                            CONTEXT |= GBREG->getPixel(w - 3, m_loopIndex) << 2;
                            CONTEXT |= GBREG->getPixel(w - 4, m_loopIndex) << 3;
                            CONTEXT |= GBREG->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 4;
                            CONTEXT |= GBREG->getPixel(w + 1, m_loopIndex - 1) << 5;
                            CONTEXT |= GBREG->getPixel(w, m_loopIndex - 1) << 6;
                            CONTEXT |= GBREG->getPixel(w - 1, m_loopIndex - 1) << 7;
                            CONTEXT |= GBREG->getPixel(w - 2, m_loopIndex - 1) << 8;
                            CONTEXT |= GBREG->getPixel(w - 3, m_loopIndex - 1) << 9;
                            break;
                    }
                    bVal = m_pArithDecoder->DECODE(&m_gbContext[CONTEXT]);
                    GBREG->setPixel(w, m_loopIndex, bVal);
                }
            }
        }
        if(pPause && pPause->NeedToPauseNow()) {
            m_loopIndex ++;
            m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return FXCODEC_STATUS_DECODE_FINISH;
}
FXCODEC_STATUS CJBig2_GRDProc::Start_decode_MMR(CJBig2_Image** pImage, CJBig2_BitStream *pStream, IFX_Pause* pPause)
{
    int bitpos, i;
    JBIG2_ALLOC((* pImage), CJBig2_Image(GBW, GBH));
    if ((* pImage)->m_pData == NULL) {
        delete (* pImage);
        (* pImage) = NULL;
        m_pModule->JBig2_Error("Generic region decoding procedure: Create Image Failed with width = %d, height = %d\n", GBW, GBH);
        m_ProssiveStatus = FXCODEC_STATUS_ERROR;
        return m_ProssiveStatus;
    }
    bitpos = (int)pStream->getBitPos();
    _FaxG4Decode(m_pModule, pStream->getBuf(), pStream->getLength(), &bitpos, (* pImage)->m_pData, GBW, GBH, (* pImage)->m_nStride);
    pStream->setBitPos(bitpos);
    for(i = 0; (FX_DWORD)i < (* pImage)->m_nStride * GBH; i++) {
        (* pImage)->m_pData[i] = ~(* pImage)->m_pData[i];
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return m_ProssiveStatus;
}
FXCODEC_STATUS CJBig2_GRDProc::decode_MMR()
{
    return m_ProssiveStatus;
}
FXCODEC_STATUS CJBig2_GRDProc::Continue_decode(IFX_Pause* pPause)
{
    if(m_ProssiveStatus != FXCODEC_STATUS_DECODE_TOBECONTINUE) {
        return m_ProssiveStatus;
    }
    switch (m_DecodeType) {
        case 1:
            return decode_Arith(pPause);
        case 2:
            return decode_Arith_V2(pPause);
        case 3:
            return decode_Arith_V1(pPause);
        case 4:
            return decode_MMR();
    }
    m_ProssiveStatus = FXCODEC_STATUS_ERROR;
    return m_ProssiveStatus;
}
FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template0_opt3(CJBig2_Image *pImage, CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    FX_BOOL SLTP, bVal;
    FX_DWORD CONTEXT;
    FX_DWORD line1, line2;
    FX_BYTE *pLine1, *pLine2, cVal;
    FX_INT32 nStride, nStride2, k;
    FX_INT32 nLineBytes, nBitsLeft, cc;
    if(m_pLine == NULL) {
        m_pLine = pImage->m_pData;
    }
    nStride = pImage->m_nStride;
    nStride2 = nStride << 1;
    nLineBytes = ((GBW + 7) >> 3) - 1;
    nBitsLeft = GBW - (nLineBytes << 3);
    FX_DWORD height = GBH & 0x7fffffff;
    for(; m_loopIndex < height; m_loopIndex++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x9b25]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            pImage->copyLine(m_loopIndex, m_loopIndex - 1);
        } else {
            if(m_loopIndex > 1) {
                pLine1 = m_pLine - nStride2;
                pLine2 = m_pLine - nStride;
                line1 = (*pLine1++) << 6;
                line2 = *pLine2++;
                CONTEXT = ((line1 & 0xf800) | (line2 & 0x07f0));
                for(cc = 0; cc < nLineBytes; cc++) {
                    line1 = (line1 << 8) | ((*pLine1++) << 6);
                    line2 = (line2 << 8) | (*pLine2++);
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal
                                   | ((line1 >> k) & 0x0800)
                                   | ((line2 >> k) & 0x0010));
                    }
                    m_pLine[cc] = cVal;
                }
                line1 <<= 8;
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal
                               | ((line1 >> (7 - k)) & 0x0800)
                               | ((line2 >> (7 - k)) & 0x0010));
                }
                m_pLine[nLineBytes] = cVal;
            } else {
                pLine2 = m_pLine - nStride;
                line2 = (m_loopIndex & 1) ? (*pLine2++) : 0;
                CONTEXT = (line2 & 0x07f0);
                for(cc = 0; cc < nLineBytes; cc++) {
                    if(m_loopIndex & 1) {
                        line2 = (line2 << 8) | (*pLine2++);
                    }
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal
                                   | ((line2 >> k) & 0x0010));
                    }
                    m_pLine[cc] = cVal;
                }
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = (((CONTEXT & 0x7bf7) << 1) | bVal
                               | ((line2 >> (7 - k)) & 0x0010));
                }
                m_pLine[nLineBytes] = cVal;
            }
        }
        m_pLine += nStride;
        if(pPause && pPause->NeedToPauseNow()) {
            m_loopIndex++;
            m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return FXCODEC_STATUS_DECODE_FINISH;
}
FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template0_unopt(CJBig2_Image * pImage, CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    FX_BOOL SLTP, bVal;
    FX_DWORD CONTEXT;
    FX_DWORD line1, line2, line3;
    for(; m_loopIndex < GBH; m_loopIndex++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x9b25]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            pImage->copyLine(m_loopIndex, m_loopIndex - 1);
        } else {
            line1 = pImage->getPixel(1, m_loopIndex - 2);
            line1 |= pImage->getPixel(0, m_loopIndex - 2) << 1;
            line2 = pImage->getPixel(2, m_loopIndex - 1);
            line2 |= pImage->getPixel(1, m_loopIndex - 1) << 1;
            line2 |= pImage->getPixel(0, m_loopIndex - 1) << 2;
            line3 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, m_loopIndex)) {
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
                if(bVal) {
                    pImage->setPixel(w, m_loopIndex, bVal);
                }
                line1 = ((line1 << 1) | pImage->getPixel(w + 2, m_loopIndex - 2)) & 0x07;
                line2 = ((line2 << 1) | pImage->getPixel(w + 3, m_loopIndex - 1)) & 0x1f;
                line3 = ((line3 << 1) | bVal) & 0x0f;
            }
        }
        if(pPause && pPause->NeedToPauseNow()) {
            m_loopIndex++;
            m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return FXCODEC_STATUS_DECODE_FINISH;
}
FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template1_opt3(CJBig2_Image *pImage, CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    FX_BOOL SLTP, bVal;
    FX_DWORD CONTEXT;
    FX_DWORD line1, line2;
    FX_BYTE *pLine1, *pLine2, cVal;
    FX_INT32 nStride, nStride2, k;
    FX_INT32 nLineBytes, nBitsLeft, cc;
    if (!m_pLine) {
        m_pLine = pImage->m_pData;
    }
    nStride = pImage->m_nStride;
    nStride2 = nStride << 1;
    nLineBytes = ((GBW + 7) >> 3) - 1;
    nBitsLeft = GBW - (nLineBytes << 3);
    for(; m_loopIndex < GBH; m_loopIndex++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0795]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            pImage->copyLine(m_loopIndex, m_loopIndex - 1);
        } else {
            if(m_loopIndex > 1) {
                pLine1 = m_pLine - nStride2;
                pLine2 = m_pLine - nStride;
                line1 = (*pLine1++) << 4;
                line2 = *pLine2++;
                CONTEXT = (line1 & 0x1e00) | ((line2 >> 1) & 0x01f8);
                for(cc = 0; cc < nLineBytes; cc++) {
                    line1 = (line1 << 8) | ((*pLine1++) << 4);
                    line2 = (line2 << 8) | (*pLine2++);
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal
                                  | ((line1 >> k) & 0x0200)
                                  | ((line2 >> (k + 1)) & 0x0008);
                    }
                    m_pLine[cc] = cVal;
                }
                line1 <<= 8;
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal
                              | ((line1 >> (7 - k)) & 0x0200)
                              | ((line2 >> (8 - k)) & 0x0008);
                }
                m_pLine[nLineBytes] = cVal;
            } else {
                pLine2 = m_pLine - nStride;
                line2 = (m_loopIndex & 1) ? (*pLine2++) : 0;
                CONTEXT = (line2 >> 1) & 0x01f8;
                for(cc = 0; cc < nLineBytes; cc++) {
                    if(m_loopIndex & 1) {
                        line2 = (line2 << 8) | (*pLine2++);
                    }
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal
                                  | ((line2 >> (k + 1)) & 0x0008);
                    }
                    m_pLine[cc] = cVal;
                }
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x0efb) << 1) | bVal
                              | ((line2 >> (8 - k)) & 0x0008);
                }
                m_pLine[nLineBytes] = cVal;
            }
        }
        m_pLine += nStride;
        if(pPause && pPause->NeedToPauseNow()) {
            m_loopIndex++;
            m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return FXCODEC_STATUS_DECODE_FINISH;
}
FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template1_unopt(CJBig2_Image * pImage, CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    FX_BOOL SLTP, bVal;
    FX_DWORD CONTEXT;
    FX_DWORD line1, line2, line3;
    for(FX_DWORD h = 0; h < GBH; h++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0795]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            pImage->copyLine(h, h - 1);
        } else {
            line1 = pImage->getPixel(2, h - 2);
            line1 |= pImage->getPixel(1, h - 2) << 1;
            line1 |= pImage->getPixel(0, h - 2) << 2;
            line2 = pImage->getPixel(2, h - 1);
            line2 |= pImage->getPixel(1, h - 1) << 1;
            line2 |= pImage->getPixel(0, h - 1) << 2;
            line3 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, h)) {
                    bVal = 0;
                } else {
                    CONTEXT = line3;
                    CONTEXT |= pImage->getPixel(w + GBAT[0], h + GBAT[1]) << 3;
                    CONTEXT |= line2 << 4;
                    CONTEXT |= line1 << 9;
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                }
                if(bVal) {
                    pImage->setPixel(w, h, bVal);
                }
                line1 = ((line1 << 1) | pImage->getPixel(w + 3, h - 2)) & 0x0f;
                line2 = ((line2 << 1) | pImage->getPixel(w + 3, h - 1)) & 0x1f;
                line3 = ((line3 << 1) | bVal) & 0x07;
            }
        }
        if(pPause && pPause->NeedToPauseNow()) {
            m_loopIndex++;
            m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return FXCODEC_STATUS_DECODE_FINISH;
}
FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template2_opt3(CJBig2_Image *pImage, CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    FX_BOOL SLTP, bVal;
    FX_DWORD CONTEXT;
    FX_DWORD line1, line2;
    FX_BYTE *pLine1, *pLine2, cVal;
    FX_INT32 nStride, nStride2, k;
    FX_INT32 nLineBytes, nBitsLeft, cc;
    if(!m_pLine) {
        m_pLine = pImage->m_pData;
    }
    nStride = pImage->m_nStride;
    nStride2 = nStride << 1;
    nLineBytes = ((GBW + 7) >> 3) - 1;
    nBitsLeft = GBW - (nLineBytes << 3);
    for(; m_loopIndex < GBH; m_loopIndex++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x00e5]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            pImage->copyLine(m_loopIndex, m_loopIndex - 1);
        } else {
            if(m_loopIndex > 1) {
                pLine1 = m_pLine - nStride2;
                pLine2 = m_pLine - nStride;
                line1 = (*pLine1++) << 1;
                line2 = *pLine2++;
                CONTEXT = (line1 & 0x0380) | ((line2 >> 3) & 0x007c);
                for(cc = 0; cc < nLineBytes; cc++) {
                    line1 = (line1 << 8) | ((*pLine1++) << 1);
                    line2 = (line2 << 8) | (*pLine2++);
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal
                                  | ((line1 >> k) & 0x0080)
                                  | ((line2 >> (k + 3)) & 0x0004);
                    }
                    m_pLine[cc] = cVal;
                }
                line1 <<= 8;
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal
                              | ((line1 >> (7 - k)) & 0x0080)
                              | ((line2 >> (10 - k)) & 0x0004);
                }
                m_pLine[nLineBytes] = cVal;
            } else {
                pLine2 = m_pLine - nStride;
                line2 = (m_loopIndex & 1) ? (*pLine2++) : 0;
                CONTEXT = (line2 >> 3) & 0x007c;
                for(cc = 0; cc < nLineBytes; cc++) {
                    if(m_loopIndex & 1) {
                        line2 = (line2 << 8) | (*pLine2++);
                    }
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal
                                  | ((line2 >> (k + 3)) & 0x0004);
                    }
                    m_pLine[cc] = cVal;
                }
                line2 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x01bd) << 1) | bVal
                              | (((line2 >> (10 - k))) & 0x0004);
                }
                m_pLine[nLineBytes] = cVal;
            }
        }
        m_pLine += nStride;
        if(pPause && m_loopIndex % 50 == 0 && pPause->NeedToPauseNow()) {
            m_loopIndex++;
            m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return FXCODEC_STATUS_DECODE_FINISH;
}
FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template2_unopt(CJBig2_Image * pImage, CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    FX_BOOL SLTP, bVal;
    FX_DWORD CONTEXT;
    FX_DWORD line1, line2, line3;
    for(; m_loopIndex < GBH; m_loopIndex++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x00e5]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            pImage->copyLine(m_loopIndex, m_loopIndex - 1);
        } else {
            line1 = pImage->getPixel(1, m_loopIndex - 2);
            line1 |= pImage->getPixel(0, m_loopIndex - 2) << 1;
            line2 = pImage->getPixel(1, m_loopIndex - 1);
            line2 |= pImage->getPixel(0, m_loopIndex - 1) << 1;
            line3 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, m_loopIndex)) {
                    bVal = 0;
                } else {
                    CONTEXT = line3;
                    CONTEXT |= pImage->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 2;
                    CONTEXT |= line2 << 3;
                    CONTEXT |= line1 << 7;
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                }
                if(bVal) {
                    pImage->setPixel(w, m_loopIndex, bVal);
                }
                line1 = ((line1 << 1) | pImage->getPixel(w + 2, m_loopIndex - 2)) & 0x07;
                line2 = ((line2 << 1) | pImage->getPixel(w + 2, m_loopIndex - 1)) & 0x0f;
                line3 = ((line3 << 1) | bVal) & 0x03;
            }
        }
        if(pPause && pPause->NeedToPauseNow()) {
            m_loopIndex++;
            m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return FXCODEC_STATUS_DECODE_FINISH;
}
FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template3_opt3(CJBig2_Image *pImage, CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    FX_BOOL SLTP, bVal;
    FX_DWORD CONTEXT;
    FX_DWORD line1;
    FX_BYTE *pLine1, cVal;
    FX_INT32 nStride, k;
    FX_INT32 nLineBytes, nBitsLeft, cc;
    if (!m_pLine) {
        m_pLine = pImage->m_pData;
    }
    nStride = pImage->m_nStride;
    nLineBytes = ((GBW + 7) >> 3) - 1;
    nBitsLeft = GBW - (nLineBytes << 3);
    for(; m_loopIndex < GBH; m_loopIndex++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0195]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            pImage->copyLine(m_loopIndex, m_loopIndex - 1);
        } else {
            if(m_loopIndex > 0) {
                pLine1 = m_pLine - nStride;
                line1 = *pLine1++;
                CONTEXT = (line1 >> 1) & 0x03f0;
                for(cc = 0; cc < nLineBytes; cc++) {
                    line1 = (line1 << 8) | (*pLine1++);
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal
                                  | ((line1 >> (k + 1)) & 0x0010);
                    }
                    m_pLine[cc] = cVal;
                }
                line1 <<= 8;
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal
                              | ((line1 >> (8 - k)) & 0x0010);
                }
                m_pLine[nLineBytes] = cVal;
            } else {
                CONTEXT = 0;
                for(cc = 0; cc < nLineBytes; cc++) {
                    cVal = 0;
                    for(k = 7; k >= 0; k--) {
                        bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                        cVal |= bVal << k;
                        CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal;
                    }
                    m_pLine[cc] = cVal;
                }
                cVal = 0;
                for(k = 0; k < nBitsLeft; k++) {
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                    cVal |= bVal << (7 - k);
                    CONTEXT = ((CONTEXT & 0x01f7) << 1) | bVal;
                }
                m_pLine[nLineBytes] = cVal;
            }
        }
        m_pLine += nStride;
        if(pPause && pPause->NeedToPauseNow()) {
            m_loopIndex++;
            m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return FXCODEC_STATUS_DECODE_FINISH;
}
FXCODEC_STATUS CJBig2_GRDProc::decode_Arith_Template3_unopt(CJBig2_Image * pImage, CJBig2_ArithDecoder *pArithDecoder, JBig2ArithCtx *gbContext, IFX_Pause* pPause)
{
    FX_BOOL SLTP, bVal;
    FX_DWORD CONTEXT;
    FX_DWORD line1, line2;
    for(; m_loopIndex < GBH; m_loopIndex++) {
        if(TPGDON) {
            SLTP = pArithDecoder->DECODE(&gbContext[0x0195]);
            LTP = LTP ^ SLTP;
        }
        if(LTP == 1) {
            pImage->copyLine(m_loopIndex, m_loopIndex - 1);
        } else {
            line1 = pImage->getPixel(1, m_loopIndex - 1);
            line1 |= pImage->getPixel(0, m_loopIndex - 1) << 1;
            line2 = 0;
            for(FX_DWORD w = 0; w < GBW; w++) {
                if(USESKIP && SKIP->getPixel(w, m_loopIndex)) {
                    bVal = 0;
                } else {
                    CONTEXT = line2;
                    CONTEXT |= pImage->getPixel(w + GBAT[0], m_loopIndex + GBAT[1]) << 4;
                    CONTEXT |= line1 << 5;
                    bVal = pArithDecoder->DECODE(&gbContext[CONTEXT]);
                }
                if(bVal) {
                    pImage->setPixel(w, m_loopIndex, bVal);
                }
                line1 = ((line1 << 1) | pImage->getPixel(w + 2, m_loopIndex - 1)) & 0x1f;
                line2 = ((line2 << 1) | bVal) & 0x0f;
            }
        }
        if(pPause && pPause->NeedToPauseNow()) {
            m_loopIndex++;
            m_ProssiveStatus = FXCODEC_STATUS_DECODE_TOBECONTINUE;
            return FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
    }
    m_ProssiveStatus = FXCODEC_STATUS_DECODE_FINISH;
    return FXCODEC_STATUS_DECODE_FINISH;
}
