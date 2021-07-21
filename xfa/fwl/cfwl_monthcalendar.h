// Copyright 2014 PDFium Authors. All rights reserved.
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

class CFWL_MessageMouse;

class CFWL_MonthCalendar final : public CFWL_Widget {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_MonthCalendar() override;

  // FWL_WidgetImp
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

    CFWL_PartStateMask AsPartStateMask() const;

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

  bool m_bInitialized = false;
  CFX_RectF m_HeadRect;
  CFX_RectF m_WeekRect;
  CFX_RectF m_LBtnRect;
  CFX_RectF m_RBtnRect;
  CFX_RectF m_DatesRect;
  CFX_RectF m_HSepRect;
  CFX_RectF m_HeadTextRect;
  CFX_RectF m_TodayRect;
  CFX_RectF m_TodayFlagRect;
  WideString m_wsHead;
  WideString m_wsToday;
  std::vector<std::unique_ptr<DATEINFO>> m_DateArray;
  int32_t m_iCurYear = 2011;
  int32_t m_iCurMonth = 1;
  int32_t m_iYear = 2011;
  int32_t m_iMonth = 1;
  int32_t m_iDay = 1;
  int32_t m_iHovered = -1;
  int32_t m_iLBtnPartStates = CFWL_PartState_Normal;
  int32_t m_iRBtnPartStates = CFWL_PartState_Normal;
  DATE m_dtMin;
  DATE m_dtMax;
  CFX_SizeF m_HeadSize;
  CFX_SizeF m_CellSize;
  CFX_SizeF m_TodaySize;
  std::vector<int32_t> m_SelDayArray;
  CFX_RectF m_ClientRect;
};

#endif  // XFA_FWL_CFWL_MONTHCALENDAR_H_
