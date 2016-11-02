// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_DATETIMEPICKER_H_
#define XFA_FWL_CORE_CFWL_DATETIMEPICKER_H_

#include "xfa/fwl/core/cfwl_widget.h"
#include "xfa/fwl/core/ifwl_datetimepicker.h"

class CFWL_DateTimePicker : public CFWL_Widget {
 public:
  CFWL_DateTimePicker(const IFWL_App*);
  ~CFWL_DateTimePicker() override;

  void Initialize();

  FWL_Error SetToday(int32_t iYear, int32_t iMonth, int32_t iDay);
  FWL_Error GetEditText(CFX_WideString& wsText);
  FWL_Error SetEditText(const CFX_WideString& wsText);
  int32_t CountSelRanges();
  int32_t GetSelRange(int32_t nIndex, int32_t& nStart);
  FWL_Error GetCurSel(int32_t& iYear, int32_t& iMonth, int32_t& iDay);
  FWL_Error SetCurSel(int32_t iYear, int32_t iMonth, int32_t iDay);
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
  FWL_Error ModifyEditStylesEx(uint32_t dwStylesExAdded,
                               uint32_t dwStylesExRemoved);

 protected:
  class CFWL_DateTimePickerDP : public IFWL_DateTimePickerDP {
   public:
    CFWL_DateTimePickerDP();

    // IFWL_DataProvider
    FWL_Error GetCaption(IFWL_Widget* pWidget,
                         CFX_WideString& wsCaption) override;

    // IFWL_DateTimePickerDP
    FWL_Error GetToday(IFWL_Widget* pWidget,
                       int32_t& iYear,
                       int32_t& iMonth,
                       int32_t& iDay) override;
    int32_t m_iYear;
    int32_t m_iMonth;
    int32_t m_iDay;
    CFX_WideString m_wsData;
  };

  CFWL_DateTimePickerDP m_DateTimePickerDP;
};

#endif  // XFA_FWL_CORE_CFWL_DATETIMEPICKER_H_
