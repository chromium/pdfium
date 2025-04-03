// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_MONTHCALENDAR_H_
#define XFA_FWL_CFWL_MONTHCALENDAR_H_

#include <memory>
#include <vector>

#include "core/fxcrt/widestring.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_widget.h"

namespace pdfium {

class CFWL_MessageMouse;

class CFWL_MonthCalendar final : public CFWL_Widget {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_MonthCalendar() override;

  // CFWL_Widget:
  FWL_Type GetClassID() const override;
  CFX_RectF GetAutosizedWidgetRect() override;
  void Update() override;
  void DrawWidget(CFGAS_GEGraphics* pGraphics,
                  const CFX_Matrix& matrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;

  void SetSelect(int32_t iYear, int32_t iMonth, int32_t iDay);

 private:
  struct DATE {
    DATE() : iYear(0), iMonth(0), iDay(0) {}

    DATE(int32_t year, int32_t month, int32_t day)
        : iYear(year), iMonth(month), iDay(day) {}

    bool operator<(const DATE& right) {
      if (iYear < right.iYear)
        return true;
      if (iYear == right.iYear) {
        if (iMonth < right.iMonth)
          return true;
        if (iMonth == right.iMonth)
          return iDay < right.iDay;
      }
      return false;
    }

    bool operator>(const DATE& right) {
      if (iYear > right.iYear)
        return true;
      if (iYear == right.iYear) {
        if (iMonth > right.iMonth)
          return true;
        if (iMonth == right.iMonth)
          return iDay > right.iDay;
      }
      return false;
    }

    int32_t iYear;
    int32_t iMonth;
    int32_t iDay;
  };

  struct DATEINFO {
    DATEINFO(int32_t day,
             int32_t dayofweek,
             bool bFlag,
             bool bSelect,
             const WideString& wsday);
    ~DATEINFO();

    Mask<CFWL_PartState> AsPartStateMask() const;

    const int32_t iDay;
    const int32_t iDayOfWeek;
    bool bFlagged;
    bool bSelected;
    CFX_RectF rect;
    const WideString wsDay;
  };

  CFWL_MonthCalendar(CFWL_App* app,
                     const Properties& properties,
                     CFWL_Widget* pOuter);

  void DrawBackground(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawHeadBK(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawLButton(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawRButton(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawCaption(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawSeparator(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawDatesInBK(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawWeek(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawToday(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawDatesIn(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawDatesOut(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);
  void DrawDatesInCircle(CFGAS_GEGraphics* pGraphics,
                         const CFX_Matrix& mtMatrix);
  CFX_SizeF CalcSize();
  void Layout();
  void CalcHeadSize();
  void CalcTodaySize();
  void CalDateItem();
  void InitDate();
  void ClearDateItem();
  void ResetDateItem();
  void NextMonth();
  void PrevMonth();
  void ChangeToMonth(int32_t iYear, int32_t iMonth);
  void RemoveSelDay();
  void AddSelDay(int32_t iDay);
  void JumpToToday();
  WideString GetHeadText(int32_t iYear, int32_t iMonth);
  WideString GetTodayText(int32_t iYear, int32_t iMonth, int32_t iDay);
  int32_t GetDayAtPoint(const CFX_PointF& point) const;
  CFX_RectF GetDayRect(int32_t iDay);
  void OnLButtonDown(CFWL_MessageMouse* pMsg);
  void OnLButtonUp(CFWL_MessageMouse* pMsg);
  void OnMouseMove(CFWL_MessageMouse* pMsg);
  void OnMouseLeave(CFWL_MessageMouse* pMsg);

  bool initialized_ = false;
  CFX_RectF head_rect_;
  CFX_RectF week_rect_;
  CFX_RectF lbtn_rect_;
  CFX_RectF rbtn_rect_;
  CFX_RectF dates_rect_;
  CFX_RectF hsep_rect_;
  CFX_RectF head_text_rect_;
  CFX_RectF today_rect_;
  CFX_RectF today_flag_rect_;
  WideString head_;
  WideString today_;
  std::vector<std::unique_ptr<DATEINFO>> date_array_;
  int32_t cur_year_ = 2011;
  int32_t cur_month_ = 1;
  int32_t year_ = 2011;
  int32_t month_ = 1;
  int32_t day_ = 1;
  int32_t hovered_ = -1;
  Mask<CFWL_PartState> lbtn_part_states_ = CFWL_PartState::kNormal;
  Mask<CFWL_PartState> rbtn_part_states_ = CFWL_PartState::kNormal;
  DATE dt_min_;
  DATE dt_max_;
  CFX_SizeF head_size_;
  CFX_SizeF cell_size_;
  CFX_SizeF today_size_;
  std::vector<int32_t> sel_day_array_;
  CFX_RectF client_rect_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_MonthCalendar;

#endif  // XFA_FWL_CFWL_MONTHCALENDAR_H_
