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

CPDFXFA_Widget::CPDFXFA_Widget(CXFA_FFWidget* pXFAFFWidget,
                               CPDFSDK_PageView* pPageView)
    : CPDFSDK_Annot(pPageView), m_pXFAFFWidget(pXFAFFWidget) {}

CPDFXFA_Widget::~CPDFXFA_Widget() = default;

CPDFXFA_Widget* CPDFXFA_Widget::AsXFAWidget() {
  return this;
}

CPDF_Annot::Subtype CPDFXFA_Widget::GetAnnotSubtype() const {
  return CPDF_Annot::Subtype::XFAWIDGET;
}

CFX_FloatRect CPDFXFA_Widget::GetRect() const {
  return GetXFAFFWidget()->GetLayoutItem()->GetAbsoluteRect().ToFloatRect();
}

bool CPDFXFA_Widget::DoHitTest(const CFX_PointF& point) {
  CXFA_FFWidget* widget = GetXFAFFWidget();
  CXFA_FFPageView* page_view = widget->GetPageView();
  if (!page_view)
    return false;

  CXFA_FFDocView* doc_view = page_view->GetDocView();
  if (!doc_view)
    return false;

  CXFA_FFWidgetHandler* widget_handler = doc_view->GetWidgetHandler();
  if (!widget_handler)
    return false;

  return widget_handler->HitTest(widget, point) != FWL_WidgetHit::Unknown;
}

bool CPDFXFA_Widget::OnChangedFocus() {
  CXFA_FFWidget* widget = GetXFAFFWidget();
  CXFA_FFPageView* page_view = widget->GetPageView();
  if (!page_view)
    return false;

  CXFA_FFDocView* doc_view = page_view->GetDocView();
  if (!doc_view)
    return false;

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
