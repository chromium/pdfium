// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_datetimepicker.h"

#include <memory>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_datetimepicker.h"
#include "xfa/fwl/core/ifwl_widget.h"

namespace {

IFWL_DateTimePicker* ToDateTimePicker(IFWL_Widget* widget) {
  return static_cast<IFWL_DateTimePicker*>(widget);
}

}  // namespace

CFWL_DateTimePicker::CFWL_DateTimePicker(const CFWL_App* app)
    : CFWL_Widget(app) {}

CFWL_DateTimePicker::~CFWL_DateTimePicker() {}

void CFWL_DateTimePicker::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_DateTimePicker>(
      m_pApp, pdfium::MakeUnique<CFWL_WidgetProperties>());

  CFWL_Widget::Initialize();
}

int32_t CFWL_DateTimePicker::CountSelRanges() {
  return ToDateTimePicker(GetWidget())->CountSelRanges();
}

int32_t CFWL_DateTimePicker::GetSelRange(int32_t nIndex, int32_t& nStart) {
  return ToDateTimePicker(GetWidget())->GetSelRange(nIndex, nStart);
}

void CFWL_DateTimePicker::GetEditText(CFX_WideString& wsText) {
  ToDateTimePicker(GetWidget())->GetEditText(wsText);
}

void CFWL_DateTimePicker::SetEditText(const CFX_WideString& wsText) {
  ToDateTimePicker(GetWidget())->SetEditText(wsText);
}

void CFWL_DateTimePicker::SetCurSel(int32_t iYear,
                                    int32_t iMonth,
                                    int32_t iDay) {
  ToDateTimePicker(GetWidget())->SetCurSel(iYear, iMonth, iDay);
}

void CFWL_DateTimePicker::GetBBox(CFX_RectF& rect) {
  ToDateTimePicker(GetWidget())->GetBBox(rect);
}

void CFWL_DateTimePicker::SetEditLimit(int32_t nLimit) {
  ToDateTimePicker(GetWidget())->SetEditLimit(nLimit);
}

void CFWL_DateTimePicker::ModifyEditStylesEx(uint32_t dwStylesExAdded,
                                             uint32_t dwStylesExRemoved) {
  ToDateTimePicker(GetWidget())
      ->ModifyEditStylesEx(dwStylesExAdded, dwStylesExRemoved);
}
