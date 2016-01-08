// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
FX_BOOL CFWL_Theme::IsValidWidget(IFWL_Widget* pWidget) {
  return !!GetTheme(pWidget);
}
FX_DWORD CFWL_Theme::GetThemeID(IFWL_Widget* pWidget) {
  return GetTheme(pWidget)->GetThemeID(pWidget);
}
FX_DWORD CFWL_Theme::SetThemeID(IFWL_Widget* pWidget,
                                FX_DWORD dwThemeID,
                                FX_BOOL bChildren) {
  int32_t iCount = m_arrThemes.GetSize();
  FX_DWORD dwID;
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_WidgetTP* pTheme = static_cast<CFWL_WidgetTP*>(m_arrThemes[i]);
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
  CFWL_WidgetTP* pTheme = GetTheme(pParams->m_pWidget);
  FXSYS_assert(pTheme);
  return pTheme->DrawBackground(pParams);
}
FX_BOOL CFWL_Theme::DrawText(CFWL_ThemeText* pParams) {
  CFWL_WidgetTP* pTheme = GetTheme(pParams->m_pWidget);
  FXSYS_assert(pTheme);
  return pTheme->DrawText(pParams);
}
void* CFWL_Theme::GetCapacity(CFWL_ThemePart* pThemePart, FX_DWORD dwCapacity) {
  CFWL_WidgetTP* pTheme = GetTheme(pThemePart->m_pWidget);
  FXSYS_assert(pTheme);
  return pTheme->GetCapacity(pThemePart, dwCapacity);
}
FX_BOOL CFWL_Theme::IsCustomizedLayout(IFWL_Widget* pWidget) {
  CFWL_WidgetTP* pTheme = GetTheme(pWidget);
  FXSYS_assert(pTheme);
  return pTheme->IsCustomizedLayout(pWidget);
}
FWL_ERR CFWL_Theme::GetPartRect(CFWL_ThemePart* pThemePart, CFX_RectF& rtPart) {
  CFWL_WidgetTP* pTheme = GetTheme(pThemePart->m_pWidget);
  FXSYS_assert(pTheme);
  return pTheme->GetPartRect(pThemePart, rtPart);
}
FX_BOOL CFWL_Theme::IsInPart(CFWL_ThemePart* pThemePart,
                             FX_FLOAT fx,
                             FX_FLOAT fy) {
  CFWL_WidgetTP* pTheme = GetTheme(pThemePart->m_pWidget);
  FXSYS_assert(pTheme);
  return pTheme->IsInPart(pThemePart, fx, fy);
}
FX_BOOL CFWL_Theme::CalcTextRect(CFWL_ThemeText* pParams, CFX_RectF& rect) {
  CFWL_WidgetTP* pTheme = GetTheme(pParams->m_pWidget);
  FXSYS_assert(pTheme);
  return pTheme->CalcTextRect(pParams, rect);
}
FWL_ERR CFWL_Theme::Initialize() {
  int32_t iCount = m_arrThemes.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_WidgetTP* pTheme = static_cast<CFWL_WidgetTP*>(m_arrThemes[i]);
    pTheme->Initialize();
  }
  FWLTHEME_Init();
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_Theme::Finalize() {
  int32_t iCount = m_arrThemes.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_WidgetTP* pTheme = static_cast<CFWL_WidgetTP*>(m_arrThemes[i]);
    pTheme->Finalize();
  }
  FWLTHEME_Release();
  return FWL_ERR_Succeeded;
}
CFWL_Theme::CFWL_Theme() {
  CFWL_FormTP* pFormTP = new CFWL_FormTP;
  CFWL_PushButtonTP* pPushButtonTP = new CFWL_PushButtonTP;
  CFWL_CheckBoxTP* pCheckBoxTP = new CFWL_CheckBoxTP;
  CFWL_ListBoxTP* pListBoxTP = new CFWL_ListBoxTP;
  CFWL_PictureBoxTP* pPictureBoxTP = new CFWL_PictureBoxTP;
  CFWL_ScrollBarTP* pSrollBarTP = new CFWL_ScrollBarTP;
  CFWL_EditTP* pEditTP = new CFWL_EditTP;
  CFWL_ComboBoxTP* pComboBoxTP = new CFWL_ComboBoxTP;
  CFWL_BarcodeTP* pBarcodeTP = new CFWL_BarcodeTP;
  CFWL_DateTimePickerTP* pDateTimePickerTP = new CFWL_DateTimePickerTP;
  CFWL_MonthCalendarTP* pMonthCalendarTP = new CFWL_MonthCalendarTP;
  CFWL_CaretTP* pCaretTP = new CFWL_CaretTP;
  m_arrThemes.Add(pFormTP);
  m_arrThemes.Add(pPushButtonTP);
  m_arrThemes.Add(pCheckBoxTP);
  m_arrThemes.Add(pListBoxTP);
  m_arrThemes.Add(pPictureBoxTP);
  m_arrThemes.Add(pSrollBarTP);
  m_arrThemes.Add(pEditTP);
  m_arrThemes.Add(pComboBoxTP);
  m_arrThemes.Add(pBarcodeTP);
  m_arrThemes.Add(pDateTimePickerTP);
  m_arrThemes.Add(pMonthCalendarTP);
  m_arrThemes.Add(pCaretTP);
}
CFWL_Theme::~CFWL_Theme() {
  for (int32_t i = 0; i < m_arrThemes.GetSize(); i++) {
    delete static_cast<CFWL_WidgetTP*>(m_arrThemes[i]);
  }
  m_arrThemes.RemoveAll();
}
FWL_ERR CFWL_Theme::SetFont(IFWL_Widget* pWidget,
                            const FX_WCHAR* strFont,
                            FX_FLOAT fFontSize,
                            FX_ARGB rgbFont) {
  int32_t iCount = m_arrThemes.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_WidgetTP* pTheme = static_cast<CFWL_WidgetTP*>(m_arrThemes[i]);
    pTheme->SetFont(pWidget, strFont, fFontSize, rgbFont);
  }
  return FWL_ERR_Succeeded;
}
CFWL_WidgetTP* CFWL_Theme::GetTheme(IFWL_Widget* pWidget) {
  int32_t iCount = m_arrThemes.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_WidgetTP* pTheme = static_cast<CFWL_WidgetTP*>(m_arrThemes[i]);
    if (pTheme->IsValidWidget(pWidget)) {
      return pTheme;
    }
  }
  return NULL;
}
