// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fpdfxfa/cpdfxfa_widget.h"

#include "fpdfsdk/ipdfsdk_annothandler.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"

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
