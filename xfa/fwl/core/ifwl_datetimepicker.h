// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_DATETIMEPICKER_H_
#define XFA_FWL_CORE_IFWL_DATETIMEPICKER_H_

#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/cfwl_widgetproperties.h"
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

class IFWL_DateTimePicker : public IFWL_Widget, public IFWL_MonthCalendarDP {
 public:
  explicit IFWL_DateTimePicker(
      const IFWL_App* app,
      std::unique_ptr<CFWL_WidgetProperties> properties);
  ~IFWL_DateTimePicker() override;

  // IFWL_Widget
  FWL_Type GetClassID() const override;
  void GetWidgetRect(CFX_RectF& rect, bool bAutoSize = false) override;
  void Update() override;
  FWL_WidgetHit HitTest(FX_FLOAT fx, FX_FLOAT fy) override;
  void DrawWidget(CFX_Graphics* pGraphics,
                  const CFX_Matrix* pMatrix = nullptr) override;
  void SetThemeProvider(IFWL_ThemeProvider* pTP) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix) override;

  FWL_Error GetCurSel(int32_t& iYear, int32_t& iMonth, int32_t& iDay);
  FWL_Error SetCurSel(int32_t iYear, int32_t iMonth, int32_t iDay);
  FWL_Error SetEditText(const CFX_WideString& wsText);
  FWL_Error GetEditText(CFX_WideString& wsText,
                        int32_t nStart = 0,
                        int32_t nCount = -1) const;
  int32_t CountSelRanges();
  int32_t GetSelRange(int32_t nIndex, int32_t& nStart);

  bool CanUndo();
  bool CanRedo();
  bool Undo();
  bool Redo();
  bool CanCopy();
  bool CanCut();
  bool CanSelectAll();
  bool Copy(CFX_WideString& wsCopy);
  bool Cut(CFX_WideString& wsCut);
  bool Paste(const CFX_WideString& wsPaste);
  bool SelectAll();
  bool Delete();
  bool DeSelect();
  FWL_Error GetBBox(CFX_RectF& rect);
  FWL_Error SetEditLimit(int32_t nLimit);
  void ModifyEditStylesEx(uint32_t dwStylesExAdded, uint32_t dwStylesExRemoved);
  IFWL_DateTimeEdit* GetDataTimeEdit();

  bool IsMonthCalendarShowed();
  void ShowMonthCalendar(bool bActivate);
  void ProcessSelChanged(int32_t iYear, int32_t iMonth, int32_t iDay);

  IFWL_FormProxy* GetFormProxy() const { return m_pForm.get(); }

  // IFWL_DataProvider
  FWL_Error GetCaption(IFWL_Widget* pWidget,
                       CFX_WideString& wsCaption) override;

  // IFWL_MonthCalendarDP
  int32_t GetCurDay(IFWL_Widget* pWidget) override;
  int32_t GetCurMonth(IFWL_Widget* pWidget) override;
  int32_t GetCurYear(IFWL_Widget* pWidget) override;

 protected:
  void DrawDropDownButton(CFX_Graphics* pGraphics,
                          IFWL_ThemeProvider* pTheme,
                          const CFX_Matrix* pMatrix);
  void FormatDateString(int32_t iYear,
                        int32_t iMonth,
                        int32_t iDay,
                        CFX_WideString& wsText);
  void ReSetEditAlignment();
  void InitProxyForm();

  CFX_RectF m_rtBtn;
  CFX_RectF m_rtClient;
  int32_t m_iBtnState;
  int32_t m_iYear;
  int32_t m_iMonth;
  int32_t m_iDay;
  bool m_bLBtnDown;
  std::unique_ptr<IFWL_DateTimeEdit> m_pEdit;
  std::unique_ptr<IFWL_MonthCalendar> m_pMonthCal;
  std::unique_ptr<IFWL_FormProxy> m_pForm;
  FX_FLOAT m_fBtn;

 private:
  FWL_Error DisForm_Initialize();
  void DisForm_InitMonthCalendar();
  void DisForm_InitDateTimeEdit();
  bool DisForm_IsMonthCalendarShowed();
  void DisForm_ShowMonthCalendar(bool bActivate);
  FWL_WidgetHit DisForm_HitTest(FX_FLOAT fx, FX_FLOAT fy);
  bool DisForm_IsNeedShowButton();
  void DisForm_Update();
  void DisForm_GetWidgetRect(CFX_RectF& rect, bool bAutoSize = false);
  FWL_Error DisForm_GetBBox(CFX_RectF& rect);
  void DisForm_DrawWidget(CFX_Graphics* pGraphics,
                          const CFX_Matrix* pMatrix = nullptr);

  void OnFocusChanged(CFWL_Message* pMsg, bool bSet);
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnMouseLeave(CFWL_MsgMouse* pMsg);
  void DisForm_OnFocusChanged(CFWL_Message* pMsg, bool bSet);

  int32_t m_iCurYear;
  int32_t m_iCurMonth;
  int32_t m_iCurDay;
};

#endif  // XFA_FWL_CORE_IFWL_DATETIMEPICKER_H_
