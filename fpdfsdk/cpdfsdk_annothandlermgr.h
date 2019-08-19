// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_ANNOTHANDLERMGR_H_
#define FPDFSDK_CPDFSDK_ANNOTHANDLERMGR_H_

#include <memory>

#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fxcrt/fx_coordinates.h"
#include "fpdfsdk/cpdfsdk_annot.h"

class CFX_Matrix;
class CFX_RenderDevice;
class CPDFSDK_FormFillEnvironment;
class CPDFSDK_BAAnnotHandler;
class CPDFSDK_WidgetHandler;
class CPDFSDK_PageView;
class IPDFSDK_AnnotHandler;

#ifdef PDF_ENABLE_XFA
class CXFA_FFWidget;
#endif  // PDF_ENABLE_XFA

class CPDFSDK_AnnotHandlerMgr {
 public:
  CPDFSDK_AnnotHandlerMgr(
      std::unique_ptr<CPDFSDK_BAAnnotHandler> pBAAnnotHandler,
      std::unique_ptr<CPDFSDK_WidgetHandler> pWidgetHandler,
      std::unique_ptr<IPDFSDK_AnnotHandler> pXFAWidgetHandler);

  ~CPDFSDK_AnnotHandlerMgr();

  void SetFormFillEnv(CPDFSDK_FormFillEnvironment* pFormFillEnv);

  CPDFSDK_Annot* NewAnnot(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPageView);
#ifdef PDF_ENABLE_XFA
  CPDFSDK_Annot* NewXFAAnnot(CXFA_FFWidget* pAnnot,
                             CPDFSDK_PageView* pPageView);
#endif  // PDF_ENABLE_XFA
  void ReleaseAnnot(std::unique_ptr<CPDFSDK_Annot> pAnnot);

  void Annot_OnLoad(CPDFSDK_Annot* pAnnot);

  WideString Annot_GetText(CPDFSDK_Annot* pAnnot);
  WideString Annot_GetSelectedText(CPDFSDK_Annot* pAnnot);
  void Annot_ReplaceSelection(CPDFSDK_Annot* pAnnot, const WideString& text);

  bool Annot_CanUndo(CPDFSDK_Annot* pAnnot);
  bool Annot_CanRedo(CPDFSDK_Annot* pAnnot);
  bool Annot_Undo(CPDFSDK_Annot* pAnnot);
  bool Annot_Redo(CPDFSDK_Annot* pAnnot);

  void Annot_OnDraw(CPDFSDK_PageView* pPageView,
                    CPDFSDK_Annot* pAnnot,
                    CFX_RenderDevice* pDevice,
                    const CFX_Matrix& mtUser2Device,
                    bool bDrawAnnots);

  void Annot_OnMouseEnter(CPDFSDK_PageView* pPageView,
                          ObservedPtr<CPDFSDK_Annot>* pAnnot,
                          uint32_t nFlags);
  void Annot_OnMouseExit(CPDFSDK_PageView* pPageView,
                         ObservedPtr<CPDFSDK_Annot>* pAnnot,
                         uint32_t nFlags);
  bool Annot_OnLButtonDown(CPDFSDK_PageView* pPageView,
                           ObservedPtr<CPDFSDK_Annot>* pAnnot,
                           uint32_t nFlags,
                           const CFX_PointF& point);
  bool Annot_OnLButtonUp(CPDFSDK_PageView* pPageView,
                         ObservedPtr<CPDFSDK_Annot>* pAnnot,
                         uint32_t nFlags,
                         const CFX_PointF& point);
  bool Annot_OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                             ObservedPtr<CPDFSDK_Annot>* pAnnot,
                             uint32_t nFlags,
                             const CFX_PointF& point);
  bool Annot_OnMouseMove(CPDFSDK_PageView* pPageView,
                         ObservedPtr<CPDFSDK_Annot>* pAnnot,
                         uint32_t nFlags,
                         const CFX_PointF& point);
  bool Annot_OnMouseWheel(CPDFSDK_PageView* pPageView,
                          ObservedPtr<CPDFSDK_Annot>* pAnnot,
                          uint32_t nFlags,
                          short zDelta,
                          const CFX_PointF& point);
  bool Annot_OnRButtonDown(CPDFSDK_PageView* pPageView,
                           ObservedPtr<CPDFSDK_Annot>* pAnnot,
                           uint32_t nFlags,
                           const CFX_PointF& point);
  bool Annot_OnRButtonUp(CPDFSDK_PageView* pPageView,
                         ObservedPtr<CPDFSDK_Annot>* pAnnot,
                         uint32_t nFlags,
                         const CFX_PointF& point);
  bool Annot_OnChar(CPDFSDK_Annot* pAnnot, uint32_t nChar, uint32_t nFlags);
  bool Annot_OnKeyDown(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);
  bool Annot_OnSetFocus(ObservedPtr<CPDFSDK_Annot>* pAnnot, uint32_t nFlag);
  bool Annot_OnKillFocus(ObservedPtr<CPDFSDK_Annot>* pAnnot, uint32_t nFlag);
  bool Annot_SetIndexSelected(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                              int index,
                              bool selected);
  bool Annot_IsIndexSelected(ObservedPtr<CPDFSDK_Annot>* pAnnot, int index);

#ifdef PDF_ENABLE_XFA
  bool Annot_OnChangeFocus(ObservedPtr<CPDFSDK_Annot>* pSetAnnot,
                           ObservedPtr<CPDFSDK_Annot>* pKillAnnot);
#endif  // PDF_ENABLE_XFA

  CFX_FloatRect Annot_OnGetViewBBox(CPDFSDK_PageView* pPageView,
                                    CPDFSDK_Annot* pAnnot);
  bool Annot_OnHitTest(CPDFSDK_PageView* pPageView,
                       CPDFSDK_Annot* pAnnot,
                       const CFX_PointF& point);

 private:
  IPDFSDK_AnnotHandler* GetAnnotHandler(CPDFSDK_Annot* pAnnot) const;
  IPDFSDK_AnnotHandler* GetAnnotHandlerOfType(
      CPDF_Annot::Subtype nAnnotSubtype) const;
  CPDFSDK_Annot* GetNextAnnot(CPDFSDK_Annot* pSDKAnnot, bool bNext);

  // |m_pBAAnnotHandler| and |m_pWidgetHandler| are always present, but
  // |m_pXFAWidgetHandler| is only present in XFA mode.
  std::unique_ptr<CPDFSDK_BAAnnotHandler> const m_pBAAnnotHandler;
  std::unique_ptr<CPDFSDK_WidgetHandler> const m_pWidgetHandler;
  std::unique_ptr<IPDFSDK_AnnotHandler> const m_pXFAWidgetHandler;
};

#endif  // FPDFSDK_CPDFSDK_ANNOTHANDLERMGR_H_
