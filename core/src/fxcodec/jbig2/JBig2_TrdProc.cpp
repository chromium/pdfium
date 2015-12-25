// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_TrdProc.h"

#include <memory>

#include "JBig2_ArithDecoder.h"
#include "JBig2_ArithIntDecoder.h"
#include "JBig2_GrrdProc.h"
#include "JBig2_HuffmanDecoder.h"

CJBig2_Image* CJBig2_TRDProc::decode_Huffman(CJBig2_BitStream* pStream,
                                             JBig2ArithCtx* grContext) {
  int32_t STRIPT, FIRSTS;
  FX_DWORD NINSTANCES;
  int32_t DT, DFS, CURS;
  int32_t SI, TI;
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
  std::unique_ptr<CJBig2_HuffmanDecoder> pHuffmanDecoder(
      new CJBig2_HuffmanDecoder(pStream));
  std::unique_ptr<CJBig2_Image> SBREG(new CJBig2_Image(SBW, SBH));
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
      uint8_t CURT = 0;
      if (SBSTRIPS != 1) {
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
      FX_DWORD IDI;
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

        std::unique_ptr<CJBig2_GRRDProc> pGRRD(new CJBig2_GRRDProc());
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
          std::unique_ptr<CJBig2_ArithDecoder> pArithDecoder(
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
  int32_t SI, TI;
  CJBig2_Image* IBI;
  FX_DWORD WI, HI;
  int32_t IDS;
  int RI;
  int32_t RDWI, RDHI, RDXI, RDYI;
  CJBig2_Image* IBOI;
  FX_DWORD WOI, HOI;
  FX_BOOL bFirst;
  int32_t bRetained;
  CJBig2_ArithIntDecoder* IADT, *IAFS, *IADS, *IAIT, *IARI, *IARDW, *IARDH,
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
  std::unique_ptr<CJBig2_Image> SBREG(new CJBig2_Image(SBW, SBH));
  SBREG->fill(SBDEFPIXEL);
  IADT->decode(pArithDecoder, &STRIPT);
  STRIPT *= SBSTRIPS;
  STRIPT = -STRIPT;
  FIRSTS = 0;
  NINSTANCES = 0;
  while (NINSTANCES < SBNUMINSTANCES) {
    IADT->decode(pArithDecoder, &DT);
    DT *= SBSTRIPS;
    STRIPT = STRIPT + DT;
    bFirst = TRUE;
    for (;;) {
      if (bFirst) {
        IAFS->decode(pArithDecoder, &DFS);
        FIRSTS = FIRSTS + DFS;
        CURS = FIRSTS;
        bFirst = FALSE;
      } else {
        if (!IADS->decode(pArithDecoder, &IDS))
          break;
        CURS = CURS + IDS + SBDSOFFSET;
      }
      if (NINSTANCES >= SBNUMINSTANCES) {
        break;
      }
      int CURT = 0;
      if (SBSTRIPS != 1)
        IAIT->decode(pArithDecoder, &CURT);

      TI = STRIPT + CURT;
      FX_DWORD IDI;
      IAID->decode(pArithDecoder, &IDI);
      if (IDI >= SBNUMSYMS)
        goto failed;

      if (SBREFINE == 0)
        RI = 0;
      else
        IARI->decode(pArithDecoder, &RI);

      if (!SBSYMS[IDI])
        goto failed;

      if (RI == 0) {
        IBI = SBSYMS[IDI];
      } else {
        IARDW->decode(pArithDecoder, &RDWI);
        IARDH->decode(pArithDecoder, &RDHI);
        IARDX->decode(pArithDecoder, &RDXI);
        IARDY->decode(pArithDecoder, &RDYI);
        IBOI = SBSYMS[IDI];
        WOI = IBOI->m_nWidth;
        HOI = IBOI->m_nHeight;
        if ((int)(WOI + RDWI) < 0 || (int)(HOI + RDHI) < 0) {
          goto failed;
        }
        std::unique_ptr<CJBig2_GRRDProc> pGRRD(new CJBig2_GRRDProc());
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
