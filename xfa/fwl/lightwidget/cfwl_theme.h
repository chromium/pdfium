// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_LIGHTWIDGET_CFWL_THEME_H_
#define XFA_FWL_LIGHTWIDGET_CFWL_THEME_H_

#include <memory>
#include <vector>

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"
#include "core/include/fxge/fx_dib.h"
#include "xfa/fwl/core/ifwl_themeprovider.h"

class CFWL_WidgetTP;
class IFWL_Widget;
class CFWL_ThemePart;
class CFWL_ThemeText;

class CFWL_Theme : public IFWL_ThemeProvider {
 public:
  CFWL_Theme();
  ~CFWL_Theme() override;

  // IFWL_ThemeProvider:
  FX_BOOL IsValidWidget(IFWL_Widget* pWidget) override;
  uint32_t GetThemeID(IFWL_Widget* pWidget) override;
  uint32_t SetThemeID(IFWL_Widget* pWidget,
                      uint32_t dwThemeID,
                      FX_BOOL bChildren = TRUE) override;
  FWL_ERR GetThemeMatrix(IFWL_Widget* pWidget, CFX_Matrix& matrix) override;
  FWL_ERR SetThemeMatrix(IFWL_Widget* pWidget,
                         const CFX_Matrix& matrix) override;
  FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams) override;
  FX_BOOL DrawText(CFWL_ThemeText* pParams) override;
  void* GetCapacity(CFWL_ThemePart* pThemePart, uint32_t dwCapacity) override;
  FX_BOOL IsCustomizedLayout(IFWL_Widget* pWidget) override;
  FWL_ERR GetPartRect(CFWL_ThemePart* pThemePart, CFX_RectF& rtPart) override;
  FX_BOOL IsInPart(CFWL_ThemePart* pThemePart,
                   FX_FLOAT fx,
                   FX_FLOAT fy) override;
  FX_BOOL CalcTextRect(CFWL_ThemeText* pParams, CFX_RectF& rect) override;

  FWL_ERR Initialize();
  FWL_ERR Finalize();
  FWL_ERR SetFont(IFWL_Widget* pWidget,
                  const FX_WCHAR* strFont,
                  FX_FLOAT fFontSize,
                  FX_ARGB rgbFont);
  CFWL_WidgetTP* GetTheme(IFWL_Widget* pWidget);

 protected:
  std::vector<std::unique_ptr<CFWL_WidgetTP>> m_ThemesArray;
};

#endif  // XFA_FWL_LIGHTWIDGET_CFWL_THEME_H_
