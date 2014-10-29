// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_THEME_LIGHT_H
#define _FWL_THEME_LIGHT_H
class CFWL_ThemeBackground;
class CFWL_ThemeText;
class CFWL_ThemePart;
class CFWL_WidgetTP;
class IFWL_Widget;
class CFWL_Theme;
class CFWL_Theme : public CFX_Object
{
public:
    virtual FX_BOOL		IsValidWidget(IFWL_Widget *pWidget);
    virtual FX_DWORD	GetThemeID(IFWL_Widget *pWidget);
    virtual FX_DWORD	SetThemeID(IFWL_Widget *pWidget, FX_DWORD dwThemeID, FX_BOOL bChildren = TRUE);
    virtual FWL_ERR		GetThemeMatrix(IFWL_Widget *pWidget, CFX_Matrix &matrix);
    virtual FWL_ERR		SetThemeMatrix(IFWL_Widget *pWidget, const CFX_Matrix &matrix);
    virtual FX_BOOL		DrawBackground(CFWL_ThemeBackground *pParams);
    virtual FX_BOOL		DrawText(CFWL_ThemeText *pParams);
    virtual FX_LPVOID   GetCapacity(CFWL_ThemePart *pThemePart, FX_DWORD dwCapacity);
    virtual FX_BOOL		IsCustomizedLayout(IFWL_Widget *pWidget);
    virtual FWL_ERR		GetPartRect(CFWL_ThemePart* pThemePart, CFX_RectF &rtPart);
    virtual FX_BOOL		IsInPart(CFWL_ThemePart* pThemePart, FX_FLOAT fx, FX_FLOAT fy);
    virtual	FX_BOOL		CalcTextRect(CFWL_ThemeText *pParams, CFX_RectF &rect);
    virtual FWL_ERR		Initialize();
    virtual FWL_ERR		Finalize();
    CFWL_Theme();
    virtual ~CFWL_Theme();
    FWL_ERR		SetFont(IFWL_Widget *pWidget, FX_LPCWSTR strFont, FX_FLOAT fFontSize, FX_ARGB rgbFont);
    CFWL_WidgetTP* GetTheme(IFWL_Widget* pWidget);
protected:
    CFX_PtrArray	m_arrThemes;
};
#endif
