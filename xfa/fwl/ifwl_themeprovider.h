// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_IFWL_THEMEPROVIDER_H_
#define XFA_FWL_IFWL_THEMEPROVIDER_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/fx_dib.h"
#include "v8/include/cppgc/garbage-collected.h"

class CFGAS_GEFont;
class CFWL_ThemeBackground;
class CFWL_ThemePart;
class CFWL_ThemeText;
class CFWL_Widget;
class CFWL_WidgetTP;

class IFWL_ThemeProvider : public cppgc::GarbageCollectedMixin {
 public:
  virtual ~IFWL_ThemeProvider();

  virtual void DrawBackground(const CFWL_ThemeBackground& pParams) = 0;
  virtual void DrawText(const CFWL_ThemeText& pParams) = 0;
  virtual void CalcTextRect(const CFWL_ThemeText& pParams,
                            CFX_RectF* pRect) = 0;
  virtual float GetCXBorderSize() const = 0;
  virtual float GetCYBorderSize() const = 0;
  virtual CFX_RectF GetUIMargin(const CFWL_ThemePart& pThemePart) const = 0;
  virtual float GetFontSize(const CFWL_ThemePart& pThemePart) const = 0;
  virtual RetainPtr<CFGAS_GEFont> GetFont(
      const CFWL_ThemePart& pThemePart) const = 0;
  virtual float GetLineHeight(const CFWL_ThemePart& pThemePart) const = 0;
  virtual float GetScrollBarWidth() const = 0;
  virtual FX_COLORREF GetTextColor(const CFWL_ThemePart& pThemePart) const = 0;
  virtual CFX_SizeF GetSpaceAboveBelow(
      const CFWL_ThemePart& pThemePart) const = 0;

 protected:
  IFWL_ThemeProvider();

  CFWL_WidgetTP* GetTheme(const CFWL_Widget* pWidget) const;

 private:
  std::unique_ptr<CFWL_WidgetTP> m_pCheckBoxTP;
  std::unique_ptr<CFWL_WidgetTP> m_pListBoxTP;
  std::unique_ptr<CFWL_WidgetTP> m_pPictureBoxTP;
  std::unique_ptr<CFWL_WidgetTP> m_pSrollBarTP;
  std::unique_ptr<CFWL_WidgetTP> m_pEditTP;
  std::unique_ptr<CFWL_WidgetTP> m_pComboBoxTP;
  std::unique_ptr<CFWL_WidgetTP> m_pMonthCalendarTP;
  std::unique_ptr<CFWL_WidgetTP> m_pDateTimePickerTP;
  std::unique_ptr<CFWL_WidgetTP> m_pPushButtonTP;
  std::unique_ptr<CFWL_WidgetTP> m_pCaretTP;
  std::unique_ptr<CFWL_WidgetTP> m_pBarcodeTP;
};

#endif  // XFA_FWL_IFWL_THEMEPROVIDER_H_
