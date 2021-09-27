// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FORMFILLER_CFFL_INTERACTIVEFORMFILLER_H_
#define FPDFSDK_FORMFILLER_CFFL_INTERACTIVEFORMFILLER_H_

#include <map>
#include <memory>
#include <utility>

#include "core/fxcrt/mask.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/pwl/ipwl_fillernotify.h"
#include "fpdfsdk/pwl/ipwl_systemhandler.h"
#include "public/fpdf_fwlevent.h"

class CFFL_FormField;
class CPDFSDK_PageView;
class CPDFSDK_Widget;

class CFFL_InteractiveFormFiller final : public IPWL_FillerNotify {
 public:
  class CallbackIface {
   public:
    virtual ~CallbackIface() = default;

    virtual void OnSetFieldInputFocus(const WideString& text) = 0;
    virtual void OnCalculate(ObservedPtr<CPDFSDK_Annot>& pAnnot) = 0;
    virtual void OnFormat(ObservedPtr<CPDFSDK_Annot>& pAnnot) = 0;
    virtual void Invalidate(IPDF_Page* pPage, const FX_RECT& rect) = 0;
    virtual CPDFSDK_PageView* GetOrCreatePageView(IPDF_Page* pPage) = 0;
    virtual CPDFSDK_PageView* GetPageView(IPDF_Page* pPage) = 0;
    virtual CFX_Timer::HandlerIface* GetTimerHandler() = 0;
    virtual IPWL_SystemHandler* GetSysHandler() = 0;
    virtual CPDFSDK_Annot* GetFocusAnnot() const = 0;
    virtual bool SetFocusAnnot(ObservedPtr<CPDFSDK_Annot>& pAnnot) = 0;

    // See PDF Reference 1.7, table 3.20 for the permission bits. Returns true
    // if any bit in |flags| is set.
    virtual bool HasPermissions(uint32_t flags) const = 0;
    virtual void OnChange() = 0;
  };

  explicit CFFL_InteractiveFormFiller(CallbackIface* pCallbackIface);
  ~CFFL_InteractiveFormFiller() override;

  CallbackIface* GetCallbackIface() { return m_pCallbackIface.Get(); }
  bool Annot_HitTest(const CPDFSDK_Widget* pWidget, const CFX_PointF& point);
  FX_RECT GetViewBBox(const CPDFSDK_PageView* pPageView,
                      CPDFSDK_Widget* pWidget);

  void OnDraw(CPDFSDK_PageView* pPageView,
              CPDFSDK_Widget* pWidget,
              CFX_RenderDevice* pDevice,
              const CFX_Matrix& mtUser2Device);
  void OnDelete(CPDFSDK_Widget* pWidget);

  void OnMouseEnter(CPDFSDK_PageView* pPageView,
                    ObservedPtr<CPDFSDK_Widget>& pWidget,
                    Mask<FWL_EVENTFLAG> nFlag);
  void OnMouseExit(CPDFSDK_PageView* pPageView,
                   ObservedPtr<CPDFSDK_Widget>& pWidget,
                   Mask<FWL_EVENTFLAG> nFlag);
  bool OnLButtonDown(CPDFSDK_PageView* pPageView,
                     ObservedPtr<CPDFSDK_Widget>& pWidget,
                     Mask<FWL_EVENTFLAG> nFlags,
                     const CFX_PointF& point);
  bool OnLButtonUp(CPDFSDK_PageView* pPageView,
                   ObservedPtr<CPDFSDK_Widget>& pWidget,
                   Mask<FWL_EVENTFLAG> nFlags,
                   const CFX_PointF& point);
  bool OnLButtonDblClk(CPDFSDK_PageView* pPageView,
                       ObservedPtr<CPDFSDK_Widget>& pWidget,
                       Mask<FWL_EVENTFLAG> nFlags,
                       const CFX_PointF& point);
  bool OnMouseMove(CPDFSDK_PageView* pPageView,
                   ObservedPtr<CPDFSDK_Widget>& pWidget,
                   Mask<FWL_EVENTFLAG> nFlags,
                   const CFX_PointF& point);
  bool OnMouseWheel(CPDFSDK_PageView* pPageView,
                    ObservedPtr<CPDFSDK_Widget>& pWidget,
                    Mask<FWL_EVENTFLAG> nFlags,
                    const CFX_PointF& point,
                    const CFX_Vector& delta);
  bool OnRButtonDown(CPDFSDK_PageView* pPageView,
                     ObservedPtr<CPDFSDK_Widget>& pWidget,
                     Mask<FWL_EVENTFLAG> nFlags,
                     const CFX_PointF& point);
  bool OnRButtonUp(CPDFSDK_PageView* pPageView,
                   ObservedPtr<CPDFSDK_Widget>& pWidget,
                   Mask<FWL_EVENTFLAG> nFlags,
                   const CFX_PointF& point);

  bool OnKeyDown(CPDFSDK_Widget* pWidget,
                 FWL_VKEYCODE nKeyCode,
                 Mask<FWL_EVENTFLAG> nFlags);
  bool OnChar(CPDFSDK_Widget* pWidget,
              uint32_t nChar,
              Mask<FWL_EVENTFLAG> nFlags);

  bool OnSetFocus(ObservedPtr<CPDFSDK_Widget>& pWidget,
                  Mask<FWL_EVENTFLAG> nFlag);
  bool OnKillFocus(ObservedPtr<CPDFSDK_Widget>& pWidget,
                   Mask<FWL_EVENTFLAG> nFlag);

  CFFL_FormField* GetFormFieldForTesting(CPDFSDK_Widget* pAnnot) {
    return GetFormField(pAnnot);
  }

  WideString GetText(CPDFSDK_Widget* pWidget);
  WideString GetSelectedText(CPDFSDK_Widget* pWidget);
  void ReplaceSelection(CPDFSDK_Widget* pWidget, const WideString& text);
  bool SelectAllText(CPDFSDK_Widget* pWidget);

  bool CanUndo(CPDFSDK_Widget* pWidget);
  bool CanRedo(CPDFSDK_Widget* pWidget);
  bool Undo(CPDFSDK_Widget* pWidget);
  bool Redo(CPDFSDK_Widget* pWidget);

  static bool IsVisible(CPDFSDK_Widget* pWidget);
  static bool IsReadOnly(CPDFSDK_Widget* pWidget);
  static bool IsValidAnnot(const CPDFSDK_PageView* pPageView,
                           CPDFSDK_Widget* pWidget);

  bool OnKeyStrokeCommit(ObservedPtr<CPDFSDK_Widget>& pWidget,
                         const CPDFSDK_PageView* pPageView,
                         Mask<FWL_EVENTFLAG> nFlag);
  bool OnValidate(ObservedPtr<CPDFSDK_Widget>& pWidget,
                  const CPDFSDK_PageView* pPageView,
                  Mask<FWL_EVENTFLAG> nFlag);
  void OnCalculate(ObservedPtr<CPDFSDK_Widget>& pWidget);
  void OnFormat(ObservedPtr<CPDFSDK_Widget>& pWidget);
  bool OnButtonUp(ObservedPtr<CPDFSDK_Widget>& pWidget,
                  const CPDFSDK_PageView* pPageView,
                  Mask<FWL_EVENTFLAG> nFlag);

  bool SetIndexSelected(ObservedPtr<CPDFSDK_Widget>& pWidget,
                        int index,
                        bool selected);
  bool IsIndexSelected(ObservedPtr<CPDFSDK_Widget>& pWidget, int index);

 private:
  using WidgetToFormFillerMap =
      std::map<CPDFSDK_Widget*, std::unique_ptr<CFFL_FormField>>;

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
      Mask<FWL_EVENTFLAG> nFlag) override;
  bool OnPopupPreOpen(const IPWL_SystemHandler::PerWindowData* pAttached,
                      Mask<FWL_EVENTFLAG> nFlag) override;
  bool OnPopupPostOpen(const IPWL_SystemHandler::PerWindowData* pAttached,
                       Mask<FWL_EVENTFLAG> nFlag) override;

#ifdef PDF_ENABLE_XFA
  void SetFocusAnnotTab(CPDFSDK_Widget* pWidget, bool bSameField, bool bNext);
  bool OnClick(ObservedPtr<CPDFSDK_Widget>& pWidget,
               const CPDFSDK_PageView* pPageView,
               Mask<FWL_EVENTFLAG> nFlag);
  bool OnFull(ObservedPtr<CPDFSDK_Widget>& pAnnot,
              const CPDFSDK_PageView* pPageView,
              Mask<FWL_EVENTFLAG> nFlag);
  bool OnPreOpen(ObservedPtr<CPDFSDK_Widget>& pWidget,
                 const CPDFSDK_PageView* pPageView,
                 Mask<FWL_EVENTFLAG> nFlag);
  bool OnPostOpen(ObservedPtr<CPDFSDK_Widget>& pWidget,
                  const CPDFSDK_PageView* pPageView,
                  Mask<FWL_EVENTFLAG> nFlag);
#endif  // PDF_ENABLE_XFA

  bool IsFillingAllowed(CPDFSDK_Widget* pWidget) const;
  CFFL_FormField* GetFormField(CPDFSDK_Widget* pAnnot);
  CFFL_FormField* GetOrCreateFormField(CPDFSDK_Widget* pWidget);
  void UnregisterFormField(CPDFSDK_Widget* pWidget);

  UnownedPtr<CallbackIface> const m_pCallbackIface;
  WidgetToFormFillerMap m_Map;
  bool m_bNotifying = false;
};

#endif  // FPDFSDK_FORMFILLER_CFFL_INTERACTIVEFORMFILLER_H_
