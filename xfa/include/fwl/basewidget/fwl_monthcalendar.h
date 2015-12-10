// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_MONTHCALENDAR_H
#define _FWL_MONTHCALENDAR_H
class IFWL_MonthCalendarDP;
class IFWL_MonthCalendar;
#define FWL_CLASS_MonthCalendar L"FWL_MONTHCALENDAR"
#define FWL_CLASSHASH_MonthCalendar 2733931374
#define FWL_STYLEEXT_MCD_MultiSelect (1L << 0)
#define FWL_STYLEEXT_MCD_NoToday (1L << 1)
#define FWL_STYLEEXT_MCD_NoTodayCircle (1L << 2)
#define FWL_STYLEEXT_MCD_WeekNumbers (1L << 3)
#define FWL_WGTCAPACITY_MC_HEADER_WIDTH 12
#define FWL_WGTCAPACITY_MC_HEADER_Height 13
#define FWL_WGTCAPACITY_MC_HEADER_BTN_WIDTH 14
#define FWL_WGTCAPACITY_MC_HEADER_BTN_HEIGHT 15
#define FWL_WGTCAPACITY_MC_HEADER_BTN_HMARGIN 16
#define FWL_WGTCAPACITY_MC_HEADER_BTN_VMARGIN 17
#define FWL_WGTCAPACITY_MC_HEADER_TEXTWIDHT 18
#define FWL_WGTCAPACITY_MC_HEADER_TEXTHEIGHT 19
#define FWL_WGTCAPACITY_MC_HEADER_TEXT_HMARGIN 20
#define FWL_WGTCAPACITY_MC_HEADER_TEXT_VMARGIN 21
#define FWL_WGTCAPACITY_MC_HSEP_WIDTH 22
#define FWL_WGTCAPACITY_MC_HSEP_HEIGHT 23
#define FWL_WGTCAPACITY_MC_VSEP_WIDTH 24
#define FWL_WGTCAPACITY_MC_VSEP_HEIGHT 25
#define FWL_WGTCAPACITY_MC_WEEKNUM_WIDTH 26
#define FWL_WGTCAPACITY_MC_SEP_DOFFSET 40
#define FWL_WGTCAPACITY_MC_SEP_X 27
#define FWL_WGTCAPACITY_MC_SEP_Y 28
#define FWL_WGTCAPACITY_MC_WEEKNUM_HEIGHT 29
#define FWL_WGTCAPACITY_MC_WEEK_WIDTH 30
#define FWL_WGTCAPACITY_MC_WEEK_HEIGHT 31
#define FWL_WGTCAPACITY_MC_DATES_CELL_WIDTH 32
#define FWL_WGTCAPACITY_MC_DATES_CELL_HEIGHT 33
#define FWL_WGTCAPACITY_MC_TODAY_WIDHT 34
#define FWL_WGTCAPACITY_MC_TODAY_HEIGHT 35
#define FWL_WGTCAPACITY_MC_TODAY_FLAG_WIDHT 36
#define FWL_WGTCAPACITY_MC_WIDTH 37
#define FWL_WGTCAPACITY_MC_HEIGHT 38
#define FWL_ITEMSTATE_MCD_Nomal (0L << 0)
#define FWL_ITEMSTATE_MCD_Flag (1L << 0)
#define FWL_ITEMSTATE_MCD_Selected (1L << 1)
#define FWL_ITEMSTATE_MCD_Focused (1L << 2)
#define FWL_PART_MCD_Border 1
#define FWL_PART_MCD_Edge 2
#define FWL_PART_MCD_Background 3
#define FWL_PART_MCD_LBtn 4
#define FWL_PART_MCD_RBtn 5
#define FWL_PART_MCD_HSeparator 6
#define FWL_PART_MCD_VSeparator 7
#define FWL_PART_MCD_TodayCircle 8
#define FWL_PART_MCD_DateInCircle 9
#define FWL_PART_MCD_DateInBK 10
#define FWL_PART_MCD_Caption 9
#define FWL_PART_MCD_DatesIn 10
#define FWL_PART_MCD_DatesOut 11
#define FWL_PART_MCD_Week 12
#define FWL_PART_MCD_Today 13
#define FWL_PART_MCD_Header 14
#define FWL_PART_MCD_WeekNum 15
#define FWL_PART_MCD_WeekNumSep 16
#define FWL_PARTSTATE_MCD_Normal (0L << 0)
#define FWL_PARTSTATE_MCD_Pressed (1L << 0)
#define FWL_PARTSTATE_MCD_Hovered (2L << 0)
#define FWL_PARTSTATE_MCD_Selected (3L << 0)
#define FWL_PARTSTATE_MCD_LSelected (1L << 2)
#define FWL_PARTSTATE_MCD_RSelected (2L << 2)
#define FWL_PARTSTATE_MCD_Flagged (1L << 3)
#define FWL_PARTSTATE_MCD_Focused (1L << 4)
#define FWL_MCCAPACITY_Sun FWL_WGTCAPACITY_MAX + 5
#define FWL_MCCAPACITY_Mon FWL_WGTCAPACITY_MAX + 6
#define FWL_MCCAPACITY_Tue FWL_WGTCAPACITY_MAX + 7
#define FWL_MCCAPACITY_Wed FWL_WGTCAPACITY_MAX + 8
#define FWL_MCCAPACITY_Thu FWL_WGTCAPACITY_MAX + 9
#define FWL_MCCAPACITY_Fri FWL_WGTCAPACITY_MAX + 10
#define FWL_MCCAPACITY_Sat FWL_WGTCAPACITY_MAX + 11
#define FWL_MCCAPACITY_January FWL_WGTCAPACITY_MAX + 12
#define FWL_MCCAPACITY_February FWL_WGTCAPACITY_MAX + 13
#define FWL_MCCAPACITY_March FWL_WGTCAPACITY_MAX + 14
#define FWL_MCCAPACITY_April FWL_WGTCAPACITY_MAX + 15
#define FWL_MCCAPACITY_May FWL_WGTCAPACITY_MAX + 16
#define FWL_MCCAPACITY_June FWL_WGTCAPACITY_MAX + 17
#define FWL_MCCAPACITY_July FWL_WGTCAPACITY_MAX + 18
#define FWL_MCCAPACITY_August FWL_WGTCAPACITY_MAX + 19
#define FWL_MCCAPACITY_September FWL_WGTCAPACITY_MAX + 20
#define FWL_MCCAPACITY_October FWL_WGTCAPACITY_MAX + 21
#define FWL_MCCAPACITY_November FWL_WGTCAPACITY_MAX + 22
#define FWL_MCCAPACITY_December FWL_WGTCAPACITY_MAX + 23
#define FWL_MCCAPACITY_Today FWL_WGTCAPACITY_MAX + 24
#define FWL_EVENT_MCD_DATESELECTED L"FWL_EVENT_MCD_DateSelected"
#define FWL_EVT_MCD_DateChanged L"FWL_EVENT_MCD_DateChanged"
#define FWL_NOTEHASH_MCD_DATASELECTED 1085596932
#define FWL_EVTHASH_MCD_DateChanged 54212227
BEGIN_FWL_EVENT_DEF(CFWL_Event_McdDateSelected, FWL_NOTEHASH_MCD_DATASELECTED)
int32_t m_iStartDay;
int32_t m_iEndDay;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EventMcdDateChanged, FWL_EVTHASH_MCD_DateChanged)
int32_t m_iOldYear;
int32_t m_iOldMonth;
int32_t m_iStartDay;
int32_t m_iEndDay;
END_FWL_EVENT_DEF
class IFWL_MonthCalendarDP : public IFWL_DataProvider {
 public:
  virtual int32_t GetCurDay(IFWL_Widget* pWidget) = 0;
  virtual int32_t GetCurMonth(IFWL_Widget* pWidget) = 0;
  virtual int32_t GetCurYear(IFWL_Widget* pWidget) = 0;
};
class IFWL_MonthCalendar : public IFWL_Widget {
 public:
  static IFWL_MonthCalendar* Create(const CFWL_WidgetImpProperties& properties,
                                    IFWL_Widget* pOuter);

  int32_t CountSelect();
  FX_BOOL GetSelect(int32_t& iYear,
                    int32_t& iMonth,
                    int32_t& iDay,
                    int32_t nIndex = 0);
  FX_BOOL SetSelect(int32_t iYear, int32_t iMonth, int32_t iDay);

 protected:
  IFWL_MonthCalendar();
};
#endif
