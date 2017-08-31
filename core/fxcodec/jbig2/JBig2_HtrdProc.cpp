// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jbig2/JBig2_HtrdProc.h"

#include <algorithm>
#include <utility>

#include "core/fxcodec/jbig2/JBig2_BitStream.h"
#include "core/fxcodec/jbig2/JBig2_GrdProc.h"
#include "core/fxcodec/jbig2/JBig2_Image.h"
#include "third_party/base/ptr_util.h"

std::unique_ptr<CJBig2_Image> CJBig2_HTRDProc::decode_Arith(
    CJBig2_ArithDecoder* pArithDecoder,
    JBig2ArithCtx* gbContext,
    IFX_PauseIndicator* pPause) {
  std::unique_ptr<CJBig2_Image> HSKIP;
  if (HENABLESKIP == 1) {
    HSKIP = pdfium::MakeUnique<CJBig2_Image>(HGW, HGH);
    for (uint32_t mg = 0; mg < HGH; ++mg) {
      for (uint32_t ng = 0; ng < HGW; ++ng) {
        int32_t x = (HGX + mg * HRY + ng * HRX) >> 8;
        int32_t y = (HGY + mg * HRX - ng * HRY) >> 8;
        if ((x + HPW <= 0) | (x >= static_cast<int32_t>(HBW)) | (y + HPH <= 0) |
            (y >= static_cast<int32_t>(HPH))) {
          HSKIP->setPixel(ng, mg, 1);
        } else {
          HSKIP->setPixel(ng, mg, 0);
        }
      }
    }
  }
  uint32_t HBPP = 1;
  while (static_cast<uint32_t>(1 << HBPP) < HNUMPATS)
    ++HBPP;

  CJBig2_GRDProc GRD;
  GRD.MMR = HMMR;
  GRD.GBW = HGW;
  GRD.GBH = HGH;
  GRD.GBTEMPLATE = HTEMPLATE;
  GRD.TPGDON = 0;
  GRD.USESKIP = HENABLESKIP;
  GRD.SKIP = HSKIP.get();
  if (HTEMPLATE <= 1)
    GRD.GBAT[0] = 3;
  else
    GRD.GBAT[0] = 2;
  GRD.GBAT[1] = -1;
  if (GRD.GBTEMPLATE == 0) {
    GRD.GBAT[2] = -3;
    GRD.GBAT[3] = -1;
    GRD.GBAT[4] = 2;
    GRD.GBAT[5] = -2;
    GRD.GBAT[6] = -2;
    GRD.GBAT[7] = -2;
  }

  uint8_t GSBPP = static_cast<uint8_t>(HBPP);
  std::vector<std::unique_ptr<CJBig2_Image>> GSPLANES(GSBPP);
  for (int32_t i = GSBPP - 1; i >= 0; --i) {
    std::unique_ptr<CJBig2_Image> pImage;
    FXCODEC_STATUS status =
        GRD.Start_decode_Arith(&pImage, pArithDecoder, gbContext, nullptr);
    while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE)
      status = GRD.Continue_decode(pPause, pArithDecoder);

    if (!pImage)
      return nullptr;

    GSPLANES[i] = std::move(pImage);
    if (i < GSBPP - 1)
      GSPLANES[i]->composeFrom(0, 0, GSPLANES[i + 1].get(), JBIG2_COMPOSE_XOR);
  }
  return decode_image(GSPLANES);
}

std::unique_ptr<CJBig2_Image> CJBig2_HTRDProc::decode_MMR(
    CJBig2_BitStream* pStream) {
  uint32_t HBPP = 1;
  while (static_cast<uint32_t>(1 << HBPP) < HNUMPATS)
    ++HBPP;

  CJBig2_GRDProc GRD;
  GRD.MMR = HMMR;
  GRD.GBW = HGW;
  GRD.GBH = HGH;

  uint8_t GSBPP = static_cast<uint8_t>(HBPP);
  std::vector<std::unique_ptr<CJBig2_Image>> GSPLANES(GSBPP);
  GRD.Start_decode_MMR(&GSPLANES[GSBPP - 1], pStream);
  if (!GSPLANES[GSBPP - 1])
    return nullptr;

  pStream->alignByte();
  pStream->offset(3);
  for (int32_t J = GSBPP - 2; J >= 0; --J) {
    GRD.Start_decode_MMR(&GSPLANES[J], pStream);
    if (!GSPLANES[J])
      return nullptr;

    pStream->alignByte();
    pStream->offset(3);
    GSPLANES[J]->composeFrom(0, 0, GSPLANES[J + 1].get(), JBIG2_COMPOSE_XOR);
  }
  return decode_image(GSPLANES);
}

std::unique_ptr<CJBig2_Image> CJBig2_HTRDProc::decode_image(
    const std::vector<std::unique_ptr<CJBig2_Image>>& GSPLANES) {
  auto HTREG = pdfium::MakeUnique<CJBig2_Image>(HBW, HBH);
  HTREG->fill(HDEFPIXEL);
  std::vector<uint32_t> GSVALS(HGW * HGH);
  for (uint32_t y = 0; y < HGH; ++y) {
    for (uint32_t x = 0; x < HGW; ++x) {
      for (uint8_t J = 0; J < GSPLANES.size(); ++J)
        GSVALS[y * HGW + x] |= GSPLANES[J]->getPixel(x, y) << J;
    }
  }
  for (uint32_t mg = 0; mg < HGH; ++mg) {
    for (uint32_t ng = 0; ng < HGW; ++ng) {
      int32_t x = (HGX + mg * HRY + ng * HRX) >> 8;
      int32_t y = (HGY + mg * HRX - ng * HRY) >> 8;
      uint32_t pat_index = std::min(GSVALS[mg * HGW + ng], HNUMPATS - 1);
      HTREG->composeFrom(x, y, (*HPATS)[pat_index].get(), HCOMBOP);
    }
  }
  return HTREG;
}
