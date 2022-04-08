// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fpdfxfa/cpdfxfa_widget.h"

#include "fpdfsdk/ipdfsdk_annothandler.h"
#include "third_party/base/check.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_ffwidgethandler.h"
#include "xfa/fxfa/parser/cxfa_node.h"

namespace {

Mask<XFA_FWL_KeyFlag> GetKeyFlags(Mask<FWL_EVENTFLAG> input) {
  Mask<XFA_FWL_KeyFlag> results;

  if (input & FWL_EVENTFLAG_ControlKey)
    results |= XFA_FWL_KeyFlag::kCtrl;
  if (input & FWL_EVENTFLAG_LeftButtonDown)
    results |= XFA_FWL_KeyFlag::kLButton;
  if (input & FWL_EVENTFLAG_MiddleButtonDown)
    results |= XFA_FWL_KeyFlag::kMButton;
  if (input & FWL_EVENTFLAG_RightButtonDown)
    results |= XFA_FWL_KeyFlag::kRButton;
  if (input & FWL_EVENTFLAG_ShiftKey)
    results |= XFA_FWL_KeyFlag::kShift;
  if (input & FWL_EVENTFLAG_AltKey)
    results |= XFA_FWL_KeyFlag::kAlt;

  return results;
}

}  // namespace

CPDFXFA_Widget::CPDFXFA_Widget(CXFA_FFWidget* pXFAFFWidget,
                               CPDFSDK_PageView* pPageView)
    : CPDFSDK_Annot(pPageView), m_pXFAFFWidget(pXFAFFWidget) {}

CPDFXFA_Widget::~CPDFXFA_Widget() = default;

CPDFXFA_Widget* CPDFXFA_Widget::AsXFAWidget() {
  return this;
}

CPDFSDK_Annot::UnsafeInputHandlers* CPDFXFA_Widget::GetUnsafeInputHandlers() {
  return this;
}

CPDF_Annot::Subtype CPDFXFA_Widget::GetAnnotSubtype() const {
  return CPDF_Annot::Subtype::XFAWIDGET;
}

CFX_FloatRect CPDFXFA_Widget::GetRect() const {
  return GetXFAFFWidget()->GetLayoutItem()->GetAbsoluteRect().ToFloatRect();
}

bool CPDFXFA_Widget::DoHitTest(const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  if (!widget_handler)
    return false;

  return widget_handler->HitTest(GetXFAFFWidget(), point) !=
         FWL_WidgetHit::Unknown;
}

bool CPDFXFA_Widget::OnChangedFocus() {
  CXFA_FFDocView* doc_view = GetDocView();
  if (!doc_view)
    return false;

  CXFA_FFWidget* widget = GetXFAFFWidget();
  if (doc_view->SetFocus(widget))
    return false;

  return doc_view->GetFocusWidget() != widget;
}

CFX_FloatRect CPDFXFA_Widget::GetViewBBox() {
  CXFA_FFWidget* widget = GetXFAFFWidget();
  CXFA_Node* node = widget->GetNode();
  DCHECK(node->IsWidgetReady());

  CFX_RectF bbox =
      widget->GetBBox(node->GetFFWidgetType() == XFA_FFWidgetType::kSignature
                          ? CXFA_FFWidget::kDrawFocus
                          : CXFA_FFWidget::kDoNotDrawFocus);

  CFX_FloatRect result = bbox.ToFloatRect();
  result.Inflate(1.0f, 1.0f);
  return result;
}

void CPDFXFA_Widget::OnMouseEnter(Mask<FWL_EVENTFLAG> nFlags) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  if (widget_handler)
    widget_handler->OnMouseEnter(GetXFAFFWidget());
}

void CPDFXFA_Widget::OnMouseExit(Mask<FWL_EVENTFLAG> nFlags) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  if (widget_handler)
    widget_handler->OnMouseExit(GetXFAFFWidget());
}

bool CPDFXFA_Widget::OnLButtonDown(Mask<FWL_EVENTFLAG> nFlags,
                                   const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnLButtonDown(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::OnLButtonUp(Mask<FWL_EVENTFLAG> nFlags,
                                 const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnLButtonUp(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::OnLButtonDblClk(Mask<FWL_EVENTFLAG> nFlags,
                                     const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnLButtonDblClk(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::OnMouseMove(Mask<FWL_EVENTFLAG> nFlags,
                                 const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnMouseMove(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::OnMouseWheel(Mask<FWL_EVENTFLAG> nFlags,
                                  const CFX_PointF& point,
                                  const CFX_Vector& delta) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler &&
         widget_handler->OnMouseWheel(GetXFAFFWidget(), GetKeyFlags(nFlags),
                                      point, delta);
}

bool CPDFXFA_Widget::OnRButtonDown(Mask<FWL_EVENTFLAG> nFlags,
                                   const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnRButtonDown(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::OnRButtonUp(Mask<FWL_EVENTFLAG> nFlags,
                                 const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnRButtonUp(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::CanUndo() {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->CanUndo(GetXFAFFWidget());
}

bool CPDFXFA_Widget::CanRedo() {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->CanRedo(GetXFAFFWidget());
}

bool CPDFXFA_Widget::Undo() {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->Undo(GetXFAFFWidget());
}

bool CPDFXFA_Widget::Redo() {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->Redo(GetXFAFFWidget());
}

CXFA_FFDocView* CPDFXFA_Widget::GetDocView() {
  CXFA_FFPageView* page_view = GetXFAFFWidget()->GetPageView();
  return page_view ? page_view->GetDocView() : nullptr;
}

CXFA_FFWidgetHandler* CPDFXFA_Widget::GetWidgetHandler() {
  CXFA_FFDocView* doc_view = GetDocView();
  return doc_view ? doc_view->GetWidgetHandler() : nullptr;
}
