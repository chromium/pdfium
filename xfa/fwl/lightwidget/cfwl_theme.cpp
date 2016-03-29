// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/lightwidget/cfwl_theme.h"

#include <algorithm>

#include "xfa/fwl/core/cfwl_themebackground.h"
#include "xfa/fwl/core/cfwl_themepart.h"
#include "xfa/fwl/core/cfwl_themetext.h"
#include "xfa/fwl/theme/cfwl_barcodetp.h"
#include "xfa/fwl/theme/cfwl_carettp.h"
#include "xfa/fwl/theme/cfwl_checkboxtp.h"
#include "xfa/fwl/theme/cfwl_comboboxtp.h"
#include "xfa/fwl/theme/cfwl_datetimepickertp.h"
#include "xfa/fwl/theme/cfwl_edittp.h"
#include "xfa/fwl/theme/cfwl_formtp.h"
#include "xfa/fwl/theme/cfwl_listboxtp.h"
#include "xfa/fwl/theme/cfwl_monthcalendartp.h"
#include "xfa/fwl/theme/cfwl_pictureboxtp.h"
#include "xfa/fwl/theme/cfwl_pushbuttontp.h"
#include "xfa/fwl/theme/cfwl_scrollbartp.h"

CFWL_Theme::CFWL_Theme() {
  m_ThemesArray.push_back(std::unique_ptr<CFWL_WidgetTP>(new CFWL_FormTP));
  m_ThemesArray.push_back(
      std::unique_ptr<CFWL_WidgetTP>(new CFWL_PushButtonTP));
  m_ThemesArray.push_back(std::unique_ptr<CFWL_WidgetTP>(new CFWL_CheckBoxTP));
  m_ThemesArray.push_back(std::unique_ptr<CFWL_WidgetTP>(new CFWL_ListBoxTP));
  m_ThemesArray.push_back(
      std::unique_ptr<CFWL_WidgetTP>(new CFWL_PictureBoxTP));
  m_ThemesArray.push_back(std::unique_ptr<CFWL_WidgetTP>(new CFWL_ScrollBarTP));
  m_ThemesArray.push_back(std::unique_ptr<CFWL_WidgetTP>(new CFWL_EditTP));
  m_ThemesArray.push_back(std::unique_ptr<CFWL_WidgetTP>(new CFWL_ComboBoxTP));
  m_ThemesArray.push_back(std::unique_ptr<CFWL_WidgetTP>(new CFWL_BarcodeTP));
  m_ThemesArray.push_back(
      std::unique_ptr<CFWL_WidgetTP>(new CFWL_DateTimePickerTP));
  m_ThemesArray.push_back(
      std::unique_ptr<CFWL_WidgetTP>(new CFWL_MonthCalendarTP));
  m_ThemesArray.push_back(std::unique_ptr<CFWL_WidgetTP>(new CFWL_CaretTP));
}

CFWL_Theme::~CFWL_Theme() {}

FX_BOOL CFWL_Theme::IsValidWidget(IFWL_Widget* pWidget) {
  return !!GetTheme(pWidget);
}

uint32_t CFWL_Theme::GetThemeID(IFWL_Widget* pWidget) {
  return GetTheme(pWidget)->GetThemeID(pWidget);
}

uint32_t CFWL_Theme::SetThemeID(IFWL_Widget* pWidget,
                                uint32_t dwThemeID,
                                FX_BOOL bChildren) {
  uint32_t dwID;
  for (const auto& pTheme : m_ThemesArray) {
    dwID = pTheme->GetThemeID(pWidget);
    pTheme->SetThemeID(pWidget, dwThemeID, FALSE);
  }
  return dwID;
}

FWL_ERR CFWL_Theme::GetThemeMatrix(IFWL_Widget* pWidget, CFX_Matrix& matrix) {
  return FWL_ERR_Succeeded;
}

FWL_ERR CFWL_Theme::SetThemeMatrix(IFWL_Widget* pWidget,
                                   const CFX_Matrix& matrix) {
  return FWL_ERR_Succeeded;
}

FX_BOOL CFWL_Theme::DrawBackground(CFWL_ThemeBackground* pParams) {
  return GetTheme(pParams->m_pWidget)->DrawBackground(pParams);
}

FX_BOOL CFWL_Theme::DrawText(CFWL_ThemeText* pParams) {
  return GetTheme(pParams->m_pWidget)->DrawText(pParams);
}

void* CFWL_Theme::GetCapacity(CFWL_ThemePart* pThemePart, uint32_t dwCapacity) {
  return GetTheme(pThemePart->m_pWidget)->GetCapacity(pThemePart, dwCapacity);
}

FX_BOOL CFWL_Theme::IsCustomizedLayout(IFWL_Widget* pWidget) {
  return GetTheme(pWidget)->IsCustomizedLayout(pWidget);
}

FWL_ERR CFWL_Theme::GetPartRect(CFWL_ThemePart* pThemePart, CFX_RectF& rtPart) {
  return GetTheme(pThemePart->m_pWidget)->GetPartRect(pThemePart, rtPart);
}

FX_BOOL CFWL_Theme::IsInPart(CFWL_ThemePart* pThemePart,
                             FX_FLOAT fx,
                             FX_FLOAT fy) {
  return GetTheme(pThemePart->m_pWidget)->IsInPart(pThemePart, fx, fy);
}

FX_BOOL CFWL_Theme::CalcTextRect(CFWL_ThemeText* pParams, CFX_RectF& rect) {
  return GetTheme(pParams->m_pWidget)->CalcTextRect(pParams, rect);
}

FWL_ERR CFWL_Theme::Initialize() {
  for (const auto& pTheme : m_ThemesArray)
    pTheme->Initialize();

  FWLTHEME_Init();
  return FWL_ERR_Succeeded;
}

FWL_ERR CFWL_Theme::Finalize() {
  for (const auto& pTheme : m_ThemesArray)
    pTheme->Finalize();

  FWLTHEME_Release();
  return FWL_ERR_Succeeded;
}

FWL_ERR CFWL_Theme::SetFont(IFWL_Widget* pWidget,
                            const FX_WCHAR* strFont,
                            FX_FLOAT fFontSize,
                            FX_ARGB rgbFont) {
  for (const auto& pTheme : m_ThemesArray)
    pTheme->SetFont(pWidget, strFont, fFontSize, rgbFont);

  return FWL_ERR_Succeeded;
}

CFWL_WidgetTP* CFWL_Theme::GetTheme(IFWL_Widget* pWidget) {
  for (const auto& pTheme : m_ThemesArray) {
    if (pTheme->IsValidWidget(pWidget))
      return pTheme.get();
  }
  return nullptr;
}
