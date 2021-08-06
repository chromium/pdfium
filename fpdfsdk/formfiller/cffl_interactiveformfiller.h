// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FORMFILLER_CFFL_INTERACTIVEFORMFILLER_H_
#define FPDFSDK_FORMFILLER_CFFL_INTERACTIVEFORMFILLER_H_

#include <map>
#include <memory>
#include <utility>

#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/pwl/ipwl_fillernotify.h"
#include "fpdfsdk/pwl/ipwl_systemhandler.h"
#include "public/fpdf_fwlevent.h"

class CFFL_FormField;
class CPDFSDK_FormFillEnvironment;
class CPDFSDK_PageView;
class CPDFSDK_Widget;

class CFFL_InteractiveFormFiller final : public IPWL_FillerNotify {
 public:
  explicit CFFL_InteractiveFormFiller(
      CPDFSDK_FormFillEnvironment* pFormFillEnv);
  ~CFFL_InteractiveFormFiller() override;

  bool Annot_HitTest(CPDFSDK_PageView* pPageView,
                     CPDFSDK_Annot* pAnnot,
                     const CFX_PointF& point);
  FX_RECT GetViewBBox(const CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot);
  void OnDraw(CPDFSDK_PageView* pPageView,
              CPDFSDK_Annot* pAnnot,
              CFX_RenderDevice* pDevice,
              const CFX_Matrix& mtUser2Device);

  void OnDelete(CPDFSDK_Annot* pAnnot);

  void OnMouseEnter(CPDFSDK_PageView* pPageView,
                    ObservedPtr<CPDFSDK_Annot>* pAnnot,
                    FWL_EventFlagMask nFlag);
  void OnMouseExit(CPDFSDK_PageView* pPageView,
                   ObservedPtr<CPDFSDK_Annot>* pAnnot,
                   FWL_EventFlagMask nFlag);
  bool OnLButtonDown(CPDFSDK_PageView* pPageView,
                     ObservedPtr<CPDFSDK_Annot>* pAnnot,
                     FWL_EventFlagMask nFlags,
                     const CFX_PointF& point);
  bool OnLButtonUp(CPDFSDK_PageView* pPageView,
                   ObservedPtr<CPDFSDK_Annot>* pAnnot,
                   FWL_EventFlagMask nFlags,
                   const CFX_PointF& point);
  bool OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                       ObservedPtr<CPDFSDK_Annot>* pAnnot,
                       FWL_EventFlagMask nFlags,
                       const CFX_PointF& point);
  bool OnMouseMove(CPDFSDK_PageView* pPageView,
                   ObservedPtr<CPDFSDK_Annot>* pAnnot,
                   FWL_EventFlagMask nFlags,
                   const CFX_PointF& point);
  bool OnMouseWheel(CPDFSDK_PageView* pPageView,
                    ObservedPtr<CPDFSDK_Annot>* pAnnot,
                    FWL_EventFlagMask nFlags,
                    const CFX_PointF& point,
                    const CFX_Vector& delta);
  bool OnRButtonDown(CPDFSDK_PageView* pPageView,
                     ObservedPtr<CPDFSDK_Annot>* pAnnot,
                     FWL_EventFlagMask nFlags,
                     const CFX_PointF& point);
  bool OnRButtonUp(CPDFSDK_PageView* pPageView,
                   ObservedPtr<CPDFSDK_Annot>* pAnnot,
                   FWL_EventFlagMask nFlags,
                   const CFX_PointF& point);

  bool OnKeyDown(CPDFSDK_Annot* pAnnot,
                 FWL_VKEYCODE nKeyCode,
                 FWL_EventFlagMask nFlags);
  bool OnChar(CPDFSDK_Annot* pAnnot, uint32_t nChar, FWL_EventFlagMask nFlags);

  bool OnSetFocus(ObservedPtr<CPDFSDK_Annot>* pAnnot, FWL_EventFlagMask nFlag);
  bool OnKillFocus(ObservedPtr<CPDFSDK_Annot>* pAnnot, FWL_EventFlagMask nFlag);

  CFFL_FormField* GetFormFieldForTesting(CPDFSDK_Annot* pAnnot) {
    return GetFormField(pAnnot);
  }

  WideString GetText(CPDFSDK_Annot* pAnnot);
  WideString GetSelectedText(CPDFSDK_Annot* pAnnot);
  void ReplaceSelection(CPDFSDK_Annot* pAnnot, const WideString& text);
  bool SelectAllText(CPDFSDK_Annot* pAnnot);

  bool CanUndo(CPDFSDK_Annot* pAnnot);
  bool CanRedo(CPDFSDK_Annot* pAnnot);
  bool Undo(CPDFSDK_Annot* pAnnot);
  bool Redo(CPDFSDK_Annot* pAnnot);

  static bool IsVisible(CPDFSDK_Widget* pWidget);
  static bool IsReadOnly(CPDFSDK_Widget* pWidget);
  static bool IsValidAnnot(const CPDFSDK_PageView* pPageView,
                           CPDFSDK_Annot* pAnnot);

  bool OnKeyStrokeCommit(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                         const CPDFSDK_PageView* pPageView,
                         FWL_EventFlagMask nFlag);
  bool OnValidate(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                  const CPDFSDK_PageView* pPageView,
                  FWL_EventFlagMask nFlag);
  void OnCalculate(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                   const CPDFSDK_PageView* pPageView,
                   FWL_EventFlagMask nFlag);
  void OnFormat(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                const CPDFSDK_PageView* pPageView,
                FWL_EventFlagMask nFlag);
  bool OnButtonUp(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                  const CPDFSDK_PageView* pPageView,
                  FWL_EventFlagMask nFlag);

  bool SetIndexSelected(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                        int index,
                        bool selected);
  bool IsIndexSelected(ObservedPtr<CPDFSDK_Annot>* pAnnot, int index);

 private:
  using WidgetToFormFillerMap =
      std::map<CPDFSDK_Annot*, std::unique_ptr<CFFL_FormField>>;

  // IPWL_FillerNotify:
  void QueryWherePopup(const IPWL_SystemHandler::PerWindowData* pAttached,
                       float fPopupMin,
                       float fPopupMax,
                       bool* bBottom,
                       float* fPopupRet) override;
  // Returns {bRC, bExit}.
  std::pair<bool, bool> OnBeforeKeyStroke(
      const IPWL_SystemHandler::PerWindowData* pAttached,
      WideString& strChange,
      const WideString& strChangeEx,
      int nSelStart,
      int nSelEnd,
      bool bKeyDown,
      FWL_EventFlagMask nFlag) override;
  bool OnPopupPreOpen(const IPWL_SystemHandler::PerWindowData* pAttached,
                      FWL_EventFlagMask nFlag) override;
  bool OnPopupPostOpen(const IPWL_SystemHandler::PerWindowData* pAttached,
                       FWL_EventFlagMask nFlag) override;

#ifdef PDF_ENABLE_XFA
  void SetFocusAnnotTab(CPDFSDK_Annot* pWidget, bool bSameField, bool bNext);
  bool OnClick(ObservedPtr<CPDFSDK_Annot>* pAnnot,
               const CPDFSDK_PageView* pPageView,
               FWL_EventFlagMask nFlag);
  bool OnFull(ObservedPtr<CPDFSDK_Widget>* pAnnot,
              const CPDFSDK_PageView* pPageView,
              FWL_EventFlagMask nFlag);
  bool OnPreOpen(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                 const CPDFSDK_PageView* pPageView,
                 FWL_EventFlagMask nFlag);
  bool OnPostOpen(ObservedPtr<CPDFSDK_Annot>* pAnnot,
                  const CPDFSDK_PageView* pPageView,
                  FWL_EventFlagMask nFlag);
#endif  // PDF_ENABLE_XFA

  bool IsFillingAllowed(CPDFSDK_Widget* pWidget) const;
  CFFL_FormField* GetFormField(CPDFSDK_Annot* pAnnot);
  CFFL_FormField* GetOrCreateFormField(CPDFSDK_Annot* pAnnot);
  void UnregisterFormField(CPDFSDK_Annot* pAnnot);

  UnownedPtr<CPDFSDK_FormFillEnvironment> const m_pFormFillEnv;
  WidgetToFormFillerMap m_Map;
  bool m_bNotifying = false;
};

#endif  // FPDFSDK_FORMFILLER_CFFL_INTERACTIVEFORMFILLER_H_
