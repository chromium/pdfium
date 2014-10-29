// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_MONTHCALENDAR_IMP_H
#define _FWL_MONTHCALENDAR_IMP_H
class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class IFWL_Widget;
class IFDE_DateTime;
class CFDE_DateTime;
extern FX_BYTE FX_DaysInMonth(FX_INT32 iYear, FX_BYTE iMonth);
class CFWL_MonthCalendarImp;
class CFWL_MonthCalendarImpDelegate;
class CFWL_MonthCalendarImp : public CFWL_WidgetImp
{
public:
    CFWL_MonthCalendarImp(IFWL_Widget *pOuter = NULL);
    CFWL_MonthCalendarImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter = NULL);
    ~CFWL_MonthCalendarImp();
    virtual FWL_ERR		GetClassName(CFX_WideString &wsClass) const;
    virtual FX_DWORD	GetClassID() const;
    virtual FWL_ERR		Initialize();
    virtual FWL_ERR		Finalize();
    virtual FWL_ERR		GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize = FALSE);
    virtual	FWL_ERR		Update();
    virtual FWL_ERR		DrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix = NULL);
    virtual FX_INT32	CountSelect();
    virtual	FX_BOOL		GetSelect(FX_INT32 &iYear, FX_INT32 &iMonth, FX_INT32 &iDay, FX_INT32 nIndex = 0);
    virtual	FX_BOOL		SetSelect(FX_INT32 iYear, FX_INT32 iMonth, FX_INT32 iDay);
protected:
    void		DrawBkground(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawHeadBK(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawLButton(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawRButton(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawCaption(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawSeperator(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawDatesInBK(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawWeek(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawWeekNumber(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawWeekNumberSep(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawToday(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawDatesIn(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawDatesOut(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawDatesInCircle(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    void		DrawTodayCircle(CFX_Graphics *pGraphics, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix);
    CFX_SizeF	CalcSize(FX_BOOL bAutoSize = FALSE);
    void		LayOut();
    void		CalcHeadSize();
    void		CalcTodaySize();
    void		CalDateItem();
    void		GetCapValue();
    FX_INT32	CalWeekNumber(FX_INT32 iYear, FX_INT32 iMonth, FX_INT32 iDay);

    FX_BOOL		GetMinDate(FX_INT32 &iYear, FX_INT32 &iMonth, FX_INT32 &iDay);
    FX_BOOL		SetMinDate(FX_INT32 iYear, FX_INT32 iMonth, FX_INT32 iDay);
    FX_BOOL		GetMaxDate(FX_INT32 &iYear, FX_INT32 &iMonth, FX_INT32 &iDay);
    FX_BOOL		SetMaxDate(FX_INT32 iYear, FX_INT32 iMonth, FX_INT32 iDay);
    FX_BOOL     InitDate();
    void		ClearDateItem();
    void		ReSetDateItem();
    FX_BOOL		NextMonth();
    FX_BOOL		PrevMonth();
    void		ChangeToMonth(FX_INT32 iYear, FX_INT32 iMonth);
    FX_BOOL		RemoveSelDay(FX_INT32 iDay, FX_BOOL bAll = FALSE);
    FX_BOOL		AddSelDay(FX_INT32 iDay);
    FX_BOOL		JumpToToday();
    void		GetHeadText(FX_INT32 iYear, FX_INT32 iMonth, CFX_WideString &wsHead);
    void		GetTodayText(FX_INT32 iYear, FX_INT32 iMonth, FX_INT32 iDay, CFX_WideString &wsToday);
    FX_INT32	GetDayAtPoint(FX_FLOAT x, FX_FLOAT y);
    FX_BOOL		GetDayRect(FX_INT32 iDay, CFX_RectF &rtDay);
    typedef struct _DATE {
        _DATE()
        {
            iYear = 0;
            iMonth = 0;
            iDay = 0;
        }
        _DATE(FX_INT32 year, FX_INT32 month, FX_INT32 day) : iYear(year), iMonth(month), iDay(day) {}
        FX_BOOL operator < (const _DATE &right)
        {
            if (iYear < right.iYear) {
                return TRUE;
            } else if (iYear == right.iYear) {
                if (iMonth < right.iMonth) {
                    return TRUE;
                } else if (iMonth == right.iMonth) {
                    return iDay < right.iDay;
                }
            }
            return FALSE;
        }
        FX_BOOL operator > (const _DATE &right)
        {
            if (iYear > right.iYear) {
                return TRUE;
            } else if (iYear == right.iYear) {
                if (iMonth > right.iMonth) {
                    return TRUE;
                } else if (iMonth == right.iMonth) {
                    return iDay > right.iDay;
                }
            }
            return FALSE;
        }
        FX_INT32 iYear;
        FX_INT32 iMonth;
        FX_INT32 iDay;

    } DATE, *LPDATE;
    FX_BOOL				m_bInit;
    CFX_RectF			m_rtHead;
    CFX_RectF			m_rtWeek;
    CFX_RectF			m_rtLBtn;
    CFX_RectF			m_rtRBtn;
    CFX_RectF			m_rtDates;
    CFX_RectF			m_rtHSep;
    CFX_RectF			m_rtHeadText;
    CFX_RectF			m_rtToday;
    CFX_RectF			m_rtTodayFlag;
    CFX_RectF			m_rtWeekNum;
    CFX_RectF			m_rtWeekNumSep;
    CFX_RectF			m_rtTemp;
    CFX_WideString		m_wsHead;
    CFX_WideString		m_wsToday;
    CFX_DateTime		*m_pDateTime;
    CFX_PtrArray		m_arrDates;
    FX_INT32			m_iCurYear;
    FX_INT32			m_iCurMonth;
    FX_INT32			m_iYear;
    FX_INT32			m_iMonth;
    FX_INT32			m_iDay;
    FX_INT32			m_iHovered;
    FX_INT32			m_iLBtnPartStates;
    FX_INT32			m_iRBtnPartStates;
    DATE				m_dtMin;
    DATE				m_dtMax;
    CFX_SizeF			m_szHead;
    CFX_SizeF			m_szCell;
    CFX_SizeF			m_szToday;
    typedef CFX_ArrayTemplate<FX_INT32>	CFWL_Int32Array;
    CFWL_Int32Array		m_arrSelDays;
    FX_INT32			m_iMaxSel;
    CFX_RectF			m_rtClient;
    FX_FLOAT m_fHeadWid;
    FX_FLOAT m_fHeadHei;
    FX_FLOAT m_fHeadBtnWid;
    FX_FLOAT m_fHeadBtnHei;
    FX_FLOAT m_fHeadBtnHMargin;
    FX_FLOAT m_fHeadBtnVMargin;
    FX_FLOAT m_fHeadTextWid;
    FX_FLOAT m_fHeadTextHei;
    FX_FLOAT m_fHeadTextHMargin;
    FX_FLOAT m_fHeadTextVMargin;
    FX_FLOAT m_fHSepWid;
    FX_FLOAT m_fHSepHei;

    FX_FLOAT m_fWeekNumWid;
    FX_FLOAT m_fSepDOffset;
    FX_FLOAT m_fSepX;
    FX_FLOAT m_fSepY;

    FX_FLOAT m_fWeekNumHeigh;
    FX_FLOAT m_fWeekWid;
    FX_FLOAT m_fWeekHei;
    FX_FLOAT m_fDateCellWid;
    FX_FLOAT m_fDateCellHei;

    FX_FLOAT m_fTodayWid;
    FX_FLOAT m_fTodayHei;
    FX_FLOAT m_fTodayFlagWid;

    FX_FLOAT m_fMCWid;
    FX_FLOAT m_fMCHei;
    friend class CFWL_MonthCalendarImpDelegate;
};
typedef struct _DATEINFO {
    _DATEINFO(FX_INT32 day, FX_INT32 dayofweek, FX_DWORD dwSt, CFX_RectF rc, CFX_WideString &wsday) : iDay(day), iDayOfWeek(dayofweek), dwStates(dwSt), rect(rc), wsDay(wsday) {}
    FX_INT32 iDay;
    FX_INT32 iDayOfWeek;
    FX_DWORD dwStates;
    CFX_RectF rect;
    CFX_WideString wsDay;
} DATEINFO, *LPDATEINFO;
class CFWL_MonthCalendarImpDelegate : public CFWL_WidgetImpDelegate
{
public:
    CFWL_MonthCalendarImpDelegate(CFWL_MonthCalendarImp *pOwner);
    virtual FX_INT32	OnProcessMessage(CFWL_Message *pMessage);
    virtual FWL_ERR		OnDrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix = NULL);
protected:
    void		OnActivate(CFWL_Message *pMsg);
    void		OnFocusChanged(CFWL_Message *pMsg, FX_BOOL bSet = TRUE);
    void		OnLButtonDown(CFWL_MsgMouse *pMsg);
    void		OnLButtonUp(CFWL_MsgMouse *pMsg);
    void		OnMouseMove(CFWL_MsgMouse *pMsg);
    void		OnMouseLeave(CFWL_MsgMouse *pMsg);
    CFWL_MonthCalendarImp	*m_pOwner;
};
#endif
