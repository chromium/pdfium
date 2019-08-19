// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fpdfxfa/cpdfxfa_widget.h"

#include "fpdfsdk/ipdfsdk_annothandler.h"
#include "xfa/fxfa/cxfa_ffwidget.h"

CPDFXFA_Widget::CPDFXFA_Widget(CXFA_FFWidget* pXFAFFWidget,
                               CPDFSDK_PageView* pPageView,
                               CPDFSDK_InteractiveForm* pInteractiveForm)
    : CPDFSDK_Annot(pPageView),
      m_pInteractiveForm(pInteractiveForm),
      m_pXFAFFWidget(pXFAFFWidget) {}

CPDFXFA_Widget::~CPDFXFA_Widget() = default;

CPDFXFA_Widget* CPDFXFA_Widget::AsXFAWidget() {
  return this;
}

CPDF_Annot::Subtype CPDFXFA_Widget::GetAnnotSubtype() const {
  return CPDF_Annot::Subtype::XFAWIDGET;
}

CFX_FloatRect CPDFXFA_Widget::GetRect() const {
  return GetXFAFFWidget()->GetLayoutItem()->GetRect(false).ToFloatRect();
}
