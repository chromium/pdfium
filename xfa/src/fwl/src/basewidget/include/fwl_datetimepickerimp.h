// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_DATETIMEPICKER_IMP_H
#define _FWL_DATETIMEPICKER_IMP_H

#include <memory>

class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class CFWL_EditImp;
class CFWL_EditImpDelegate;
class CFWL_MonthCalendarImp;
class CFWL_MonthCalendarImpDelegate;
class CFWL_FormProxyImp;
class CFWL_DateTimeEdit;
class CFWL_DateTimeEditImpDelegate;
class CFWL_DateTimeCalendar;
class CFWL_DateTimeCalendarImpDelegate;
class CFWL_DateTimePickerImp;
class CFWL_DateTimePickerImpDelegate;

class IFWL_DateTimeForm : public IFWL_Form {
 public:
  static IFWL_DateTimeForm* Create(const CFWL_WidgetImpProperties& properties,
                                   IFWL_Widget* pOuter);

 protected:
  IFWL_DateTimeForm() {}
};

class IFWL_DateTimeCalender : public IFWL_MonthCalendar {
 public:
  static IFWL_DateTimeCalender* Create(
      const CFWL_WidgetImpProperties& properties,
      IFWL_Widget* pOuter);

 protected:
  IFWL_DateTimeCalender() {}
};

class IFWL_DateTimeEdit : public IFWL_Edit {
 public:
  static IFWL_DateTimeEdit* Create(const CFWL_WidgetImpProperties& properties,
                                   IFWL_Widget* pOuter);

 protected:
  IFWL_DateTimeEdit() {}
};

class CFWL_DateTimeEdit : public CFWL_EditImp {
 public:
  CFWL_DateTimeEdit(const CFWL_WidgetImpProperties& properties,
                    IFWL_Widget* pOuter);
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

 protected:
  friend class CFWL_DateTimeEditImpDelegate;
};
class CFWL_DateTimeEditImpDelegate : public CFWL_EditImpDelegate {
 public:
  CFWL_DateTimeEditImpDelegate(CFWL_DateTimeEdit* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;

 private:
  int32_t DisForm_OnProcessMessage(CFWL_Message* pMessage);

 protected:
  CFWL_DateTimeEdit* m_pOwner;
};
class CFWL_DateTimeCalendar : public CFWL_MonthCalendarImp {
 public:
  CFWL_DateTimeCalendar(const CFWL_WidgetImpProperties& properties,
                        IFWL_Widget* pOuter);
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

 protected:
  friend class CFWL_DateTimeCalendarImpDelegate;
};
class CFWL_DateTimeCalendarImpDelegate : public CFWL_MonthCalendarImpDelegate {
 public:
  CFWL_DateTimeCalendarImpDelegate(CFWL_DateTimeCalendar* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;

  void OnLButtonDownEx(CFWL_MsgMouse* pMsg);
  void OnLButtonUpEx(CFWL_MsgMouse* pMsg);
  void OnMouseMoveEx(CFWL_MsgMouse* pMsg);

 private:
  int32_t DisForm_OnProcessMessage(CFWL_Message* pMessage);
  void DisForm_OnLButtonUpEx(CFWL_MsgMouse* pMsg);

 protected:
  CFWL_DateTimeCalendar* m_pOwner;
  FX_BOOL m_bFlag;
};
class CFWL_DateTimePickerImp : public CFWL_WidgetImp {
 public:
  CFWL_DateTimePickerImp(const CFWL_WidgetImpProperties& properties,
                         IFWL_Widget* pOuter);
  virtual ~CFWL_DateTimePickerImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR Update();
  virtual FX_DWORD HitTest(FX_FLOAT fx, FX_FLOAT fy);
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);
  virtual FWL_ERR SetThemeProvider(IFWL_ThemeProvider* pTP);
  virtual FWL_ERR GetCurSel(int32_t& iYear, int32_t& iMonth, int32_t& iDay);
  virtual FWL_ERR SetCurSel(int32_t iYear, int32_t iMonth, int32_t iDay);
  virtual FWL_ERR SetEditText(const CFX_WideString& wsText);
  virtual FWL_ERR GetEditText(CFX_WideString& wsText,
                              int32_t nStart = 0,
                              int32_t nCount = -1) const;

 public:
  virtual FX_BOOL CanUndo();
  virtual FX_BOOL CanRedo();
  virtual FX_BOOL Undo();
  virtual FX_BOOL Redo();
  virtual FX_BOOL CanCopy();
  virtual FX_BOOL CanCut();
  virtual FX_BOOL CanSelectAll();
  virtual FX_BOOL Copy(CFX_WideString& wsCopy);
  virtual FX_BOOL Cut(CFX_WideString& wsCut);
  virtual FX_BOOL Paste(const CFX_WideString& wsPaste);
  virtual FX_BOOL SelectAll();
  virtual FX_BOOL Delete();
  virtual FX_BOOL DeSelect();
  virtual FWL_ERR GetBBox(CFX_RectF& rect);
  virtual FWL_ERR SetEditLimit(int32_t nLimit);
  virtual FWL_ERR ModifyEditStylesEx(FX_DWORD dwStylesExAdded,
                                     FX_DWORD dwStylesExRemoved);

 public:
  IFWL_DateTimeEdit* GetDataTimeEdit();

 protected:
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

 private:
  FWL_ERR DisForm_Initialize();
  void DisForm_InitDateTimeCalendar();
  void DisForm_InitDateTimeEdit();
  FX_BOOL DisForm_IsMonthCalendarShowed();
  void DisForm_ShowMonthCalendar(FX_BOOL bActivate);
  FX_DWORD DisForm_HitTest(FX_FLOAT fx, FX_FLOAT fy);
  FX_BOOL DisForm_IsNeedShowButton();
  FWL_ERR DisForm_Update();
  FWL_ERR DisForm_GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  FWL_ERR DisForm_GetBBox(CFX_RectF& rect);
  FWL_ERR DisForm_DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);

 protected:
  CFX_RectF m_rtBtn;
  CFX_RectF m_rtClient;
  int32_t m_iBtnState;
  int32_t m_iYear;
  int32_t m_iMonth;
  int32_t m_iDay;
  FX_BOOL m_bLBtnDown;
  std::unique_ptr<IFWL_DateTimeEdit> m_pEdit;
  std::unique_ptr<IFWL_DateTimeCalender> m_pMonthCal;
  std::unique_ptr<IFWL_DateTimeForm> m_pForm;
  FX_FLOAT m_fBtn;
  class CFWL_MonthCalendarImpDP : public IFWL_MonthCalendarDP {
   public:
    CFWL_MonthCalendarImpDP() {
      m_iCurYear = 2010;
      m_iCurMonth = 3;
      m_iCurDay = 29;
    }
    virtual FWL_ERR GetCaption(IFWL_Widget* pWidget,
                               CFX_WideString& wsCaption) {
      return FWL_ERR_Succeeded;
    }
    virtual int32_t GetCurDay(IFWL_Widget* pWidget) { return m_iCurDay; }
    virtual int32_t GetCurMonth(IFWL_Widget* pWidget) { return m_iCurMonth; }
    virtual int32_t GetCurYear(IFWL_Widget* pWidget) { return m_iCurYear; }
    int32_t m_iCurDay;
    int32_t m_iCurYear;
    int32_t m_iCurMonth;
  };

  CFWL_MonthCalendarImpDP m_MonthCalendarDP;
  friend class CFWL_DateTimeEditImpDelegate;
  friend class CFWL_DateTimeCalendar;
  friend class CFWL_DateTimeCalendarImpDelegate;
  friend class CFWL_DateTimePickerImpDelegate;
};
class CFWL_DateTimePickerImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_DateTimePickerImpDelegate(CFWL_DateTimePickerImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;

 protected:
  void OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnMouseLeave(CFWL_MsgMouse* pMsg);

  CFWL_DateTimePickerImp* m_pOwner;

 private:
  void DisForm_OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
};
#endif
