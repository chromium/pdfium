// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/ifwl_themeprovider.h"

#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/theme/cfwl_barcodetp.h"
#include "xfa/fwl/theme/cfwl_carettp.h"
#include "xfa/fwl/theme/cfwl_checkboxtp.h"
#include "xfa/fwl/theme/cfwl_comboboxtp.h"
#include "xfa/fwl/theme/cfwl_datetimepickertp.h"
#include "xfa/fwl/theme/cfwl_edittp.h"
#include "xfa/fwl/theme/cfwl_listboxtp.h"
#include "xfa/fwl/theme/cfwl_monthcalendartp.h"
#include "xfa/fwl/theme/cfwl_pictureboxtp.h"
#include "xfa/fwl/theme/cfwl_pushbuttontp.h"
#include "xfa/fwl/theme/cfwl_scrollbartp.h"

namespace pdfium {

IFWL_ThemeProvider::IFWL_ThemeProvider(cppgc::Heap* pHeap)
    : check_box_tp_(cppgc::MakeGarbageCollected<CFWL_CheckBoxTP>(
          pHeap->GetAllocationHandle())),
      list_box_tp_(cppgc::MakeGarbageCollected<CFWL_ListBoxTP>(
          pHeap->GetAllocationHandle())),
      picture_box_tp_(cppgc::MakeGarbageCollected<CFWL_PictureBoxTP>(
          pHeap->GetAllocationHandle())),
      sroll_bar_tp_(cppgc::MakeGarbageCollected<CFWL_ScrollBarTP>(
          pHeap->GetAllocationHandle())),
      edit_tp_(cppgc::MakeGarbageCollected<CFWL_EditTP>(
          pHeap->GetAllocationHandle())),
      combo_box_tp_(cppgc::MakeGarbageCollected<CFWL_ComboBoxTP>(
          pHeap->GetAllocationHandle())),
      month_calendar_tp_(cppgc::MakeGarbageCollected<CFWL_MonthCalendarTP>(
          pHeap->GetAllocationHandle())),
      date_time_picker_tp_(cppgc::MakeGarbageCollected<CFWL_DateTimePickerTP>(
          pHeap->GetAllocationHandle())),
      push_button_tp_(cppgc::MakeGarbageCollected<CFWL_PushButtonTP>(
          pHeap->GetAllocationHandle())),
      caret_tp_(cppgc::MakeGarbageCollected<CFWL_CaretTP>(
          pHeap->GetAllocationHandle())),
      barcode_tp_(cppgc::MakeGarbageCollected<CFWL_BarcodeTP>(
          pHeap->GetAllocationHandle())) {}

IFWL_ThemeProvider::~IFWL_ThemeProvider() = default;

void IFWL_ThemeProvider::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(check_box_tp_);
  visitor->Trace(list_box_tp_);
  visitor->Trace(picture_box_tp_);
  visitor->Trace(sroll_bar_tp_);
  visitor->Trace(edit_tp_);
  visitor->Trace(combo_box_tp_);
  visitor->Trace(month_calendar_tp_);
  visitor->Trace(date_time_picker_tp_);
  visitor->Trace(push_button_tp_);
  visitor->Trace(caret_tp_);
  visitor->Trace(barcode_tp_);
}

CFWL_WidgetTP* IFWL_ThemeProvider::GetTheme(const CFWL_Widget* pWidget) const {
  switch (pWidget->GetClassID()) {
    case FWL_Type::CheckBox:
      return check_box_tp_;
    case FWL_Type::ListBox:
      return list_box_tp_;
    case FWL_Type::PictureBox:
      return picture_box_tp_;
    case FWL_Type::ScrollBar:
      return sroll_bar_tp_;
    case FWL_Type::Edit:
      return edit_tp_;
    case FWL_Type::ComboBox:
      return combo_box_tp_;
    case FWL_Type::MonthCalendar:
      return month_calendar_tp_;
    case FWL_Type::DateTimePicker:
      return date_time_picker_tp_;
    case FWL_Type::PushButton:
      return push_button_tp_;
    case FWL_Type::Caret:
      return caret_tp_;
    case FWL_Type::Barcode:
      return barcode_tp_;
    default:
      return nullptr;
  }
}

}  // namespace pdfium
