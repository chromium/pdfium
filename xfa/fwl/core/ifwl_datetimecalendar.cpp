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
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter)
    : IFWL_MonthCalendar(app, properties, pOuter) {
  SetDelegate(pdfium::MakeUnique<CFWL_DateTimeCalendarImpDelegate>(this));
}

CFWL_DateTimeCalendarImpDelegate::CFWL_DateTimeCalendarImpDelegate(
    IFWL_DateTimeCalendar* pOwner)
    : CFWL_MonthCalendarImpDelegate(pOwner), m_pOwner(pOwner) {
  m_bFlag = FALSE;
}

void CFWL_DateTimeCalendarImpDelegate::OnProcessMessage(
    CFWL_Message* pMessage) {
  CFWL_MessageType dwCode = pMessage->GetClassID();
  if (dwCode == CFWL_MessageType::SetFocus ||
      dwCode == CFWL_MessageType::KillFocus) {
    IFWL_Widget* pOuter = m_pOwner->GetOuter();
    pOuter->GetCurrentDelegate()->OnProcessMessage(pMessage);
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
  CFWL_MonthCalendarImpDelegate::OnProcessMessage(pMessage);
}

void CFWL_DateTimeCalendarImpDelegate::OnLButtonDownEx(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_rtLBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iLBtnPartStates = CFWL_PartState_Pressed;
    m_pOwner->PrevMonth();
    m_pOwner->Repaint(&m_pOwner->m_rtClient);
  } else if (m_pOwner->m_rtRBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iRBtnPartStates |= CFWL_PartState_Pressed;
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
    if (pIPicker->IsMonthCalendarShowed()) {
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
  pIPicker->m_pForm->GetWidgetRect(rt);
  rt.Set(0, 0, rt.width, rt.height);
  if (iCurSel > 0) {
    FWL_DATEINFO* lpDatesInfo = m_pOwner->m_arrDates.GetAt(iCurSel - 1);
    CFX_RectF rtInvalidate(lpDatesInfo->rect);
    if (iOldSel > 0 && iOldSel <= m_pOwner->m_arrDates.GetSize()) {
      lpDatesInfo = m_pOwner->m_arrDates.GetAt(iOldSel - 1);
      rtInvalidate.Union(lpDatesInfo->rect);
    }
    m_pOwner->AddSelDay(iCurSel);
    if (!m_pOwner->m_pOuter)
      return;
    pIPicker->ProcessSelChanged(m_pOwner->m_iCurYear, m_pOwner->m_iCurMonth,
                                iCurSel);
    pIPicker->ShowMonthCalendar(FALSE);
  } else if (m_bFlag && (!rt.Contains(pMsg->m_fx, pMsg->m_fy))) {
    pIPicker->ShowMonthCalendar(FALSE);
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

void CFWL_DateTimeCalendarImpDelegate::DisForm_OnProcessMessage(
    CFWL_Message* pMessage) {
  if (pMessage->GetClassID() == CFWL_MessageType::Mouse) {
    CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
    if (pMsg->m_dwCmd == FWL_MouseCommand::LeftButtonUp) {
      DisForm_OnLButtonUpEx(pMsg);
      return;
    }
  }
  CFWL_MonthCalendarImpDelegate::OnProcessMessage(pMessage);
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
    FWL_DATEINFO* lpDatesInfo = m_pOwner->m_arrDates.GetAt(iCurSel - 1);
    CFX_RectF rtInvalidate(lpDatesInfo->rect);
    if (iOldSel > 0 && iOldSel <= m_pOwner->m_arrDates.GetSize()) {
      lpDatesInfo = m_pOwner->m_arrDates.GetAt(iOldSel - 1);
      rtInvalidate.Union(lpDatesInfo->rect);
    }
    m_pOwner->AddSelDay(iCurSel);
    IFWL_DateTimePicker* pDateTime =
        static_cast<IFWL_DateTimePicker*>(m_pOwner->m_pOuter);
    pDateTime->ProcessSelChanged(m_pOwner->m_iCurYear, m_pOwner->m_iCurMonth,
                                 iCurSel);
    pDateTime->ShowMonthCalendar(FALSE);
  }
}
