// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_GsidProc.h"

#include "../../../../third_party/base/nonstd_unique_ptr.h"
#include "../../../include/fxcrt/fx_basic.h"
#include "JBig2_BitStream.h"
#include "JBig2_GrdProc.h"
#include "JBig2_Image.h"

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
  FXCODEC_STATUS status = pGRD->Start_decode_Arith(
      &GSPLANES[GSBPP - 1], pArithDecoder, gbContext, nullptr);
  while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
    pGRD->Continue_decode(pPause);
  }
  if (!GSPLANES[GSBPP - 1]) {
    goto failed;
  }
  J = GSBPP - 2;
  while (J >= 0) {
    FXCODEC_STATUS status = pGRD->Start_decode_Arith(
        &GSPLANES[J], pArithDecoder, gbContext, nullptr);
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
  FXCODEC_STATUS status =
      pGRD->Start_decode_MMR(&GSPLANES[GSBPP - 1], pStream, nullptr);
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
    FXCODEC_STATUS status =
        pGRD->Start_decode_MMR(&GSPLANES[J], pStream, nullptr);
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
