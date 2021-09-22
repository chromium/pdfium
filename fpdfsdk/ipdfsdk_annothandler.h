// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_IPDFSDK_ANNOTHANDLER_H_
#define FPDFSDK_IPDFSDK_ANNOTHANDLER_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/mask.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "public/fpdf_fwlevent.h"

class CFX_Matrix;
class CFX_RenderDevice;
class CPDF_Annot;
class CPDFSDK_FormFillEnvironment;
class CPDFSDK_PageView;

#ifdef PDF_ENABLE_XFA
class CXFA_FFWidget;
#endif

class IPDFSDK_AnnotHandler {
 public:
  IPDFSDK_AnnotHandler();
  virtual ~IPDFSDK_AnnotHandler();

  void SetFormFillEnvironment(CPDFSDK_FormFillEnvironment* pFormFillEnv) {
    m_pFormFillEnv = pFormFillEnv;
  }
  CPDFSDK_FormFillEnvironment* GetFormFillEnvironment() const {
    return m_pFormFillEnv.Get();
  }

  virtual bool CanAnswer(CPDFSDK_Annot* pAnnot) = 0;
  virtual std::unique_ptr<CPDFSDK_Annot> NewAnnot(
      CPDF_Annot* pAnnot,
      CPDFSDK_PageView* pPageView) = 0;

  virtual void ReleaseAnnot(std::unique_ptr<CPDFSDK_Annot> pAnnot) = 0;
  virtual CFX_FloatRect GetViewBBox(CPDFSDK_Annot* pAnnot) = 0;
  virtual WideString GetText(CPDFSDK_Annot* pAnnot) = 0;
  virtual WideString GetSelectedText(CPDFSDK_Annot* pAnnot) = 0;
  virtual void ReplaceSelection(CPDFSDK_Annot* pAnnot,
                                const WideString& text) = 0;
  virtual bool SelectAllText(CPDFSDK_Annot* pAnnot) = 0;
  virtual bool CanUndo(CPDFSDK_Annot* pAnnot) = 0;
  virtual bool CanRedo(CPDFSDK_Annot* pAnnot) = 0;
  virtual bool Undo(CPDFSDK_Annot* pAnnot) = 0;
  virtual bool Redo(CPDFSDK_Annot* pAnnot) = 0;
  virtual bool HitTest(CPDFSDK_Annot* pAnnot, const CFX_PointF& point) = 0;
  virtual void OnDraw(CPDFSDK_Annot* pAnnot,
                      CFX_RenderDevice* pDevice,
                      const CFX_Matrix& mtUser2Device,
                      bool bDrawAnnots) = 0;
  virtual void OnLoad(CPDFSDK_Annot* pAnnot) = 0;
  virtual void OnMouseEnter(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                            Mask<FWL_EVENTFLAG> nFlag) = 0;
  virtual void OnMouseExit(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                           Mask<FWL_EVENTFLAG> nFlag) = 0;
  virtual bool OnLButtonDown(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                             Mask<FWL_EVENTFLAG> nFlags,
                             const CFX_PointF& point) = 0;
  virtual bool OnLButtonUp(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                           Mask<FWL_EVENTFLAG> nFlags,
                           const CFX_PointF& point) = 0;
  virtual bool OnLButtonDblClk(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                               Mask<FWL_EVENTFLAG> nFlags,
                               const CFX_PointF& point) = 0;
  virtual bool OnMouseMove(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                           Mask<FWL_EVENTFLAG> nFlags,
                           const CFX_PointF& point) = 0;
  virtual bool OnMouseWheel(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                            Mask<FWL_EVENTFLAG> nFlags,
                            const CFX_PointF& point,
                            const CFX_Vector& delta) = 0;
  virtual bool OnRButtonDown(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                             Mask<FWL_EVENTFLAG> nFlags,
                             const CFX_PointF& point) = 0;
  virtual bool OnRButtonUp(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                           Mask<FWL_EVENTFLAG> nFlags,
                           const CFX_PointF& point) = 0;
  virtual bool OnRButtonDblClk(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                               Mask<FWL_EVENTFLAG> nFlags,
                               const CFX_PointF& point) = 0;
  virtual bool OnChar(CPDFSDK_Annot* pAnnot,
                      uint32_t nChar,
                      Mask<FWL_EVENTFLAG> nFlags) = 0;
  virtual bool OnKeyDown(CPDFSDK_Annot* pAnnot,
                         FWL_VKEYCODE nKeyCode,
                         Mask<FWL_EVENTFLAG> nFlag) = 0;
  virtual bool OnKeyUp(CPDFSDK_Annot* pAnnot,
                       FWL_VKEYCODE nKeyCode,
                       Mask<FWL_EVENTFLAG> nFlag) = 0;
  virtual bool OnSetFocus(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                          Mask<FWL_EVENTFLAG> nFlag) = 0;
  virtual bool OnKillFocus(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                           Mask<FWL_EVENTFLAG> nFlag) = 0;
  virtual bool SetIndexSelected(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                                int index,
                                bool selected) = 0;
  virtual bool IsIndexSelected(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                               int index) = 0;

#ifdef PDF_ENABLE_XFA
  virtual std::unique_ptr<CPDFSDK_Annot> NewAnnotForXFA(
      CXFA_FFWidget* pFFWidget,
      CPDFSDK_PageView* pPageView);
  virtual bool OnXFAChangedFocus(ObservedPtr<CPDFSDK_Annot>& pNewAnnot);
#endif  // PDF_ENABLE_XFA

 private:
  UnownedPtr<CPDFSDK_FormFillEnvironment> m_pFormFillEnv;
};

#endif  // FPDFSDK_IPDFSDK_ANNOTHANDLER_H_
