// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FPDFXFA_CPDFXFA_WIDGET_H_
#define FPDFSDK_FPDFXFA_CPDFXFA_WIDGET_H_

#include "core/fxcrt/fx_coordinates.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "v8/include/cppgc/persistent.h"
#include "xfa/fxfa/cxfa_ffwidget.h"

class CPDFSDK_PageView;
class CXFA_FFDocView;
class CXFA_FFWidgetHandler;

class CPDFXFA_Widget final : public CPDFSDK_Annot,
                             CPDFSDK_Annot::UnsafeInputHandlers {
 public:
  CPDFXFA_Widget(CXFA_FFWidget* pXFAFFWidget, CPDFSDK_PageView* pPageView);
  ~CPDFXFA_Widget() override;

  // CPDFSDK_Annot:
  CPDFXFA_Widget* AsXFAWidget() override;
  CPDFSDK_Annot::UnsafeInputHandlers* GetUnsafeInputHandlers() override;
  CPDF_Annot::Subtype GetAnnotSubtype() const override;
  CFX_FloatRect GetRect() const override;
  void OnDraw(CFX_RenderDevice* pDevice,
              const CFX_Matrix& mtUser2Device,
              bool bDrawAnnots) override;
  bool DoHitTest(const CFX_PointF& point) override;
  CFX_FloatRect GetViewBBox() override;
  bool CanUndo() override;
  bool CanRedo() override;
  bool Undo() override;
  bool Redo() override;
  WideString GetText() override;
  WideString GetSelectedText() override;
  void ReplaceAndKeepSelection(const WideString& text) override;
  void ReplaceSelection(const WideString& text) override;
  bool SelectAllText() override;
  bool SetIndexSelected(int index, bool selected) override;
  bool IsIndexSelected(int index) override;

  CXFA_FFWidget* GetXFAFFWidget() const { return m_pXFAFFWidget.Get(); }

  bool OnChangedFocus();

 private:
  // CPDFSDK_Annot::UnsafeInputHandlers:
  void OnMouseEnter(Mask<FWL_EVENTFLAG> nFlags) override;
  void OnMouseExit(Mask<FWL_EVENTFLAG> nFlags) override;
  bool OnLButtonDown(Mask<FWL_EVENTFLAG> nFlags,
                     const CFX_PointF& point) override;
  bool OnLButtonUp(Mask<FWL_EVENTFLAG> nFlags,
                   const CFX_PointF& point) override;
  bool OnLButtonDblClk(Mask<FWL_EVENTFLAG> nFlags,
                       const CFX_PointF& point) override;
  bool OnMouseMove(Mask<FWL_EVENTFLAG> nFlags,
                   const CFX_PointF& point) override;
  bool OnMouseWheel(Mask<FWL_EVENTFLAG> nFlags,
                    const CFX_PointF& point,
                    const CFX_Vector& delta) override;
  bool OnRButtonDown(Mask<FWL_EVENTFLAG> nFlags,
                     const CFX_PointF& point) override;
  bool OnRButtonUp(Mask<FWL_EVENTFLAG> nFlags,
                   const CFX_PointF& point) override;
  bool OnChar(uint32_t nChar, Mask<FWL_EVENTFLAG> nFlags) override;
  bool OnKeyDown(FWL_VKEYCODE nKeyCode, Mask<FWL_EVENTFLAG> nFlags) override;
  bool OnSetFocus(Mask<FWL_EVENTFLAG> nFlags) override;
  bool OnKillFocus(Mask<FWL_EVENTFLAG> nFlags) override;

  CXFA_FFDocView* GetDocView();
  CXFA_FFWidgetHandler* GetWidgetHandler();

  cppgc::Persistent<CXFA_FFWidget> const m_pXFAFFWidget;
};

#endif  // FPDFSDK_FPDFXFA_CPDFXFA_WIDGET_H_
