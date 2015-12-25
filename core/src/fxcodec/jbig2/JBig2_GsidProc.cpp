// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_GsidProc.h"

#include <memory>

#include "JBig2_BitStream.h"
#include "JBig2_GrdProc.h"
#include "JBig2_Image.h"
#include "JBig2_List.h"
#include "core/include/fxcrt/fx_basic.h"

FX_DWORD* CJBig2_GSIDProc::decode_Arith(CJBig2_ArithDecoder* pArithDecoder,
                                        JBig2ArithCtx* gbContext,
                                        IFX_Pause* pPause) {
  std::unique_ptr<CJBig2_GRDProc> pGRD(new CJBig2_GRDProc());
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

  CJBig2_List<CJBig2_Image> GSPLANES(GSBPP);
  for (int32_t i = GSBPP - 1; i >= 0; --i) {
    CJBig2_Image* pImage = nullptr;
    FXCODEC_STATUS status =
        pGRD->Start_decode_Arith(&pImage, pArithDecoder, gbContext, nullptr);
    while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE)
      pGRD->Continue_decode(pPause);

    if (!pImage)
      return nullptr;

    GSPLANES.set(i, pImage);

    if (i < GSBPP - 1)
      pImage->composeFrom(0, 0, GSPLANES.get(i + 1), JBIG2_COMPOSE_XOR);
  }
  std::unique_ptr<FX_DWORD, FxFreeDeleter> GSVALS(
      FX_Alloc2D(FX_DWORD, GSW, GSH));
  JBIG2_memset(GSVALS.get(), 0, sizeof(FX_DWORD) * GSW * GSH);
  for (FX_DWORD y = 0; y < GSH; ++y) {
    for (FX_DWORD x = 0; x < GSW; ++x) {
      for (int32_t i = 0; i < GSBPP; ++i) {
        GSVALS.get()[y * GSW + x] |= GSPLANES.get(i)->getPixel(x, y) << i;
      }
    }
  }
  return GSVALS.release();
}

FX_DWORD* CJBig2_GSIDProc::decode_MMR(CJBig2_BitStream* pStream,
                                      IFX_Pause* pPause) {
  std::unique_ptr<CJBig2_GRDProc> pGRD(new CJBig2_GRDProc());
  pGRD->MMR = GSMMR;
  pGRD->GBW = GSW;
  pGRD->GBH = GSH;

  std::unique_ptr<CJBig2_Image*> GSPLANES(FX_Alloc(CJBig2_Image*, GSBPP));
  JBIG2_memset(GSPLANES.get(), 0, sizeof(CJBig2_Image*) * GSBPP);
  FXCODEC_STATUS status =
      pGRD->Start_decode_MMR(&GSPLANES.get()[GSBPP - 1], pStream, nullptr);
  while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
    pGRD->Continue_decode(pPause);
  }
  if (!GSPLANES.get()[GSBPP - 1])
    return nullptr;

  pStream->alignByte();
  pStream->offset(3);
  int32_t J = GSBPP - 2;
  while (J >= 0) {
    FXCODEC_STATUS status =
        pGRD->Start_decode_MMR(&GSPLANES.get()[J], pStream, nullptr);
    while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
      pGRD->Continue_decode(pPause);
    }
    if (!GSPLANES.get()[J]) {
      for (int32_t K = GSBPP - 1; K > J; --K) {
        delete GSPLANES.get()[K];
        return nullptr;
      }
    }
    pStream->alignByte();
    pStream->offset(3);
    GSPLANES.get()[J]->composeFrom(0, 0, GSPLANES.get()[J + 1],
                                   JBIG2_COMPOSE_XOR);
    J = J - 1;
  }
  std::unique_ptr<FX_DWORD> GSVALS(FX_Alloc2D(FX_DWORD, GSW, GSH));
  JBIG2_memset(GSVALS.get(), 0, sizeof(FX_DWORD) * GSW * GSH);
  for (FX_DWORD y = 0; y < GSH; ++y) {
    for (FX_DWORD x = 0; x < GSW; ++x) {
      for (J = 0; J < GSBPP; ++J) {
        GSVALS.get()[y * GSW + x] |= GSPLANES.get()[J]->getPixel(x, y) << J;
      }
    }
  }
  for (J = 0; J < GSBPP; ++J) {
    delete GSPLANES.get()[J];
  }
  return GSVALS.release();
}
