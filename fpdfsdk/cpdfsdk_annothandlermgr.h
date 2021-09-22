// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_ANNOTHANDLERMGR_H_
#define FPDFSDK_CPDFSDK_ANNOTHANDLERMGR_H_

#include <memory>

#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/mask.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "public/fpdf_fwlevent.h"

class CFX_Matrix;
class CFX_RenderDevice;
class CPDFSDK_FormFillEnvironment;
class CPDFSDK_PageView;
class IPDFSDK_AnnotHandler;

#ifdef PDF_ENABLE_XFA
class CXFA_FFWidget;
#endif  // PDF_ENABLE_XFA

class CPDFSDK_AnnotHandlerMgr {
 public:
  CPDFSDK_AnnotHandlerMgr(
      std::unique_ptr<IPDFSDK_AnnotHandler> pBAAnnotHandler,
      std::unique_ptr<IPDFSDK_AnnotHandler> pWidgetHandler,
      std::unique_ptr<IPDFSDK_AnnotHandler> pXFAWidgetHandler);

  ~CPDFSDK_AnnotHandlerMgr();

  void SetFormFillEnv(CPDFSDK_FormFillEnvironment* pFormFillEnv);

  std::unique_ptr<CPDFSDK_Annot> NewAnnot(CPDF_Annot* pAnnot,
                                          CPDFSDK_PageView* pPageView);
#ifdef PDF_ENABLE_XFA
  std::unique_ptr<CPDFSDK_Annot> NewAnnotForXFA(CXFA_FFWidget* pFFWidget,
                                                CPDFSDK_PageView* pPageView);
#endif  // PDF_ENABLE_XFA
  void ReleaseAnnot(std::unique_ptr<CPDFSDK_Annot> pAnnot);

  void Annot_OnLoad(CPDFSDK_Annot* pAnnot);

  WideString Annot_GetText(CPDFSDK_Annot* pAnnot);
  WideString Annot_GetSelectedText(CPDFSDK_Annot* pAnnot);
  void Annot_ReplaceSelection(CPDFSDK_Annot* pAnnot, const WideString& text);
  bool Annot_SelectAllText(CPDFSDK_Annot* pAnnot);

  bool Annot_CanUndo(CPDFSDK_Annot* pAnnot);
  bool Annot_CanRedo(CPDFSDK_Annot* pAnnot);
  bool Annot_Undo(CPDFSDK_Annot* pAnnot);
  bool Annot_Redo(CPDFSDK_Annot* pAnnot);

  void Annot_OnDraw(CPDFSDK_Annot* pAnnot,
                    CFX_RenderDevice* pDevice,
                    const CFX_Matrix& mtUser2Device,
                    bool bDrawAnnots);

  void Annot_OnMouseEnter(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                          Mask<FWL_EVENTFLAG> nFlags);
  void Annot_OnMouseExit(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                         Mask<FWL_EVENTFLAG> nFlags);
  bool Annot_OnLButtonDown(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                           Mask<FWL_EVENTFLAG> nFlags,
                           const CFX_PointF& point);
  bool Annot_OnLButtonUp(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                         Mask<FWL_EVENTFLAG> nFlags,
                         const CFX_PointF& point);
  bool Annot_OnLButtonDblClk(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                             Mask<FWL_EVENTFLAG> nFlags,
                             const CFX_PointF& point);
  bool Annot_OnMouseMove(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                         Mask<FWL_EVENTFLAG> nFlags,
                         const CFX_PointF& point);
  bool Annot_OnMouseWheel(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                          Mask<FWL_EVENTFLAG> nFlags,
                          const CFX_PointF& point,
                          const CFX_Vector& delta);
  bool Annot_OnRButtonDown(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                           Mask<FWL_EVENTFLAG> nFlags,
                           const CFX_PointF& point);
  bool Annot_OnRButtonUp(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                         Mask<FWL_EVENTFLAG> nFlags,
                         const CFX_PointF& point);
  bool Annot_OnChar(CPDFSDK_Annot* pAnnot,
                    uint32_t nChar,
                    Mask<FWL_EVENTFLAG> nFlags);
  bool Annot_OnKeyDown(CPDFSDK_Annot* pAnnot,
                       FWL_VKEYCODE nKeyCode,
                       Mask<FWL_EVENTFLAG> nFlag);
  bool Annot_OnSetFocus(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                        Mask<FWL_EVENTFLAG> nFlag);
  bool Annot_OnKillFocus(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                         Mask<FWL_EVENTFLAG> nFlag);
  bool Annot_SetIndexSelected(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                              int index,
                              bool selected);
  bool Annot_IsIndexSelected(ObservedPtr<CPDFSDK_Annot>& pAnnot, int index);

#ifdef PDF_ENABLE_XFA
  bool Annot_OnChangeFocus(ObservedPtr<CPDFSDK_Annot>& pSetAnnot);
#endif  // PDF_ENABLE_XFA

  CFX_FloatRect Annot_OnGetViewBBox(CPDFSDK_Annot* pAnnot);
  bool Annot_OnHitTest(CPDFSDK_Annot* pAnnot, const CFX_PointF& point);

 private:
  friend class CPDFSDK_BAAnnotHandlerTest;

  IPDFSDK_AnnotHandler* GetAnnotHandler(CPDFSDK_Annot* pAnnot) const;
  IPDFSDK_AnnotHandler* GetAnnotHandlerOfType(
      CPDF_Annot::Subtype nAnnotSubtype) const;

  // |m_pBAAnnotHandler| and |m_pWidgetHandler| are always present, but
  // |m_pXFAWidgetHandler| is only present in XFA mode.
  std::unique_ptr<IPDFSDK_AnnotHandler> const m_pBAAnnotHandler;
  std::unique_ptr<IPDFSDK_AnnotHandler> const m_pWidgetHandler;
  std::unique_ptr<IPDFSDK_AnnotHandler> const m_pXFAWidgetHandler;
};

#endif  // FPDFSDK_CPDFSDK_ANNOTHANDLERMGR_H_
