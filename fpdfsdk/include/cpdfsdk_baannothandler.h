// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_CPDFSDK_BAANNOTHANDLER_H_
#define FPDFSDK_INCLUDE_CPDFSDK_BAANNOTHANDLER_H_

#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "fpdfsdk/include/ipdfsdk_annothandler.h"

class CFFL_IFormFiller;
class CFX_Matrix;
class CFX_RenderDevice;
class CPDF_Annot;
class CPDFSDK_Environment;
class CPDFSDK_Annot;
class CPDFSDK_PageView;

#ifdef PDF_ENABLE_XFA
class CXFA_FFWidget;
#endif  // PDF_ENABLE_XFA

class CPDFSDK_BAAnnotHandler : public IPDFSDK_AnnotHandler {
 public:
  CPDFSDK_BAAnnotHandler();
  ~CPDFSDK_BAAnnotHandler() override;

  FX_BOOL CanAnswer(CPDFSDK_Annot* pAnnot) override;
  CPDFSDK_Annot* NewAnnot(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPage) override;
#ifdef PDF_ENABLE_XFA
  CPDFSDK_Annot* NewAnnot(CXFA_FFWidget* hWidget,
                          CPDFSDK_PageView* pPage) override;
#endif  // PDF_ENABLE_XFA
  void ReleaseAnnot(CPDFSDK_Annot* pAnnot) override;
  void DeleteAnnot(CPDFSDK_Annot* pAnnot) override;
  CFX_FloatRect GetViewBBox(CPDFSDK_PageView* pPageView,
                            CPDFSDK_Annot* pAnnot) override;
  FX_BOOL HitTest(CPDFSDK_PageView* pPageView,
                  CPDFSDK_Annot* pAnnot,
                  const CFX_FloatPoint& point) override;
  void OnDraw(CPDFSDK_PageView* pPageView,
              CPDFSDK_Annot* pAnnot,
              CFX_RenderDevice* pDevice,
              CFX_Matrix* pUser2Device,
              bool bDrawAnnots) override;
  void OnCreate(CPDFSDK_Annot* pAnnot) override;
  void OnLoad(CPDFSDK_Annot* pAnnot) override;
  void OnDelete(CPDFSDK_Annot* pAnnot) override;
  void OnRelease(CPDFSDK_Annot* pAnnot) override;
  void OnMouseEnter(CPDFSDK_PageView* pPageView,
                    CPDFSDK_Annot* pAnnot,
                    uint32_t nFlag) override;
  void OnMouseExit(CPDFSDK_PageView* pPageView,
                   CPDFSDK_Annot* pAnnot,
                   uint32_t nFlag) override;
  FX_BOOL OnLButtonDown(CPDFSDK_PageView* pPageView,
                        CPDFSDK_Annot* pAnnot,
                        uint32_t nFlags,
                        const CFX_FloatPoint& point) override;
  FX_BOOL OnLButtonUp(CPDFSDK_PageView* pPageView,
                      CPDFSDK_Annot* pAnnot,
                      uint32_t nFlags,
                      const CFX_FloatPoint& point) override;
  FX_BOOL OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                          CPDFSDK_Annot* pAnnot,
                          uint32_t nFlags,
                          const CFX_FloatPoint& point) override;
  FX_BOOL OnMouseMove(CPDFSDK_PageView* pPageView,
                      CPDFSDK_Annot* pAnnot,
                      uint32_t nFlags,
                      const CFX_FloatPoint& point) override;
  FX_BOOL OnMouseWheel(CPDFSDK_PageView* pPageView,
                       CPDFSDK_Annot* pAnnot,
                       uint32_t nFlags,
                       short zDelta,
                       const CFX_FloatPoint& point) override;
  FX_BOOL OnRButtonDown(CPDFSDK_PageView* pPageView,
                        CPDFSDK_Annot* pAnnot,
                        uint32_t nFlags,
                        const CFX_FloatPoint& point) override;
  FX_BOOL OnRButtonUp(CPDFSDK_PageView* pPageView,
                      CPDFSDK_Annot* pAnnot,
                      uint32_t nFlags,
                      const CFX_FloatPoint& point) override;
  FX_BOOL OnRButtonDblClk(CPDFSDK_PageView* pPageView,
                          CPDFSDK_Annot* pAnnot,
                          uint32_t nFlags,
                          const CFX_FloatPoint& point) override;
  FX_BOOL OnChar(CPDFSDK_Annot* pAnnot,
                 uint32_t nChar,
                 uint32_t nFlags) override;
  FX_BOOL OnKeyDown(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag) override;
  FX_BOOL OnKeyUp(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag) override;
  void OnDeSelected(CPDFSDK_Annot* pAnnot) override;
  void OnSelected(CPDFSDK_Annot* pAnnot) override;
  FX_BOOL OnSetFocus(CPDFSDK_Annot* pAnnot, uint32_t nFlag) override;
  FX_BOOL OnKillFocus(CPDFSDK_Annot* pAnnot, uint32_t nFlag) override;
#ifdef PDF_ENABLE_XFA
  FX_BOOL OnXFAChangedFocus(CPDFSDK_Annot* pOldAnnot,
                            CPDFSDK_Annot* pNewAnnot) override;
#endif  // PDF_ENABLE_XFA
};

#endif  // FPDFSDK_INCLUDE_CPDFSDK_BAANNOTHANDLER_H_
