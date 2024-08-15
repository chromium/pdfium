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
    : m_pCheckBoxTP(cppgc::MakeGarbageCollected<CFWL_CheckBoxTP>(
          pHeap->GetAllocationHandle())),
      m_pListBoxTP(cppgc::MakeGarbageCollected<CFWL_ListBoxTP>(
          pHeap->GetAllocationHandle())),
      m_pPictureBoxTP(cppgc::MakeGarbageCollected<CFWL_PictureBoxTP>(
          pHeap->GetAllocationHandle())),
      m_pSrollBarTP(cppgc::MakeGarbageCollected<CFWL_ScrollBarTP>(
          pHeap->GetAllocationHandle())),
      m_pEditTP(cppgc::MakeGarbageCollected<CFWL_EditTP>(
          pHeap->GetAllocationHandle())),
      m_pComboBoxTP(cppgc::MakeGarbageCollected<CFWL_ComboBoxTP>(
          pHeap->GetAllocationHandle())),
      m_pMonthCalendarTP(cppgc::MakeGarbageCollected<CFWL_MonthCalendarTP>(
          pHeap->GetAllocationHandle())),
      m_pDateTimePickerTP(cppgc::MakeGarbageCollected<CFWL_DateTimePickerTP>(
          pHeap->GetAllocationHandle())),
      m_pPushButtonTP(cppgc::MakeGarbageCollected<CFWL_PushButtonTP>(
          pHeap->GetAllocationHandle())),
      m_pCaretTP(cppgc::MakeGarbageCollected<CFWL_CaretTP>(
          pHeap->GetAllocationHandle())),
      m_pBarcodeTP(cppgc::MakeGarbageCollected<CFWL_BarcodeTP>(
          pHeap->GetAllocationHandle())) {}

IFWL_ThemeProvider::~IFWL_ThemeProvider() = default;

void IFWL_ThemeProvider::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(m_pCheckBoxTP);
  visitor->Trace(m_pListBoxTP);
  visitor->Trace(m_pPictureBoxTP);
  visitor->Trace(m_pSrollBarTP);
  visitor->Trace(m_pEditTP);
  visitor->Trace(m_pComboBoxTP);
  visitor->Trace(m_pMonthCalendarTP);
  visitor->Trace(m_pDateTimePickerTP);
  visitor->Trace(m_pPushButtonTP);
  visitor->Trace(m_pCaretTP);
  visitor->Trace(m_pBarcodeTP);
}

CFWL_WidgetTP* IFWL_ThemeProvider::GetTheme(const CFWL_Widget* pWidget) const {
  switch (pWidget->GetClassID()) {
    case FWL_Type::CheckBox:
      return m_pCheckBoxTP;
    case FWL_Type::ListBox:
      return m_pListBoxTP;
    case FWL_Type::PictureBox:
      return m_pPictureBoxTP;
    case FWL_Type::ScrollBar:
      return m_pSrollBarTP;
    case FWL_Type::Edit:
      return m_pEditTP;
    case FWL_Type::ComboBox:
      return m_pComboBoxTP;
    case FWL_Type::MonthCalendar:
      return m_pMonthCalendarTP;
    case FWL_Type::DateTimePicker:
      return m_pDateTimePickerTP;
    case FWL_Type::PushButton:
      return m_pPushButtonTP;
    case FWL_Type::Caret:
      return m_pCaretTP;
    case FWL_Type::Barcode:
      return m_pBarcodeTP;
    default:
      return nullptr;
  }
}

}  // namespace pdfium
