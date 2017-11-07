// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jbig2/JBig2_TrdProc.h"

#include <memory>

#include "core/fxcodec/jbig2/JBig2_ArithDecoder.h"
#include "core/fxcodec/jbig2/JBig2_ArithIntDecoder.h"
#include "core/fxcodec/jbig2/JBig2_GrrdProc.h"
#include "core/fxcodec/jbig2/JBig2_HuffmanDecoder.h"
#include "core/fxcrt/maybe_owned.h"
#include "third_party/base/ptr_util.h"

CJBig2_TRDProc::CJBig2_TRDProc() {}

CJBig2_TRDProc::~CJBig2_TRDProc() {}

std::unique_ptr<CJBig2_Image> CJBig2_TRDProc::decode_Huffman(
    CJBig2_BitStream* pStream,
    JBig2ArithCtx* grContext) {
  auto pHuffmanDecoder = pdfium::MakeUnique<CJBig2_HuffmanDecoder>(pStream);
  auto SBREG = pdfium::MakeUnique<CJBig2_Image>(SBW, SBH);
  SBREG->fill(SBDEFPIXEL);
  int32_t INITIAL_STRIPT;
  if (pHuffmanDecoder->decodeAValue(SBHUFFDT, &INITIAL_STRIPT) != 0)
    return nullptr;

  FX_SAFE_INT32 STRIPT = INITIAL_STRIPT;
  STRIPT *= SBSTRIPS;
  STRIPT = -STRIPT;
  FX_SAFE_INT32 FIRSTS = 0;
  uint32_t NINSTANCES = 0;
  while (NINSTANCES < SBNUMINSTANCES) {
    int32_t INITIAL_DT;
    if (pHuffmanDecoder->decodeAValue(SBHUFFDT, &INITIAL_DT) != 0)
      return nullptr;

    FX_SAFE_INT32 DT = INITIAL_DT;
    DT *= SBSTRIPS;
    STRIPT += DT;
    bool bFirst = true;
    FX_SAFE_INT32 CURS = 0;
    for (;;) {
      if (bFirst) {
        int32_t DFS;
        if (pHuffmanDecoder->decodeAValue(SBHUFFFS, &DFS) != 0)
          return nullptr;

        FIRSTS += DFS;
        CURS = FIRSTS;
        bFirst = false;
      } else {
        int32_t IDS;
        int32_t nVal = pHuffmanDecoder->decodeAValue(SBHUFFDS, &IDS);
        if (nVal == JBIG2_OOB)
          break;

        if (nVal != 0)
          return nullptr;

        CURS += IDS;
        CURS += SBDSOFFSET;
      }
      uint8_t CURT = 0;
      if (SBSTRIPS != 1) {
        uint32_t nTmp = 1;
        while (static_cast<uint32_t>(1 << nTmp) < SBSTRIPS)
          ++nTmp;
        int32_t nVal;
        if (pStream->readNBits(nTmp, &nVal) != 0)
          return nullptr;

        CURT = nVal;
      }
      FX_SAFE_INT32 SAFE_TI = STRIPT + CURT;
      if (!SAFE_TI.IsValid())
        return nullptr;

      int32_t TI = SAFE_TI.ValueOrDie();
      pdfium::base::CheckedNumeric<int32_t> nVal = 0;
      int32_t nBits = 0;
      uint32_t IDI;
      for (;;) {
        uint32_t nTmp;
        if (pStream->read1Bit(&nTmp) != 0)
          return nullptr;

        nVal <<= 1;
        if (!nVal.IsValid())
          return nullptr;

        nVal |= nTmp;
        ++nBits;
        for (IDI = 0; IDI < SBNUMSYMS; ++IDI) {
          if ((nBits == SBSYMCODES[IDI].codelen) &&
              (nVal.ValueOrDie() == SBSYMCODES[IDI].code)) {
            break;
          }
        }
        if (IDI < SBNUMSYMS)
          break;
      }
      bool RI = 0;
      if (SBREFINE != 0 && pStream->read1Bit(&RI) != 0)
        return nullptr;

      MaybeOwned<CJBig2_Image> IBI;
      if (RI == 0) {
        IBI = SBSYMS[IDI];
      } else {
        int32_t RDWI;
        int32_t RDHI;
        int32_t RDXI;
        int32_t RDYI;
        int32_t HUFFRSIZE;
        if ((pHuffmanDecoder->decodeAValue(SBHUFFRDW, &RDWI) != 0) ||
            (pHuffmanDecoder->decodeAValue(SBHUFFRDH, &RDHI) != 0) ||
            (pHuffmanDecoder->decodeAValue(SBHUFFRDX, &RDXI) != 0) ||
            (pHuffmanDecoder->decodeAValue(SBHUFFRDY, &RDYI) != 0) ||
            (pHuffmanDecoder->decodeAValue(SBHUFFRSIZE, &HUFFRSIZE) != 0)) {
          return nullptr;
        }
        pStream->alignByte();
        uint32_t nTmp = pStream->getOffset();
        CJBig2_Image* IBOI = SBSYMS[IDI];
        if (!IBOI)
          return nullptr;

        uint32_t WOI = IBOI->width();
        uint32_t HOI = IBOI->height();
        if (static_cast<int>(WOI + RDWI) < 0 ||
            static_cast<int>(HOI + RDHI) < 0) {
          return nullptr;
        }

        auto pGRRD = pdfium::MakeUnique<CJBig2_GRRDProc>();
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

        auto pArithDecoder = pdfium::MakeUnique<CJBig2_ArithDecoder>(pStream);
        IBI = pGRRD->decode(pArithDecoder.get(), grContext);
        if (!IBI)
          return nullptr;

        pStream->alignByte();
        pStream->offset(2);
        if (static_cast<uint32_t>(HUFFRSIZE) != (pStream->getOffset() - nTmp))
          return nullptr;
      }
      if (!IBI)
        continue;

      uint32_t WI = IBI->width();
      uint32_t HI = IBI->height();
      if (TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPRIGHT) ||
                              (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
        CURS += WI - 1;
      } else if (TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_BOTTOMLEFT) ||
                                     (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
        CURS += HI - 1;
      }
      if (!CURS.IsValid())
        return nullptr;

      int32_t SI = CURS.ValueOrDie();
      if (TRANSPOSED == 0) {
        switch (REFCORNER) {
          case JBIG2_CORNER_TOPLEFT:
            SBREG->composeFrom(SI, TI, IBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_TOPRIGHT:
            SBREG->composeFrom(SI - WI + 1, TI, IBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMLEFT:
            SBREG->composeFrom(SI, TI - HI + 1, IBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMRIGHT:
            SBREG->composeFrom(SI - WI + 1, TI - HI + 1, IBI.Get(), SBCOMBOP);
            break;
        }
      } else {
        switch (REFCORNER) {
          case JBIG2_CORNER_TOPLEFT:
            SBREG->composeFrom(TI, SI, IBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_TOPRIGHT:
            SBREG->composeFrom(TI - WI + 1, SI, IBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMLEFT:
            SBREG->composeFrom(TI, SI - HI + 1, IBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMRIGHT:
            SBREG->composeFrom(TI - WI + 1, SI - HI + 1, IBI.Get(), SBCOMBOP);
            break;
        }
      }
      if (TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPLEFT) ||
                              (REFCORNER == JBIG2_CORNER_BOTTOMLEFT))) {
        CURS += WI - 1;
      } else if (TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_TOPLEFT) ||
                                     (REFCORNER == JBIG2_CORNER_TOPRIGHT))) {
        CURS += HI - 1;
      }
      NINSTANCES = NINSTANCES + 1;
    }
  }
  return SBREG;
}

std::unique_ptr<CJBig2_Image> CJBig2_TRDProc::decode_Arith(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* grContext,
    JBig2IntDecoderState* pIDS) {
  MaybeOwned<CJBig2_ArithIntDecoder> pIADT;
  MaybeOwned<CJBig2_ArithIntDecoder> pIAFS;
  MaybeOwned<CJBig2_ArithIntDecoder> pIADS;
  MaybeOwned<CJBig2_ArithIntDecoder> pIAIT;
  MaybeOwned<CJBig2_ArithIntDecoder> pIARI;
  MaybeOwned<CJBig2_ArithIntDecoder> pIARDW;
  MaybeOwned<CJBig2_ArithIntDecoder> pIARDH;
  MaybeOwned<CJBig2_ArithIntDecoder> pIARDX;
  MaybeOwned<CJBig2_ArithIntDecoder> pIARDY;
  MaybeOwned<CJBig2_ArithIaidDecoder> pIAID;
  if (pIDS) {
    pIADT = pIDS->IADT;
    pIAFS = pIDS->IAFS;
    pIADS = pIDS->IADS;
    pIAIT = pIDS->IAIT;
    pIARI = pIDS->IARI;
    pIARDW = pIDS->IARDW;
    pIARDH = pIDS->IARDH;
    pIARDX = pIDS->IARDX;
    pIARDY = pIDS->IARDY;
    pIAID = pIDS->IAID;
  } else {
    pIADT = pdfium::MakeUnique<CJBig2_ArithIntDecoder>();
    pIAFS = pdfium::MakeUnique<CJBig2_ArithIntDecoder>();
    pIADS = pdfium::MakeUnique<CJBig2_ArithIntDecoder>();
    pIAIT = pdfium::MakeUnique<CJBig2_ArithIntDecoder>();
    pIARI = pdfium::MakeUnique<CJBig2_ArithIntDecoder>();
    pIARDW = pdfium::MakeUnique<CJBig2_ArithIntDecoder>();
    pIARDH = pdfium::MakeUnique<CJBig2_ArithIntDecoder>();
    pIARDX = pdfium::MakeUnique<CJBig2_ArithIntDecoder>();
    pIARDY = pdfium::MakeUnique<CJBig2_ArithIntDecoder>();
    pIAID = pdfium::MakeUnique<CJBig2_ArithIaidDecoder>(SBSYMCODELEN);
  }
  auto SBREG = pdfium::MakeUnique<CJBig2_Image>(SBW, SBH);
  SBREG->fill(SBDEFPIXEL);
  int32_t INITIAL_STRIPT;
  if (!pIADT->decode(pArithDecoder, &INITIAL_STRIPT))
    return nullptr;

  FX_SAFE_INT32 STRIPT = INITIAL_STRIPT;
  STRIPT *= SBSTRIPS;
  STRIPT = -STRIPT;
  FX_SAFE_INT32 FIRSTS = 0;
  uint32_t NINSTANCES = 0;
  while (NINSTANCES < SBNUMINSTANCES) {
    FX_SAFE_INT32 CURS = 0;
    int32_t INITIAL_DT;
    if (!pIADT->decode(pArithDecoder, &INITIAL_DT))
      return nullptr;

    FX_SAFE_INT32 DT = INITIAL_DT;
    DT *= SBSTRIPS;
    STRIPT += DT;
    bool bFirst = true;
    for (;;) {
      if (bFirst) {
        int32_t DFS;
        pIAFS->decode(pArithDecoder, &DFS);
        FIRSTS += DFS;
        CURS = FIRSTS;
        bFirst = false;
      } else {
        int32_t IDS;
        if (!pIADS->decode(pArithDecoder, &IDS))
          break;

        CURS += IDS;
        CURS += SBDSOFFSET;
      }
      if (NINSTANCES >= SBNUMINSTANCES)
        break;

      int CURT = 0;
      if (SBSTRIPS != 1)
        pIAIT->decode(pArithDecoder, &CURT);

      FX_SAFE_INT32 SAFE_TI = STRIPT + CURT;
      if (!SAFE_TI.IsValid())
        return nullptr;

      int32_t TI = SAFE_TI.ValueOrDie();
      uint32_t IDI;
      pIAID->decode(pArithDecoder, &IDI);
      if (IDI >= SBNUMSYMS)
        return nullptr;

      int RI;
      if (SBREFINE == 0)
        RI = 0;
      else
        pIARI->decode(pArithDecoder, &RI);

      MaybeOwned<CJBig2_Image> pIBI;
      if (RI == 0) {
        pIBI = SBSYMS[IDI];
      } else {
        int32_t RDWI;
        int32_t RDHI;
        int32_t RDXI;
        int32_t RDYI;
        pIARDW->decode(pArithDecoder, &RDWI);
        pIARDH->decode(pArithDecoder, &RDHI);
        pIARDX->decode(pArithDecoder, &RDXI);
        pIARDY->decode(pArithDecoder, &RDYI);
        CJBig2_Image* IBOI = SBSYMS[IDI];
        if (!IBOI)
          return nullptr;

        uint32_t WOI = IBOI->width();
        uint32_t HOI = IBOI->height();
        if (static_cast<int>(WOI + RDWI) < 0 ||
            static_cast<int>(HOI + RDHI) < 0) {
          return nullptr;
        }

        auto pGRRD = pdfium::MakeUnique<CJBig2_GRRDProc>();
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
        pIBI = pGRRD->decode(pArithDecoder, grContext);
      }
      if (!pIBI)
        return nullptr;

      uint32_t WI = pIBI->width();
      uint32_t HI = pIBI->height();
      if (TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPRIGHT) ||
                              (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
        CURS += WI - 1;
      } else if (TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_BOTTOMLEFT) ||
                                     (REFCORNER == JBIG2_CORNER_BOTTOMRIGHT))) {
        CURS += HI - 1;
      }
      if (!CURS.IsValid())
        return nullptr;

      int32_t SI = CURS.ValueOrDie();
      if (TRANSPOSED == 0) {
        switch (REFCORNER) {
          case JBIG2_CORNER_TOPLEFT:
            SBREG->composeFrom(SI, TI, pIBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_TOPRIGHT:
            SBREG->composeFrom(SI - WI + 1, TI, pIBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMLEFT:
            SBREG->composeFrom(SI, TI - HI + 1, pIBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMRIGHT:
            SBREG->composeFrom(SI - WI + 1, TI - HI + 1, pIBI.Get(), SBCOMBOP);
            break;
        }
      } else {
        switch (REFCORNER) {
          case JBIG2_CORNER_TOPLEFT:
            SBREG->composeFrom(TI, SI, pIBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_TOPRIGHT:
            SBREG->composeFrom(TI - WI + 1, SI, pIBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMLEFT:
            SBREG->composeFrom(TI, SI - HI + 1, pIBI.Get(), SBCOMBOP);
            break;
          case JBIG2_CORNER_BOTTOMRIGHT:
            SBREG->composeFrom(TI - WI + 1, SI - HI + 1, pIBI.Get(), SBCOMBOP);
            break;
        }
      }
      if (TRANSPOSED == 0 && ((REFCORNER == JBIG2_CORNER_TOPLEFT) ||
                              (REFCORNER == JBIG2_CORNER_BOTTOMLEFT))) {
        CURS += WI - 1;
      } else if (TRANSPOSED == 1 && ((REFCORNER == JBIG2_CORNER_TOPLEFT) ||
                                     (REFCORNER == JBIG2_CORNER_TOPRIGHT))) {
        CURS += HI - 1;
      }
      ++NINSTANCES;
    }
  }
  return SBREG;
}
