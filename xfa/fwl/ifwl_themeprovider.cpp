// Copyright 2020 PDFium Authors. All rights reserved.
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

IFWL_ThemeProvider::IFWL_ThemeProvider()
    : m_pCheckBoxTP(std::make_unique<CFWL_CheckBoxTP>()),
      m_pListBoxTP(std::make_unique<CFWL_ListBoxTP>()),
      m_pPictureBoxTP(std::make_unique<CFWL_PictureBoxTP>()),
      m_pSrollBarTP(std::make_unique<CFWL_ScrollBarTP>()),
      m_pEditTP(std::make_unique<CFWL_EditTP>()),
      m_pComboBoxTP(std::make_unique<CFWL_ComboBoxTP>()),
      m_pMonthCalendarTP(std::make_unique<CFWL_MonthCalendarTP>()),
      m_pDateTimePickerTP(std::make_unique<CFWL_DateTimePickerTP>()),
      m_pPushButtonTP(std::make_unique<CFWL_PushButtonTP>()),
      m_pCaretTP(std::make_unique<CFWL_CaretTP>()),
      m_pBarcodeTP(std::make_unique<CFWL_BarcodeTP>()) {}

IFWL_ThemeProvider::~IFWL_ThemeProvider() = default;

CFWL_WidgetTP* IFWL_ThemeProvider::GetTheme(const CFWL_Widget* pWidget) const {
  switch (pWidget->GetClassID()) {
    case FWL_Type::CheckBox:
      return m_pCheckBoxTP.get();
    case FWL_Type::ListBox:
      return m_pListBoxTP.get();
    case FWL_Type::PictureBox:
      return m_pPictureBoxTP.get();
    case FWL_Type::ScrollBar:
      return m_pSrollBarTP.get();
    case FWL_Type::Edit:
      return m_pEditTP.get();
    case FWL_Type::ComboBox:
      return m_pComboBoxTP.get();
    case FWL_Type::MonthCalendar:
      return m_pMonthCalendarTP.get();
    case FWL_Type::DateTimePicker:
      return m_pDateTimePickerTP.get();
    case FWL_Type::PushButton:
      return m_pPushButtonTP.get();
    case FWL_Type::Caret:
      return m_pCaretTP.get();
    case FWL_Type::Barcode:
      return m_pBarcodeTP.get();
    default:
      return nullptr;
  }
}
