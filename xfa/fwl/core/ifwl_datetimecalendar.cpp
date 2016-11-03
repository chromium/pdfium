// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_datetimecalendar.h"

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/core/cfwl_widgetmgr.h"
#include "xfa/fwl/core/ifwl_datetimepicker.h"
#include "xfa/fwl/core/ifwl_formproxy.h"

IFWL_DateTimeCalendar::IFWL_DateTimeCalendar(
    const IFWL_App* app,
    std::unique_ptr<CFWL_WidgetProperties> properties,
    IFWL_Widget* pOuter)
    : IFWL_MonthCalendar(app, std::move(properties), pOuter), m_bFlag(false) {}

void IFWL_DateTimeCalendar::OnProcessMessage(CFWL_Message* pMessage) {
  CFWL_MessageType dwCode = pMessage->GetClassID();
  if (dwCode == CFWL_MessageType::SetFocus ||
      dwCode == CFWL_MessageType::KillFocus) {
    IFWL_Widget* pOuter = GetOuter();
    pOuter->GetDelegate()->OnProcessMessage(pMessage);
    return;
  }
  if (dwCode == CFWL_MessageType::Mouse) {
    CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
    if (pMsg->m_dwCmd == FWL_MouseCommand::LeftButtonDown)
      OnLButtonDownEx(pMsg);
    else if (pMsg->m_dwCmd == FWL_MouseCommand::LeftButtonUp)
      OnLButtonUpEx(pMsg);
    return;
  }
  IFWL_MonthCalendar::OnProcessMessage(pMessage);
}

void IFWL_DateTimeCalendar::OnLButtonDownEx(CFWL_MsgMouse* pMsg) {
  if (m_rtLBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_iLBtnPartStates = CFWL_PartState_Pressed;
    PrevMonth();
    Repaint(&m_rtClient);
  } else if (m_rtRBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_iRBtnPartStates |= CFWL_PartState_Pressed;
    NextMonth();
    Repaint(&m_rtClient);
  } else if (m_rtToday.Contains(pMsg->m_fx, pMsg->m_fy)) {
    if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_NoToday) == 0) {
      JumpToToday();
      Repaint(&m_rtClient);
    }
  } else {
    IFWL_DateTimePicker* pIPicker = static_cast<IFWL_DateTimePicker*>(m_pOuter);
    if (pIPicker->IsMonthCalendarShowed())
      m_bFlag = 1;
  }
}

void IFWL_DateTimeCalendar::OnLButtonUpEx(CFWL_MsgMouse* pMsg) {
  if (m_pWidgetMgr->IsFormDisabled())
    return DisForm_OnLButtonUpEx(pMsg);
  if (m_rtLBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_iLBtnPartStates = 0;
    Repaint(&m_rtLBtn);
    return;
  }
  if (m_rtRBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_iRBtnPartStates = 0;
    Repaint(&m_rtRBtn);
    return;
  }
  if (m_rtToday.Contains(pMsg->m_fx, pMsg->m_fy))
    return;

  int32_t iOldSel = 0;
  if (m_arrSelDays.GetSize() > 0)
    iOldSel = m_arrSelDays[0];

  int32_t iCurSel = GetDayAtPoint(pMsg->m_fx, pMsg->m_fy);
  CFX_RectF rt;
  IFWL_DateTimePicker* pIPicker = static_cast<IFWL_DateTimePicker*>(m_pOuter);
  pIPicker->GetFormProxy()->GetWidgetRect(rt);
  rt.Set(0, 0, rt.width, rt.height);
  if (iCurSel > 0) {
    FWL_DATEINFO* lpDatesInfo = m_arrDates.GetAt(iCurSel - 1);
    CFX_RectF rtInvalidate(lpDatesInfo->rect);
    if (iOldSel > 0 && iOldSel <= m_arrDates.GetSize()) {
      lpDatesInfo = m_arrDates.GetAt(iOldSel - 1);
      rtInvalidate.Union(lpDatesInfo->rect);
    }
    AddSelDay(iCurSel);
    if (!m_pOuter)
      return;

    pIPicker->ProcessSelChanged(m_iCurYear, m_iCurMonth, iCurSel);
    pIPicker->ShowMonthCalendar(false);
  } else if (m_bFlag && (!rt.Contains(pMsg->m_fx, pMsg->m_fy))) {
    pIPicker->ShowMonthCalendar(false);
  }
  m_bFlag = 0;
}

void IFWL_DateTimeCalendar::OnMouseMoveEx(CFWL_MsgMouse* pMsg) {
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_MultiSelect)
    return;

  bool bRepaint = false;
  CFX_RectF rtInvalidate;
  rtInvalidate.Set(0, 0, 0, 0);
  if (m_rtDates.Contains(pMsg->m_fx, pMsg->m_fy)) {
    int32_t iHover = GetDayAtPoint(pMsg->m_fx, pMsg->m_fy);
    bRepaint = m_iHovered != iHover;
    if (bRepaint) {
      if (m_iHovered > 0)
        GetDayRect(m_iHovered, rtInvalidate);
      if (iHover > 0) {
        CFX_RectF rtDay;
        GetDayRect(iHover, rtDay);
        if (rtInvalidate.IsEmpty())
          rtInvalidate = rtDay;
        else
          rtInvalidate.Union(rtDay);
      }
    }
    m_iHovered = iHover;
    CFWL_Event_DtpHoverChanged ev;
    ev.hoverday = iHover;
    DispatchEvent(&ev);
  } else {
    bRepaint = m_iHovered > 0;
    if (bRepaint)
      GetDayRect(m_iHovered, rtInvalidate);

    m_iHovered = -1;
  }
  if (bRepaint && !rtInvalidate.IsEmpty())
    Repaint(&rtInvalidate);
}

void IFWL_DateTimeCalendar::DisForm_OnProcessMessage(CFWL_Message* pMessage) {
  if (pMessage->GetClassID() == CFWL_MessageType::Mouse) {
    CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
    if (pMsg->m_dwCmd == FWL_MouseCommand::LeftButtonUp) {
      DisForm_OnLButtonUpEx(pMsg);
      return;
    }
  }
  IFWL_MonthCalendar::OnProcessMessage(pMessage);
}

void IFWL_DateTimeCalendar::DisForm_OnLButtonUpEx(CFWL_MsgMouse* pMsg) {
  if (m_rtLBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_iLBtnPartStates = 0;
    Repaint(&(m_rtLBtn));
    return;
  }
  if (m_rtRBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_iRBtnPartStates = 0;
    Repaint(&(m_rtRBtn));
    return;
  }
  if (m_rtToday.Contains(pMsg->m_fx, pMsg->m_fy))
    return;

  int32_t iOldSel = 0;
  if (m_arrSelDays.GetSize() > 0)
    iOldSel = m_arrSelDays[0];

  int32_t iCurSel = GetDayAtPoint(pMsg->m_fx, pMsg->m_fy);
  if (iCurSel > 0) {
    FWL_DATEINFO* lpDatesInfo = m_arrDates.GetAt(iCurSel - 1);
    CFX_RectF rtInvalidate(lpDatesInfo->rect);
    if (iOldSel > 0 && iOldSel <= m_arrDates.GetSize()) {
      lpDatesInfo = m_arrDates.GetAt(iOldSel - 1);
      rtInvalidate.Union(lpDatesInfo->rect);
    }
    AddSelDay(iCurSel);
    IFWL_DateTimePicker* pDateTime =
        static_cast<IFWL_DateTimePicker*>(m_pOuter);
    pDateTime->ProcessSelChanged(m_iCurYear, m_iCurMonth, iCurSel);
    pDateTime->ShowMonthCalendar(false);
  }
}
