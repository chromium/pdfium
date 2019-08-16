// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fpdfxfa/cpdfxfa_widget.h"

#include "fpdfsdk/ipdfsdk_annothandler.h"
#include "xfa/fxfa/cxfa_ffwidget.h"

CPDFXFA_Widget::CPDFXFA_Widget(CXFA_FFWidget* pAnnot,
                               CPDFSDK_PageView* pPageView,
                               CPDFSDK_InteractiveForm* pInteractiveForm)
    : CPDFSDK_Annot(pPageView),
      m_pInteractiveForm(pInteractiveForm),
      m_pXFAWidget(pAnnot) {}

CPDFXFA_Widget::~CPDFXFA_Widget() = default;

bool CPDFXFA_Widget::IsXFAField() const {
  return true;
}

CXFA_FFWidget* CPDFXFA_Widget::GetXFAWidget() const {
  return m_pXFAWidget.Get();
}

CPDF_Annot::Subtype CPDFXFA_Widget::GetAnnotSubtype() const {
  return CPDF_Annot::Subtype::XFAWIDGET;
}

CFX_FloatRect CPDFXFA_Widget::GetRect() const {
  return GetXFAWidget()->GetLayoutItem()->GetRect(false).ToFloatRect();
}
