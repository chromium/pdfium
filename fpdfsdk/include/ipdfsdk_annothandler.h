// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_IPDFSDK_ANNOTHANDLER_H_
#define FPDFSDK_INCLUDE_IPDFSDK_ANNOTHANDLER_H_

#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_coordinates.h"

class CFX_Matrix;
class CFX_RenderDevice;
class CPDF_Annot;
class CPDFSDK_Annot;
class CPDFSDK_PageView;

#ifdef PDF_ENABLE_XFA
class CXFA_FFWidget;
#endif  // PDF_ENABLE_XFA

class IPDFSDK_AnnotHandler {
 public:
  virtual ~IPDFSDK_AnnotHandler() {}

  virtual FX_BOOL CanAnswer(CPDFSDK_Annot* pAnnot) = 0;
  virtual CPDFSDK_Annot* NewAnnot(CPDF_Annot* pAnnot,
                                  CPDFSDK_PageView* pPage) = 0;

#ifdef PDF_ENABLE_XFA
  virtual CPDFSDK_Annot* NewAnnot(CXFA_FFWidget* hWidget,
                                  CPDFSDK_PageView* pPage) = 0;
#endif  // PDF_ENABLE_XFA

  virtual void ReleaseAnnot(CPDFSDK_Annot* pAnnot) = 0;
  virtual void DeleteAnnot(CPDFSDK_Annot* pAnnot) = 0;
  virtual CFX_FloatRect GetViewBBox(CPDFSDK_PageView* pPageView,
                                    CPDFSDK_Annot* pAnnot) = 0;
  virtual FX_BOOL HitTest(CPDFSDK_PageView* pPageView,
                          CPDFSDK_Annot* pAnnot,
                          const CFX_FloatPoint& point) = 0;
  virtual void OnDraw(CPDFSDK_PageView* pPageView,
                      CPDFSDK_Annot* pAnnot,
                      CFX_RenderDevice* pDevice,
                      CFX_Matrix* pUser2Device,
                      bool bDrawAnnots) = 0;
  virtual void OnCreate(CPDFSDK_Annot* pAnnot) = 0;
  virtual void OnLoad(CPDFSDK_Annot* pAnnot) = 0;
  virtual void OnDelete(CPDFSDK_Annot* pAnnot) = 0;
  virtual void OnRelease(CPDFSDK_Annot* pAnnot) = 0;
  virtual void OnMouseEnter(CPDFSDK_PageView* pPageView,
                            CPDFSDK_Annot* pAnnot,
                            uint32_t nFlag) = 0;
  virtual void OnMouseExit(CPDFSDK_PageView* pPageView,
                           CPDFSDK_Annot* pAnnot,
                           uint32_t nFlag) = 0;
  virtual FX_BOOL OnLButtonDown(CPDFSDK_PageView* pPageView,
                                CPDFSDK_Annot* pAnnot,
                                uint32_t nFlags,
                                const CFX_FloatPoint& point) = 0;
  virtual FX_BOOL OnLButtonUp(CPDFSDK_PageView* pPageView,
                              CPDFSDK_Annot* pAnnot,
                              uint32_t nFlags,
                              const CFX_FloatPoint& point) = 0;
  virtual FX_BOOL OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                                  CPDFSDK_Annot* pAnnot,
                                  uint32_t nFlags,
                                  const CFX_FloatPoint& point) = 0;
  virtual FX_BOOL OnMouseMove(CPDFSDK_PageView* pPageView,
                              CPDFSDK_Annot* pAnnot,
                              uint32_t nFlags,
                              const CFX_FloatPoint& point) = 0;
  virtual FX_BOOL OnMouseWheel(CPDFSDK_PageView* pPageView,
                               CPDFSDK_Annot* pAnnot,
                               uint32_t nFlags,
                               short zDelta,
                               const CFX_FloatPoint& point) = 0;
  virtual FX_BOOL OnRButtonDown(CPDFSDK_PageView* pPageView,
                                CPDFSDK_Annot* pAnnot,
                                uint32_t nFlags,
                                const CFX_FloatPoint& point) = 0;
  virtual FX_BOOL OnRButtonUp(CPDFSDK_PageView* pPageView,
                              CPDFSDK_Annot* pAnnot,
                              uint32_t nFlags,
                              const CFX_FloatPoint& point) = 0;
  virtual FX_BOOL OnRButtonDblClk(CPDFSDK_PageView* pPageView,
                                  CPDFSDK_Annot* pAnnot,
                                  uint32_t nFlags,
                                  const CFX_FloatPoint& point) = 0;
  virtual FX_BOOL OnChar(CPDFSDK_Annot* pAnnot,
                         uint32_t nChar,
                         uint32_t nFlags) = 0;
  virtual FX_BOOL OnKeyDown(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag) = 0;
  virtual FX_BOOL OnKeyUp(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag) = 0;
  virtual void OnDeSelected(CPDFSDK_Annot* pAnnot) = 0;
  virtual void OnSelected(CPDFSDK_Annot* pAnnot) = 0;
  virtual FX_BOOL OnSetFocus(CPDFSDK_Annot* pAnnot, uint32_t nFlag) = 0;
  virtual FX_BOOL OnKillFocus(CPDFSDK_Annot* pAnnot, uint32_t nFlag) = 0;
#ifdef PDF_ENABLE_XFA
  virtual FX_BOOL OnXFAChangedFocus(CPDFSDK_Annot* pOldAnnot,
                                    CPDFSDK_Annot* pNewAnnot) = 0;
#endif  // PDF_ENABLE_XFA
};

#endif  // FPDFSDK_INCLUDE_IPDFSDK_ANNOTHANDLER_H_
