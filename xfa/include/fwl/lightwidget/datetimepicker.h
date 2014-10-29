// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_DATETIMEPICKER_LIGHT_H
#define _FWL_DATETIMEPICKER_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class IFWL_DateTimePickerDP;
class CFWL_DateTimePicker;
class CFWL_DateTimePickerDP;
class CFWL_DateTimePicker : public CFWL_Widget
{
public:
    static CFWL_DateTimePicker* Create();
    FWL_ERR	Initialize(const CFWL_WidgetProperties *pProperties = NULL);
    FWL_ERR SetToday(FX_INT32 iYear, FX_INT32 iMonth, FX_INT32 iDay);
    FWL_ERR	GetEditText(CFX_WideString &wsText);
    FWL_ERR	SetEditText(FX_WSTR wsText);
    FX_INT32 CountSelRanges();
    FX_INT32 GetSelRange(FX_INT32 nIndex, FX_INT32 &nStart);
    FWL_ERR GetCurSel(FX_INT32 &iYear, FX_INT32 &iMonth, FX_INT32 &iDay);
    FWL_ERR	SetCurSel(FX_INT32 iYear, FX_INT32 iMonth, FX_INT32 iDay);
    FX_BOOL		CanUndo();
    FX_BOOL		CanRedo();
    FX_BOOL		Undo();
    FX_BOOL		Redo();
    FX_BOOL		CanCopy();
    FX_BOOL		CanCut();
    FX_BOOL		CanSelectAll();
    FX_BOOL		Copy(CFX_WideString &wsCopy);
    FX_BOOL		Cut(CFX_WideString &wsCut);
    FX_BOOL		Paste(const CFX_WideString &wsPaste);
    FX_BOOL		SelectAll();
    FX_BOOL		Delete();
    FX_BOOL		DeSelect();
    FWL_ERR		GetBBox(CFX_RectF &rect);
    FWL_ERR		SetEditLimit(FX_INT32 nLimit);
    FWL_ERR		ModifyEditStylesEx(FX_DWORD dwStylesExAdded, FX_DWORD dwStylesExRemoved);
protected:
    CFWL_DateTimePicker();
    virtual ~CFWL_DateTimePicker();
    class CFWL_DateTimePickerDP : public IFWL_DateTimePickerDP
    {
    public:
        CFWL_DateTimePickerDP();
        virtual FWL_ERR GetCaption(IFWL_Widget *pWidget, CFX_WideString &wsCaption);
        virtual FWL_ERR GetToday(IFWL_Widget *pWidget, FX_INT32 &iYear, FX_INT32 &iMonth, FX_INT32 &iDay);
        FX_INT32 m_iYear;
        FX_INT32 m_iMonth;
        FX_INT32 m_iDay;
        CFX_WideString	m_wsData;
    };
    CFWL_DateTimePickerDP m_DateTimePickerDP;
};
#endif
