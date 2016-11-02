// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_DATETIMEPICKER_H_
#define XFA_FWL_CORE_IFWL_DATETIMEPICKER_H_

#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/cfwl_widgetimpproperties.h"
#include "xfa/fwl/core/ifwl_dataprovider.h"
#include "xfa/fwl/core/ifwl_monthcalendar.h"
#include "xfa/fwl/core/ifwl_widget.h"

#define FWL_STYLEEXT_DTP_AllowEdit (1L << 0)
#define FWL_STYLEEXT_DTP_LongDateFormat (0L << 1)
#define FWL_STYLEEXT_DTP_ShortDateFormat (1L << 1)
#define FWL_STYLEEXT_DTP_TimeFormat (2L << 1)
#define FWL_STYLEEXT_DTP_Spin (1L << 3)
#define FWL_STYLEEXT_DTP_EditHNear (0L << 4)
#define FWL_STYLEEXT_DTP_EditHCenter (1L << 4)
#define FWL_STYLEEXT_DTP_EditHFar (2L << 4)
#define FWL_STYLEEXT_DTP_EditVNear (0L << 6)
#define FWL_STYLEEXT_DTP_EditVCenter (1L << 6)
#define FWL_STYLEEXT_DTP_EditVFar (2L << 6)
#define FWL_STYLEEXT_DTP_EditJustified (1L << 8)
#define FWL_STYLEEXT_DTP_EditDistributed (2L << 8)
#define FWL_STYLEEXT_DTP_EditHAlignMask (3L << 4)
#define FWL_STYLEEXT_DTP_EditVAlignMask (3L << 6)
#define FWL_STYLEEXT_DTP_EditHAlignModeMask (3L << 8)

class IFWL_DateTimeCalendar;
class IFWL_DateTimeEdit;
class IFWL_FormProxy;

FWL_EVENT_DEF(CFWL_Event_DtpDropDown, CFWL_EventType::DropDown)

FWL_EVENT_DEF(CFWL_Event_DtpCloseUp, CFWL_EventType::CloseUp)

FWL_EVENT_DEF(CFWL_Event_DtpEditChanged,
              CFWL_EventType::EditChanged,
              CFX_WideString m_wsText;)

FWL_EVENT_DEF(CFWL_Event_DtpHoverChanged,
              CFWL_EventType::HoverChanged,
              int32_t hoverday;)

FWL_EVENT_DEF(CFWL_Event_DtpSelectChanged,
              CFWL_EventType::SelectChanged,
              int32_t iYear;
              int32_t iMonth;
              int32_t iDay;)

class IFWL_DateTimePickerDP : public IFWL_DataProvider {
 public:
  virtual FWL_Error GetToday(IFWL_Widget* pWidget,
                             int32_t& iYear,
                             int32_t& iMonth,
                             int32_t& iDay) = 0;
};

class IFWL_DateTimePicker : public IFWL_Widget {
 public:
  explicit IFWL_DateTimePicker(const IFWL_App* app,
                               const CFWL_WidgetImpProperties& properties);
  ~IFWL_DateTimePicker() override;

  // IFWL_Widget
  FWL_Type GetClassID() const override;
  FWL_Error GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE) override;
  FWL_Error Update() override;
  FWL_WidgetHit HitTest(FX_FLOAT fx, FX_FLOAT fy) override;
  FWL_Error DrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = nullptr) override;
  FWL_Error SetThemeProvider(IFWL_ThemeProvider* pTP) override;

  FWL_Error GetCurSel(int32_t& iYear, int32_t& iMonth, int32_t& iDay);
  FWL_Error SetCurSel(int32_t iYear, int32_t iMonth, int32_t iDay);
  FWL_Error SetEditText(const CFX_WideString& wsText);
  FWL_Error GetEditText(CFX_WideString& wsText,
                        int32_t nStart = 0,
                        int32_t nCount = -1) const;
  int32_t CountSelRanges();
  int32_t GetSelRange(int32_t nIndex, int32_t& nStart);

  FX_BOOL CanUndo();
  FX_BOOL CanRedo();
  FX_BOOL Undo();
  FX_BOOL Redo();
  FX_BOOL CanCopy();
  FX_BOOL CanCut();
  FX_BOOL CanSelectAll();
  FX_BOOL Copy(CFX_WideString& wsCopy);
  FX_BOOL Cut(CFX_WideString& wsCut);
  FX_BOOL Paste(const CFX_WideString& wsPaste);
  FX_BOOL SelectAll();
  FX_BOOL Delete();
  FX_BOOL DeSelect();
  FWL_Error GetBBox(CFX_RectF& rect);
  FWL_Error SetEditLimit(int32_t nLimit);
  FWL_Error ModifyEditStylesEx(uint32_t dwStylesExAdded,
                               uint32_t dwStylesExRemoved);
  IFWL_DateTimeEdit* GetDataTimeEdit();

 protected:
  friend class CFWL_DateTimeEditImpDelegate;
  friend class IFWL_DateTimeCalendar;
  friend class CFWL_DateTimeCalendarImpDelegate;
  friend class CFWL_DateTimePickerImpDelegate;

  class CFWL_MonthCalendarImpDP : public IFWL_MonthCalendarDP {
   public:
    CFWL_MonthCalendarImpDP();

    // IFWL_DataProvider
    FWL_Error GetCaption(IFWL_Widget* pWidget,
                         CFX_WideString& wsCaption) override;

    // IFWL_MonthCalendarDP
    int32_t GetCurDay(IFWL_Widget* pWidget) override;
    int32_t GetCurMonth(IFWL_Widget* pWidget) override;
    int32_t GetCurYear(IFWL_Widget* pWidget) override;

    int32_t m_iCurDay;
    int32_t m_iCurYear;
    int32_t m_iCurMonth;
  };

  void DrawDropDownButton(CFX_Graphics* pGraphics,
                          IFWL_ThemeProvider* pTheme,
                          const CFX_Matrix* pMatrix);
  void FormatDateString(int32_t iYear,
                        int32_t iMonth,
                        int32_t iDay,
                        CFX_WideString& wsText);
  void ShowMonthCalendar(FX_BOOL bActivate);
  FX_BOOL IsMonthCalendarShowed();
  void ReSetEditAlignment();
  void InitProxyForm();
  void ProcessSelChanged(int32_t iYear, int32_t iMonth, int32_t iDay);

  CFX_RectF m_rtBtn;
  CFX_RectF m_rtClient;
  int32_t m_iBtnState;
  int32_t m_iYear;
  int32_t m_iMonth;
  int32_t m_iDay;
  FX_BOOL m_bLBtnDown;
  std::unique_ptr<IFWL_DateTimeEdit> m_pEdit;
  std::unique_ptr<IFWL_DateTimeCalendar> m_pMonthCal;
  std::unique_ptr<IFWL_FormProxy> m_pForm;
  FX_FLOAT m_fBtn;
  CFWL_MonthCalendarImpDP m_MonthCalendarDP;

 private:
  FWL_Error DisForm_Initialize();
  void DisForm_InitDateTimeCalendar();
  void DisForm_InitDateTimeEdit();
  FX_BOOL DisForm_IsMonthCalendarShowed();
  void DisForm_ShowMonthCalendar(FX_BOOL bActivate);
  FWL_WidgetHit DisForm_HitTest(FX_FLOAT fx, FX_FLOAT fy);
  FX_BOOL DisForm_IsNeedShowButton();
  FWL_Error DisForm_Update();
  FWL_Error DisForm_GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  FWL_Error DisForm_GetBBox(CFX_RectF& rect);
  FWL_Error DisForm_DrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = nullptr);
};

class CFWL_DateTimePickerImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_DateTimePickerImpDelegate(IFWL_DateTimePicker* pOwner);

  // CFWL_WidgetImpDelegate
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix = nullptr) override;

 protected:
  void OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnMouseLeave(CFWL_MsgMouse* pMsg);

  IFWL_DateTimePicker* m_pOwner;

 private:
  void DisForm_OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
};

#endif  // XFA_FWL_CORE_IFWL_DATETIMEPICKER_H_
