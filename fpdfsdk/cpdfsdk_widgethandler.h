// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_WIDGETHANDLER_H_
#define FPDFSDK_CPDFSDK_WIDGETHANDLER_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "fpdfsdk/ipdfsdk_annothandler.h"

class CFX_Matrix;
class CFX_RenderDevice;
class CPDF_Annot;
class CPDFSDK_Annot;
class CPDFSDK_PageView;

class CPDFSDK_WidgetHandler final : public IPDFSDK_AnnotHandler {
 public:
  CPDFSDK_WidgetHandler();
  ~CPDFSDK_WidgetHandler() override;

  // IPDFSDK_AnnotHandler:
  bool CanAnswer(CPDFSDK_Annot* pAnnot) override;
  std::unique_ptr<CPDFSDK_Annot> NewAnnot(CPDF_Annot* pAnnot,
                                          CPDFSDK_PageView* pPageView) override;

  void ReleaseAnnot(std::unique_ptr<CPDFSDK_Annot> pAnnot) override;
  CFX_FloatRect GetViewBBox(CPDFSDK_Annot* pAnnot) override;
  WideString GetText(CPDFSDK_Annot* pAnnot) override;
  WideString GetSelectedText(CPDFSDK_Annot* pAnnot) override;
  void ReplaceSelection(CPDFSDK_Annot* pAnnot, const WideString& text) override;
  bool SelectAllText(CPDFSDK_Annot* pAnnot) override;
  bool CanUndo(CPDFSDK_Annot* pAnnot) override;
  bool CanRedo(CPDFSDK_Annot* pAnnot) override;
  bool Undo(CPDFSDK_Annot* pAnnot) override;
  bool Redo(CPDFSDK_Annot* pAnnot) override;
  bool HitTest(CPDFSDK_Annot* pAnnot, const CFX_PointF& point) override;
  void OnDraw(CPDFSDK_Annot* pAnnot,
              CFX_RenderDevice* pDevice,
              const CFX_Matrix& mtUser2Device,
              bool bDrawAnnots) override;
  void OnLoad(CPDFSDK_Annot* pAnnot) override;
  void OnMouseEnter(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                    Mask<FWL_EVENTFLAG> nFlag) override;
  void OnMouseExit(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                   Mask<FWL_EVENTFLAG> nFlag) override;
  bool OnLButtonDown(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                     Mask<FWL_EVENTFLAG> nFlags,
                     const CFX_PointF& point) override;
  bool OnLButtonUp(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                   Mask<FWL_EVENTFLAG> nFlags,
                   const CFX_PointF& point) override;
  bool OnLButtonDblClk(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                       Mask<FWL_EVENTFLAG> nFlags,
                       const CFX_PointF& point) override;
  bool OnMouseMove(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                   Mask<FWL_EVENTFLAG> nFlags,
                   const CFX_PointF& point) override;
  bool OnMouseWheel(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                    Mask<FWL_EVENTFLAG> nFlags,
                    const CFX_PointF& point,
                    const CFX_Vector& delta) override;
  bool OnRButtonDown(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                     Mask<FWL_EVENTFLAG> nFlags,
                     const CFX_PointF& point) override;
  bool OnRButtonUp(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                   Mask<FWL_EVENTFLAG> nFlags,
                   const CFX_PointF& point) override;
  bool OnRButtonDblClk(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                       Mask<FWL_EVENTFLAG> nFlags,
                       const CFX_PointF& point) override;
  bool OnChar(CPDFSDK_Annot* pAnnot,
              uint32_t nChar,
              Mask<FWL_EVENTFLAG> nFlags) override;
  bool OnKeyDown(CPDFSDK_Annot* pAnnot,
                 FWL_VKEYCODE nKeyCode,
                 Mask<FWL_EVENTFLAG> nFlag) override;
  bool OnKeyUp(CPDFSDK_Annot* pAnnot,
               FWL_VKEYCODE nKeyCode,
               Mask<FWL_EVENTFLAG> nFlag) override;
  bool OnSetFocus(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                  Mask<FWL_EVENTFLAG> nFlag) override;
  bool OnKillFocus(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                   Mask<FWL_EVENTFLAG> nFlag) override;
  bool SetIndexSelected(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                        int index,
                        bool selected) override;
  bool IsIndexSelected(ObservedPtr<CPDFSDK_Annot>& pAnnot, int index) override;

#ifdef PDF_ENABLE_XFA
  std::unique_ptr<CPDFSDK_Annot> NewAnnotForXFA(
      CXFA_FFWidget* pWiget,
      CPDFSDK_PageView* pPageView) override;
  bool OnXFAChangedFocus(ObservedPtr<CPDFSDK_Annot>& pNewAnnot) override;
#endif

 private:
  bool IsFocusableAnnot(const CPDF_Annot::Subtype& annot_type) const;
};

#endif  // FPDFSDK_CPDFSDK_WIDGETHANDLER_H_
