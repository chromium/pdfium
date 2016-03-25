// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_THEMEPROVIDER_H_
#define XFA_FWL_CORE_IFWL_THEMEPROVIDER_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/fwl_error.h"

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

class CFWL_ThemeBackground;
class CFWL_ThemePart;
class CFWL_ThemeText;
class IFWL_Widget;

class IFWL_ThemeProvider {
 public:
  virtual ~IFWL_ThemeProvider() {}
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget) = 0;
  virtual uint32_t GetThemeID(IFWL_Widget* pWidget) = 0;
  virtual uint32_t SetThemeID(IFWL_Widget* pWidget,
                              uint32_t dwThemeID,
                              FX_BOOL bChildren = TRUE) = 0;
  virtual FWL_ERR GetThemeMatrix(IFWL_Widget* pWidget, CFX_Matrix& matrix) = 0;
  virtual FWL_ERR SetThemeMatrix(IFWL_Widget* pWidget,
                                 const CFX_Matrix& matrix) = 0;
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams) = 0;
  virtual FX_BOOL DrawText(CFWL_ThemeText* pParams) = 0;
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart,
                            uint32_t dwCapacity) = 0;
  virtual FX_BOOL IsCustomizedLayout(IFWL_Widget* pWidget) = 0;
  virtual FWL_ERR GetPartRect(CFWL_ThemePart* pThemePart,
                              CFX_RectF& rtPart) = 0;
  virtual FX_BOOL IsInPart(CFWL_ThemePart* pThemePart,
                           FX_FLOAT fx,
                           FX_FLOAT fy) = 0;
  virtual FX_BOOL CalcTextRect(CFWL_ThemeText* pParams, CFX_RectF& rect) = 0;
};

#endif  // XFA_FWL_CORE_IFWL_THEMEPROVIDER_H_
