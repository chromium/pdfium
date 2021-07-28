// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_monthcalendar.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxcrt/cfx_datetime.h"
#include "core/fxcrt/stl_util.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/notreached.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fwl/cfwl_datetimepicker.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace {

constexpr float kMonthCalHSepHeight = 1.0f;
constexpr float kMonthCalHMargin = 3.0f;
constexpr float kMonthCalVMargin = 2.0f;
constexpr float kMonthCalRows = 9.0f;
constexpr float kMonthCalColumns = 7.0f;
constexpr float kMonthCalHeaderBtnVMargin = 7.0f;
constexpr float kMonthCalHeaderBtnHMargin = 5.0f;

WideString GetAbbreviatedDayOfWeek(int day) {
  switch (day) {
    case 0:
      return L"Sun";
    case 1:
      return L"Mon";
    case 2:
      return L"Tue";
    case 3:
      return L"Wed";
    case 4:
      return L"Thu";
    case 5:
      return L"Fri";
    case 6:
      return L"Sat";
    default:
      NOTREACHED();
      return L"";
  }
}

WideString GetMonth(int month) {
  switch (month) {
    case 0:
      return L"January";
    case 1:
      return L"February";
    case 2:
      return L"March";
    case 3:
      return L"April";
    case 4:
      return L"May";
    case 5:
      return L"June";
    case 6:
      return L"July";
    case 7:
      return L"August";
    case 8:
      return L"September";
    case 9:
      return L"October";
    case 10:
      return L"November";
    case 11:
      return L"December";
    default:
      NOTREACHED();
      return L"";
  }
}

}  // namespace

CFWL_MonthCalendar::CFWL_MonthCalendar(CFWL_App* app,
                                       const Properties& properties,
                                       CFWL_Widget* pOuter)
    : CFWL_Widget(app, properties, pOuter) {}

CFWL_MonthCalendar::~CFWL_MonthCalendar() = default;

FWL_Type CFWL_MonthCalendar::GetClassID() const {
  return FWL_Type::MonthCalendar;
}

CFX_RectF CFWL_MonthCalendar::GetAutosizedWidgetRect() {
  CFX_SizeF fs = CalcSize();
  CFX_RectF rect(0, 0, fs.width, fs.height);
  InflateWidgetRect(rect);
  return rect;
}

void CFWL_MonthCalendar::Update() {
  if (IsLocked())
    return;

  if (!m_bInitialized) {
    InitDate();
    m_bInitialized = true;
  }
  ClearDateItem();
  ResetDateItem();
  Layout();
}

void CFWL_MonthCalendar::DrawWidget(CFGAS_GEGraphics* pGraphics,
                                    const CFX_Matrix& matrix) {
  if (!pGraphics)
    return;

  if (HasBorder())
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);

  DrawBackground(pGraphics, matrix);
  DrawHeadBK(pGraphics, matrix);
  DrawLButton(pGraphics, matrix);
  DrawRButton(pGraphics, matrix);
  DrawSeparator(pGraphics, matrix);
  DrawDatesInBK(pGraphics, matrix);
  DrawDatesInCircle(pGraphics, matrix);
  DrawCaption(pGraphics, matrix);
  DrawWeek(pGraphics, matrix);
  DrawDatesIn(pGraphics, matrix);
  DrawDatesOut(pGraphics, matrix);
  DrawToday(pGraphics, matrix);
}

void CFWL_MonthCalendar::SetSelect(int32_t iYear,
                                   int32_t iMonth,
                                   int32_t iDay) {
  ChangeToMonth(iYear, iMonth);
  AddSelDay(iDay);
}

void CFWL_MonthCalendar::DrawBackground(CFGAS_GEGraphics* pGraphics,
                                        const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground params(this, pGraphics);
  params.m_iPart = CFWL_ThemePart::Part::kBackground;
  params.m_PartRect = m_ClientRect;
  params.m_matrix = mtMatrix;
  GetThemeProvider()->DrawBackground(params);
}

void CFWL_MonthCalendar::DrawHeadBK(CFGAS_GEGraphics* pGraphics,
                                    const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground params(this, pGraphics);
  params.m_iPart = CFWL_ThemePart::Part::kHeader;
  params.m_PartRect = m_HeadRect;
  params.m_matrix = mtMatrix;
  GetThemeProvider()->DrawBackground(params);
}

void CFWL_MonthCalendar::DrawLButton(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground params(this, pGraphics);
  params.m_iPart = CFWL_ThemePart::Part::kLBtn;
  params.m_dwStates = m_iLBtnPartStates;
  params.m_PartRect = m_LBtnRect;
  params.m_matrix = mtMatrix;
  GetThemeProvider()->DrawBackground(params);
}

void CFWL_MonthCalendar::DrawRButton(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground params(this, pGraphics);
  params.m_iPart = CFWL_ThemePart::Part::kRBtn;
  params.m_dwStates = m_iRBtnPartStates;
  params.m_PartRect = m_RBtnRect;
  params.m_matrix = mtMatrix;
  GetThemeProvider()->DrawBackground(params);
}

void CFWL_MonthCalendar::DrawCaption(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& mtMatrix) {
  CFWL_ThemeText textParam(this, pGraphics);
  textParam.m_iPart = CFWL_ThemePart::Part::kCaption;
  textParam.m_wsText = GetHeadText(m_iCurYear, m_iCurMonth);
  m_HeadSize = CalcTextSize(textParam.m_wsText, false);
  CalcHeadSize();
  textParam.m_PartRect = m_HeadTextRect;
  textParam.m_dwTTOStyles.single_line_ = true;
  textParam.m_iTTOAlign = FDE_TextAlignment::kCenter;
  textParam.m_matrix = mtMatrix;
  GetThemeProvider()->DrawText(textParam);
}

void CFWL_MonthCalendar::DrawSeparator(CFGAS_GEGraphics* pGraphics,
                                       const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground params(this, pGraphics);
  params.m_iPart = CFWL_ThemePart::Part::kHSeparator;
  params.m_PartRect = m_HSepRect;
  params.m_matrix = mtMatrix;
  GetThemeProvider()->DrawBackground(params);
}

void CFWL_MonthCalendar::DrawDatesInBK(CFGAS_GEGraphics* pGraphics,
                                       const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground params(this, pGraphics);
  params.m_iPart = CFWL_ThemePart::Part::kDateInBK;
  params.m_matrix = mtMatrix;

  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  int32_t iCount = fxcrt::CollectionSize<int32_t>(m_DateArray);
  for (int32_t j = 0; j < iCount; j++) {
    DATEINFO* pDataInfo = m_DateArray[j].get();
    if (pDataInfo->bSelected) {
      params.m_dwStates |= CFWL_PartState_Selected;
      if (pDataInfo->bFlagged) {
        params.m_dwStates |= CFWL_PartState_Flagged;
      }
    } else if (j == m_iHovered - 1) {
      params.m_dwStates |= CFWL_PartState_Hovered;
    } else if (pDataInfo->bFlagged) {
      params.m_dwStates = CFWL_PartState_Flagged;
      pTheme->DrawBackground(params);
    }
    params.m_PartRect = pDataInfo->rect;
    pTheme->DrawBackground(params);
    params.m_dwStates = 0;
  }
}

void CFWL_MonthCalendar::DrawWeek(CFGAS_GEGraphics* pGraphics,
                                  const CFX_Matrix& mtMatrix) {
  CFWL_ThemeText params(this, pGraphics);
  params.m_iPart = CFWL_ThemePart::Part::kWeek;
  params.m_iTTOAlign = FDE_TextAlignment::kCenter;
  params.m_dwTTOStyles.single_line_ = true;
  params.m_matrix = mtMatrix;

  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  CFX_RectF rtDayOfWeek;
  for (int32_t i = 0; i < 7; ++i) {
    rtDayOfWeek = CFX_RectF(
        m_WeekRect.left + i * (m_CellSize.width + kMonthCalHMargin * 2),
        m_WeekRect.top, m_CellSize);

    params.m_PartRect = rtDayOfWeek;
    params.m_wsText = GetAbbreviatedDayOfWeek(i);
    pTheme->DrawText(params);
  }
}

void CFWL_MonthCalendar::DrawToday(CFGAS_GEGraphics* pGraphics,
                                   const CFX_Matrix& mtMatrix) {
  CFWL_ThemeText params(this, pGraphics);
  params.m_iPart = CFWL_ThemePart::Part::kToday;
  params.m_iTTOAlign = FDE_TextAlignment::kCenterLeft;
  params.m_wsText = GetTodayText(m_iYear, m_iMonth, m_iDay);
  m_TodaySize = CalcTextSize(params.m_wsText, false);
  CalcTodaySize();
  params.m_PartRect = m_TodayRect;
  params.m_dwTTOStyles.single_line_ = true;
  params.m_matrix = mtMatrix;
  GetThemeProvider()->DrawText(params);
}

void CFWL_MonthCalendar::DrawDatesIn(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& mtMatrix) {
  CFWL_ThemeText params(this, pGraphics);
  params.m_iPart = CFWL_ThemePart::Part::kDatesIn;
  params.m_iTTOAlign = FDE_TextAlignment::kCenter;
  params.m_matrix = mtMatrix;

  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  int32_t iCount = fxcrt::CollectionSize<int32_t>(m_DateArray);
  for (int32_t j = 0; j < iCount; j++) {
    DATEINFO* pDataInfo = m_DateArray[j].get();
    params.m_wsText = pDataInfo->wsDay;
    params.m_PartRect = pDataInfo->rect;
    params.m_dwStates = pDataInfo->AsPartStateMask();
    if (j + 1 == m_iHovered)
      params.m_dwStates |= CFWL_PartState_Hovered;

    params.m_dwTTOStyles.single_line_ = true;
    pTheme->DrawText(params);
  }
}

void CFWL_MonthCalendar::DrawDatesOut(CFGAS_GEGraphics* pGraphics,
                                      const CFX_Matrix& mtMatrix) {
  CFWL_ThemeText params(this, pGraphics);
  params.m_iPart = CFWL_ThemePart::Part::kDatesOut;
  params.m_iTTOAlign = FDE_TextAlignment::kCenter;
  params.m_matrix = mtMatrix;
  GetThemeProvider()->DrawText(params);
}

void CFWL_MonthCalendar::DrawDatesInCircle(CFGAS_GEGraphics* pGraphics,
                                           const CFX_Matrix& mtMatrix) {
  if (m_iMonth != m_iCurMonth || m_iYear != m_iCurYear)
    return;

  if (m_iDay < 1 || m_iDay > fxcrt::CollectionSize<int32_t>(m_DateArray))
    return;

  DATEINFO* pDate = m_DateArray[m_iDay - 1].get();
  if (!pDate)
    return;

  CFWL_ThemeBackground params(this, pGraphics);
  params.m_iPart = CFWL_ThemePart::Part::kDateInCircle;
  params.m_PartRect = pDate->rect;
  params.m_matrix = mtMatrix;
  GetThemeProvider()->DrawBackground(params);
}

CFX_SizeF CFWL_MonthCalendar::CalcSize() {
  float fMaxWeekW = 0.0f;
  float fMaxWeekH = 0.0f;
  for (int i = 0; i < 7; ++i) {
    CFX_SizeF sz = CalcTextSize(GetAbbreviatedDayOfWeek(i), false);
    fMaxWeekW = (fMaxWeekW >= sz.width) ? fMaxWeekW : sz.width;
    fMaxWeekH = (fMaxWeekH >= sz.height) ? fMaxWeekH : sz.height;
  }
  float fDayMaxW = 0.0f;
  float fDayMaxH = 0.0f;
  for (int day = 10; day <= 31; day++) {
    CFX_SizeF sz = CalcTextSize(WideString::Format(L"%d", day), false);
    fDayMaxW = (fDayMaxW >= sz.width) ? fDayMaxW : sz.width;
    fDayMaxH = (fDayMaxH >= sz.height) ? fDayMaxH : sz.height;
  }
  m_CellSize.width =
      static_cast<int>(0.5 + (fMaxWeekW >= fDayMaxW ? fMaxWeekW : fDayMaxW));
  m_CellSize.height = fMaxWeekH >= fDayMaxH ? fMaxWeekH : fDayMaxH;

  CFX_SizeF fs;
  fs.width = m_CellSize.width * kMonthCalColumns +
             kMonthCalHMargin * kMonthCalColumns * 2 +
             kMonthCalHeaderBtnHMargin * 2;

  float fMonthMaxW = 0.0f;
  float fMonthMaxH = 0.0f;
  for (int i = 0; i < 12; ++i) {
    CFX_SizeF sz = CalcTextSize(GetMonth(i), false);
    fMonthMaxW = (fMonthMaxW >= sz.width) ? fMonthMaxW : sz.width;
    fMonthMaxH = (fMonthMaxH >= sz.height) ? fMonthMaxH : sz.height;
  }

  CFX_SizeF szYear = CalcTextSize(GetHeadText(m_iYear, m_iMonth), false);
  fMonthMaxH = std::max(fMonthMaxH, szYear.height);
  m_HeadSize = CFX_SizeF(fMonthMaxW + szYear.width, fMonthMaxH);
  fMonthMaxW =
      m_HeadSize.width + kMonthCalHeaderBtnHMargin * 2 + m_CellSize.width * 2;
  fs.width = std::max(fs.width, fMonthMaxW);

  m_wsToday = GetTodayText(m_iYear, m_iMonth, m_iDay);
  m_TodaySize = CalcTextSize(m_wsToday, false);
  m_TodaySize.height = (m_TodaySize.height >= m_CellSize.height)
                           ? m_TodaySize.height
                           : m_CellSize.height;
  fs.height = m_CellSize.width + m_CellSize.height * (kMonthCalRows - 2) +
              m_TodaySize.height + kMonthCalVMargin * kMonthCalRows * 2 +
              kMonthCalHeaderBtnVMargin * 4;
  return fs;
}

void CFWL_MonthCalendar::CalcHeadSize() {
  float fHeadHMargin = (m_ClientRect.width - m_HeadSize.width) / 2;
  float fHeadVMargin = (m_CellSize.width - m_HeadSize.height) / 2;
  m_HeadTextRect = CFX_RectF(m_ClientRect.left + fHeadHMargin,
                             m_ClientRect.top + kMonthCalHeaderBtnVMargin +
                                 kMonthCalVMargin + fHeadVMargin,
                             m_HeadSize);
}

void CFWL_MonthCalendar::CalcTodaySize() {
  m_TodayFlagRect = CFX_RectF(
      m_ClientRect.left + kMonthCalHeaderBtnHMargin + kMonthCalHMargin,
      m_DatesRect.bottom() + kMonthCalHeaderBtnVMargin + kMonthCalVMargin,
      m_CellSize.width, m_TodaySize.height);
  m_TodayRect = CFX_RectF(
      m_ClientRect.left + kMonthCalHeaderBtnHMargin + m_CellSize.width +
          kMonthCalHMargin * 2,
      m_DatesRect.bottom() + kMonthCalHeaderBtnVMargin + kMonthCalVMargin,
      m_TodaySize);
}

void CFWL_MonthCalendar::Layout() {
  m_ClientRect = GetClientRect();

  m_HeadRect = CFX_RectF(
      m_ClientRect.left + kMonthCalHeaderBtnHMargin, m_ClientRect.top,
      m_ClientRect.width - kMonthCalHeaderBtnHMargin * 2,
      m_CellSize.width + (kMonthCalHeaderBtnVMargin + kMonthCalVMargin) * 2);
  m_WeekRect = CFX_RectF(m_ClientRect.left + kMonthCalHeaderBtnHMargin,
                         m_HeadRect.bottom(),
                         m_ClientRect.width - kMonthCalHeaderBtnHMargin * 2,
                         m_CellSize.height + kMonthCalVMargin * 2);
  m_LBtnRect = CFX_RectF(m_ClientRect.left + kMonthCalHeaderBtnHMargin,
                         m_ClientRect.top + kMonthCalHeaderBtnVMargin,
                         m_CellSize.width, m_CellSize.width);
  m_RBtnRect = CFX_RectF(m_ClientRect.left + m_ClientRect.width -
                             kMonthCalHeaderBtnHMargin - m_CellSize.width,
                         m_ClientRect.top + kMonthCalHeaderBtnVMargin,
                         m_CellSize.width, m_CellSize.width);
  m_HSepRect = CFX_RectF(
      m_ClientRect.left + kMonthCalHeaderBtnHMargin + kMonthCalHMargin,
      m_WeekRect.bottom() - kMonthCalVMargin,
      m_ClientRect.width - (kMonthCalHeaderBtnHMargin + kMonthCalHMargin) * 2,
      kMonthCalHSepHeight);
  m_DatesRect = CFX_RectF(m_ClientRect.left + kMonthCalHeaderBtnHMargin,
                          m_WeekRect.bottom(),
                          m_ClientRect.width - kMonthCalHeaderBtnHMargin * 2,
                          m_CellSize.height * (kMonthCalRows - 3) +
                              kMonthCalVMargin * (kMonthCalRows - 3) * 2);

  CalDateItem();
}

void CFWL_MonthCalendar::CalDateItem() {
  bool bNewWeek = false;
  int32_t iWeekOfMonth = 0;
  float fLeft = m_DatesRect.left;
  float fTop = m_DatesRect.top;
  for (const auto& pDateInfo : m_DateArray) {
    if (bNewWeek) {
      iWeekOfMonth++;
      bNewWeek = false;
    }
    pDateInfo->rect = CFX_RectF(
        fLeft +
            pDateInfo->iDayOfWeek * (m_CellSize.width + (kMonthCalHMargin * 2)),
        fTop + iWeekOfMonth * (m_CellSize.height + (kMonthCalVMargin * 2)),
        m_CellSize.width + (kMonthCalHMargin * 2),
        m_CellSize.height + (kMonthCalVMargin * 2));
    if (pDateInfo->iDayOfWeek >= 6)
      bNewWeek = true;
  }
}

void CFWL_MonthCalendar::InitDate() {
  CFX_DateTime now = CFX_DateTime::Now();

  m_iYear = now.GetYear();
  m_iMonth = now.GetMonth();
  m_iDay = now.GetDay();
  m_iCurYear = m_iYear;
  m_iCurMonth = m_iMonth;

  m_wsToday = GetTodayText(m_iYear, m_iMonth, m_iDay);
  m_wsHead = GetHeadText(m_iCurYear, m_iCurMonth);
  m_dtMin = DATE(1500, 12, 1);
  m_dtMax = DATE(2200, 1, 1);
}

void CFWL_MonthCalendar::ClearDateItem() {
  m_DateArray.clear();
}

void CFWL_MonthCalendar::ResetDateItem() {
  int32_t iDays = FX_DaysInMonth(m_iCurYear, m_iCurMonth);
  int32_t iDayOfWeek =
      CFX_DateTime(m_iCurYear, m_iCurMonth, 1, 0, 0, 0, 0).GetDayOfWeek();
  for (int32_t i = 0; i < iDays; ++i, ++iDayOfWeek) {
    if (iDayOfWeek >= 7)
      iDayOfWeek = 0;

    const bool bFlagged =
        m_iYear == m_iCurYear && m_iMonth == m_iCurMonth && m_iDay == i + 1;
    const bool bSelected = pdfium::Contains(m_SelDayArray, i + 1);
    m_DateArray.push_back(
        std::make_unique<DATEINFO>(i + 1, iDayOfWeek, bFlagged, bSelected,
                                   WideString::Format(L"%d", i + 1)));
  }
}

void CFWL_MonthCalendar::NextMonth() {
  int32_t iYear = m_iCurYear;
  int32_t iMonth = m_iCurMonth;
  if (iMonth >= 12) {
    iMonth = 1;
    iYear++;
  } else {
    iMonth++;
  }
  DATE dt(m_iCurYear, m_iCurMonth, 1);
  if (!(dt < m_dtMax))
    return;

  m_iCurYear = iYear, m_iCurMonth = iMonth;
  ChangeToMonth(m_iCurYear, m_iCurMonth);
}

void CFWL_MonthCalendar::PrevMonth() {
  int32_t iYear = m_iCurYear;
  int32_t iMonth = m_iCurMonth;
  if (iMonth <= 1) {
    iMonth = 12;
    iYear--;
  } else {
    iMonth--;
  }

  DATE dt(m_iCurYear, m_iCurMonth, 1);
  if (!(dt > m_dtMin))
    return;

  m_iCurYear = iYear, m_iCurMonth = iMonth;
  ChangeToMonth(m_iCurYear, m_iCurMonth);
}

void CFWL_MonthCalendar::ChangeToMonth(int32_t iYear, int32_t iMonth) {
  m_iCurYear = iYear;
  m_iCurMonth = iMonth;
  m_iHovered = -1;

  ClearDateItem();
  ResetDateItem();
  CalDateItem();
  m_wsHead = GetHeadText(m_iCurYear, m_iCurMonth);
}

void CFWL_MonthCalendar::RemoveSelDay() {
  int32_t iDatesCount = fxcrt::CollectionSize<int32_t>(m_DateArray);
  for (int32_t iSelDay : m_SelDayArray) {
    if (iSelDay <= iDatesCount)
      m_DateArray[iSelDay - 1]->bSelected = false;
  }
  m_SelDayArray.clear();
}

void CFWL_MonthCalendar::AddSelDay(int32_t iDay) {
  DCHECK(iDay > 0);
  if (!pdfium::Contains(m_SelDayArray, iDay))
    return;

  RemoveSelDay();
  if (iDay <= fxcrt::CollectionSize<int32_t>(m_DateArray))
    m_DateArray[iDay - 1]->bSelected = true;

  m_SelDayArray.push_back(iDay);
}

void CFWL_MonthCalendar::JumpToToday() {
  if (m_iYear != m_iCurYear || m_iMonth != m_iCurMonth) {
    m_iCurYear = m_iYear;
    m_iCurMonth = m_iMonth;
    ChangeToMonth(m_iYear, m_iMonth);
    AddSelDay(m_iDay);
    return;
  }

  if (!pdfium::Contains(m_SelDayArray, m_iDay))
    AddSelDay(m_iDay);
}

WideString CFWL_MonthCalendar::GetHeadText(int32_t iYear, int32_t iMonth) {
  DCHECK(iMonth > 0);
  DCHECK(iMonth < 13);

  static const wchar_t* const pMonth[] = {L"January", L"February", L"March",
                                          L"April",   L"May",      L"June",
                                          L"July",    L"August",   L"September",
                                          L"October", L"November", L"December"};
  return WideString::Format(L"%ls, %d", pMonth[iMonth - 1], iYear);
}

WideString CFWL_MonthCalendar::GetTodayText(int32_t iYear,
                                            int32_t iMonth,
                                            int32_t iDay) {
  return WideString::Format(L"Today, %d/%d/%d", iDay, iMonth, iYear);
}

int32_t CFWL_MonthCalendar::GetDayAtPoint(const CFX_PointF& point) const {
  int i = 1;  // one-based day values.
  for (const auto& pDateInfo : m_DateArray) {
    if (pDateInfo->rect.Contains(point))
      return i;
    ++i;
  }
  return -1;
}

CFX_RectF CFWL_MonthCalendar::GetDayRect(int32_t iDay) {
  if (iDay <= 0 || iDay > fxcrt::CollectionSize<int32_t>(m_DateArray))
    return CFX_RectF();

  DATEINFO* pDateInfo = m_DateArray[iDay - 1].get();
  return pDateInfo ? pDateInfo->rect : CFX_RectF();
}

void CFWL_MonthCalendar::OnProcessMessage(CFWL_Message* pMessage) {
  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus:
    case CFWL_Message::Type::kKillFocus:
      GetOuter()->GetDelegate()->OnProcessMessage(pMessage);
      break;
    case CFWL_Message::Type::kKey:
      break;
    case CFWL_Message::Type::kMouse: {
      CFWL_MessageMouse* pMouse = static_cast<CFWL_MessageMouse*>(pMessage);
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
    default:
      break;
  }
  // Dst target could be |this|, continue only if not destroyed by above.
  if (pMessage->GetDstTarget())
    CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_MonthCalendar::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                      const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_MonthCalendar::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  if (m_LBtnRect.Contains(pMsg->m_pos)) {
    m_iLBtnPartStates = CFWL_PartState_Pressed;
    PrevMonth();
    RepaintRect(m_ClientRect);
  } else if (m_RBtnRect.Contains(pMsg->m_pos)) {
    m_iRBtnPartStates |= CFWL_PartState_Pressed;
    NextMonth();
    RepaintRect(m_ClientRect);
  } else if (m_TodayRect.Contains(pMsg->m_pos)) {
    JumpToToday();
    RepaintRect(m_ClientRect);
  }
}

void CFWL_MonthCalendar::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  if (m_LBtnRect.Contains(pMsg->m_pos)) {
    m_iLBtnPartStates = 0;
    RepaintRect(m_LBtnRect);
    return;
  }
  if (m_RBtnRect.Contains(pMsg->m_pos)) {
    m_iRBtnPartStates = 0;
    RepaintRect(m_RBtnRect);
    return;
  }
  if (m_TodayRect.Contains(pMsg->m_pos))
    return;

  int32_t iOldSel = 0;
  if (!m_SelDayArray.empty())
    iOldSel = m_SelDayArray[0];

  int32_t iCurSel = GetDayAtPoint(pMsg->m_pos);
  if (iCurSel > 0) {
    DATEINFO* pDateInfo = m_DateArray[iCurSel - 1].get();
    CFX_RectF rtInvalidate(pDateInfo->rect);
    if (iOldSel > 0 && iOldSel <= fxcrt::CollectionSize<int32_t>(m_DateArray)) {
      pDateInfo = m_DateArray[iOldSel - 1].get();
      rtInvalidate.Union(pDateInfo->rect);
    }
    AddSelDay(iCurSel);
    CFWL_DateTimePicker* pDateTime =
        static_cast<CFWL_DateTimePicker*>(GetOuter());
    pDateTime->ProcessSelChanged(m_iCurYear, m_iCurMonth, iCurSel);
    pDateTime->HideMonthCalendar();
  }
}

void CFWL_MonthCalendar::OnMouseMove(CFWL_MessageMouse* pMsg) {
  bool bRepaint = false;
  CFX_RectF rtInvalidate;
  if (m_DatesRect.Contains(pMsg->m_pos)) {
    int32_t iHover = GetDayAtPoint(pMsg->m_pos);
    bRepaint = m_iHovered != iHover;
    if (bRepaint) {
      if (m_iHovered > 0)
        rtInvalidate = GetDayRect(m_iHovered);
      if (iHover > 0) {
        CFX_RectF rtDay = GetDayRect(iHover);
        if (rtInvalidate.IsEmpty())
          rtInvalidate = rtDay;
        else
          rtInvalidate.Union(rtDay);
      }
    }
    m_iHovered = iHover;
  } else {
    bRepaint = m_iHovered > 0;
    if (bRepaint)
      rtInvalidate = GetDayRect(m_iHovered);

    m_iHovered = -1;
  }
  if (bRepaint && !rtInvalidate.IsEmpty())
    RepaintRect(rtInvalidate);
}

void CFWL_MonthCalendar::OnMouseLeave(CFWL_MessageMouse* pMsg) {
  if (m_iHovered <= 0)
    return;

  CFX_RectF rtInvalidate = GetDayRect(m_iHovered);
  m_iHovered = -1;
  if (!rtInvalidate.IsEmpty())
    RepaintRect(rtInvalidate);
}

CFWL_MonthCalendar::DATEINFO::DATEINFO(int32_t day,
                                       int32_t dayofweek,
                                       bool bFlag,
                                       bool bSelect,
                                       const WideString& wsday)
    : iDay(day),
      iDayOfWeek(dayofweek),
      bFlagged(bFlag),
      bSelected(bSelect),
      wsDay(wsday) {}

CFWL_MonthCalendar::DATEINFO::~DATEINFO() = default;

CFWL_PartStateMask CFWL_MonthCalendar::DATEINFO::AsPartStateMask() const {
  CFWL_PartStateMask dwStates = 0;
  if (bFlagged)
    dwStates |= CFWL_PartState_Flagged;
  if (bSelected)
    dwStates |= CFWL_PartState_Selected;
  return dwStates;
}
