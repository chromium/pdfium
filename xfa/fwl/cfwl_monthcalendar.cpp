// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_monthcalendar.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxcrt/cfx_datetime.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/stl_util.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fwl/cfwl_datetimepicker.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace pdfium {

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
      return WideString::FromASCII("Sun");
    case 1:
      return WideString::FromASCII("Mon");
    case 2:
      return WideString::FromASCII("Tue");
    case 3:
      return WideString::FromASCII("Wed");
    case 4:
      return WideString::FromASCII("Thu");
    case 5:
      return WideString::FromASCII("Fri");
    case 6:
      return WideString::FromASCII("Sat");
    default:
      NOTREACHED();
  }
}

WideString GetMonth(int month) {
  switch (month) {
    case 0:
      return WideString::FromASCII("January");
    case 1:
      return WideString::FromASCII("February");
    case 2:
      return WideString::FromASCII("March");
    case 3:
      return WideString::FromASCII("April");
    case 4:
      return WideString::FromASCII("May");
    case 5:
      return WideString::FromASCII("June");
    case 6:
      return WideString::FromASCII("July");
    case 7:
      return WideString::FromASCII("August");
    case 8:
      return WideString::FromASCII("September");
    case 9:
      return WideString::FromASCII("October");
    case 10:
      return WideString::FromASCII("November");
    case 11:
      return WideString::FromASCII("December");
    default:
      NOTREACHED();
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

  if (!initialized_) {
    InitDate();
    initialized_ = true;
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
  CFWL_ThemeBackground params(CFWL_ThemePart::Part::kBackground, this,
                              pGraphics);
  params.part_rect_ = client_rect_;
  params.matrix_ = mtMatrix;
  GetThemeProvider()->DrawBackground(params);
}

void CFWL_MonthCalendar::DrawHeadBK(CFGAS_GEGraphics* pGraphics,
                                    const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground params(CFWL_ThemePart::Part::kHeader, this, pGraphics);
  params.part_rect_ = head_rect_;
  params.matrix_ = mtMatrix;
  GetThemeProvider()->DrawBackground(params);
}

void CFWL_MonthCalendar::DrawLButton(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground params(CFWL_ThemePart::Part::kLBtn, this, pGraphics);
  params.states_ = lbtn_part_states_;
  params.part_rect_ = lbtn_rect_;
  params.matrix_ = mtMatrix;
  GetThemeProvider()->DrawBackground(params);
}

void CFWL_MonthCalendar::DrawRButton(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground params(CFWL_ThemePart::Part::kRBtn, this, pGraphics);
  params.states_ = rbtn_part_states_;
  params.part_rect_ = rbtn_rect_;
  params.matrix_ = mtMatrix;
  GetThemeProvider()->DrawBackground(params);
}

void CFWL_MonthCalendar::DrawCaption(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& mtMatrix) {
  CFWL_ThemeText textParam(CFWL_ThemePart::Part::kCaption, this, pGraphics);
  textParam.text_ = GetHeadText(cur_year_, cur_month_);
  head_size_ = CalcTextSize(textParam.text_, false);
  CalcHeadSize();
  textParam.part_rect_ = head_text_rect_;
  textParam.tto_styles_.single_line_ = true;
  textParam.tto_align_ = FDE_TextAlignment::kCenter;
  textParam.matrix_ = mtMatrix;
  GetThemeProvider()->DrawText(textParam);
}

void CFWL_MonthCalendar::DrawSeparator(CFGAS_GEGraphics* pGraphics,
                                       const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground params(CFWL_ThemePart::Part::kHSeparator, this,
                              pGraphics);
  params.part_rect_ = hsep_rect_;
  params.matrix_ = mtMatrix;
  GetThemeProvider()->DrawBackground(params);
}

void CFWL_MonthCalendar::DrawDatesInBK(CFGAS_GEGraphics* pGraphics,
                                       const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground params(CFWL_ThemePart::Part::kDateInBK, this, pGraphics);
  params.matrix_ = mtMatrix;

  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  int32_t iCount = fxcrt::CollectionSize<int32_t>(date_array_);
  for (int32_t j = 0; j < iCount; j++) {
    DATEINFO* pDataInfo = date_array_[j].get();
    if (pDataInfo->bSelected) {
      params.states_ |= CFWL_PartState::kSelected;
      if (pDataInfo->bFlagged) {
        params.states_ |= CFWL_PartState::kFlagged;
      }
    } else if (j == hovered_ - 1) {
      params.states_ |= CFWL_PartState::kHovered;
    } else if (pDataInfo->bFlagged) {
      params.states_ = CFWL_PartState::kFlagged;
      pTheme->DrawBackground(params);
    }
    params.part_rect_ = pDataInfo->rect;
    pTheme->DrawBackground(params);
    params.states_ = CFWL_PartState::kNormal;
  }
}

void CFWL_MonthCalendar::DrawWeek(CFGAS_GEGraphics* pGraphics,
                                  const CFX_Matrix& mtMatrix) {
  CFWL_ThemeText params(CFWL_ThemePart::Part::kWeek, this, pGraphics);
  params.tto_align_ = FDE_TextAlignment::kCenter;
  params.tto_styles_.single_line_ = true;
  params.matrix_ = mtMatrix;

  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  CFX_RectF rtDayOfWeek;
  for (int32_t i = 0; i < 7; ++i) {
    rtDayOfWeek = CFX_RectF(
        week_rect_.left + i * (cell_size_.width + kMonthCalHMargin * 2),
        week_rect_.top, cell_size_);

    params.part_rect_ = rtDayOfWeek;
    params.text_ = GetAbbreviatedDayOfWeek(i);
    pTheme->DrawText(params);
  }
}

void CFWL_MonthCalendar::DrawToday(CFGAS_GEGraphics* pGraphics,
                                   const CFX_Matrix& mtMatrix) {
  CFWL_ThemeText params(CFWL_ThemePart::Part::kToday, this, pGraphics);
  params.tto_align_ = FDE_TextAlignment::kCenterLeft;
  params.text_ = GetTodayText(year_, month_, day_);
  today_size_ = CalcTextSize(params.text_, false);
  CalcTodaySize();
  params.part_rect_ = today_rect_;
  params.tto_styles_.single_line_ = true;
  params.matrix_ = mtMatrix;
  GetThemeProvider()->DrawText(params);
}

void CFWL_MonthCalendar::DrawDatesIn(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& mtMatrix) {
  CFWL_ThemeText params(CFWL_ThemePart::Part::kDatesIn, this, pGraphics);
  params.tto_align_ = FDE_TextAlignment::kCenter;
  params.matrix_ = mtMatrix;

  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  int32_t iCount = fxcrt::CollectionSize<int32_t>(date_array_);
  for (int32_t j = 0; j < iCount; j++) {
    DATEINFO* pDataInfo = date_array_[j].get();
    params.text_ = pDataInfo->wsDay;
    params.part_rect_ = pDataInfo->rect;
    params.states_ = pDataInfo->AsPartStateMask();
    if (j + 1 == hovered_) {
      params.states_ |= CFWL_PartState::kHovered;
    }

    params.tto_styles_.single_line_ = true;
    pTheme->DrawText(params);
  }
}

void CFWL_MonthCalendar::DrawDatesOut(CFGAS_GEGraphics* pGraphics,
                                      const CFX_Matrix& mtMatrix) {
  CFWL_ThemeText params(CFWL_ThemePart::Part::kDatesOut, this, pGraphics);
  params.tto_align_ = FDE_TextAlignment::kCenter;
  params.matrix_ = mtMatrix;
  GetThemeProvider()->DrawText(params);
}

void CFWL_MonthCalendar::DrawDatesInCircle(CFGAS_GEGraphics* pGraphics,
                                           const CFX_Matrix& mtMatrix) {
  if (month_ != cur_month_ || year_ != cur_year_) {
    return;
  }

  if (day_ < 1 || day_ > fxcrt::CollectionSize<int32_t>(date_array_)) {
    return;
  }

  DATEINFO* pDate = date_array_[day_ - 1].get();
  if (!pDate)
    return;

  CFWL_ThemeBackground params(CFWL_ThemePart::Part::kDateInCircle, this,
                              pGraphics);
  params.part_rect_ = pDate->rect;
  params.matrix_ = mtMatrix;
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
    CFX_SizeF sz = CalcTextSize(WideString::FormatInteger(day), false);
    fDayMaxW = (fDayMaxW >= sz.width) ? fDayMaxW : sz.width;
    fDayMaxH = (fDayMaxH >= sz.height) ? fDayMaxH : sz.height;
  }
  cell_size_.width =
      static_cast<int>(0.5 + (fMaxWeekW >= fDayMaxW ? fMaxWeekW : fDayMaxW));
  cell_size_.height = fMaxWeekH >= fDayMaxH ? fMaxWeekH : fDayMaxH;

  CFX_SizeF fs;
  fs.width = cell_size_.width * kMonthCalColumns +
             kMonthCalHMargin * kMonthCalColumns * 2 +
             kMonthCalHeaderBtnHMargin * 2;

  float fMonthMaxW = 0.0f;
  float fMonthMaxH = 0.0f;
  for (int i = 0; i < 12; ++i) {
    CFX_SizeF sz = CalcTextSize(GetMonth(i), false);
    fMonthMaxW = (fMonthMaxW >= sz.width) ? fMonthMaxW : sz.width;
    fMonthMaxH = (fMonthMaxH >= sz.height) ? fMonthMaxH : sz.height;
  }

  CFX_SizeF szYear = CalcTextSize(GetHeadText(year_, month_), false);
  fMonthMaxH = std::max(fMonthMaxH, szYear.height);
  head_size_ = CFX_SizeF(fMonthMaxW + szYear.width, fMonthMaxH);
  fMonthMaxW =
      head_size_.width + kMonthCalHeaderBtnHMargin * 2 + cell_size_.width * 2;
  fs.width = std::max(fs.width, fMonthMaxW);

  today_ = GetTodayText(year_, month_, day_);
  today_size_ = CalcTextSize(today_, false);
  today_size_.height = (today_size_.height >= cell_size_.height)
                           ? today_size_.height
                           : cell_size_.height;
  fs.height = cell_size_.width + cell_size_.height * (kMonthCalRows - 2) +
              today_size_.height + kMonthCalVMargin * kMonthCalRows * 2 +
              kMonthCalHeaderBtnVMargin * 4;
  return fs;
}

void CFWL_MonthCalendar::CalcHeadSize() {
  float fHeadHMargin = (client_rect_.width - head_size_.width) / 2;
  float fHeadVMargin = (cell_size_.width - head_size_.height) / 2;
  head_text_rect_ = CFX_RectF(client_rect_.left + fHeadHMargin,
                              client_rect_.top + kMonthCalHeaderBtnVMargin +
                                  kMonthCalVMargin + fHeadVMargin,
                              head_size_);
}

void CFWL_MonthCalendar::CalcTodaySize() {
  today_flag_rect_ = CFX_RectF(
      client_rect_.left + kMonthCalHeaderBtnHMargin + kMonthCalHMargin,
      dates_rect_.bottom() + kMonthCalHeaderBtnVMargin + kMonthCalVMargin,
      cell_size_.width, today_size_.height);
  today_rect_ = CFX_RectF(
      client_rect_.left + kMonthCalHeaderBtnHMargin + cell_size_.width +
          kMonthCalHMargin * 2,
      dates_rect_.bottom() + kMonthCalHeaderBtnVMargin + kMonthCalVMargin,
      today_size_);
}

void CFWL_MonthCalendar::Layout() {
  client_rect_ = GetClientRect();

  head_rect_ = CFX_RectF(
      client_rect_.left + kMonthCalHeaderBtnHMargin, client_rect_.top,
      client_rect_.width - kMonthCalHeaderBtnHMargin * 2,
      cell_size_.width + (kMonthCalHeaderBtnVMargin + kMonthCalVMargin) * 2);
  week_rect_ = CFX_RectF(client_rect_.left + kMonthCalHeaderBtnHMargin,
                         head_rect_.bottom(),
                         client_rect_.width - kMonthCalHeaderBtnHMargin * 2,
                         cell_size_.height + kMonthCalVMargin * 2);
  lbtn_rect_ = CFX_RectF(client_rect_.left + kMonthCalHeaderBtnHMargin,
                         client_rect_.top + kMonthCalHeaderBtnVMargin,
                         cell_size_.width, cell_size_.width);
  rbtn_rect_ = CFX_RectF(client_rect_.left + client_rect_.width -
                             kMonthCalHeaderBtnHMargin - cell_size_.width,
                         client_rect_.top + kMonthCalHeaderBtnVMargin,
                         cell_size_.width, cell_size_.width);
  hsep_rect_ = CFX_RectF(
      client_rect_.left + kMonthCalHeaderBtnHMargin + kMonthCalHMargin,
      week_rect_.bottom() - kMonthCalVMargin,
      client_rect_.width - (kMonthCalHeaderBtnHMargin + kMonthCalHMargin) * 2,
      kMonthCalHSepHeight);
  dates_rect_ = CFX_RectF(client_rect_.left + kMonthCalHeaderBtnHMargin,
                          week_rect_.bottom(),
                          client_rect_.width - kMonthCalHeaderBtnHMargin * 2,
                          cell_size_.height * (kMonthCalRows - 3) +
                              kMonthCalVMargin * (kMonthCalRows - 3) * 2);

  CalDateItem();
}

void CFWL_MonthCalendar::CalDateItem() {
  bool bNewWeek = false;
  int32_t iWeekOfMonth = 0;
  float fLeft = dates_rect_.left;
  float fTop = dates_rect_.top;
  for (const auto& pDateInfo : date_array_) {
    if (bNewWeek) {
      iWeekOfMonth++;
      bNewWeek = false;
    }
    pDateInfo->rect = CFX_RectF(
        fLeft +
            pDateInfo->iDayOfWeek * (cell_size_.width + (kMonthCalHMargin * 2)),
        fTop + iWeekOfMonth * (cell_size_.height + (kMonthCalVMargin * 2)),
        cell_size_.width + (kMonthCalHMargin * 2),
        cell_size_.height + (kMonthCalVMargin * 2));
    if (pDateInfo->iDayOfWeek >= 6)
      bNewWeek = true;
  }
}

void CFWL_MonthCalendar::InitDate() {
  CFX_DateTime now = CFX_DateTime::Now();

  year_ = now.GetYear();
  month_ = now.GetMonth();
  day_ = now.GetDay();
  cur_year_ = year_;
  cur_month_ = month_;

  today_ = GetTodayText(year_, month_, day_);
  head_ = GetHeadText(cur_year_, cur_month_);
  dt_min_ = DATE(1500, 12, 1);
  dt_max_ = DATE(2200, 1, 1);
}

void CFWL_MonthCalendar::ClearDateItem() {
  date_array_.clear();
}

void CFWL_MonthCalendar::ResetDateItem() {
  int32_t iDays = FX_DaysInMonth(cur_year_, cur_month_);
  int32_t iDayOfWeek =
      CFX_DateTime(cur_year_, cur_month_, 1, 0, 0, 0, 0).GetDayOfWeek();
  for (int32_t i = 0; i < iDays; ++i, ++iDayOfWeek) {
    if (iDayOfWeek >= 7)
      iDayOfWeek = 0;

    const bool bFlagged =
        year_ == cur_year_ && month_ == cur_month_ && day_ == i + 1;
    const bool bSelected = Contains(sel_day_array_, i + 1);
    date_array_.push_back(
        std::make_unique<DATEINFO>(i + 1, iDayOfWeek, bFlagged, bSelected,
                                   WideString::FormatInteger(i + 1)));
  }
}

void CFWL_MonthCalendar::NextMonth() {
  int32_t iYear = cur_year_;
  int32_t iMonth = cur_month_;
  if (iMonth >= 12) {
    iMonth = 1;
    iYear++;
  } else {
    iMonth++;
  }
  DATE dt(cur_year_, cur_month_, 1);
  if (!(dt < dt_max_)) {
    return;
  }

  cur_year_ = iYear, cur_month_ = iMonth;
  ChangeToMonth(cur_year_, cur_month_);
}

void CFWL_MonthCalendar::PrevMonth() {
  int32_t iYear = cur_year_;
  int32_t iMonth = cur_month_;
  if (iMonth <= 1) {
    iMonth = 12;
    iYear--;
  } else {
    iMonth--;
  }

  DATE dt(cur_year_, cur_month_, 1);
  if (!(dt > dt_min_)) {
    return;
  }

  cur_year_ = iYear, cur_month_ = iMonth;
  ChangeToMonth(cur_year_, cur_month_);
}

void CFWL_MonthCalendar::ChangeToMonth(int32_t iYear, int32_t iMonth) {
  cur_year_ = iYear;
  cur_month_ = iMonth;
  hovered_ = -1;

  ClearDateItem();
  ResetDateItem();
  CalDateItem();
  head_ = GetHeadText(cur_year_, cur_month_);
}

void CFWL_MonthCalendar::RemoveSelDay() {
  int32_t iDatesCount = fxcrt::CollectionSize<int32_t>(date_array_);
  for (int32_t iSelDay : sel_day_array_) {
    if (iSelDay <= iDatesCount)
      date_array_[iSelDay - 1]->bSelected = false;
  }
  sel_day_array_.clear();
}

void CFWL_MonthCalendar::AddSelDay(int32_t iDay) {
  DCHECK(iDay > 0);
  if (!Contains(sel_day_array_, iDay)) {
    return;
  }

  RemoveSelDay();
  if (iDay <= fxcrt::CollectionSize<int32_t>(date_array_)) {
    date_array_[iDay - 1]->bSelected = true;
  }

  sel_day_array_.push_back(iDay);
}

void CFWL_MonthCalendar::JumpToToday() {
  if (year_ != cur_year_ || month_ != cur_month_) {
    cur_year_ = year_;
    cur_month_ = month_;
    ChangeToMonth(year_, month_);
    AddSelDay(day_);
    return;
  }

  if (!Contains(sel_day_array_, day_)) {
    AddSelDay(day_);
  }
}

WideString CFWL_MonthCalendar::GetHeadText(int32_t iYear, int32_t iMonth) {
  static const std::array<const wchar_t*, 12> pMonth = {
      {L"January", L"February", L"March", L"April", L"May", L"June", L"July",
       L"August", L"September", L"October", L"November", L"December"}};
  return WideString::Format(L"%ls, %d", pMonth[iMonth - 1], iYear);
}

WideString CFWL_MonthCalendar::GetTodayText(int32_t iYear,
                                            int32_t iMonth,
                                            int32_t iDay) {
  return WideString::Format(L"Today, %d/%d/%d", iDay, iMonth, iYear);
}

int32_t CFWL_MonthCalendar::GetDayAtPoint(const CFX_PointF& point) const {
  int i = 1;  // one-based day values.
  for (const auto& pDateInfo : date_array_) {
    if (pDateInfo->rect.Contains(point))
      return i;
    ++i;
  }
  return -1;
}

CFX_RectF CFWL_MonthCalendar::GetDayRect(int32_t iDay) {
  if (iDay <= 0 || iDay > fxcrt::CollectionSize<int32_t>(date_array_)) {
    return CFX_RectF();
  }

  DATEINFO* pDateInfo = date_array_[iDay - 1].get();
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
      switch (pMouse->cmd_) {
        case CFWL_MessageMouse::MouseCommand::kLeftButtonDown:
          OnLButtonDown(pMouse);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeftButtonUp:
          OnLButtonUp(pMouse);
          break;
        case CFWL_MessageMouse::MouseCommand::kMove:
          OnMouseMove(pMouse);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeave:
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
  if (lbtn_rect_.Contains(pMsg->pos_)) {
    lbtn_part_states_ = CFWL_PartState::kPressed;
    PrevMonth();
    RepaintRect(client_rect_);
  } else if (rbtn_rect_.Contains(pMsg->pos_)) {
    rbtn_part_states_ |= CFWL_PartState::kPressed;
    NextMonth();
    RepaintRect(client_rect_);
  } else if (today_rect_.Contains(pMsg->pos_)) {
    JumpToToday();
    RepaintRect(client_rect_);
  }
}

void CFWL_MonthCalendar::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  if (lbtn_rect_.Contains(pMsg->pos_)) {
    lbtn_part_states_ = CFWL_PartState::kNormal;
    RepaintRect(lbtn_rect_);
    return;
  }
  if (rbtn_rect_.Contains(pMsg->pos_)) {
    rbtn_part_states_ = CFWL_PartState::kNormal;
    RepaintRect(rbtn_rect_);
    return;
  }
  if (today_rect_.Contains(pMsg->pos_)) {
    return;
  }

  int32_t iOldSel = 0;
  if (!sel_day_array_.empty()) {
    iOldSel = sel_day_array_[0];
  }

  int32_t iCurSel = GetDayAtPoint(pMsg->pos_);
  if (iCurSel > 0) {
    DATEINFO* pDateInfo = date_array_[iCurSel - 1].get();
    CFX_RectF rtInvalidate(pDateInfo->rect);
    if (iOldSel > 0 && iOldSel <= fxcrt::CollectionSize<int32_t>(date_array_)) {
      pDateInfo = date_array_[iOldSel - 1].get();
      rtInvalidate.Union(pDateInfo->rect);
    }
    AddSelDay(iCurSel);
    CFWL_DateTimePicker* pDateTime =
        static_cast<CFWL_DateTimePicker*>(GetOuter());
    pDateTime->ProcessSelChanged(cur_year_, cur_month_, iCurSel);
    pDateTime->HideMonthCalendar();
  }
}

void CFWL_MonthCalendar::OnMouseMove(CFWL_MessageMouse* pMsg) {
  bool bRepaint = false;
  CFX_RectF rtInvalidate;
  if (dates_rect_.Contains(pMsg->pos_)) {
    int32_t iHover = GetDayAtPoint(pMsg->pos_);
    bRepaint = hovered_ != iHover;
    if (bRepaint) {
      if (hovered_ > 0) {
        rtInvalidate = GetDayRect(hovered_);
      }
      if (iHover > 0) {
        CFX_RectF rtDay = GetDayRect(iHover);
        if (rtInvalidate.IsEmpty())
          rtInvalidate = rtDay;
        else
          rtInvalidate.Union(rtDay);
      }
    }
    hovered_ = iHover;
  } else {
    bRepaint = hovered_ > 0;
    if (bRepaint)
      rtInvalidate = GetDayRect(hovered_);

    hovered_ = -1;
  }
  if (bRepaint && !rtInvalidate.IsEmpty())
    RepaintRect(rtInvalidate);
}

void CFWL_MonthCalendar::OnMouseLeave(CFWL_MessageMouse* pMsg) {
  if (hovered_ <= 0) {
    return;
  }

  CFX_RectF rtInvalidate = GetDayRect(hovered_);
  hovered_ = -1;
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

Mask<CFWL_PartState> CFWL_MonthCalendar::DATEINFO::AsPartStateMask() const {
  Mask<CFWL_PartState> dwStates = CFWL_PartState::kNormal;
  if (bFlagged)
    dwStates |= CFWL_PartState::kFlagged;
  if (bSelected)
    dwStates |= CFWL_PartState::kSelected;
  return dwStates;
}

}  // namespace pdfium
