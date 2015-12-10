// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_DATETIMEPICKER_H
#define _FWL_DATETIMEPICKER_H
class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_DateTimePickerDP;
class IFWL_DateTimePicker;
#define FWL_CLASS_DateTimePicker L"FWL_DATETIMEPICKER"
#define FWL_CLASSHASH_DateTimePicker 3851176257
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
#define FWL_PART_DTP_Border 1
#define FWL_PART_DTP_Edge 2
#define FWL_PART_DTP_Background 3
#define FWL_PART_DTP_DropDownButton 4
#define FWL_PARTSTATE_DTP_Normal (0L << 0)
#define FWL_PARTSTATE_DTP_Hovered (1L << 0)
#define FWL_PARTSTATE_DTP_Pressed (2L << 0)
#define FWL_PARTSTATE_DTP_Disabled (3L << 0)
#define FWL_EVT_DTP_DropDown L"FWL_EVENT_DTP_DropDown"
#define FWL_EVTHASH_DTP_DropDown 264728733
#define FWL_EVT_DTP_CloseUp L"FWL_EVENT_DTP_CloseUp"
#define FWL_EVTHASH_DTP_CloseUp 4280973803
#define FWL_EVT_DTP_EditChanged L"FWL_EVENT_DTP_EditChanged"
#define FWL_EVTHASH_DTP_EditChanged 4009610944
#define FWL_EVT_DTP_HoverChanged L"FWL_EVENT_DTP_HoverChanged"
#define FWL_EVTHASH_DTP_HoverChanged 686674750
#define FWL_EVT_DTP_SelectChanged L"FWL_EVENT_DTP_SelectChanged"
#define FWL_EVTHASH_DTP_SelectChanged 1589616858
BEGIN_FWL_EVENT_DEF(CFWL_Event_DtpDropDown, FWL_EVTHASH_DTP_DropDown)
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_Event_DtpCloseUp, FWL_EVTHASH_DTP_CloseUp)
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_Event_DtpEditChanged, FWL_EVTHASH_DTP_EditChanged)
CFX_WideString m_wsText;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_Event_DtpHoverChanged, FWL_EVTHASH_DTP_HoverChanged)
int32_t hoverday;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_Event_DtpSelectChanged, FWL_EVTHASH_DTP_SelectChanged)
int32_t iYear;
int32_t iMonth;
int32_t iDay;
END_FWL_EVENT_DEF
class IFWL_DateTimePickerDP : public IFWL_DataProvider {
 public:
  virtual FWL_ERR GetToday(IFWL_Widget* pWidget,
                           int32_t& iYear,
                           int32_t& iMonth,
                           int32_t& iDay) = 0;
};
class IFWL_DateTimePicker : public IFWL_Widget {
 public:
  static IFWL_DateTimePicker* Create(const CFWL_WidgetImpProperties& properties,
                                     IFWL_Widget* pOuter);

  FWL_ERR GetCurSel(int32_t& iYear, int32_t& iMonth, int32_t& iDay);
  FWL_ERR SetCurSel(int32_t iYear, int32_t iMonth, int32_t iDay);
  FWL_ERR SetEditText(const CFX_WideString& wsText);
  FWL_ERR GetEditText(CFX_WideString& wsText,
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
  FWL_ERR GetBBox(CFX_RectF& rect);
  FWL_ERR SetEditLimit(int32_t nLimit);
  FWL_ERR ModifyEditStylesEx(FX_DWORD dwStylesExAdded,
                             FX_DWORD dwStylesExRemoved);

 protected:
  IFWL_DateTimePicker();
};
#endif
