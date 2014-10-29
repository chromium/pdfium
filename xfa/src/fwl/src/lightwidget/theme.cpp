// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
FX_BOOL CFWL_Theme::IsValidWidget(IFWL_Widget *pWidget)
{
    CFWL_WidgetTP *pTheme = GetTheme(pWidget);
    return pTheme != NULL;
}
FX_DWORD CFWL_Theme::GetThemeID(IFWL_Widget *pWidget)
{
    CFWL_WidgetTP *pTheme = GetTheme(pWidget);
    FXSYS_assert(pTheme);
    return pTheme->GetThemeID(pWidget);
}
FX_DWORD CFWL_Theme::SetThemeID(IFWL_Widget *pWidget, FX_DWORD dwThemeID, FX_BOOL bChildren )
{
    FX_INT32 iCount = m_arrThemes.GetSize();
    FX_DWORD dwID;
    for (FX_INT32 i = 0; i < iCount; i ++) {
        CFWL_WidgetTP *pTheme = (CFWL_WidgetTP*)m_arrThemes[i];
        dwID = pTheme->GetThemeID(pWidget);
        pTheme->SetThemeID(pWidget, dwThemeID, FALSE);
    }
    return dwID;
}
FWL_ERR	CFWL_Theme::GetThemeMatrix(IFWL_Widget *pWidget, CFX_Matrix &matrix)
{
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_Theme::SetThemeMatrix(IFWL_Widget *pWidget, const CFX_Matrix &matrix)
{
    return FWL_ERR_Succeeded;
}
FX_BOOL CFWL_Theme::DrawBackground(CFWL_ThemeBackground *pParams)
{
    CFWL_WidgetTP *pTheme = GetTheme(pParams->m_pWidget);
    FXSYS_assert(pTheme);
    return pTheme->DrawBackground(pParams);
}
FX_BOOL CFWL_Theme::DrawText(CFWL_ThemeText *pParams)
{
    CFWL_WidgetTP *pTheme = GetTheme(pParams->m_pWidget);
    FXSYS_assert(pTheme);
    return pTheme->DrawText(pParams);
}
FX_LPVOID CFWL_Theme::GetCapacity(CFWL_ThemePart *pThemePart, FX_DWORD dwCapacity)
{
    CFWL_WidgetTP *pTheme = GetTheme(pThemePart->m_pWidget);
    FXSYS_assert(pTheme);
    return pTheme->GetCapacity(pThemePart, dwCapacity);
}
FX_BOOL CFWL_Theme::IsCustomizedLayout(IFWL_Widget *pWidget)
{
    CFWL_WidgetTP *pTheme = GetTheme(pWidget);
    FXSYS_assert(pTheme);
    return pTheme->IsCustomizedLayout(pWidget);
}
FWL_ERR CFWL_Theme::GetPartRect(CFWL_ThemePart* pThemePart, CFX_RectF &rtPart)
{
    CFWL_WidgetTP *pTheme = GetTheme(pThemePart->m_pWidget);
    FXSYS_assert(pTheme);
    return pTheme->GetPartRect(pThemePart, rtPart);
}
FX_BOOL CFWL_Theme::IsInPart(CFWL_ThemePart* pThemePart, FX_FLOAT fx, FX_FLOAT fy)
{
    CFWL_WidgetTP *pTheme = GetTheme(pThemePart->m_pWidget);
    FXSYS_assert(pTheme);
    return pTheme->IsInPart(pThemePart, fx, fy);
}
FX_BOOL	CFWL_Theme::CalcTextRect(CFWL_ThemeText *pParams, CFX_RectF &rect)
{
    CFWL_WidgetTP *pTheme = GetTheme(pParams->m_pWidget);
    FXSYS_assert(pTheme);
    return pTheme->CalcTextRect(pParams, rect);
}
FWL_ERR	CFWL_Theme::Initialize()
{
    FX_INT32 iCount = m_arrThemes.GetSize();
    for (FX_INT32 i = 0; i < iCount; i ++) {
        CFWL_WidgetTP *pTheme = (CFWL_WidgetTP*)m_arrThemes[i];
        pTheme->Initialize();
    }
    FWLTHEME_Init();
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_Theme::Finalize()
{
    FX_INT32 iCount = m_arrThemes.GetSize();
    for (FX_INT32 i = 0; i < iCount; i ++) {
        CFWL_WidgetTP *pTheme = (CFWL_WidgetTP*)m_arrThemes[i];
        pTheme->Finalize();
    }
    FWLTHEME_Release();
    return FWL_ERR_Succeeded;
}
CFWL_Theme::CFWL_Theme()
{
    CFWL_FormTP *pFormTP = FX_NEW CFWL_FormTP;
    CFWL_PushButtonTP		*pPushButtonTP = FX_NEW CFWL_PushButtonTP;
    CFWL_CheckBoxTP			*pCheckBoxTP = FX_NEW CFWL_CheckBoxTP;
    CFWL_ListBoxTP			*pListBoxTP = FX_NEW CFWL_ListBoxTP;
    CFWL_PictureBoxTP		*pPictureBoxTP = FX_NEW CFWL_PictureBoxTP;
    CFWL_ScrollBarTP		*pSrollBarTP = FX_NEW CFWL_ScrollBarTP;
    CFWL_EditTP				*pEditTP = FX_NEW CFWL_EditTP;
    CFWL_ComboBoxTP			*pComboBoxTP = FX_NEW CFWL_ComboBoxTP;
    CFWL_BarcodeTP			*pBarcodeTP = FX_NEW CFWL_BarcodeTP;
    CFWL_DateTimePickerTP	*pDateTimePickerTP = FX_NEW CFWL_DateTimePickerTP;
    CFWL_MonthCalendarTP	*pMonthCalendarTP = FX_NEW CFWL_MonthCalendarTP;
    CFWL_CaretTP			*pCaretTP = FX_NEW CFWL_CaretTP;
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
CFWL_Theme::~CFWL_Theme()
{
    FX_INT32 iCount = m_arrThemes.GetSize();
    for (FX_INT32 i = 0; i < iCount; i ++) {
        CFWL_WidgetTP *pTheme = (CFWL_WidgetTP*)m_arrThemes[i];
        delete pTheme;
    }
    m_arrThemes.RemoveAll();
}
FWL_ERR	CFWL_Theme::SetFont(IFWL_Widget *pWidget, FX_LPCWSTR strFont, FX_FLOAT fFontSize, FX_ARGB rgbFont)
{
    FX_INT32 iCount = m_arrThemes.GetSize();
    for (FX_INT32 i = 0; i < iCount; i ++) {
        CFWL_WidgetTP *pTheme = (CFWL_WidgetTP*)m_arrThemes[i];
        pTheme->SetFont(pWidget, strFont, fFontSize, rgbFont);
    }
    return FWL_ERR_Succeeded;
}
CFWL_WidgetTP* CFWL_Theme::GetTheme(IFWL_Widget* pWidget)
{
    FX_INT32 iCount = m_arrThemes.GetSize();
    for (FX_INT32 i = 0; i < iCount; i ++) {
        CFWL_WidgetTP *pTheme = (CFWL_WidgetTP*)m_arrThemes[i];
        if (pTheme->IsValidWidget(pWidget)) {
            return pTheme;
        }
    }
    return NULL;
}
