// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_DATETIMEPICKER_H_
#define XFA_FWL_CFWL_DATETIMEPICKER_H_

#include <memory>

#include "xfa/fwl/cfwl_datetimeedit.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_monthcalendar.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/cfwl_widgetproperties.h"

#define FWL_STYLEEXT_DTP_LongDateFormat 0
#define FWL_STYLEEXT_DTP_ShortDateFormat (1L << 1)
#define FWL_STYLEEXT_DTP_EditHNear 0
#define FWL_STYLEEXT_DTP_EditHCenter (1L << 4)
#define FWL_STYLEEXT_DTP_EditHFar (2L << 4)
#define FWL_STYLEEXT_DTP_EditVNear 0
#define FWL_STYLEEXT_DTP_EditVCenter (1L << 6)
#define FWL_STYLEEXT_DTP_EditVFar (2L << 6)
#define FWL_STYLEEXT_DTP_EditJustified (1L << 8)
#define FWL_STYLEEXT_DTP_EditHAlignMask (3L << 4)
#define FWL_STYLEEXT_DTP_EditVAlignMask (3L << 6)

class CFWL_DateTimeEdit;
class CFWL_FormProxy;

class CFWL_DateTimePicker : public CFWL_Widget {
 public:
  explicit CFWL_DateTimePicker(const CFWL_App* pApp);
  ~CFWL_DateTimePicker() override;

  // CFWL_Widget
  FWL_Type GetClassID() const override;
  void Update() override;
  FWL_WidgetHit HitTest(const CFX_PointF& point) override;
  void DrawWidget(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix) override;
  void SetThemeProvider(IFWL_ThemeProvider* pTP) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix) override;

  void GetCurSel(int32_t& iYear, int32_t& iMonth, int32_t& iDay);
  void SetCurSel(int32_t iYear, int32_t iMonth, int32_t iDay);

  void SetEditText(const CFX_WideString& wsText);
  CFX_WideString GetEditText() const;

  int32_t CountSelRanges() const { return m_pEdit->CountSelRanges(); }
  int32_t GetSelRange(int32_t nIndex, int32_t* nStart) const {
    return m_pEdit->GetSelRange(nIndex, nStart);
  }

  CFX_RectF GetBBox() const;
  void SetEditLimit(int32_t nLimit) { m_pEdit->SetLimit(nLimit); }
  void ModifyEditStylesEx(uint32_t dwStylesExAdded, uint32_t dwStylesExRemoved);

  bool IsMonthCalendarVisible() const;
  void ShowMonthCalendar(bool bActivate);
  void ProcessSelChanged(int32_t iYear, int32_t iMonth, int32_t iDay);

  CFWL_FormProxy* GetFormProxy() const { return m_pForm.get(); }

 private:
  void DrawDropDownButton(CFX_Graphics* pGraphics,
                          IFWL_ThemeProvider* pTheme,
                          const CFX_Matrix* pMatrix);
  void FormatDateString(int32_t iYear,
                        int32_t iMonth,
                        int32_t iDay,
                        CFX_WideString& wsText);
  void ResetEditAlignment();
  void InitProxyForm();
  void OnFocusChanged(CFWL_Message* pMsg, bool bSet);
  void OnLButtonDown(CFWL_MessageMouse* pMsg);
  void OnLButtonUp(CFWL_MessageMouse* pMsg);
  void OnMouseMove(CFWL_MessageMouse* pMsg);
  void OnMouseLeave(CFWL_MessageMouse* pMsg);

  bool DisForm_IsMonthCalendarVisible() const;
  void DisForm_ShowMonthCalendar(bool bActivate);
  FWL_WidgetHit DisForm_HitTest(const CFX_PointF& point) const;
  bool DisForm_IsNeedShowButton() const;
  void DisForm_Update();
  CFX_RectF DisForm_GetBBox() const;
  void DisForm_DrawWidget(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix);
  void DisForm_OnFocusChanged(CFWL_Message* pMsg, bool bSet);

  CFX_RectF m_rtBtn;
  CFX_RectF m_rtClient;
  int32_t m_iBtnState;
  int32_t m_iYear;
  int32_t m_iMonth;
  int32_t m_iDay;
  bool m_bLBtnDown;
  std::unique_ptr<CFWL_DateTimeEdit> m_pEdit;
  std::unique_ptr<CFWL_MonthCalendar> m_pMonthCal;
  std::unique_ptr<CFWL_FormProxy> m_pForm;
  FX_FLOAT m_fBtn;
};

#endif  // XFA_FWL_CFWL_DATETIMEPICKER_H_
