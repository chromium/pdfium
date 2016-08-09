// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/lightwidget/cfwl_datetimepicker.h"

#include <memory>

#include "xfa/fwl/basewidget/ifwl_datetimepicker.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_widget.h"

IFWL_DateTimePicker* CFWL_DateTimePicker::GetWidget() {
  return static_cast<IFWL_DateTimePicker*>(m_pIface.get());
}

const IFWL_DateTimePicker* CFWL_DateTimePicker::GetWidget() const {
  return static_cast<IFWL_DateTimePicker*>(m_pIface.get());
}

CFWL_DateTimePicker* CFWL_DateTimePicker::Create() {
  return new CFWL_DateTimePicker;
}

FWL_Error CFWL_DateTimePicker::Initialize(
    const CFWL_WidgetProperties* pProperties) {
  if (m_pIface)
    return FWL_Error::Indefinite;
  if (pProperties) {
    *m_pProperties = *pProperties;
  }
  std::unique_ptr<IFWL_DateTimePicker> pDateTimePicker(
      IFWL_DateTimePicker::Create(
          m_pProperties->MakeWidgetImpProperties(&m_DateTimePickerDP),
          nullptr));
  FWL_Error ret = pDateTimePicker->Initialize();
  if (ret != FWL_Error::Succeeded) {
    return ret;
  }
  m_pIface = std::move(pDateTimePicker);
  CFWL_Widget::Initialize();
  return FWL_Error::Succeeded;
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
  return GetWidget()->CountSelRanges();
}

int32_t CFWL_DateTimePicker::GetSelRange(int32_t nIndex, int32_t& nStart) {
  return GetWidget()->GetSelRange(nIndex, nStart);
}

FWL_Error CFWL_DateTimePicker::GetEditText(CFX_WideString& wsText) {
  return GetWidget()->GetEditText(wsText);
}

FWL_Error CFWL_DateTimePicker::SetEditText(const CFX_WideString& wsText) {
  return GetWidget()->SetEditText(wsText);
}

FWL_Error CFWL_DateTimePicker::GetCurSel(int32_t& iYear,
                                         int32_t& iMonth,
                                         int32_t& iDay) {
  return GetWidget()->GetCurSel(iYear, iMonth, iDay);
}

FWL_Error CFWL_DateTimePicker::SetCurSel(int32_t iYear,
                                         int32_t iMonth,
                                         int32_t iDay) {
  return GetWidget()->SetCurSel(iYear, iMonth, iDay);
}

CFWL_DateTimePicker::CFWL_DateTimePicker() {}

CFWL_DateTimePicker::~CFWL_DateTimePicker() {}

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
  return GetWidget()->CanUndo();
}

FX_BOOL CFWL_DateTimePicker::CanRedo() {
  return GetWidget()->CanRedo();
}

FX_BOOL CFWL_DateTimePicker::Undo() {
  return GetWidget()->Undo();
}

FX_BOOL CFWL_DateTimePicker::Redo() {
  return GetWidget()->Redo();
}

FX_BOOL CFWL_DateTimePicker::CanCopy() {
  return GetWidget()->CanCopy();
}

FX_BOOL CFWL_DateTimePicker::CanCut() {
  return GetWidget()->CanCut();
}

FX_BOOL CFWL_DateTimePicker::CanSelectAll() {
  return GetWidget()->CanSelectAll();
}

FX_BOOL CFWL_DateTimePicker::Copy(CFX_WideString& wsCopy) {
  return GetWidget()->Copy(wsCopy);
}

FX_BOOL CFWL_DateTimePicker::Cut(CFX_WideString& wsCut) {
  return GetWidget()->Copy(wsCut);
}

FX_BOOL CFWL_DateTimePicker::Paste(const CFX_WideString& wsPaste) {
  return GetWidget()->Paste(wsPaste);
}

FX_BOOL CFWL_DateTimePicker::SelectAll() {
  return GetWidget()->SelectAll();
}

FX_BOOL CFWL_DateTimePicker::Delete() {
  return GetWidget()->Delete();
}

FX_BOOL CFWL_DateTimePicker::DeSelect() {
  return GetWidget()->DeSelect();
}

FWL_Error CFWL_DateTimePicker::GetBBox(CFX_RectF& rect) {
  return GetWidget()->GetBBox(rect);
}

FWL_Error CFWL_DateTimePicker::SetEditLimit(int32_t nLimit) {
  return GetWidget()->SetEditLimit(nLimit);
}

FWL_Error CFWL_DateTimePicker::ModifyEditStylesEx(uint32_t dwStylesExAdded,
                                                  uint32_t dwStylesExRemoved) {
  return GetWidget()->ModifyEditStylesEx(dwStylesExAdded, dwStylesExRemoved);
}
