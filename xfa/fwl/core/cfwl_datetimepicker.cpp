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

CFWL_DateTimePicker::CFWL_DateTimePicker(const IFWL_App* app)
    : CFWL_Widget(app) {}

CFWL_DateTimePicker::~CFWL_DateTimePicker() {}

void CFWL_DateTimePicker::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_DateTimePicker>(
      m_pApp, m_pProperties->MakeWidgetImpProperties(&m_DateTimePickerDP));

  CFWL_Widget::Initialize();
}

FWL_Error CFWL_DateTimePicker::SetToday(int32_t iYear,
                                        int32_t iMonth,
                                        int32_t iDay) {
  m_DateTimePickerDP.m_iYear = iYear;
  m_DateTimePickerDP.m_iMonth = iMonth;
  m_DateTimePickerDP.m_iDay = iDay;
  return FWL_Error::Succeeded;
}

int32_t CFWL_DateTimePicker::CountSelRanges() {
  return ToDateTimePicker(GetWidget())->CountSelRanges();
}

int32_t CFWL_DateTimePicker::GetSelRange(int32_t nIndex, int32_t& nStart) {
  return ToDateTimePicker(GetWidget())->GetSelRange(nIndex, nStart);
}

FWL_Error CFWL_DateTimePicker::GetEditText(CFX_WideString& wsText) {
  return ToDateTimePicker(GetWidget())->GetEditText(wsText);
}

FWL_Error CFWL_DateTimePicker::SetEditText(const CFX_WideString& wsText) {
  return ToDateTimePicker(GetWidget())->SetEditText(wsText);
}

FWL_Error CFWL_DateTimePicker::GetCurSel(int32_t& iYear,
                                         int32_t& iMonth,
                                         int32_t& iDay) {
  return ToDateTimePicker(GetWidget())->GetCurSel(iYear, iMonth, iDay);
}

FWL_Error CFWL_DateTimePicker::SetCurSel(int32_t iYear,
                                         int32_t iMonth,
                                         int32_t iDay) {
  return ToDateTimePicker(GetWidget())->SetCurSel(iYear, iMonth, iDay);
}

CFWL_DateTimePicker::CFWL_DateTimePickerDP::CFWL_DateTimePickerDP() {
  m_iYear = 2011;
  m_iMonth = 1;
  m_iDay = 1;
}

FWL_Error CFWL_DateTimePicker::CFWL_DateTimePickerDP::GetCaption(
    IFWL_Widget* pWidget,
    CFX_WideString& wsCaption) {
  wsCaption = m_wsData;
  return FWL_Error::Succeeded;
}

FWL_Error CFWL_DateTimePicker::CFWL_DateTimePickerDP::GetToday(
    IFWL_Widget* pWidget,
    int32_t& iYear,
    int32_t& iMonth,
    int32_t& iDay) {
  iYear = m_iYear;
  iMonth = m_iMonth;
  iDay = m_iDay;
  return FWL_Error::Succeeded;
}

FX_BOOL CFWL_DateTimePicker::CanUndo() {
  return ToDateTimePicker(GetWidget())->CanUndo();
}

FX_BOOL CFWL_DateTimePicker::CanRedo() {
  return ToDateTimePicker(GetWidget())->CanRedo();
}

FX_BOOL CFWL_DateTimePicker::Undo() {
  return ToDateTimePicker(GetWidget())->Undo();
}

FX_BOOL CFWL_DateTimePicker::Redo() {
  return ToDateTimePicker(GetWidget())->Redo();
}

FX_BOOL CFWL_DateTimePicker::CanCopy() {
  return ToDateTimePicker(GetWidget())->CanCopy();
}

FX_BOOL CFWL_DateTimePicker::CanCut() {
  return ToDateTimePicker(GetWidget())->CanCut();
}

FX_BOOL CFWL_DateTimePicker::CanSelectAll() {
  return ToDateTimePicker(GetWidget())->CanSelectAll();
}

FX_BOOL CFWL_DateTimePicker::Copy(CFX_WideString& wsCopy) {
  return ToDateTimePicker(GetWidget())->Copy(wsCopy);
}

FX_BOOL CFWL_DateTimePicker::Cut(CFX_WideString& wsCut) {
  return ToDateTimePicker(GetWidget())->Copy(wsCut);
}

FX_BOOL CFWL_DateTimePicker::Paste(const CFX_WideString& wsPaste) {
  return ToDateTimePicker(GetWidget())->Paste(wsPaste);
}

FX_BOOL CFWL_DateTimePicker::SelectAll() {
  return ToDateTimePicker(GetWidget())->SelectAll();
}

FX_BOOL CFWL_DateTimePicker::Delete() {
  return ToDateTimePicker(GetWidget())->Delete();
}

FX_BOOL CFWL_DateTimePicker::DeSelect() {
  return ToDateTimePicker(GetWidget())->DeSelect();
}

FWL_Error CFWL_DateTimePicker::GetBBox(CFX_RectF& rect) {
  return ToDateTimePicker(GetWidget())->GetBBox(rect);
}

FWL_Error CFWL_DateTimePicker::SetEditLimit(int32_t nLimit) {
  return ToDateTimePicker(GetWidget())->SetEditLimit(nLimit);
}

FWL_Error CFWL_DateTimePicker::ModifyEditStylesEx(uint32_t dwStylesExAdded,
                                                  uint32_t dwStylesExRemoved) {
  return ToDateTimePicker(GetWidget())
      ->ModifyEditStylesEx(dwStylesExAdded, dwStylesExRemoved);
}
