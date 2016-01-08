// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_monthcalendarimp.h"
#define MONTHCAL_HSEP_HEIGHT 1
#define MONTHCAL_VSEP_WIDTH 1
#define MONTHCAL_HMARGIN 3
#define MONTHCAL_VMARGIN 2
#define MONTHCAL_ROWS 9
#define MONTHCAL_COLUMNS 7
#define MONTHCAL_HEADER_BTN_VMARGIN 7
#define MONTHCAL_HEADER_BTN_HMARGIN 5

// static
IFWL_MonthCalendar* IFWL_MonthCalendar::Create(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter) {
  IFWL_MonthCalendar* pMonthCalendar = new IFWL_MonthCalendar;
  CFWL_MonthCalendarImp* pMonthCalendarImpl =
      new CFWL_MonthCalendarImp(properties, pOuter);
  pMonthCalendar->SetImpl(pMonthCalendarImpl);
  pMonthCalendarImpl->SetInterface(pMonthCalendar);
  return pMonthCalendar;
}
IFWL_MonthCalendar::IFWL_MonthCalendar() {}
int32_t IFWL_MonthCalendar::CountSelect() {
  return static_cast<CFWL_MonthCalendarImp*>(GetImpl())->CountSelect();
}
FX_BOOL IFWL_MonthCalendar::GetSelect(int32_t& iYear,
                                      int32_t& iMonth,
                                      int32_t& iDay,
                                      int32_t nIndex) {
  return static_cast<CFWL_MonthCalendarImp*>(GetImpl())
      ->GetSelect(iYear, iMonth, iDay, nIndex);
}
FX_BOOL IFWL_MonthCalendar::SetSelect(int32_t iYear,
                                      int32_t iMonth,
                                      int32_t iDay) {
  return static_cast<CFWL_MonthCalendarImp*>(GetImpl())
      ->SetSelect(iYear, iMonth, iDay);
}

CFWL_MonthCalendarImp::CFWL_MonthCalendarImp(
    const CFWL_WidgetImpProperties& properties,
    IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter),
      m_iCurYear(2011),
      m_iCurMonth(1),
      m_iYear(2011),
      m_iMonth(1),
      m_iDay(1),
      m_iHovered(-1),
      m_iLBtnPartStates(FWL_PARTSTATE_MCD_Normal),
      m_iRBtnPartStates(FWL_PARTSTATE_MCD_Normal) {
  m_rtHead.Reset();
  m_rtWeek.Reset();
  m_rtLBtn.Reset();
  m_rtRBtn.Reset();
  m_rtDates.Reset();
  m_rtHSep.Reset();
  m_rtHeadText.Reset();
  m_rtToday.Reset();
  m_rtTodayFlag.Reset();
  m_rtClient.Reset();
  m_rtWeekNum.Reset();
  m_rtWeekNumSep.Reset();
  m_szHead.Reset();
  m_szCell.Reset();
  m_szToday.Reset();
  m_pDateTime = new CFX_DateTime;
  m_bInit = FALSE;
  m_iMaxSel = 1;
}
CFWL_MonthCalendarImp::~CFWL_MonthCalendarImp() {
  ClearDateItem();
  delete m_pDateTime;
  m_arrSelDays.RemoveAll();
}
FWL_ERR CFWL_MonthCalendarImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_MonthCalendar;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_MonthCalendarImp::GetClassID() const {
  return FWL_CLASSHASH_MonthCalendar;
}
FWL_ERR CFWL_MonthCalendarImp::Initialize() {
  if (CFWL_WidgetImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  m_pDelegate = new CFWL_MonthCalendarImpDelegate(this);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_MonthCalendarImp::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return CFWL_WidgetImp::Finalize();
}
FWL_ERR CFWL_MonthCalendarImp::GetWidgetRect(CFX_RectF& rect,
                                             FX_BOOL bAutoSize) {
  if (bAutoSize) {
    CFX_SizeF fs = CalcSize(TRUE);
    rect.Set(0, 0, fs.x, fs.y);
    CFWL_WidgetImp::GetWidgetRect(rect, TRUE);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_MonthCalendarImp::Update() {
  if (IsLocked()) {
    return FWL_ERR_Indefinite;
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  GetCapValue();
  if (!m_bInit) {
    m_bInit = InitDate();
  }
  ClearDateItem();
  ReSetDateItem();
  LayOut();
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_MonthCalendarImp::DrawWidget(CFX_Graphics* pGraphics,
                                          const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  if (m_pProperties->m_pThemeProvider == NULL) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  if (HasBorder()) {
    DrawBorder(pGraphics, FWL_PART_MCD_Border, pTheme, pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, FWL_PART_MCD_Edge, pTheme, pMatrix);
  }
  DrawBkground(pGraphics, pTheme, pMatrix);
  DrawHeadBK(pGraphics, pTheme, pMatrix);
  DrawLButton(pGraphics, pTheme, pMatrix);
  DrawRButton(pGraphics, pTheme, pMatrix);
  DrawSeperator(pGraphics, pTheme, pMatrix);
  DrawDatesInBK(pGraphics, pTheme, pMatrix);
  DrawDatesInCircle(pGraphics, pTheme, pMatrix);
  DrawCaption(pGraphics, pTheme, pMatrix);
  DrawWeek(pGraphics, pTheme, pMatrix);
  DrawDatesIn(pGraphics, pTheme, pMatrix);
  DrawDatesOut(pGraphics, pTheme, pMatrix);
  DrawToday(pGraphics, pTheme, pMatrix);
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_WeekNumbers) {
    DrawWeekNumberSep(pGraphics, pTheme, pMatrix);
    DrawWeekNumber(pGraphics, pTheme, pMatrix);
  }
  return FWL_ERR_Succeeded;
}
int32_t CFWL_MonthCalendarImp::CountSelect() {
  return m_arrSelDays.GetSize();
}
FX_BOOL CFWL_MonthCalendarImp::GetSelect(int32_t& iYear,
                                         int32_t& iMonth,
                                         int32_t& iDay,
                                         int32_t nIndex) {
  if (nIndex >= m_arrSelDays.GetSize()) {
    return FALSE;
  }
  iYear = m_iCurYear;
  iMonth = m_iCurMonth;
  iDay = m_arrSelDays[nIndex];
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarImp::SetSelect(int32_t iYear,
                                         int32_t iMonth,
                                         int32_t iDay) {
  ChangeToMonth(iYear, iMonth);
  return AddSelDay(iDay);
}
void CFWL_MonthCalendarImp::DrawBkground(CFX_Graphics* pGraphics,
                                         IFWL_ThemeProvider* pTheme,
                                         const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_Background;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  params.m_rtPart = m_rtClient;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawBackground(&params);
}
void CFWL_MonthCalendarImp::DrawHeadBK(CFX_Graphics* pGraphics,
                                       IFWL_ThemeProvider* pTheme,
                                       const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_Header;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  params.m_rtPart = m_rtHead;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawBackground(&params);
}
void CFWL_MonthCalendarImp::DrawLButton(CFX_Graphics* pGraphics,
                                        IFWL_ThemeProvider* pTheme,
                                        const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_LBtn;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = m_iLBtnPartStates;
  params.m_rtPart = m_rtLBtn;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawBackground(&params);
}
void CFWL_MonthCalendarImp::DrawRButton(CFX_Graphics* pGraphics,
                                        IFWL_ThemeProvider* pTheme,
                                        const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_RBtn;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = m_iRBtnPartStates;
  params.m_rtPart = m_rtRBtn;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawBackground(&params);
}
void CFWL_MonthCalendarImp::DrawCaption(CFX_Graphics* pGraphics,
                                        IFWL_ThemeProvider* pTheme,
                                        const CFX_Matrix* pMatrix) {
  CFWL_ThemeText textParam;
  textParam.m_pWidget = m_pInterface;
  textParam.m_iPart = FWL_PART_MCD_Caption;
  textParam.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  textParam.m_pGraphics = pGraphics;
  int32_t iYear;
  int32_t iMonth;
  iYear = m_iCurYear;
  iMonth = m_iCurMonth;
  CFX_WideString wsCation;
  GetHeadText(iYear, iMonth, wsCation);
  textParam.m_wsText = wsCation;
  m_szHead = CalcTextSize(textParam.m_wsText, m_pProperties->m_pThemeProvider);
  CalcHeadSize();
  textParam.m_rtPart = m_rtHeadText;
  textParam.m_dwTTOStyles = FDE_TTOSTYLE_SingleLine;
  textParam.m_iTTOAlign = FDE_TTOALIGNMENT_Center;
  if (pMatrix) {
    textParam.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawText(&textParam);
}
void CFWL_MonthCalendarImp::DrawSeperator(CFX_Graphics* pGraphics,
                                          IFWL_ThemeProvider* pTheme,
                                          const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_HSeparator;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  params.m_rtPart = m_rtHSep;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawBackground(&params);
}
void CFWL_MonthCalendarImp::DrawDatesInBK(CFX_Graphics* pGraphics,
                                          IFWL_ThemeProvider* pTheme,
                                          const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_DateInBK;
  params.m_pGraphics = pGraphics;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  int32_t iCount = m_arrDates.GetSize();
  for (int32_t j = 0; j < iCount; j++) {
    LPDATEINFO pDataInfo = (LPDATEINFO)m_arrDates.GetAt(j);
    if (pDataInfo->dwStates & FWL_ITEMSTATE_MCD_Selected) {
      params.m_dwStates |= FWL_PARTSTATE_MCD_Selected;
      if (((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_NoTodayCircle) ==
           0) &&
          pDataInfo->dwStates & FWL_ITEMSTATE_MCD_Flag) {
        params.m_dwStates |= FWL_PARTSTATE_MCD_Flagged;
      }
      if (pDataInfo->dwStates & FWL_ITEMSTATE_MCD_Focused) {
        params.m_dwStates |= FWL_PARTSTATE_MCD_Focused;
      }
    } else if (j == m_iHovered - 1) {
      params.m_dwStates |= FWL_PARTSTATE_MCD_Hovered;
    } else if (pDataInfo->dwStates & FWL_ITEMSTATE_MCD_Flag) {
      params.m_dwStates = FWL_PARTSTATE_MCD_Flagged;
      pTheme->DrawBackground(&params);
    }
    params.m_rtPart = pDataInfo->rect;
    pTheme->DrawBackground(&params);
    params.m_dwStates = 0;
  }
}
void CFWL_MonthCalendarImp::DrawWeek(CFX_Graphics* pGraphics,
                                     IFWL_ThemeProvider* pTheme,
                                     const CFX_Matrix* pMatrix) {
  CFWL_ThemeText params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_Week;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  params.m_iTTOAlign = FDE_TTOALIGNMENT_Center;
  int32_t iWeek;
  iWeek = m_pDateTime->GetDayOfWeek();
  CFX_RectF rtDayOfWeek;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  for (int32_t i = 0; i < 7; i++) {
    rtDayOfWeek.Set(m_rtWeek.left + i * (m_szCell.x + MONTHCAL_HMARGIN * 2),
                    m_rtWeek.top, m_szCell.x, m_szCell.y);
    CFX_WideString* wsWeekDay = static_cast<CFX_WideString*>(
        pTheme->GetCapacity(&params, i + FWL_MCCAPACITY_Sun));
    params.m_rtPart = rtDayOfWeek;
    params.m_wsText = *wsWeekDay;
    params.m_dwTTOStyles = FDE_TTOSTYLE_SingleLine;
    pTheme->DrawText(&params);
  }
}
void CFWL_MonthCalendarImp::DrawWeekNumber(CFX_Graphics* pGraphics,
                                           IFWL_ThemeProvider* pTheme,
                                           const CFX_Matrix* pMatrix) {
  CFWL_ThemeText params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_WeekNum;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  params.m_iTTOAlign = FDE_TTOALIGNMENT_CenterLeft;
  CFX_WideString wsWeekNum;
  params.m_dwTTOStyles = FDE_TTOSTYLE_SingleLine;
  params.m_iTTOAlign = FDE_TTOALIGNMENT_Center;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  int32_t iWeekNum = 0;
  int32_t iMonthNum = m_pDateTime->GetMonth();
  int32_t iDayNum = FX_DaysInMonth(m_iCurYear, iMonthNum);
  int32_t iTemp = 0;
  FX_FLOAT fVStartPos = m_rtClient.top + m_fHeadHei + m_fHSepHei;
  FX_FLOAT fHStartPos = m_rtClient.left;
  for (int32_t i = 1; i <= iDayNum; i += 7) {
    iTemp++;
    iWeekNum = CalWeekNumber(m_iCurYear, iMonthNum, i);
    m_rtWeekNum.Set(fHStartPos, fVStartPos + m_fDateCellHei * iTemp,
                    m_fWeekNumWid, m_fDateCellHei);
    wsWeekNum.Format(L"%d", iWeekNum);
    params.m_wsText = wsWeekNum;
    params.m_rtPart = m_rtWeekNum;
    pTheme->DrawText(&params);
  }
}
void CFWL_MonthCalendarImp::DrawWeekNumberSep(CFX_Graphics* pGraphics,
                                              IFWL_ThemeProvider* pTheme,
                                              const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_WeekNumSep;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  params.m_rtPart = m_rtWeekNumSep;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawBackground(&params);
}
void CFWL_MonthCalendarImp::DrawToday(CFX_Graphics* pGraphics,
                                      IFWL_ThemeProvider* pTheme,
                                      const CFX_Matrix* pMatrix) {
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_NoToday) {
    return;
  }
  CFWL_ThemeText params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_Today;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  params.m_iTTOAlign = FDE_TTOALIGNMENT_CenterLeft;
  CFX_WideString* wsDay = static_cast<CFX_WideString*>(
      pTheme->GetCapacity(&params, FWL_MCCAPACITY_Today));
  CFX_WideString wsText;
  GetTodayText(m_iYear, m_iMonth, m_iDay, wsText);
  params.m_wsText = *wsDay + wsText;
  m_szToday = CalcTextSize(params.m_wsText, m_pProperties->m_pThemeProvider);
  CalcTodaySize();
  params.m_rtPart = m_rtToday;
  params.m_dwTTOStyles = FDE_TTOSTYLE_SingleLine;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawText(&params);
}
void CFWL_MonthCalendarImp::DrawDatesIn(CFX_Graphics* pGraphics,
                                        IFWL_ThemeProvider* pTheme,
                                        const CFX_Matrix* pMatrix) {
  CFWL_ThemeText params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_DatesIn;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  params.m_iTTOAlign = FDE_TTOALIGNMENT_Center;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  int32_t iCount = m_arrDates.GetSize();
  for (int32_t j = 0; j < iCount; j++) {
    LPDATEINFO pDataInfo = (LPDATEINFO)m_arrDates.GetAt(j);
    params.m_wsText = pDataInfo->wsDay;
    params.m_rtPart = pDataInfo->rect;
    params.m_dwStates = pDataInfo->dwStates;
    if (j + 1 == m_iHovered) {
      params.m_dwStates |= FWL_PARTSTATE_MCD_Hovered;
    }
    params.m_dwTTOStyles = FDE_TTOSTYLE_SingleLine;
    pTheme->DrawText(&params);
  }
}
void CFWL_MonthCalendarImp::DrawDatesOut(CFX_Graphics* pGraphics,
                                         IFWL_ThemeProvider* pTheme,
                                         const CFX_Matrix* pMatrix) {
  CFWL_ThemeText params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_DatesOut;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  params.m_iTTOAlign = FDE_TTOALIGNMENT_Center;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawText(&params);
}
void CFWL_MonthCalendarImp::DrawDatesInCircle(CFX_Graphics* pGraphics,
                                              IFWL_ThemeProvider* pTheme,
                                              const CFX_Matrix* pMatrix) {
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_NoTodayCircle) {
    return;
  }
  if (m_iMonth != m_iCurMonth || m_iYear != m_iCurYear) {
    return;
  }
  if (m_iDay < 1 || m_iDay > m_arrDates.GetSize()) {
    return;
  }
  LPDATEINFO pDate = (LPDATEINFO)m_arrDates[m_iDay - 1];
  if (!pDate)
    return;
  CFWL_ThemeBackground params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_DateInCircle;
  params.m_pGraphics = pGraphics;
  params.m_rtPart = pDate->rect;
  params.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawBackground(&params);
}
void CFWL_MonthCalendarImp::DrawTodayCircle(CFX_Graphics* pGraphics,
                                            IFWL_ThemeProvider* pTheme,
                                            const CFX_Matrix* pMatrix) {
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_NoToday) {
    return;
  }
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_NoTodayCircle) {
    return;
  }
  CFWL_ThemeBackground params;
  params.m_pWidget = m_pInterface;
  params.m_iPart = FWL_PART_MCD_TodayCircle;
  params.m_pGraphics = pGraphics;
  params.m_dwStates = FWL_PARTSTATE_MCD_Normal;
  params.m_rtPart = m_rtTodayFlag;
  if (pMatrix) {
    params.m_matrix.Concat(*pMatrix);
  }
  pTheme->DrawBackground(&params);
}
CFX_SizeF CFWL_MonthCalendarImp::CalcSize(FX_BOOL bAutoSize) {
  CFX_SizeF fs;
  fs.Set(0, 0);
  if (!m_pProperties->m_pThemeProvider)
    return fs;
  if (bAutoSize) {
    CFWL_ThemePart params;
    params.m_pWidget = m_pInterface;
    IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
    CFX_WideString* wsText = NULL;
    FX_FLOAT fMaxWeekW = 0.0f;
    FX_FLOAT fMaxWeekH = 0.0f;
    for (FX_DWORD week = FWL_MCCAPACITY_Sun; week <= FWL_MCCAPACITY_Sat;
         week++) {
      wsText = static_cast<CFX_WideString*>(pTheme->GetCapacity(&params, week));
      CFX_SizeF sz = CalcTextSize(*wsText, m_pProperties->m_pThemeProvider);
      fMaxWeekW = (fMaxWeekW >= sz.x) ? fMaxWeekW : sz.x;
      fMaxWeekH = (fMaxWeekH >= sz.y) ? fMaxWeekH : sz.y;
    }
    FX_FLOAT fDayMaxW = 0.0f;
    FX_FLOAT fDayMaxH = 0.0f;
    for (int day = 10; day <= 31; day++) {
      CFX_WideString wsDay;
      wsDay.Format(L"%d", day);
      CFX_SizeF sz = CalcTextSize(wsDay, m_pProperties->m_pThemeProvider);
      fDayMaxW = (fDayMaxW >= sz.x) ? fDayMaxW : sz.x;
      fDayMaxH = (fDayMaxH >= sz.y) ? fDayMaxH : sz.y;
    }
    m_szCell.x = FX_FLOAT((fMaxWeekW >= fDayMaxW) ? (int)(fMaxWeekW + 0.5)
                                                  : (int)(fDayMaxW + 0.5));
    m_szCell.y = (fMaxWeekH >= fDayMaxH) ? fMaxWeekH : fDayMaxH;
    fs.x = m_szCell.x * MONTHCAL_COLUMNS +
           MONTHCAL_HMARGIN * MONTHCAL_COLUMNS * 2 +
           MONTHCAL_HEADER_BTN_HMARGIN * 2;
    FX_FLOAT fMonthMaxW = 0.0f;
    FX_FLOAT fMonthMaxH = 0.0f;
    for (FX_DWORD month = FWL_MCCAPACITY_January;
         month <= FWL_MCCAPACITY_December; month++) {
      wsText =
          static_cast<CFX_WideString*>(pTheme->GetCapacity(&params, month));
      CFX_SizeF sz = CalcTextSize(*wsText, m_pProperties->m_pThemeProvider);
      fMonthMaxW = (fMonthMaxW >= sz.x) ? fMonthMaxW : sz.x;
      fMonthMaxH = (fMonthMaxH >= sz.y) ? fMonthMaxH : sz.y;
    }
    CFX_WideString wsYear;
    GetHeadText(m_iYear, m_iMonth, wsYear);
    CFX_SizeF szYear = CalcTextSize(wsYear, m_pProperties->m_pThemeProvider);
    fMonthMaxH = (fMonthMaxH >= szYear.y) ? fMonthMaxH : szYear.y;
    m_szHead.Set(fMonthMaxW + szYear.x, fMonthMaxH);
    fMonthMaxW = m_szHead.x + MONTHCAL_HEADER_BTN_HMARGIN * 2 + m_szCell.x * 2;
    fs.x = (fs.x >= fMonthMaxW) ? fs.x : fMonthMaxW;
    CFX_WideString wsToday;
    GetTodayText(m_iYear, m_iMonth, m_iDay, wsToday);
    wsText = static_cast<CFX_WideString*>(
        pTheme->GetCapacity(&params, FWL_MCCAPACITY_Today));
    m_wsToday = *wsText + wsToday;
    m_szToday = CalcTextSize(wsToday, m_pProperties->m_pThemeProvider);
    m_szToday.y = (m_szToday.y >= m_szCell.y) ? m_szToday.y : m_szCell.y;
    fs.y = m_szCell.x + m_szCell.y * (MONTHCAL_ROWS - 2) + m_szToday.y +
           MONTHCAL_VMARGIN * MONTHCAL_ROWS * 2 +
           MONTHCAL_HEADER_BTN_VMARGIN * 4;
  } else {
    GetClientRect(m_rtClient);
    fs.Set(m_rtClient.width, m_rtClient.height);
  }
  return fs;
}
void CFWL_MonthCalendarImp::CalcHeadSize() {
  FX_FLOAT fHeadHMargin = (m_rtClient.width - m_szHead.x) / 2;
  FX_FLOAT fHeadVMargin = (m_szCell.x - m_szHead.y) / 2;
  m_rtHeadText.Set(m_rtClient.left + fHeadHMargin,
                   m_rtClient.top + MONTHCAL_HEADER_BTN_VMARGIN +
                       MONTHCAL_VMARGIN + fHeadVMargin,
                   m_szHead.x, m_szHead.y);
}
void CFWL_MonthCalendarImp::CalcTodaySize() {
  m_rtTodayFlag.Set(
      m_rtClient.left + MONTHCAL_HEADER_BTN_HMARGIN + MONTHCAL_HMARGIN,
      m_rtDates.bottom() + MONTHCAL_HEADER_BTN_VMARGIN + MONTHCAL_VMARGIN,
      m_szCell.x, m_szToday.y);
  m_rtToday.Set(
      m_rtClient.left + MONTHCAL_HEADER_BTN_HMARGIN + m_szCell.x +
          MONTHCAL_HMARGIN * 2,
      m_rtDates.bottom() + MONTHCAL_HEADER_BTN_VMARGIN + MONTHCAL_VMARGIN,
      m_szToday.x, m_szToday.y);
}
void CFWL_MonthCalendarImp::LayOut() {
  GetClientRect(m_rtClient);
  {
    m_rtHead.Set(
        m_rtClient.left + MONTHCAL_HEADER_BTN_HMARGIN, m_rtClient.top,
        m_rtClient.width - MONTHCAL_HEADER_BTN_HMARGIN * 2,
        m_szCell.x + (MONTHCAL_HEADER_BTN_VMARGIN + MONTHCAL_VMARGIN) * 2);
    m_rtWeek.Set(m_rtClient.left + MONTHCAL_HEADER_BTN_HMARGIN,
                 m_rtHead.bottom(),
                 m_rtClient.width - MONTHCAL_HEADER_BTN_HMARGIN * 2,
                 m_szCell.y + MONTHCAL_VMARGIN * 2);
    m_rtLBtn.Set(m_rtClient.left + MONTHCAL_HEADER_BTN_HMARGIN,
                 m_rtClient.top + MONTHCAL_HEADER_BTN_VMARGIN, m_szCell.x,
                 m_szCell.x);
    m_rtRBtn.Set(m_rtClient.left + m_rtClient.width -
                     MONTHCAL_HEADER_BTN_HMARGIN - m_szCell.x,
                 m_rtClient.top + MONTHCAL_HEADER_BTN_VMARGIN, m_szCell.x,
                 m_szCell.x);
    m_rtHSep.Set(
        m_rtClient.left + MONTHCAL_HEADER_BTN_HMARGIN + MONTHCAL_HMARGIN,
        m_rtWeek.bottom() - MONTHCAL_VMARGIN,
        m_rtClient.width - (MONTHCAL_HEADER_BTN_HMARGIN + MONTHCAL_HMARGIN) * 2,
        MONTHCAL_HSEP_HEIGHT);
    m_rtDates.Set(m_rtClient.left + MONTHCAL_HEADER_BTN_HMARGIN,
                  m_rtWeek.bottom(),
                  m_rtClient.width - MONTHCAL_HEADER_BTN_HMARGIN * 2,
                  m_szCell.y * (MONTHCAL_ROWS - 3) +
                      MONTHCAL_VMARGIN * (MONTHCAL_ROWS - 3) * 2);
  }
  CalDateItem();
}
void CFWL_MonthCalendarImp::CalDateItem() {
  FX_BOOL bNewWeek = FALSE;
  int32_t iWeekOfMonth = 0;
  FX_FLOAT fLeft = m_rtDates.left;
  FX_FLOAT fTop = m_rtDates.top;
  int32_t iCount = m_arrDates.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    LPDATEINFO pDateInfo = (LPDATEINFO)m_arrDates.GetAt(i);
    if (bNewWeek) {
      iWeekOfMonth++;
      bNewWeek = FALSE;
    }
    pDateInfo->rect.Set(
        fLeft + pDateInfo->iDayOfWeek * (m_szCell.x + (MONTHCAL_HMARGIN * 2)),
        fTop + iWeekOfMonth * (m_szCell.y + (MONTHCAL_VMARGIN * 2)),
        m_szCell.x + (MONTHCAL_HMARGIN * 2),
        m_szCell.y + (MONTHCAL_VMARGIN * 2));
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_WeekNumbers) {
      pDateInfo->rect.Offset(m_fWeekNumWid, 0);
    }
    if (pDateInfo->iDayOfWeek >= 6) {
      bNewWeek = TRUE;
    }
  }
}
void CFWL_MonthCalendarImp::GetCapValue() {
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  CFWL_ThemePart part;
  part.m_pWidget = m_pInterface;
  m_fHeadWid = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HEADER_WIDTH));
  m_fHeadHei = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HEADER_Height));
  m_fHeadBtnWid = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HEADER_BTN_WIDTH));
  m_fHeadBtnHei = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HEADER_BTN_HEIGHT));
  m_fHeadBtnHMargin = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HEADER_BTN_HMARGIN));
  m_fHeadBtnVMargin = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HEADER_BTN_VMARGIN));
  m_fHeadTextWid = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HEADER_TEXTWIDHT));
  m_fHeadTextHei = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HEADER_TEXTHEIGHT));
  m_fHeadTextHMargin = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HEADER_TEXT_HMARGIN));
  m_fHeadTextVMargin = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HEADER_TEXT_VMARGIN));
  m_fHSepWid = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HSEP_WIDTH));
  m_fHSepHei = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HSEP_HEIGHT));
  m_fWeekNumWid = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_WEEKNUM_WIDTH));
  m_fSepDOffset = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_SEP_DOFFSET));
  m_fSepX = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_SEP_X));
  m_fSepY = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_SEP_Y));
  m_fWeekNumHeigh = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_WEEKNUM_HEIGHT));
  m_fWeekWid = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_WEEK_WIDTH));
  m_fWeekHei = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_WEEK_HEIGHT));
  m_fDateCellWid = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_DATES_CELL_WIDTH));
  m_fDateCellHei = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_DATES_CELL_HEIGHT));
  m_fTodayWid = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_TODAY_WIDHT));
  m_fTodayHei = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_TODAY_HEIGHT));
  m_fTodayFlagWid = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_TODAY_FLAG_WIDHT));
  m_fMCWid = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_WIDTH));
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_WeekNumbers) {
    m_fMCWid += m_fWeekNumWid;
  }
  m_fMCHei = *static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, FWL_WGTCAPACITY_MC_HEIGHT));
}
int32_t CFWL_MonthCalendarImp::CalWeekNumber(int32_t iYear,
                                             int32_t iMonth,
                                             int32_t iDay) {
  return 0;
}
FX_BOOL CFWL_MonthCalendarImp::GetMinDate(int32_t& iYear,
                                          int32_t& iMonth,
                                          int32_t& iDay) {
  iYear = m_dtMin.iYear;
  iMonth = m_dtMin.iMonth;
  iDay = m_dtMin.iDay;
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarImp::SetMinDate(int32_t iYear,
                                          int32_t iMonth,
                                          int32_t iDay) {
  m_dtMin = DATE(iYear, iMonth, iDay);
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarImp::GetMaxDate(int32_t& iYear,
                                          int32_t& iMonth,
                                          int32_t& iDay) {
  iYear = m_dtMax.iYear;
  iMonth = m_dtMax.iMonth;
  iDay = m_dtMax.iDay;
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarImp::SetMaxDate(int32_t iYear,
                                          int32_t iMonth,
                                          int32_t iDay) {
  m_dtMax = DATE(iYear, iMonth, iDay);
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarImp::InitDate() {
  if (m_pProperties->m_pDataProvider) {
    IFWL_MonthCalendarDP* pDateProv =
        static_cast<IFWL_MonthCalendarDP*>(m_pProperties->m_pDataProvider);
    m_iYear = pDateProv->GetCurYear(m_pInterface);
    m_iMonth = pDateProv->GetCurMonth(m_pInterface);
    m_iDay = pDateProv->GetCurDay(m_pInterface);
    m_iCurYear = m_iYear;
    m_iCurMonth = m_iMonth;
  } else {
    m_iDay = 1;
    m_iMonth = 1;
    m_iYear = 1;
    m_iCurYear = m_iYear;
    m_iCurMonth = m_iMonth;
  }
  GetTodayText(m_iYear, m_iMonth, m_iDay, m_wsToday);
  GetHeadText(m_iCurYear, m_iCurMonth, m_wsHead);
  m_dtMin = DATE(1500, 12, 1);
  m_dtMax = DATE(2200, 1, 1);
  return TRUE;
}
void CFWL_MonthCalendarImp::ClearDateItem() {
  int32_t iCount = m_arrDates.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    LPDATEINFO pData = (LPDATEINFO)m_arrDates.GetAt(i);
    delete pData;
  }
  m_arrDates.RemoveAll();
}
void CFWL_MonthCalendarImp::ReSetDateItem() {
  m_pDateTime->Set(m_iCurYear, m_iCurMonth, 1);
  int32_t iDays = FX_DaysInMonth(m_iCurYear, m_iCurMonth);
  int32_t iDayOfWeek = m_pDateTime->GetDayOfWeek();
  for (int32_t i = 0; i < iDays; i++) {
    if (iDayOfWeek >= 7) {
      iDayOfWeek = 0;
    }
    CFX_WideString wsDay;
    wsDay.Format(L"%d", i + 1);
    FX_DWORD dwStates = 0;
    if (m_iYear == m_iCurYear && m_iMonth == m_iCurMonth && m_iDay == (i + 1)) {
      dwStates |= FWL_ITEMSTATE_MCD_Flag;
    }
    if (m_arrSelDays.Find(i + 1) != -1) {
      dwStates |= FWL_ITEMSTATE_MCD_Selected;
    }
    CFX_RectF rtDate;
    rtDate.Set(0, 0, 0, 0);
    LPDATEINFO pData = new DATEINFO(i + 1, iDayOfWeek, dwStates, rtDate, wsDay);
    m_arrDates.Add(pData);
    iDayOfWeek++;
  }
}
FX_BOOL CFWL_MonthCalendarImp::NextMonth() {
  int32_t iYear = m_iCurYear, iMonth = m_iCurMonth;
  if (iMonth >= 12) {
    iMonth = 1;
    iYear++;
  } else {
    iMonth++;
  }
  DATE dt(m_iCurYear, m_iCurMonth, 1);
  if (!(dt < m_dtMax)) {
    return FALSE;
  }
  m_iCurYear = iYear, m_iCurMonth = iMonth;
  ChangeToMonth(m_iCurYear, m_iCurMonth);
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarImp::PrevMonth() {
  int32_t iYear = m_iCurYear, iMonth = m_iCurMonth;
  if (iMonth <= 1) {
    iMonth = 12;
    iYear--;
  } else {
    iMonth--;
  }
  DATE dt(m_iCurYear, m_iCurMonth, 1);
  if (!(dt > m_dtMin)) {
    return FALSE;
  }
  m_iCurYear = iYear, m_iCurMonth = iMonth;
  ChangeToMonth(m_iCurYear, m_iCurMonth);
  return TRUE;
}
void CFWL_MonthCalendarImp::ChangeToMonth(int32_t iYear, int32_t iMonth) {
  m_iCurYear = iYear;
  m_iCurMonth = iMonth;
  m_iHovered = -1;
  ClearDateItem();
  ReSetDateItem();
  CalDateItem();
  GetHeadText(m_iCurYear, m_iCurMonth, m_wsHead);
}
FX_BOOL CFWL_MonthCalendarImp::RemoveSelDay(int32_t iDay, FX_BOOL bAll) {
  if (iDay == -1 && !bAll) {
    return FALSE;
  }
  if (bAll) {
    int32_t iCount = m_arrSelDays.GetSize();
    int32_t iDatesCount = m_arrDates.GetSize();
    for (int32_t i = 0; i < iCount; i++) {
      int32_t iSelDay = m_arrSelDays.GetAt(i);
      if (iSelDay <= iDatesCount) {
        LPDATEINFO pDateInfo = (LPDATEINFO)m_arrDates.GetAt(iSelDay - 1);
        pDateInfo->dwStates &= ~FWL_ITEMSTATE_MCD_Selected;
      }
    }
    m_arrSelDays.RemoveAll();
  } else {
    int32_t index = m_arrSelDays.Find(iDay);
    if (index == -1) {
      return FALSE;
    }
    int32_t iSelDay = m_arrSelDays.GetAt(iDay);
    int32_t iDatesCount = m_arrDates.GetSize();
    if (iSelDay <= iDatesCount) {
      LPDATEINFO pDateInfo = (LPDATEINFO)m_arrDates.GetAt(iSelDay - 1);
      pDateInfo->dwStates &= ~FWL_ITEMSTATE_MCD_Selected;
    }
    m_arrSelDays.RemoveAt(index);
  }
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarImp::AddSelDay(int32_t iDay) {
  FXSYS_assert(iDay > 0);
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_MultiSelect) {
  } else {
    if (m_arrSelDays.Find(iDay) == -1) {
      RemoveSelDay(-1, TRUE);
      if (iDay <= m_arrDates.GetSize()) {
        LPDATEINFO pDateInfo = (LPDATEINFO)m_arrDates.GetAt(iDay - 1);
        pDateInfo->dwStates |= FWL_ITEMSTATE_MCD_Selected;
      }
      m_arrSelDays.Add(iDay);
    }
  }
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarImp::JumpToToday() {
  if (m_iYear != m_iCurYear || m_iMonth != m_iCurMonth) {
    m_iCurYear = m_iYear;
    m_iCurMonth = m_iMonth;
    ChangeToMonth(m_iYear, m_iMonth);
    AddSelDay(m_iDay);
  } else {
    if (m_arrSelDays.Find(m_iDay) == -1) {
      AddSelDay(m_iDay);
    }
  }
  return TRUE;
}
void CFWL_MonthCalendarImp::GetHeadText(int32_t iYear,
                                        int32_t iMonth,
                                        CFX_WideString& wsHead) {
  FXSYS_assert(iMonth > 0 && iMonth < 13);
  static const FX_WCHAR* const pMonth[] = {
      L"January",   L"February", L"March",    L"April",
      L"May",       L"June",     L"July",     L"August",
      L"September", L"October",  L"November", L"December"};
  wsHead.Format(L"%s, %d", pMonth[iMonth - 1], iYear);
}
void CFWL_MonthCalendarImp::GetTodayText(int32_t iYear,
                                         int32_t iMonth,
                                         int32_t iDay,
                                         CFX_WideString& wsToday) {
  wsToday.Format(L", %d/%d/%d", iDay, iMonth, iYear);
}
int32_t CFWL_MonthCalendarImp::GetDayAtPoint(FX_FLOAT x, FX_FLOAT y) {
  int32_t iCount = m_arrDates.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    LPDATEINFO pDateInfo = (LPDATEINFO)m_arrDates.GetAt(i);
    if (pDateInfo->rect.Contains(x, y)) {
      return ++i;
    }
  }
  return -1;
}
FX_BOOL CFWL_MonthCalendarImp::GetDayRect(int32_t iDay, CFX_RectF& rtDay) {
  if (iDay <= 0 || iDay > m_arrDates.GetSize()) {
    return FALSE;
  }
  LPDATEINFO pDateInfo = (LPDATEINFO)m_arrDates[iDay - 1];
  if (!pDateInfo)
    return FALSE;
  rtDay = pDateInfo->rect;
  return TRUE;
}
CFWL_MonthCalendarImpDelegate::CFWL_MonthCalendarImpDelegate(
    CFWL_MonthCalendarImp* pOwner)
    : m_pOwner(pOwner) {}
int32_t CFWL_MonthCalendarImpDelegate::OnProcessMessage(
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
    case FWL_MSGHASH_Key: {
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
        default: { break; }
      }
      break;
    }
    default: {
      iRet = 0;
      break;
    }
  }
  CFWL_WidgetImpDelegate::OnProcessMessage(pMessage);
  return iRet;
}
FWL_ERR CFWL_MonthCalendarImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                                    const CFX_Matrix* pMatrix) {
  return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
void CFWL_MonthCalendarImpDelegate::OnActivate(CFWL_Message* pMsg) {
  return;
}
void CFWL_MonthCalendarImpDelegate::OnFocusChanged(CFWL_Message* pMsg,
                                                   FX_BOOL bSet) {
  if (bSet) {
    m_pOwner->m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
  } else {
    m_pOwner->m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
  }
  m_pOwner->Repaint(&m_pOwner->m_rtClient);
}
void CFWL_MonthCalendarImpDelegate::OnLButtonDown(CFWL_MsgMouse* pMsg) {
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
    if (m_pOwner->m_pProperties->m_dwStyleExes & FWL_STYLEEXT_MCD_MultiSelect) {
    } else {
      int32_t iOldSel = 0;
      if (m_pOwner->m_arrSelDays.GetSize() > 0) {
        iOldSel = m_pOwner->m_arrSelDays[0];
      } else {
        return;
      }
      int32_t iCurSel = m_pOwner->GetDayAtPoint(pMsg->m_fx, pMsg->m_fy);
      FX_BOOL bSelChanged = iCurSel > 0 && iCurSel != iOldSel;
      if (bSelChanged) {
        LPDATEINFO lpDatesInfo =
            (LPDATEINFO)m_pOwner->m_arrDates.GetAt(iCurSel - 1);
        CFX_RectF rtInvalidate(lpDatesInfo->rect);
        if (iOldSel > 0) {
          lpDatesInfo = (LPDATEINFO)m_pOwner->m_arrDates.GetAt(iOldSel - 1);
          rtInvalidate.Union(lpDatesInfo->rect);
        }
        m_pOwner->AddSelDay(iCurSel);
        CFWL_EvtClick wmClick;
        wmClick.m_pSrcTarget = m_pOwner->m_pInterface;
        m_pOwner->DispatchEvent(&wmClick);
        CFWL_EventMcdDateChanged wmDateSelected;
        wmDateSelected.m_iStartDay = iCurSel;
        wmDateSelected.m_iEndDay = iCurSel;
        wmDateSelected.m_iOldMonth = m_pOwner->m_iCurMonth;
        wmDateSelected.m_iOldYear = m_pOwner->m_iCurYear;
        wmDateSelected.m_pSrcTarget = m_pOwner->m_pInterface;
        m_pOwner->DispatchEvent(&wmDateSelected);
        m_pOwner->Repaint(&rtInvalidate);
      }
    }
  }
}
void CFWL_MonthCalendarImpDelegate::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_rtLBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iLBtnPartStates = 0;
    m_pOwner->Repaint(&m_pOwner->m_rtLBtn);
  } else if (m_pOwner->m_rtRBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pOwner->m_iRBtnPartStates = 0;
    m_pOwner->Repaint(&m_pOwner->m_rtRBtn);
  } else if (m_pOwner->m_rtDates.Contains(pMsg->m_fx, pMsg->m_fy)) {
    int32_t iDay = m_pOwner->GetDayAtPoint(pMsg->m_fx, pMsg->m_fy);
    if (iDay != -1) {
      m_pOwner->AddSelDay(iDay);
    }
  }
}
void CFWL_MonthCalendarImpDelegate::OnMouseMove(CFWL_MsgMouse* pMsg) {
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
void CFWL_MonthCalendarImpDelegate::OnMouseLeave(CFWL_MsgMouse* pMsg) {
  if (m_pOwner->m_iHovered > 0) {
    CFX_RectF rtInvalidate;
    rtInvalidate.Set(0, 0, 0, 0);
    m_pOwner->GetDayRect(m_pOwner->m_iHovered, rtInvalidate);
    m_pOwner->m_iHovered = -1;
    if (!rtInvalidate.IsEmpty()) {
      m_pOwner->Repaint(&rtInvalidate);
    }
  }
}
