// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_datetimepicker.h"

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/core/cfwl_message.h"
#include "xfa/fwl/core/cfwl_themebackground.h"
#include "xfa/fwl/core/cfwl_widgetmgr.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/ifwl_datetimecalendar.h"
#include "xfa/fwl/core/ifwl_datetimeedit.h"
#include "xfa/fwl/core/ifwl_formproxy.h"
#include "xfa/fwl/core/ifwl_spinbutton.h"
#include "xfa/fwl/core/ifwl_themeprovider.h"

namespace {

const int kDateTimePickerWidth = 100;
const int kDateTimePickerHeight = 20;

}  // namespace

IFWL_DateTimePicker::IFWL_DateTimePicker(
    const IFWL_App* app,
    const CFWL_WidgetImpProperties& properties)
    : IFWL_Widget(app, properties, nullptr),
      m_iBtnState(1),
      m_iYear(-1),
      m_iMonth(-1),
      m_iDay(-1),
      m_bLBtnDown(false) {
  m_rtBtn.Set(0, 0, 0, 0);

  m_pProperties->m_dwStyleExes = FWL_STYLEEXT_DTP_ShortDateFormat;
  CFWL_WidgetImpProperties propMonth;
  propMonth.m_dwStyles = FWL_WGTSTYLE_Popup | FWL_WGTSTYLE_Border;
  propMonth.m_dwStates = FWL_WGTSTATE_Invisible;
  propMonth.m_pDataProvider = &m_MonthCalendarDP;
  propMonth.m_pParent = this;
  propMonth.m_pThemeProvider = m_pProperties->m_pThemeProvider;

  m_pMonthCal.reset(new IFWL_DateTimeCalendar(m_pOwnerApp, propMonth, this));
  CFX_RectF rtMonthCal;
  m_pMonthCal->GetWidgetRect(rtMonthCal, true);
  rtMonthCal.Set(0, 0, rtMonthCal.width, rtMonthCal.height);
  m_pMonthCal->SetWidgetRect(rtMonthCal);
  CFWL_WidgetImpProperties propEdit;
  propEdit.m_pParent = this;
  propEdit.m_pThemeProvider = m_pProperties->m_pThemeProvider;

  m_pEdit.reset(new IFWL_DateTimeEdit(m_pOwnerApp, propEdit, this));
  RegisterEventTarget(m_pMonthCal.get());
  RegisterEventTarget(m_pEdit.get());
}

IFWL_DateTimePicker::~IFWL_DateTimePicker() {
  UnregisterEventTarget();
}

FWL_Type IFWL_DateTimePicker::GetClassID() const {
  return FWL_Type::DateTimePicker;
}

FWL_Error IFWL_DateTimePicker::GetWidgetRect(CFX_RectF& rect, bool bAutoSize) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_GetWidgetRect(rect, bAutoSize);
  }
  if (bAutoSize) {
    rect.Set(0, 0, kDateTimePickerWidth, kDateTimePickerHeight);
    IFWL_Widget::GetWidgetRect(rect, true);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_DateTimePicker::Update() {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_Update();
  }
  if (m_iLock) {
    return FWL_Error::Indefinite;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  m_pEdit->SetThemeProvider(m_pProperties->m_pThemeProvider);
  GetClientRect(m_rtClient);
  FX_FLOAT* pFWidth = static_cast<FX_FLOAT*>(
      GetThemeCapacity(CFWL_WidgetCapacity::ScrollBarWidth));
  if (!pFWidth)
    return FWL_Error::Indefinite;
  FX_FLOAT fBtn = *pFWidth;
  m_rtBtn.Set(m_rtClient.right() - fBtn, m_rtClient.top, fBtn - 1,
              m_rtClient.height - 1);
  CFX_RectF rtEdit;
  rtEdit.Set(m_rtClient.left, m_rtClient.top, m_rtClient.width - fBtn,
             m_rtClient.height);
  m_pEdit->SetWidgetRect(rtEdit);
  ReSetEditAlignment();
  m_pEdit->Update();
  if (!(m_pMonthCal->GetThemeProvider())) {
    m_pMonthCal->SetThemeProvider(m_pProperties->m_pThemeProvider);
  }
  if (m_pProperties->m_pDataProvider) {
    IFWL_DateTimePickerDP* pData =
        static_cast<IFWL_DateTimePickerDP*>(m_pProperties->m_pDataProvider);
    pData->GetToday(this, m_MonthCalendarDP.m_iCurYear,
                    m_MonthCalendarDP.m_iCurMonth, m_MonthCalendarDP.m_iCurDay);
  }
  CFX_RectF rtMonthCal;
  m_pMonthCal->GetWidgetRect(rtMonthCal, true);
  CFX_RectF rtPopUp;
  rtPopUp.Set(rtMonthCal.left, rtMonthCal.top + kDateTimePickerHeight,
              rtMonthCal.width, rtMonthCal.height);
  m_pMonthCal->SetWidgetRect(rtPopUp);
  m_pMonthCal->Update();
  return FWL_Error::Succeeded;
}

int32_t IFWL_DateTimePicker::CountSelRanges() {
  return GetDataTimeEdit()->CountSelRanges();
}

int32_t IFWL_DateTimePicker::GetSelRange(int32_t nIndex, int32_t& nStart) {
  return GetDataTimeEdit()->GetSelRange(nIndex, nStart);
}

FWL_WidgetHit IFWL_DateTimePicker::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  if (m_pWidgetMgr->IsFormDisabled())
    return DisForm_HitTest(fx, fy);
  if (m_rtClient.Contains(fx, fy))
    return FWL_WidgetHit::Client;
  if (IsMonthCalendarShowed()) {
    CFX_RectF rect;
    m_pMonthCal->GetWidgetRect(rect);
    if (rect.Contains(fx, fy))
      return FWL_WidgetHit::Client;
  }
  return FWL_WidgetHit::Unknown;
}

FWL_Error IFWL_DateTimePicker::DrawWidget(CFX_Graphics* pGraphics,
                                          const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_Error::Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_Error::Indefinite;
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  if (HasBorder()) {
    DrawBorder(pGraphics, CFWL_Part::Border, pTheme, pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, CFWL_Part::Edge, pTheme, pMatrix);
  }
  if (!m_rtBtn.IsEmpty()) {
    DrawDropDownButton(pGraphics, pTheme, pMatrix);
  }
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_DrawWidget(pGraphics, pMatrix);
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_DateTimePicker::SetThemeProvider(IFWL_ThemeProvider* pTP) {
  m_pProperties->m_pThemeProvider = pTP;
  m_pMonthCal->SetThemeProvider(pTP);
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_DateTimePicker::GetCurSel(int32_t& iYear,
                                         int32_t& iMonth,
                                         int32_t& iDay) {
  iYear = m_iYear;
  iMonth = m_iMonth;
  iDay = m_iDay;
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_DateTimePicker::SetCurSel(int32_t iYear,
                                         int32_t iMonth,
                                         int32_t iDay) {
  if (iYear <= 0 || iYear >= 3000)
    return FWL_Error::Indefinite;
  if (iMonth <= 0 || iMonth >= 13)
    return FWL_Error::Indefinite;
  if (iDay <= 0 || iDay >= 32)
    return FWL_Error::Indefinite;
  m_iYear = iYear;
  m_iMonth = iMonth;
  m_iDay = iDay;
  m_pMonthCal->SetSelect(iYear, iMonth, iDay);
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_DateTimePicker::SetEditText(const CFX_WideString& wsText) {
  if (!m_pEdit)
    return FWL_Error::Indefinite;

  FWL_Error iRet = m_pEdit->SetText(wsText);
  Repaint(&m_rtClient);
  CFWL_Event_DtpEditChanged ev;
  ev.m_wsText = wsText;
  DispatchEvent(&ev);
  return iRet;
}

FWL_Error IFWL_DateTimePicker::GetEditText(CFX_WideString& wsText,
                                           int32_t nStart,
                                           int32_t nCount) const {
  if (m_pEdit) {
    return m_pEdit->GetText(wsText, nStart, nCount);
  }
  return FWL_Error::Indefinite;
}

bool IFWL_DateTimePicker::CanUndo() {
  return m_pEdit->CanUndo();
}

bool IFWL_DateTimePicker::CanRedo() {
  return m_pEdit->CanRedo();
}

bool IFWL_DateTimePicker::Undo() {
  return m_pEdit->Undo();
}

bool IFWL_DateTimePicker::Redo() {
  return m_pEdit->Redo();
}

bool IFWL_DateTimePicker::CanCopy() {
  int32_t nCount = m_pEdit->CountSelRanges();
  return nCount > 0;
}

bool IFWL_DateTimePicker::CanCut() {
  if (m_pEdit->GetStylesEx() & FWL_STYLEEXT_EDT_ReadOnly) {
    return false;
  }
  int32_t nCount = m_pEdit->CountSelRanges();
  return nCount > 0;
}

bool IFWL_DateTimePicker::CanSelectAll() {
  return m_pEdit->GetTextLength() > 0;
}

bool IFWL_DateTimePicker::Copy(CFX_WideString& wsCopy) {
  return m_pEdit->Copy(wsCopy);
}

bool IFWL_DateTimePicker::Cut(CFX_WideString& wsCut) {
  return m_pEdit->Cut(wsCut);
}

bool IFWL_DateTimePicker::Paste(const CFX_WideString& wsPaste) {
  return m_pEdit->Paste(wsPaste);
}

bool IFWL_DateTimePicker::SelectAll() {
  return m_pEdit->AddSelRange(0) == FWL_Error::Succeeded;
}

bool IFWL_DateTimePicker::Delete() {
  return m_pEdit->ClearText() == FWL_Error::Succeeded;
}

bool IFWL_DateTimePicker::DeSelect() {
  return m_pEdit->ClearSelections() == FWL_Error::Succeeded;
}

FWL_Error IFWL_DateTimePicker::GetBBox(CFX_RectF& rect) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_GetBBox(rect);
  }
  rect = m_pProperties->m_rtWidget;
  if (IsMonthCalendarShowed()) {
    CFX_RectF rtMonth;
    m_pMonthCal->GetWidgetRect(rtMonth);
    rtMonth.Offset(m_pProperties->m_rtWidget.left,
                   m_pProperties->m_rtWidget.top);
    rect.Union(rtMonth);
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_DateTimePicker::SetEditLimit(int32_t nLimit) {
  return m_pEdit->SetLimit(nLimit);
}

FWL_Error IFWL_DateTimePicker::ModifyEditStylesEx(uint32_t dwStylesExAdded,
                                                  uint32_t dwStylesExRemoved) {
  return m_pEdit->ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}

void IFWL_DateTimePicker::DrawDropDownButton(CFX_Graphics* pGraphics,
                                             IFWL_ThemeProvider* pTheme,
                                             const CFX_Matrix* pMatrix) {
  if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_DTP_Spin) ==
      FWL_STYLEEXT_DTP_Spin) {
    return;
  }

  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = CFWL_Part::DropDownButton;
  param.m_dwStates = m_iBtnState;
  param.m_pGraphics = pGraphics;
  param.m_rtPart = m_rtBtn;
  if (pMatrix)
    param.m_matrix.Concat(*pMatrix);

  pTheme->DrawBackground(&param);
}

void IFWL_DateTimePicker::FormatDateString(int32_t iYear,
                                           int32_t iMonth,
                                           int32_t iDay,
                                           CFX_WideString& wsText) {
  if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_DTP_ShortDateFormat) ==
      FWL_STYLEEXT_DTP_ShortDateFormat) {
    wsText.Format(L"%d-%d-%d", iYear, iMonth, iDay);
  } else if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_DTP_LongDateFormat) ==
             FWL_STYLEEXT_DTP_LongDateFormat) {
    wsText.Format(L"%d Year %d Month %d Day", iYear, iMonth, iDay);
  } else if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_DTP_TimeFormat) ==
             FWL_STYLEEXT_DTP_TimeFormat) {
  }
}

void IFWL_DateTimePicker::ShowMonthCalendar(bool bActivate) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_ShowMonthCalendar(bActivate);
  }
  if (IsMonthCalendarShowed() == bActivate) {
    return;
  }
  if (!m_pForm) {
    InitProxyForm();
  }
  if (bActivate) {
    CFX_RectF rtMonth;
    m_pMonthCal->GetWidgetRect(rtMonth);
    CFX_RectF rtAnchor;
    rtAnchor.Set(0, 0, m_pProperties->m_rtWidget.width,
                 m_pProperties->m_rtWidget.height);
    GetPopupPos(0, rtMonth.height, rtAnchor, rtMonth);
    m_pForm->SetWidgetRect(rtMonth);
    rtMonth.left = rtMonth.top = 0;
    m_pMonthCal->SetStates(FWL_WGTSTATE_Invisible, !bActivate);
    m_pMonthCal->SetWidgetRect(rtMonth);
    m_pMonthCal->Update();
    m_pForm->DoModal();
  } else {
    m_pForm->EndDoModal();
  }
}

bool IFWL_DateTimePicker::IsMonthCalendarShowed() {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_IsMonthCalendarShowed();
  }
  if (!m_pForm)
    return false;
  return !(m_pForm->GetStates() & FWL_WGTSTATE_Invisible);
}

void IFWL_DateTimePicker::ReSetEditAlignment() {
  if (!m_pEdit)
    return;
  uint32_t dwStylExes = m_pProperties->m_dwStyleExes;
  uint32_t dwAdd = 0;
  switch (dwStylExes & FWL_STYLEEXT_DTP_EditHAlignMask) {
    case FWL_STYLEEXT_DTP_EditHCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_HCenter;
      break;
    }
    case FWL_STYLEEXT_DTP_EditHFar: {
      dwAdd |= FWL_STYLEEXT_EDT_HFar;
      break;
    }
    default: { dwAdd |= FWL_STYLEEXT_EDT_HNear; }
  }
  switch (dwStylExes & FWL_STYLEEXT_DTP_EditVAlignMask) {
    case FWL_STYLEEXT_DTP_EditVCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_VCenter;
      break;
    }
    case FWL_STYLEEXT_DTP_EditVFar: {
      dwAdd |= FWL_STYLEEXT_EDT_VFar;
      break;
    }
    default: { dwAdd |= FWL_STYLEEXT_EDT_VNear; }
  }
  if (dwStylExes & FWL_STYLEEXT_DTP_EditJustified) {
    dwAdd |= FWL_STYLEEXT_EDT_Justified;
  }
  if (dwStylExes & FWL_STYLEEXT_DTP_EditDistributed) {
    dwAdd |= FWL_STYLEEXT_EDT_Distributed;
  }
  m_pEdit->ModifyStylesEx(dwAdd, FWL_STYLEEXT_EDT_HAlignMask |
                                     FWL_STYLEEXT_EDT_HAlignModeMask |
                                     FWL_STYLEEXT_EDT_VAlignMask);
}

void IFWL_DateTimePicker::ProcessSelChanged(int32_t iYear,
                                            int32_t iMonth,
                                            int32_t iDay) {
  m_iYear = iYear;
  m_iMonth = iMonth;
  m_iDay = iDay;
  CFX_WideString wsText;
  FormatDateString(m_iYear, m_iMonth, m_iDay, wsText);
  m_pEdit->SetText(wsText);
  m_pEdit->Update();
  Repaint(&m_rtClient);
  CFWL_Event_DtpSelectChanged ev;
  ev.m_pSrcTarget = this;
  ev.iYear = m_iYear;
  ev.iMonth = m_iMonth;
  ev.iDay = m_iDay;
  DispatchEvent(&ev);
}

void IFWL_DateTimePicker::InitProxyForm() {
  if (m_pForm)
    return;
  if (!m_pMonthCal)
    return;
  CFWL_WidgetImpProperties propForm;
  propForm.m_dwStyles = FWL_WGTSTYLE_Popup;
  propForm.m_dwStates = FWL_WGTSTATE_Invisible;
  propForm.m_pOwner = this;

  m_pForm.reset(new IFWL_FormProxy(m_pOwnerApp, propForm, m_pMonthCal.get()));
  m_pMonthCal->SetParent(m_pForm.get());
}

IFWL_DateTimeEdit* IFWL_DateTimePicker::GetDataTimeEdit() {
  return m_pEdit.get();
}

FWL_Error IFWL_DateTimePicker::DisForm_Initialize() {
  m_pProperties->m_dwStyleExes = FWL_STYLEEXT_DTP_ShortDateFormat;
  DisForm_InitDateTimeCalendar();
  DisForm_InitDateTimeEdit();
  RegisterEventTarget(m_pMonthCal.get());
  RegisterEventTarget(m_pEdit.get());
  return FWL_Error::Succeeded;
}

void IFWL_DateTimePicker::DisForm_InitDateTimeCalendar() {
  if (m_pMonthCal) {
    return;
  }
  CFWL_WidgetImpProperties propMonth;
  propMonth.m_dwStyles =
      FWL_WGTSTYLE_Popup | FWL_WGTSTYLE_Border | FWL_WGTSTYLE_EdgeSunken;
  propMonth.m_dwStates = FWL_WGTSTATE_Invisible;
  propMonth.m_pParent = this;
  propMonth.m_pDataProvider = &m_MonthCalendarDP;
  propMonth.m_pThemeProvider = m_pProperties->m_pThemeProvider;

  m_pMonthCal.reset(new IFWL_DateTimeCalendar(m_pOwnerApp, propMonth, this));
  CFX_RectF rtMonthCal;
  m_pMonthCal->GetWidgetRect(rtMonthCal, true);
  rtMonthCal.Set(0, 0, rtMonthCal.width, rtMonthCal.height);
  m_pMonthCal->SetWidgetRect(rtMonthCal);
}

void IFWL_DateTimePicker::DisForm_InitDateTimeEdit() {
  if (m_pEdit) {
    return;
  }
  CFWL_WidgetImpProperties propEdit;
  propEdit.m_pParent = this;
  propEdit.m_pThemeProvider = m_pProperties->m_pThemeProvider;

  m_pEdit.reset(new IFWL_DateTimeEdit(m_pOwnerApp, propEdit, this));
}

bool IFWL_DateTimePicker::DisForm_IsMonthCalendarShowed() {
  if (!m_pMonthCal)
    return false;
  return !(m_pMonthCal->GetStates() & FWL_WGTSTATE_Invisible);
}

void IFWL_DateTimePicker::DisForm_ShowMonthCalendar(bool bActivate) {
  bool bShowed = IsMonthCalendarShowed();
  if (bShowed == bActivate) {
    return;
  }
  if (bActivate) {
    CFX_RectF rtMonthCal;
    m_pMonthCal->GetWidgetRect(rtMonthCal, true);
    FX_FLOAT fPopupMin = rtMonthCal.height;
    FX_FLOAT fPopupMax = rtMonthCal.height;
    CFX_RectF rtAnchor(m_pProperties->m_rtWidget);
    rtAnchor.width = rtMonthCal.width;
    rtMonthCal.left = m_rtClient.left;
    rtMonthCal.top = rtAnchor.Height();
    GetPopupPos(fPopupMin, fPopupMax, rtAnchor, rtMonthCal);
    m_pMonthCal->SetWidgetRect(rtMonthCal);
    if (m_iYear > 0 && m_iMonth > 0 && m_iDay > 0) {
      m_pMonthCal->SetSelect(m_iYear, m_iMonth, m_iDay);
    }
    m_pMonthCal->Update();
  }
  m_pMonthCal->SetStates(FWL_WGTSTATE_Invisible, !bActivate);
  if (bActivate) {
    CFWL_MsgSetFocus msg;
    msg.m_pDstTarget = m_pMonthCal.get();
    msg.m_pSrcTarget = m_pEdit.get();
    m_pEdit->GetDelegate()->OnProcessMessage(&msg);
  }
  CFX_RectF rtInvalidate, rtCal;
  rtInvalidate.Set(0, 0, m_pProperties->m_rtWidget.width,
                   m_pProperties->m_rtWidget.height);
  m_pMonthCal->GetWidgetRect(rtCal);
  rtInvalidate.Union(rtCal);
  rtInvalidate.Inflate(2, 2);
  Repaint(&rtInvalidate);
}

FWL_WidgetHit IFWL_DateTimePicker::DisForm_HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  CFX_RectF rect;
  rect.Set(0, 0, m_pProperties->m_rtWidget.width,
           m_pProperties->m_rtWidget.height);
  if (rect.Contains(fx, fy))
    return FWL_WidgetHit::Edit;
  if (DisForm_IsNeedShowButton())
    rect.width += m_fBtn;
  if (rect.Contains(fx, fy))
    return FWL_WidgetHit::Client;
  if (IsMonthCalendarShowed()) {
    m_pMonthCal->GetWidgetRect(rect);
    if (rect.Contains(fx, fy))
      return FWL_WidgetHit::Client;
  }
  return FWL_WidgetHit::Unknown;
}

bool IFWL_DateTimePicker::DisForm_IsNeedShowButton() {
  bool bFocus = m_pProperties->m_dwStates & FWL_WGTSTATE_Focused ||
                m_pMonthCal->GetStates() & FWL_WGTSTATE_Focused ||
                m_pEdit->GetStates() & FWL_WGTSTATE_Focused;
  return bFocus;
}

FWL_Error IFWL_DateTimePicker::DisForm_Update() {
  if (m_iLock)
    return FWL_Error::Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    m_pProperties->m_pThemeProvider = GetAvailableTheme();

  m_pEdit->SetThemeProvider(m_pProperties->m_pThemeProvider);
  GetClientRect(m_rtClient);
  m_pEdit->SetWidgetRect(m_rtClient);
  ReSetEditAlignment();
  m_pEdit->Update();
  if (!m_pMonthCal->GetThemeProvider())
    m_pMonthCal->SetThemeProvider(m_pProperties->m_pThemeProvider);

  if (m_pProperties->m_pDataProvider) {
    IFWL_DateTimePickerDP* pData =
        static_cast<IFWL_DateTimePickerDP*>(m_pProperties->m_pDataProvider);
    pData->GetToday(this, m_MonthCalendarDP.m_iCurYear,
                    m_MonthCalendarDP.m_iCurMonth, m_MonthCalendarDP.m_iCurDay);
  }
  FX_FLOAT* pWidth = static_cast<FX_FLOAT*>(
      GetThemeCapacity(CFWL_WidgetCapacity::ScrollBarWidth));
  if (!pWidth)
    return FWL_Error::Succeeded;

  m_fBtn = *pWidth;
  CFX_RectF rtMonthCal;
  m_pMonthCal->GetWidgetRect(rtMonthCal, true);
  CFX_RectF rtPopUp;
  rtPopUp.Set(rtMonthCal.left, rtMonthCal.top + kDateTimePickerHeight,
              rtMonthCal.width, rtMonthCal.height);
  m_pMonthCal->SetWidgetRect(rtPopUp);
  m_pMonthCal->Update();
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_DateTimePicker::DisForm_GetWidgetRect(CFX_RectF& rect,
                                                     bool bAutoSize) {
  rect = m_pProperties->m_rtWidget;
  if (DisForm_IsNeedShowButton()) {
    rect.width += m_fBtn;
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_DateTimePicker::DisForm_GetBBox(CFX_RectF& rect) {
  rect = m_pProperties->m_rtWidget;
  if (DisForm_IsNeedShowButton()) {
    rect.width += m_fBtn;
  }
  if (IsMonthCalendarShowed()) {
    CFX_RectF rtMonth;
    m_pMonthCal->GetWidgetRect(rtMonth);
    rtMonth.Offset(m_pProperties->m_rtWidget.left,
                   m_pProperties->m_rtWidget.top);
    rect.Union(rtMonth);
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_DateTimePicker::DisForm_DrawWidget(CFX_Graphics* pGraphics,
                                                  const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_Error::Indefinite;
  if (m_pEdit) {
    CFX_RectF rtEdit;
    m_pEdit->GetWidgetRect(rtEdit);
    CFX_Matrix mt;
    mt.Set(1, 0, 0, 1, rtEdit.left, rtEdit.top);
    if (pMatrix) {
      mt.Concat(*pMatrix);
    }
    m_pEdit->DrawWidget(pGraphics, &mt);
  }
  if (IsMonthCalendarShowed()) {
    CFX_RectF rtMonth;
    m_pMonthCal->GetWidgetRect(rtMonth);
    CFX_Matrix mt;
    mt.Set(1, 0, 0, 1, rtMonth.left, rtMonth.top);
    if (pMatrix) {
      mt.Concat(*pMatrix);
    }
    m_pMonthCal->DrawWidget(pGraphics, &mt);
  }
  return FWL_Error::Succeeded;
}

void IFWL_DateTimePicker::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  switch (pMessage->GetClassID()) {
    case CFWL_MessageType::SetFocus:
      OnFocusChanged(pMessage, true);
      break;
    case CFWL_MessageType::KillFocus:
      OnFocusChanged(pMessage, false);
      break;
    case CFWL_MessageType::Mouse: {
      CFWL_MsgMouse* pMouse = static_cast<CFWL_MsgMouse*>(pMessage);
      switch (pMouse->m_dwCmd) {
        case FWL_MouseCommand::LeftButtonDown:
          OnLButtonDown(pMouse);
          break;
        case FWL_MouseCommand::LeftButtonUp:
          OnLButtonUp(pMouse);
          break;
        case FWL_MouseCommand::Move:
          OnMouseMove(pMouse);
          break;
        case FWL_MouseCommand::Leave:
          OnMouseLeave(pMouse);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_MessageType::Key: {
      if (m_pEdit->GetStates() & FWL_WGTSTATE_Focused) {
        m_pEdit->GetDelegate()->OnProcessMessage(pMessage);
        return;
      }
      break;
    }
    default:
      break;
  }

  IFWL_Widget::OnProcessMessage(pMessage);
}

void IFWL_DateTimePicker::OnDrawWidget(CFX_Graphics* pGraphics,
                                       const CFX_Matrix* pMatrix) {
  DrawWidget(pGraphics, pMatrix);
}

void IFWL_DateTimePicker::OnFocusChanged(CFWL_Message* pMsg, bool bSet) {
  if (!pMsg)
    return;
  if (m_pWidgetMgr->IsFormDisabled())
    return DisForm_OnFocusChanged(pMsg, bSet);

  if (bSet) {
    m_pProperties->m_dwStates |= (FWL_WGTSTATE_Focused);
    Repaint(&m_rtClient);
  } else {
    m_pProperties->m_dwStates &= ~(FWL_WGTSTATE_Focused);
    Repaint(&m_rtClient);
  }
  if (pMsg->m_pSrcTarget == m_pMonthCal.get() && IsMonthCalendarShowed()) {
    ShowMonthCalendar(false);
  }
  Repaint(&m_rtClient);
}

void IFWL_DateTimePicker::OnLButtonDown(CFWL_MsgMouse* pMsg) {
  if (!pMsg)
    return;
  if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0)
    SetFocus(true);
  if (m_rtBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    if (IsMonthCalendarShowed()) {
      ShowMonthCalendar(false);
      CFWL_Event_DtpCloseUp ev;
      DispatchEvent(&ev);
    } else {
      if (!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_DTP_TimeFormat)) {
        ShowMonthCalendar(true);
        CFWL_Event_DtpDropDown ev;
        DispatchEvent(&ev);
      }
      m_bLBtnDown = true;
      Repaint(&m_rtClient);
    }
  }
}

void IFWL_DateTimePicker::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  if (!pMsg)
    return;

  m_bLBtnDown = false;
  if (m_rtBtn.Contains(pMsg->m_fx, pMsg->m_fy))
    m_iBtnState = CFWL_PartState_Hovered;
  else
    m_iBtnState = CFWL_PartState_Normal;
  Repaint(&m_rtBtn);
}

void IFWL_DateTimePicker::OnMouseMove(CFWL_MsgMouse* pMsg) {
  if (!m_rtBtn.Contains(pMsg->m_fx, pMsg->m_fy))
    m_iBtnState = CFWL_PartState_Normal;

  Repaint(&m_rtBtn);
}

void IFWL_DateTimePicker::OnMouseLeave(CFWL_MsgMouse* pMsg) {
  if (!pMsg)
    return;
  m_iBtnState = CFWL_PartState_Normal;
  Repaint(&m_rtBtn);
}

void IFWL_DateTimePicker::DisForm_OnFocusChanged(CFWL_Message* pMsg,
                                                 bool bSet) {
  CFX_RectF rtInvalidate(m_rtBtn);
  if (bSet) {
    m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
    if (m_pEdit && !(m_pEdit->GetStylesEx() & FWL_STYLEEXT_EDT_ReadOnly)) {
      m_rtBtn.Set(m_pProperties->m_rtWidget.width, 0, m_fBtn,
                  m_pProperties->m_rtWidget.height - 1);
    }
    rtInvalidate = m_rtBtn;
    pMsg->m_pDstTarget = m_pEdit.get();
    m_pEdit->GetDelegate()->OnProcessMessage(pMsg);
  } else {
    m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
    m_rtBtn.Set(0, 0, 0, 0);
    if (DisForm_IsMonthCalendarShowed())
      ShowMonthCalendar(false);
    if (m_pEdit->GetStates() & FWL_WGTSTATE_Focused) {
      pMsg->m_pSrcTarget = m_pEdit.get();
      m_pEdit->GetDelegate()->OnProcessMessage(pMsg);
    }
  }
  rtInvalidate.Inflate(2, 2);
  Repaint(&rtInvalidate);
}

IFWL_DateTimePicker::CFWL_MonthCalendarImpDP::CFWL_MonthCalendarImpDP() {
  m_iCurYear = 2010;
  m_iCurMonth = 3;
  m_iCurDay = 29;
}

FWL_Error IFWL_DateTimePicker::CFWL_MonthCalendarImpDP::GetCaption(
    IFWL_Widget* pWidget,
    CFX_WideString& wsCaption) {
  return FWL_Error::Succeeded;
}

int32_t IFWL_DateTimePicker::CFWL_MonthCalendarImpDP::GetCurDay(
    IFWL_Widget* pWidget) {
  return m_iCurDay;
}

int32_t IFWL_DateTimePicker::CFWL_MonthCalendarImpDP::GetCurMonth(
    IFWL_Widget* pWidget) {
  return m_iCurMonth;
}

int32_t IFWL_DateTimePicker::CFWL_MonthCalendarImpDP::GetCurYear(
    IFWL_Widget* pWidget) {
  return m_iCurYear;
}
