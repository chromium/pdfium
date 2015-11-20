// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_THEME_H
#define _FWL_THEME_H
class IFWL_Widget;
class CFWL_ThemePart;
class CFWL_ThemeBackground;
class CFWL_ThemeText;
class CFWL_ThemeElement;
class IFWL_ThemeProvider;
#define FWL_WGTCAPACITY_CXBorder 1
#define FWL_WGTCAPACITY_CYBorder 2
#define FWL_WGTCAPACITY_ScrollBarWidth 3
#define FWL_WGTCAPACITY_EdgeFlat 4
#define FWL_WGTCAPACITY_EdgeRaised 5
#define FWL_WGTCAPACITY_EdgeSunken 6
#define FWL_WGTCAPACITY_Font 7
#define FWL_WGTCAPACITY_FontSize 8
#define FWL_WGTCAPACITY_TextColor 9
#define FWL_WGTCAPACITY_TextSelColor 10
#define FWL_WGTCAPACITY_LineHeight 11
#define FWL_WGTCAPACITY_UIMargin 12
#define FWL_WGTCAPACITY_SpaceAboveBelow 13
#define FWL_WGTCAPACITY_MAX 65535
class CFWL_ThemePart {
 public:
  CFWL_ThemePart()
      : m_pWidget(NULL), m_iPart(0), m_dwStates(0), m_dwData(0), m_pData(NULL) {
    m_rtPart.Reset();
    m_matrix.SetIdentity();
  }
  CFX_Matrix m_matrix;
  CFX_RectF m_rtPart;
  IFWL_Widget* m_pWidget;
  int32_t m_iPart;
  FX_DWORD m_dwStates;
  FX_DWORD m_dwData;
  void* m_pData;
};
class CFWL_ThemeBackground : public CFWL_ThemePart {
 public:
  CFWL_ThemeBackground() : m_pGraphics(NULL), m_pImage(NULL), m_pPath(NULL) {}
  CFX_Graphics* m_pGraphics;
  CFX_DIBitmap* m_pImage;
  CFX_Path* m_pPath;
};
class CFWL_ThemeText : public CFWL_ThemePart {
 public:
  CFWL_ThemeText() : m_pGraphics(NULL) {}
  CFX_WideString m_wsText;
  FX_DWORD m_dwTTOStyles;
  int32_t m_iTTOAlign;
  CFX_Graphics* m_pGraphics;
};
class IFWL_ThemeProvider {
 public:
  virtual ~IFWL_ThemeProvider() {}
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget) = 0;
  virtual FX_DWORD GetThemeID(IFWL_Widget* pWidget) = 0;
  virtual FX_DWORD SetThemeID(IFWL_Widget* pWidget,
                              FX_DWORD dwThemeID,
                              FX_BOOL bChildren = TRUE) = 0;
  virtual FWL_ERR GetThemeMatrix(IFWL_Widget* pWidget, CFX_Matrix& matrix) = 0;
  virtual FWL_ERR SetThemeMatrix(IFWL_Widget* pWidget,
                                 const CFX_Matrix& matrix) = 0;
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams) = 0;
  virtual FX_BOOL DrawText(CFWL_ThemeText* pParams) = 0;
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart,
                            FX_DWORD dwCapacity) = 0;
  virtual FX_BOOL IsCustomizedLayout(IFWL_Widget* pWidget) = 0;
  virtual FWL_ERR GetPartRect(CFWL_ThemePart* pThemePart,
                              CFX_RectF& rtPart) = 0;
  virtual FX_BOOL IsInPart(CFWL_ThemePart* pThemePart,
                           FX_FLOAT fx,
                           FX_FLOAT fy) = 0;
  virtual FX_BOOL CalcTextRect(CFWL_ThemeText* pParams, CFX_RectF& rect) = 0;
};
#endif
