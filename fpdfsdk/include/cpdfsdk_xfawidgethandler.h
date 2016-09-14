// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_CPDFSDK_XFAWIDGETHANDLER_H_
#define FPDFSDK_INCLUDE_CPDFSDK_XFAWIDGETHANDLER_H_

#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "fpdfsdk/include/ipdfsdk_annothandler.h"

class CFX_Matrix;
class CFX_RenderDevice;
class CPDF_Annot;
class CPDFSDK_Environment;
class CPDFSDK_Annot;
class CPDFSDK_PageView;
class CXFA_FFWidget;
class CXFA_FFWidgetHandler;

class CPDFSDK_XFAWidgetHandler : public IPDFSDK_AnnotHandler {
 public:
  explicit CPDFSDK_XFAWidgetHandler(CPDFSDK_Environment* pApp);
  ~CPDFSDK_XFAWidgetHandler() override;

  FX_BOOL CanAnswer(CPDFSDK_Annot* pAnnot) override;
  CPDFSDK_Annot* NewAnnot(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPage) override;
  CPDFSDK_Annot* NewAnnot(CXFA_FFWidget* pAnnot,
                          CPDFSDK_PageView* pPage) override;
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
  FX_BOOL OnXFAChangedFocus(CPDFSDK_Annot* pOldAnnot,
                            CPDFSDK_Annot* pNewAnnot) override;

 private:
  CXFA_FFWidgetHandler* GetXFAWidgetHandler(CPDFSDK_Annot* pAnnot);
  uint32_t GetFWLFlags(uint32_t dwFlag);

  CPDFSDK_Environment* m_pApp;
};

#endif  // FPDFSDK_INCLUDE_CPDFSDK_XFAWIDGETHANDLER_H_
