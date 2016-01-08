// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_panelimp.h"
#include "xfa/src/fwl/src/core/include/fwl_formimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetmgrimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_formproxyimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_editimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_monthcalendarimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_datetimepickerimp.h"
#define FWL_DTP_WIDTH 100
#define FWL_DTP_HEIGHT 20

// static
IFWL_DateTimePicker* IFWL_DateTimePicker::Create(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter) {
  IFWL_DateTimePicker* pDateTimePicker = new IFWL_DateTimePicker;
  CFWL_DateTimePickerImp* pDateTimePickerImpl =
      new CFWL_DateTimePickerImp(properties, pOuter);
  pDateTimePicker->SetImpl(pDateTimePickerImpl);
  pDateTimePickerImpl->SetInterface(pDateTimePicker);
  return pDateTimePicker;
}

// Static
IFWL_DateTimeForm* IFWL_DateTimeForm::Create(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter) {
  IFWL_DateTimeForm* pDateTimeForm = new IFWL_DateTimeForm;
  CFWL_FormProxyImp* pFormProxyImpl = new CFWL_FormProxyImp(properties, pOuter);
  pDateTimeForm->SetImpl(pFormProxyImpl);
  pFormProxyImpl->SetInterface(pDateTimeForm);
  return pDateTimeForm;
}

// static
IFWL_DateTimeCalender* IFWL_DateTimeCalender::Create(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter) {
  IFWL_DateTimeCalender* pDateTimeCalendar = new IFWL_DateTimeCalender;
  CFWL_DateTimeCalendar* pDateTimeCalendarImpl =
      new CFWL_DateTimeCalendar(properties, pOuter);
  pDateTimeCalendar->SetImpl(pDateTimeCalendarImpl);
  pDateTimeCalendarImpl->SetInterface(pDateTimeCalendar);
  return pDateTimeCalendar;
}

// static
IFWL_DateTimeEdit* IFWL_DateTimeEdit::Create(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter) {
  IFWL_DateTimeEdit* pDateTimeEdit = new IFWL_DateTimeEdit;
  CFWL_DateTimeEdit* pDateTimeEditImpl =
      new CFWL_DateTimeEdit(properties, pOuter);
  pDateTimeEdit->SetImpl(pDateTimeEditImpl);
  pDateTimeEditImpl->SetInterface(pDateTimeEdit);
  return pDateTimeEdit;
}

IFWL_DateTimePicker::IFWL_DateTimePicker() {
}
int32_t IFWL_DateTimePicker::CountSelRanges() {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())
      ->GetDataTimeEdit()
      ->CountSelRanges();
}
int32_t IFWL_DateTimePicker::GetSelRange(int32_t nIndex, int32_t& nStart) {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())
      ->GetDataTimeEdit()
      ->GetSelRange(nIndex, nStart);
}
FWL_ERR IFWL_DateTimePicker::GetCurSel(int32_t& iYear,
                                       int32_t& iMonth,
                                       int32_t& iDay) {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())
      ->GetCurSel(iYear, iMonth, iDay);
}
FWL_ERR IFWL_DateTimePicker::SetCurSel(int32_t iYear,
                                       int32_t iMonth,
                                       int32_t iDay) {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())
      ->SetCurSel(iYear, iMonth, iDay);
}
FWL_ERR IFWL_DateTimePicker::SetEditText(const CFX_WideString& wsText) {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->SetEditText(wsText);
}
FWL_ERR IFWL_DateTimePicker::GetEditText(CFX_WideString& wsText,
                                         int32_t nStart,
                                         int32_t nCount) const {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())
      ->GetEditText(wsText, nStart, nCount);
}
FX_BOOL IFWL_DateTimePicker::CanUndo() {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->CanUndo();
}
FX_BOOL IFWL_DateTimePicker::CanRedo() {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->CanRedo();
}
FX_BOOL IFWL_DateTimePicker::Undo() {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->Undo();
}
FX_BOOL IFWL_DateTimePicker::Redo() {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->Redo();
}
FX_BOOL IFWL_DateTimePicker::CanCopy() {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->CanCopy();
}
FX_BOOL IFWL_DateTimePicker::CanCut() {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->CanCut();
}
FX_BOOL IFWL_DateTimePicker::CanSelectAll() {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->CanSelectAll();
}
FX_BOOL IFWL_DateTimePicker::Copy(CFX_WideString& wsCopy) {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->Copy(wsCopy);
}
FX_BOOL IFWL_DateTimePicker::Cut(CFX_WideString& wsCut) {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->Cut(wsCut);
}
FX_BOOL IFWL_DateTimePicker::Paste(const CFX_WideString& wsPaste) {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->Paste(wsPaste);
}
FX_BOOL IFWL_DateTimePicker::SelectAll() {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->SelectAll();
}
FX_BOOL IFWL_DateTimePicker::Delete() {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->Delete();
}
FX_BOOL IFWL_DateTimePicker::DeSelect() {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->DeSelect();
}
FWL_ERR IFWL_DateTimePicker::GetBBox(CFX_RectF& rect) {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->GetBBox(rect);
}
FWL_ERR IFWL_DateTimePicker::SetEditLimit(int32_t nLimit) {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())->SetEditLimit(nLimit);
}
FWL_ERR IFWL_DateTimePicker::ModifyEditStylesEx(FX_DWORD dwStylesExAdded,
                                                FX_DWORD dwStylesExRemoved) {
  return static_cast<CFWL_DateTimePickerImp*>(GetImpl())
      ->ModifyEditStylesEx(dwStylesExAdded, dwStylesExRemoved);
}
CFWL_DateTimeEdit::CFWL_DateTimeEdit(const CFWL_WidgetImpProperties& properties,
                                     IFWL_Widget* pOuter)
    : CFWL_EditImp(properties, pOuter) {}
FWL_ERR CFWL_DateTimeEdit::Initialize() {
  m_pDelegate = new CFWL_DateTimeEditImpDelegate(this);
  if (CFWL_EditImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimeEdit::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_EditImp::Finalize();
}
CFWL_DateTimeEditImpDelegate::CFWL_DateTimeEditImpDelegate(
    CFWL_DateTimeEdit* pOwner)
    : CFWL_EditImpDelegate(pOwner), m_pOwner(pOwner) {
}
int32_t CFWL_DateTimeEditImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (m_pOwner->m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_OnProcessMessage(pMessage);
  }
  FX_DWORD dwHashCode = pMessage->GetClassID();
  if (dwHashCode == FWL_MSGHASH_SetFocus ||
      dwHashCode == FWL_MSGHASH_KillFocus) {
    IFWL_Widget* pOuter = m_pOwner->GetOuter();
    IFWL_WidgetDelegate* pDelegate = pOuter->SetDelegate(NULL);
    pDelegate->OnProcessMessage(pMessage);
  }
  return 1;
}
int32_t CFWL_DateTimeEditImpDelegate::DisForm_OnProcessMessage(
    CFWL_Message* pMessage) {
  FX_DWORD dwHashCode = pMessage->GetClassID();
  if (m_pOwner->m_pWidgetMgr->IsFormDisabled()) {
    if (dwHashCode == FWL_MSGHASH_Mouse) {
      CFWL_MsgMouse* pMouse = static_cast<CFWL_MsgMouse*>(pMessage);
      if (pMouse->m_dwCmd == FWL_MSGMOUSECMD_LButtonDown ||
          pMouse->m_dwCmd == FWL_MSGMOUSECMD_RButtonDown) {
        if ((m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0) {
          m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
        }
        CFWL_DateTimePickerImp* pDateTime =
            static_cast<CFWL_DateTimePickerImp*>(m_pOwner->m_pOuter->GetImpl());
        if (pDateTime->IsMonthCalendarShowed()) {
          CFX_RectF rtInvalidate;
          pDateTime->GetWidgetRect(rtInvalidate);
          pDateTime->ShowMonthCalendar(FALSE);
          rtInvalidate.Offset(-rtInvalidate.left, -rtInvalidate.top);
          pDateTime->Repaint(&rtInvalidate);
        }
      }
    } else if (dwHashCode == FWL_MSGHASH_Key) {
      return CFWL_EditImpDelegate::OnProcessMessage(pMessage);
    }
  }
  return CFWL_EditImpDelegate::OnProcessMessage(pMessage);
}
CFWL_DateTimeCalendar::CFWL_DateTimeCalendar(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter)
    : CFWL_MonthCalendarImp(properties, pOuter) {}
FWL_ERR CFWL_DateTimeCalendar::Initialize() {
  if (CFWL_MonthCalendarImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  delete m_pDelegate;
  m_pDelegate = new CFWL_DateTimeCalendarImpDelegate(this);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimeCalendar::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_MonthCalendarImp::Finalize();
}
CFWL_DateTimeCalendarImpDelegate::CFWL_DateTimeCalendarImpDelegate(
    CFWL_DateTimeCalendar* pOwner)
    : CFWL_MonthCalendarImpDelegate(pOwner), m_pOwner(pOwner) {
  m_bFlag = FALSE;
}
int32_t CFWL_DateTimeCalendarImpDelegate::OnProcessMessage(
    CFWL_Message* pMessage) {
  FX_DWORD dwCode = pMessage->GetClassID();
  if (dwCode == FWL_MSGHASH_SetFocus || dwCode == FWL_MSGHASH_KillFocus) {
    IFWL_Widget* pOuter = m_pOwner->GetOuter();
    IFWL_WidgetDelegate* pDelegate = pOuter->SetDelegate(NULL);
    return pDelegate->OnProcessMessage(pMessage);
  } else if (dwCode == FWL_MSGHASH_Mouse) {
    CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
    if (pMsg->m_dwCmd == FWL_MSGMOUSECMD_LButtonDown) {
      OnLButtonDownEx(pMsg);
      return 1;
    } else if (pMsg->m_dwCmd == FWL_MSGMOUSECMD_LButtonUp) {
      OnLButtonUpEx(pMsg);
      return 1;
    }
  }
  return CFWL_MonthCalendarImpDelegate::OnProcessMessage(pMessage);
}
void CFWL_DateTimeCalendarImpDelegate::OnLButtonDownEx(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_rtLBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iLBtnPartStates = FWL_PARTSTATE_MCD_Pressed;
    m_pOwner->PrevMonth();
    m_pOwner->Repaint(&m_pOwner->m_rtClient);
  } else if (m_pOwner->m_rtRBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iRBtnPartStates |= FWL_PARTSTATE_MCD_Pressed;
    m_pOwner->NextMonth();
    m_pOwner->Repaint(&m_pOwner->m_rtClient);
  } else if (m_pOwner->m_rtToday.Contains(pMsg->m_fx, pMsg->m_fy)) {
    if ((m_pOwner->m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_NoToday) ==
        0) {
      m_pOwner->JumpToToday();
      m_pOwner->Repaint(&m_pOwner->m_rtClient);
    }
  } else {
    IFWL_DateTimePicker* pIPicker =
        static_cast<IFWL_DateTimePicker*>(m_pOwner->m_pOuter);
    CFWL_DateTimePickerImp* pPicker =
        static_cast<CFWL_DateTimePickerImp*>(pIPicker->GetImpl());
    if (pPicker->IsMonthCalendarShowed()) {
      m_bFlag = 1;
    }
  }
}
void CFWL_DateTimeCalendarImpDelegate::OnLButtonUpEx(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_OnLButtonUpEx(pMsg);
  }
  if (m_pOwner->m_rtLBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iLBtnPartStates = 0;
    m_pOwner->Repaint(&m_pOwner->m_rtLBtn);
    return;
  }
  if (m_pOwner->m_rtRBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iRBtnPartStates = 0;
    m_pOwner->Repaint(&m_pOwner->m_rtRBtn);
    return;
  }
  if (m_pOwner->m_rtToday.Contains(pMsg->m_fx, pMsg->m_fy)) {
    return;
  }
  int32_t iOldSel = 0;
  if (m_pOwner->m_arrSelDays.GetSize() > 0) {
    iOldSel = m_pOwner->m_arrSelDays[0];
  }
  int32_t iCurSel = m_pOwner->GetDayAtPoint(pMsg->m_fx, pMsg->m_fy);
  CFX_RectF rt;
  IFWL_DateTimePicker* pIPicker =
      static_cast<IFWL_DateTimePicker*>(m_pOwner->m_pOuter);
  CFWL_DateTimePickerImp* pPicker =
      static_cast<CFWL_DateTimePickerImp*>(pIPicker->GetImpl());
  pPicker->m_pForm->GetWidgetRect(rt);
  rt.Set(0, 0, rt.width, rt.height);
  if (iCurSel > 0) {
    LPDATEINFO lpDatesInfo =
        (LPDATEINFO)m_pOwner->m_arrDates.GetAt(iCurSel - 1);
    CFX_RectF rtInvalidate(lpDatesInfo->rect);
    if (iOldSel > 0 && iOldSel <= m_pOwner->m_arrDates.GetSize()) {
      lpDatesInfo = (LPDATEINFO)m_pOwner->m_arrDates.GetAt(iOldSel - 1);
      rtInvalidate.Union(lpDatesInfo->rect);
    }
    m_pOwner->AddSelDay(iCurSel);
    if (!m_pOwner->m_pOuter)
      return;
    pPicker->ProcessSelChanged(m_pOwner->m_iCurYear, m_pOwner->m_iCurMonth,
                               iCurSel);
    pPicker->ShowMonthCalendar(FALSE);
  } else if (m_bFlag && (!rt.Contains(pMsg->m_fx, pMsg->m_fy))) {
    IFWL_DateTimePicker* pIPicker =
        static_cast<IFWL_DateTimePicker*>(m_pOwner->m_pOuter);
    CFWL_DateTimePickerImp* pPicker =
        static_cast<CFWL_DateTimePickerImp*>(pIPicker->GetImpl());
    pPicker->ShowMonthCalendar(FALSE);
  }
  m_bFlag = 0;
}
void CFWL_DateTimeCalendarImpDelegate::OnMouseMoveEx(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_MultiSelect) {
    return;
  }
  FX_BOOL bRepaint = FALSE;
  CFX_RectF rtInvalidate;
  rtInvalidate.Set(0, 0, 0, 0);
  if (m_pOwner->m_rtDates.Contains(pMsg->m_fx, pMsg->m_fy)) {
    int32_t iHover = m_pOwner->GetDayAtPoint(pMsg->m_fx, pMsg->m_fy);
    bRepaint = m_pOwner->m_iHovered != iHover;
    if (bRepaint) {
      if (m_pOwner->m_iHovered > 0) {
        m_pOwner->GetDayRect(m_pOwner->m_iHovered, rtInvalidate);
      }
      if (iHover > 0) {
        CFX_RectF rtDay;
        m_pOwner->GetDayRect(iHover, rtDay);
        if (rtInvalidate.IsEmpty()) {
          rtInvalidate = rtDay;
        } else {
          rtInvalidate.Union(rtDay);
        }
      }
    }
    m_pOwner->m_iHovered = iHover;
    CFWL_Event_DtpHoverChanged ev;
    ev.hoverday = iHover;
    m_pOwner->DispatchEvent(&ev);
  } else {
    bRepaint = m_pOwner->m_iHovered > 0;
    if (bRepaint) {
      m_pOwner->GetDayRect(m_pOwner->m_iHovered, rtInvalidate);
    }
    m_pOwner->m_iHovered = -1;
  }
  if (bRepaint && !rtInvalidate.IsEmpty()) {
    m_pOwner->Repaint(&rtInvalidate);
  }
}
int32_t CFWL_DateTimeCalendarImpDelegate::DisForm_OnProcessMessage(
    CFWL_Message* pMessage) {
  if (pMessage->GetClassID() == FWL_MSGHASH_Mouse) {
    CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
    if (pMsg->m_dwCmd == FWL_MSGMOUSECMD_LButtonUp) {
      DisForm_OnLButtonUpEx(pMsg);
      return 1;
    }
  }
  return CFWL_MonthCalendarImpDelegate::OnProcessMessage(pMessage);
}
void CFWL_DateTimeCalendarImpDelegate::DisForm_OnLButtonUpEx(
    CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_rtLBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iLBtnPartStates = 0;
    m_pOwner->Repaint(&(m_pOwner->m_rtLBtn));
    return;
  }
  if (m_pOwner->m_rtRBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iRBtnPartStates = 0;
    m_pOwner->Repaint(&(m_pOwner->m_rtRBtn));
    return;
  }
  if (m_pOwner->m_rtToday.Contains(pMsg->m_fx, pMsg->m_fy)) {
    return;
  }
  int32_t iOldSel = 0;
  if (m_pOwner->m_arrSelDays.GetSize() > 0) {
    iOldSel = m_pOwner->m_arrSelDays[0];
  }
  int32_t iCurSel = m_pOwner->GetDayAtPoint(pMsg->m_fx, pMsg->m_fy);
  if (iCurSel > 0) {
    LPDATEINFO lpDatesInfo =
        (LPDATEINFO)m_pOwner->m_arrDates.GetAt(iCurSel - 1);
    CFX_RectF rtInvalidate(lpDatesInfo->rect);
    if (iOldSel > 0 && iOldSel <= m_pOwner->m_arrDates.GetSize()) {
      lpDatesInfo = (LPDATEINFO)m_pOwner->m_arrDates.GetAt(iOldSel - 1);
      rtInvalidate.Union(lpDatesInfo->rect);
    }
    m_pOwner->AddSelDay(iCurSel);
    CFWL_DateTimePickerImp* pDateTime =
        static_cast<CFWL_DateTimePickerImp*>(m_pOwner->m_pOuter->GetImpl());
    pDateTime->ProcessSelChanged(m_pOwner->m_iCurYear, m_pOwner->m_iCurMonth,
                                 iCurSel);
    pDateTime->ShowMonthCalendar(FALSE);
  }
}
CFWL_DateTimePickerImp::CFWL_DateTimePickerImp(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter),
      m_iBtnState(1),
      m_iYear(-1),
      m_iMonth(-1),
      m_iDay(-1),
      m_bLBtnDown(FALSE),
      m_pEdit(nullptr),
      m_pMonthCal(nullptr),
      m_pForm(nullptr) {
  m_rtBtn.Set(0, 0, 0, 0);
}
CFWL_DateTimePickerImp::~CFWL_DateTimePickerImp() {
}
FWL_ERR CFWL_DateTimePickerImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_DateTimePicker;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_DateTimePickerImp::GetClassID() const {
  return FWL_CLASSHASH_DateTimePicker;
}
FWL_ERR CFWL_DateTimePickerImp::Initialize() {
  if (CFWL_WidgetImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  m_pDelegate = new CFWL_DateTimePickerImpDelegate(this);
  m_pProperties->m_dwStyleExes = FWL_STYLEEXT_DTP_ShortDateFormat;
  CFWL_WidgetImpProperties propMonth;
  propMonth.m_dwStyles = FWL_WGTSTYLE_Popup | FWL_WGTSTYLE_Border;
  propMonth.m_dwStates = FWL_WGTSTATE_Invisible;
  propMonth.m_pDataProvider = &m_MonthCalendarDP;
  propMonth.m_pParent = m_pInterface;
  propMonth.m_pThemeProvider = m_pProperties->m_pThemeProvider;
  m_pMonthCal.reset(IFWL_DateTimeCalender::Create(propMonth, m_pInterface));
  m_pMonthCal->Initialize();
  CFX_RectF rtMonthCal;
  m_pMonthCal->GetWidgetRect(rtMonthCal, TRUE);
  rtMonthCal.Set(0, 0, rtMonthCal.width, rtMonthCal.height);
  m_pMonthCal->SetWidgetRect(rtMonthCal);
  CFWL_WidgetImpProperties propEdit;
  propEdit.m_pParent = m_pInterface;
  propEdit.m_pThemeProvider = m_pProperties->m_pThemeProvider;
  m_pEdit.reset(IFWL_DateTimeEdit::Create(propEdit, m_pInterface));
  m_pEdit->Initialize();
  RegisterEventTarget(m_pMonthCal.get());
  RegisterEventTarget(m_pEdit.get());
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimePickerImp::Finalize() {
  if (m_pEdit) {
    m_pEdit->Finalize();
  }
  if (m_pMonthCal) {
    m_pMonthCal->Finalize();
  }
  if (m_pForm) {
    m_pForm->Finalize();
  }
  UnregisterEventTarget();
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_DateTimePickerImp::GetWidgetRect(CFX_RectF& rect,
                                              FX_BOOL bAutoSize) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_GetWidgetRect(rect, bAutoSize);
  }
  if (bAutoSize) {
    rect.Set(0, 0, FWL_DTP_WIDTH, FWL_DTP_HEIGHT);
    CFWL_WidgetImp::GetWidgetRect(rect, TRUE);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimePickerImp::Update() {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_Update();
  }
  if (m_iLock) {
    return FWL_ERR_Indefinite;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  m_pEdit->SetThemeProvider(m_pProperties->m_pThemeProvider);
  GetClientRect(m_rtClient);
  FX_FLOAT* pFWidth =
      static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_ScrollBarWidth));
  if (!pFWidth)
    return FWL_ERR_Indefinite;
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
    pData->GetToday(m_pInterface, m_MonthCalendarDP.m_iCurYear,
                    m_MonthCalendarDP.m_iCurMonth, m_MonthCalendarDP.m_iCurDay);
  }
  CFX_RectF rtMonthCal;
  m_pMonthCal->GetWidgetRect(rtMonthCal, TRUE);
  CFX_RectF rtPopUp;
  rtPopUp.Set(rtMonthCal.left, rtMonthCal.top + FWL_DTP_HEIGHT,
              rtMonthCal.width, rtMonthCal.height);
  m_pMonthCal->SetWidgetRect(rtPopUp);
  m_pMonthCal->Update();
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_DateTimePickerImp::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_HitTest(fx, fy);
  }
  if (m_rtClient.Contains(fx, fy)) {
    return FWL_WGTHITTEST_Client;
  }
  if (IsMonthCalendarShowed()) {
    CFX_RectF rect;
    m_pMonthCal->GetWidgetRect(rect);
    if (rect.Contains(fx, fy)) {
      return FWL_WGTHITTEST_Client;
    }
  }
  return FWL_WGTHITTEST_Unknown;
}
FWL_ERR CFWL_DateTimePickerImp::DrawWidget(CFX_Graphics* pGraphics,
                                           const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_ERR_Indefinite;
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  if (HasBorder()) {
    DrawBorder(pGraphics, FWL_PART_DTP_Border, pTheme, pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, FWL_PART_DTP_Edge, pTheme, pMatrix);
  }
  if (!m_rtBtn.IsEmpty()) {
    DrawDropDownButton(pGraphics, pTheme, pMatrix);
  }
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_DrawWidget(pGraphics, pMatrix);
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimePickerImp::SetThemeProvider(IFWL_ThemeProvider* pTP) {
  m_pProperties->m_pThemeProvider = pTP;
  m_pMonthCal->SetThemeProvider(pTP);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimePickerImp::GetCurSel(int32_t& iYear,
                                          int32_t& iMonth,
                                          int32_t& iDay) {
  iYear = m_iYear;
  iMonth = m_iMonth;
  iDay = m_iDay;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimePickerImp::SetCurSel(int32_t iYear,
                                          int32_t iMonth,
                                          int32_t iDay) {
  if (iYear <= 0 || iYear >= 3000)
    return FWL_ERR_Indefinite;
  if (iMonth <= 0 || iMonth >= 13)
    return FWL_ERR_Indefinite;
  if (iDay <= 0 || iDay >= 32)
    return FWL_ERR_Indefinite;
  m_iYear = iYear;
  m_iMonth = iMonth;
  m_iDay = iDay;
  m_pMonthCal->SetSelect(iYear, iMonth, iDay);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimePickerImp::SetEditText(const CFX_WideString& wsText) {
  if (!m_pEdit)
    return FWL_ERR_Indefinite;
  int32_t iRet = m_pEdit->SetText(wsText);
  Repaint(&m_rtClient);
  CFWL_Event_DtpEditChanged ev;
  ev.m_wsText = wsText;
  DispatchEvent(&ev);
  return iRet;
}
FWL_ERR CFWL_DateTimePickerImp::GetEditText(CFX_WideString& wsText,
                                            int32_t nStart,
                                            int32_t nCount) const {
  if (m_pEdit) {
    return m_pEdit->GetText(wsText, nStart, nCount);
  }
  return FWL_ERR_Indefinite;
}
FX_BOOL CFWL_DateTimePickerImp::CanUndo() {
  return m_pEdit->CanUndo();
}
FX_BOOL CFWL_DateTimePickerImp::CanRedo() {
  return m_pEdit->CanRedo();
}
FX_BOOL CFWL_DateTimePickerImp::Undo() {
  return m_pEdit->Undo();
}
FX_BOOL CFWL_DateTimePickerImp::Redo() {
  return m_pEdit->Redo();
}
FX_BOOL CFWL_DateTimePickerImp::CanCopy() {
  int32_t nCount = m_pEdit->CountSelRanges();
  return nCount > 0;
}
FX_BOOL CFWL_DateTimePickerImp::CanCut() {
  if (m_pEdit->GetStylesEx() & FWL_STYLEEXT_EDT_ReadOnly) {
    return FALSE;
  }
  int32_t nCount = m_pEdit->CountSelRanges();
  return nCount > 0;
}
FX_BOOL CFWL_DateTimePickerImp::CanSelectAll() {
  return m_pEdit->GetTextLength() > 0;
}
FX_BOOL CFWL_DateTimePickerImp::Copy(CFX_WideString& wsCopy) {
  return m_pEdit->Copy(wsCopy);
}
FX_BOOL CFWL_DateTimePickerImp::Cut(CFX_WideString& wsCut) {
  return m_pEdit->Cut(wsCut);
}
FX_BOOL CFWL_DateTimePickerImp::Paste(const CFX_WideString& wsPaste) {
  return m_pEdit->Paste(wsPaste);
}
FX_BOOL CFWL_DateTimePickerImp::SelectAll() {
  return m_pEdit->AddSelRange(0) == FWL_ERR_Succeeded;
}
FX_BOOL CFWL_DateTimePickerImp::Delete() {
  return m_pEdit->ClearText() == FWL_ERR_Succeeded;
}
FX_BOOL CFWL_DateTimePickerImp::DeSelect() {
  return m_pEdit->ClearSelections() == FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimePickerImp::GetBBox(CFX_RectF& rect) {
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
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimePickerImp::SetEditLimit(int32_t nLimit) {
  return m_pEdit->SetLimit(nLimit);
}
FWL_ERR CFWL_DateTimePickerImp::ModifyEditStylesEx(FX_DWORD dwStylesExAdded,
                                                   FX_DWORD dwStylesExRemoved) {
  return m_pEdit->ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}
void CFWL_DateTimePickerImp::DrawDropDownButton(CFX_Graphics* pGraphics,
                                                IFWL_ThemeProvider* pTheme,
                                                const CFX_Matrix* pMatrix) {
  if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_DTP_Spin) ==
      FWL_STYLEEXT_DTP_Spin) {
    CFWL_WidgetImpProperties prop;
    prop.m_dwStyleExes |= FWL_STYLEEXE_SPB_Vert;
    prop.m_pParent = m_pInterface;
    prop.m_rtWidget = m_rtBtn;
    IFWL_SpinButton* pSpin = IFWL_SpinButton::Create(prop, m_pInterface);
    pSpin->Initialize();
  } else {
    CFWL_ThemeBackground param;
    param.m_pWidget = m_pInterface;
    param.m_iPart = FWL_PART_DTP_DropDownButton;
    param.m_dwStates = m_iBtnState;
    param.m_pGraphics = pGraphics;
    param.m_rtPart = m_rtBtn;
    if (pMatrix) {
      param.m_matrix.Concat(*pMatrix);
    }
    pTheme->DrawBackground(&param);
  }
}
void CFWL_DateTimePickerImp::FormatDateString(int32_t iYear,
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
void CFWL_DateTimePickerImp::ShowMonthCalendar(FX_BOOL bActivate) {
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
FX_BOOL CFWL_DateTimePickerImp::IsMonthCalendarShowed() {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_IsMonthCalendarShowed();
  }
  if (!m_pForm)
    return FALSE;
  return !(m_pForm->GetStates() & FWL_WGTSTATE_Invisible);
}
void CFWL_DateTimePickerImp::ReSetEditAlignment() {
  if (!m_pEdit)
    return;
  FX_DWORD dwStylExes = m_pProperties->m_dwStyleExes;
  FX_DWORD dwAdd = 0;
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
void CFWL_DateTimePickerImp::ProcessSelChanged(int32_t iYear,
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
  ev.m_pSrcTarget = m_pInterface;
  ev.iYear = m_iYear;
  ev.iMonth = m_iMonth;
  ev.iDay = m_iDay;
  DispatchEvent(&ev);
}
void CFWL_DateTimePickerImp::InitProxyForm() {
  if (m_pForm)
    return;
  if (!m_pMonthCal)
    return;
  CFWL_WidgetImpProperties propForm;
  propForm.m_dwStyles = FWL_WGTSTYLE_Popup;
  propForm.m_dwStates = FWL_WGTSTATE_Invisible;
  propForm.m_pOwner = m_pInterface;
  m_pForm.reset(IFWL_DateTimeForm::Create(propForm, m_pMonthCal.get()));
  m_pForm->Initialize();
  m_pMonthCal->SetParent(m_pForm.get());
}
IFWL_DateTimeEdit* CFWL_DateTimePickerImp::GetDataTimeEdit() {
  return m_pEdit.get();
}
FWL_ERR CFWL_DateTimePickerImp::DisForm_Initialize() {
  m_pProperties->m_dwStyleExes = FWL_STYLEEXT_DTP_ShortDateFormat;
  DisForm_InitDateTimeCalendar();
  DisForm_InitDateTimeEdit();
  RegisterEventTarget(m_pMonthCal.get());
  RegisterEventTarget(m_pEdit.get());
  return FWL_ERR_Succeeded;
}
void CFWL_DateTimePickerImp::DisForm_InitDateTimeCalendar() {
  if (m_pMonthCal) {
    return;
  }
  CFWL_WidgetImpProperties propMonth;
  propMonth.m_dwStyles =
      FWL_WGTSTYLE_Popup | FWL_WGTSTYLE_Border | FWL_WGTSTYLE_EdgeSunken;
  propMonth.m_dwStates = FWL_WGTSTATE_Invisible;
  propMonth.m_pParent = m_pInterface;
  propMonth.m_pDataProvider = &m_MonthCalendarDP;
  propMonth.m_pThemeProvider = m_pProperties->m_pThemeProvider;
  m_pMonthCal.reset(IFWL_DateTimeCalender::Create(propMonth, m_pInterface));
  m_pMonthCal->Initialize();
  CFX_RectF rtMonthCal;
  m_pMonthCal->GetWidgetRect(rtMonthCal, TRUE);
  rtMonthCal.Set(0, 0, rtMonthCal.width, rtMonthCal.height);
  m_pMonthCal->SetWidgetRect(rtMonthCal);
}
void CFWL_DateTimePickerImp::DisForm_InitDateTimeEdit() {
  if (m_pEdit) {
    return;
  }
  CFWL_WidgetImpProperties propEdit;
  propEdit.m_pParent = m_pInterface;
  propEdit.m_pThemeProvider = m_pProperties->m_pThemeProvider;
  m_pEdit.reset(IFWL_DateTimeEdit::Create(propEdit, m_pInterface));
  m_pEdit->Initialize();
}
FX_BOOL CFWL_DateTimePickerImp::DisForm_IsMonthCalendarShowed() {
  if (!m_pMonthCal)
    return FALSE;
  return !(m_pMonthCal->GetStates() & FWL_WGTSTATE_Invisible);
}
void CFWL_DateTimePickerImp::DisForm_ShowMonthCalendar(FX_BOOL bActivate) {
  FX_BOOL bShowed = IsMonthCalendarShowed();
  if (bShowed == bActivate) {
    return;
  }
  if (bActivate) {
    CFX_RectF rtMonthCal;
    m_pMonthCal->GetWidgetRect(rtMonthCal, TRUE);
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
    IFWL_WidgetDelegate* pDelegate = m_pEdit->SetDelegate(NULL);
    pDelegate->OnProcessMessage(&msg);
  }
  CFX_RectF rtInvalidate, rtCal;
  rtInvalidate.Set(0, 0, m_pProperties->m_rtWidget.width,
                   m_pProperties->m_rtWidget.height);
  m_pMonthCal->GetWidgetRect(rtCal);
  rtInvalidate.Union(rtCal);
  rtInvalidate.Inflate(2, 2);
  Repaint(&rtInvalidate);
}
FX_DWORD CFWL_DateTimePickerImp::DisForm_HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  CFX_RectF rect;
  rect.Set(0, 0, m_pProperties->m_rtWidget.width,
           m_pProperties->m_rtWidget.height);
  if (rect.Contains(fx, fy)) {
    return FWL_WGTHITTEST_Edit;
  }
  if (DisForm_IsNeedShowButton()) {
    rect.width += m_fBtn;
  }
  if (rect.Contains(fx, fy)) {
    return FWL_WGTHITTEST_Client;
  }
  if (IsMonthCalendarShowed()) {
    m_pMonthCal->GetWidgetRect(rect);
    if (rect.Contains(fx, fy)) {
      return FWL_WGTHITTEST_Client;
    }
  }
  return FWL_WGTHITTEST_Unknown;
}
FX_BOOL CFWL_DateTimePickerImp::DisForm_IsNeedShowButton() {
  FX_BOOL bFocus = m_pProperties->m_dwStates & FWL_WGTSTATE_Focused ||
                   m_pMonthCal->GetStates() & FWL_WGTSTATE_Focused ||
                   m_pEdit->GetStates() & FWL_WGTSTATE_Focused;
  return bFocus;
}
FWL_ERR CFWL_DateTimePickerImp::DisForm_Update() {
  if (m_iLock) {
    return FWL_ERR_Indefinite;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  m_pEdit->SetThemeProvider(m_pProperties->m_pThemeProvider);
  GetClientRect(m_rtClient);
  m_pEdit->SetWidgetRect(m_rtClient);
  ReSetEditAlignment();
  m_pEdit->Update();
  if (m_pMonthCal->GetThemeProvider() == NULL) {
    m_pMonthCal->SetThemeProvider(m_pProperties->m_pThemeProvider);
  }
  if (m_pProperties->m_pDataProvider) {
    IFWL_DateTimePickerDP* pData =
        static_cast<IFWL_DateTimePickerDP*>(m_pProperties->m_pDataProvider);
    pData->GetToday(m_pInterface, m_MonthCalendarDP.m_iCurYear,
                    m_MonthCalendarDP.m_iCurMonth, m_MonthCalendarDP.m_iCurDay);
  }
  FX_FLOAT* pWidth =
      static_cast<FX_FLOAT*>(GetThemeCapacity(FWL_WGTCAPACITY_ScrollBarWidth));
  if (!pWidth)
    return 0;
  m_fBtn = *pWidth;
  CFX_RectF rtMonthCal;
  m_pMonthCal->GetWidgetRect(rtMonthCal, TRUE);
  CFX_RectF rtPopUp;
  rtPopUp.Set(rtMonthCal.left, rtMonthCal.top + FWL_DTP_HEIGHT,
              rtMonthCal.width, rtMonthCal.height);
  m_pMonthCal->SetWidgetRect(rtPopUp);
  m_pMonthCal->Update();
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimePickerImp::DisForm_GetWidgetRect(CFX_RectF& rect,
                                                      FX_BOOL bAutoSize) {
  rect = m_pProperties->m_rtWidget;
  if (DisForm_IsNeedShowButton()) {
    rect.width += m_fBtn;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimePickerImp::DisForm_GetBBox(CFX_RectF& rect) {
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
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_DateTimePickerImp::DisForm_DrawWidget(CFX_Graphics* pGraphics,
                                                   const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_ERR_Indefinite;
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
  return FWL_ERR_Succeeded;
}
CFWL_DateTimePickerImpDelegate::CFWL_DateTimePickerImpDelegate(
    CFWL_DateTimePickerImp* pOwner)
    : m_pOwner(pOwner) {}
int32_t CFWL_DateTimePickerImpDelegate::OnProcessMessage(
    CFWL_Message* pMessage) {
  if (!pMessage)
    return 0;
  FX_DWORD dwMsgCode = pMessage->GetClassID();
  int32_t iRet = 1;
  switch (dwMsgCode) {
    case FWL_MSGHASH_SetFocus:
    case FWL_MSGHASH_KillFocus: {
      OnFocusChanged(pMessage, dwMsgCode == FWL_MSGHASH_SetFocus);
      break;
    }
    case FWL_MSGHASH_Mouse: {
      CFWL_MsgMouse* pMouse = static_cast<CFWL_MsgMouse*>(pMessage);
      FX_DWORD dwCmd = pMouse->m_dwCmd;
      switch (dwCmd) {
        case FWL_MSGMOUSECMD_LButtonDown: {
          OnLButtonDown(pMouse);
          break;
        }
        case FWL_MSGMOUSECMD_LButtonUp: {
          OnLButtonUp(pMouse);
          break;
        }
        case FWL_MSGMOUSECMD_MouseMove: {
          OnMouseMove(pMouse);
          break;
        }
        case FWL_MSGMOUSECMD_MouseLeave: {
          OnMouseLeave(pMouse);
          break;
        }
        default: {}
      }
      break;
    }
    default: { iRet = 0; }
  }
  if (dwMsgCode == FWL_MSGHASH_Key &&
      m_pOwner->m_pEdit->GetStates() & FWL_WGTSTATE_Focused) {
    IFWL_WidgetDelegate* pDelegate = m_pOwner->m_pEdit->SetDelegate(NULL);
    return pDelegate->OnProcessMessage(pMessage);
  }
  return CFWL_WidgetImpDelegate::OnProcessMessage(pMessage);
}
FWL_ERR CFWL_DateTimePickerImpDelegate::OnDrawWidget(
    CFX_Graphics* pGraphics,
    const CFX_Matrix* pMatrix) {
  return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
void CFWL_DateTimePickerImpDelegate::OnFocusChanged(CFWL_Message* pMsg,
                                                    FX_BOOL bSet) {
  if (!pMsg)
    return;
  if (m_pOwner->m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_OnFocusChanged(pMsg, bSet);
  }
  if (bSet) {
    m_pOwner->m_pProperties->m_dwStates |= (FWL_WGTSTATE_Focused);
    m_pOwner->Repaint(&m_pOwner->m_rtClient);
  } else {
    m_pOwner->m_pProperties->m_dwStates &= ~(FWL_WGTSTATE_Focused);
    m_pOwner->Repaint(&m_pOwner->m_rtClient);
  }
  if (pMsg->m_pSrcTarget == m_pOwner->m_pMonthCal.get() &&
      m_pOwner->IsMonthCalendarShowed()) {
    m_pOwner->ShowMonthCalendar(FALSE);
  }
  m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_DateTimePickerImpDelegate::OnLButtonDown(CFWL_MsgMouse* pMsg) {
  if (!pMsg)
    return;
  if ((m_pOwner->m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0) {
    m_pOwner->SetFocus(TRUE);
  }
  if (m_pOwner->m_rtBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    if (m_pOwner->IsMonthCalendarShowed()) {
      m_pOwner->ShowMonthCalendar(FALSE);
      CFWL_Event_DtpCloseUp ev;
      m_pOwner->DispatchEvent(&ev);
    } else {
      if (!(m_pOwner->m_pProperties->m_dwStyleExes &
            FWL_STYLEEXT_DTP_TimeFormat)) {
        m_pOwner->ShowMonthCalendar(TRUE);
        CFWL_Event_DtpDropDown ev;
        m_pOwner->DispatchEvent(&ev);
      } else {
      }
      m_pOwner->m_bLBtnDown = TRUE;
      m_pOwner->Repaint(&m_pOwner->m_rtClient);
    }
  }
}
void CFWL_DateTimePickerImpDelegate::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  if (!pMsg)
    return;
  m_pOwner->m_bLBtnDown = FALSE;
  if (m_pOwner->m_rtBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iBtnState = FWL_PARTSTATE_DTP_Hovered;
  } else {
    m_pOwner->m_iBtnState = FWL_PARTSTATE_DTP_Normal;
  }
  m_pOwner->Repaint(&m_pOwner->m_rtBtn);
}
void CFWL_DateTimePickerImpDelegate::OnMouseMove(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_rtBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
  } else {
    m_pOwner->m_iBtnState = FWL_PARTSTATE_DTP_Normal;
  }
  m_pOwner->Repaint(&m_pOwner->m_rtBtn);
}
void CFWL_DateTimePickerImpDelegate::OnMouseLeave(CFWL_MsgMouse* pMsg) {
  if (!pMsg)
    return;
  m_pOwner->m_iBtnState = FWL_PARTSTATE_DTP_Normal;
  m_pOwner->Repaint(&m_pOwner->m_rtBtn);
}
void CFWL_DateTimePickerImpDelegate::DisForm_OnFocusChanged(CFWL_Message* pMsg,
                                                            FX_BOOL bSet) {
  CFX_RectF rtInvalidate(m_pOwner->m_rtBtn);
  if (bSet) {
    m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
    if (m_pOwner->m_pEdit &&
        !(m_pOwner->m_pEdit->GetStylesEx() & FWL_STYLEEXT_EDT_ReadOnly)) {
      m_pOwner->m_rtBtn.Set(m_pOwner->m_pProperties->m_rtWidget.width, 0,
                            m_pOwner->m_fBtn,
                            m_pOwner->m_pProperties->m_rtWidget.height - 1);
    }
    rtInvalidate = m_pOwner->m_rtBtn;
    pMsg->m_pDstTarget = m_pOwner->m_pEdit.get();
    IFWL_WidgetDelegate* pDelegate = m_pOwner->m_pEdit->SetDelegate(NULL);
    pDelegate->OnProcessMessage(pMsg);
  } else {
    m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
    m_pOwner->m_rtBtn.Set(0, 0, 0, 0);
    if (m_pOwner->DisForm_IsMonthCalendarShowed()) {
      m_pOwner->ShowMonthCalendar(FALSE);
    }
    if (m_pOwner->m_pEdit->GetStates() & FWL_WGTSTATE_Focused) {
      pMsg->m_pSrcTarget = m_pOwner->m_pEdit.get();
      IFWL_WidgetDelegate* pDelegate = m_pOwner->m_pEdit->SetDelegate(NULL);
      pDelegate->OnProcessMessage(pMsg);
    }
  }
  rtInvalidate.Inflate(2, 2);
  m_pOwner->Repaint(&rtInvalidate);
}
