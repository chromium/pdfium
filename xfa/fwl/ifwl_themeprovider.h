// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_IFWL_THEMEPROVIDER_H_
#define XFA_FWL_IFWL_THEMEPROVIDER_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/fx_dib.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"

class CFGAS_GEFont;

namespace pdfium {

class CFWL_ThemeBackground;
class CFWL_ThemePart;
class CFWL_ThemeText;
class CFWL_Widget;
class CFWL_WidgetTP;

class IFWL_ThemeProvider : public cppgc::GarbageCollectedMixin {
 public:
  virtual ~IFWL_ThemeProvider();

  // GarbageCollectedMixin:
  void Trace(cppgc::Visitor* visitor) const override;

  virtual void DrawBackground(const CFWL_ThemeBackground& pParams) = 0;
  virtual void DrawText(const CFWL_ThemeText& pParams) = 0;
  virtual void CalcTextRect(const CFWL_ThemeText& pParams,
                            CFX_RectF* pRect) = 0;
  virtual float GetCXBorderSize() const = 0;
  virtual float GetCYBorderSize() const = 0;
  virtual CFX_RectF GetUIMargin(const CFWL_ThemePart& pThemePart) const = 0;
  virtual float GetFontSize(const CFWL_ThemePart& pThemePart) const = 0;
  virtual RetainPtr<CFGAS_GEFont> GetFont(const CFWL_ThemePart& pThemePart) = 0;
  virtual RetainPtr<CFGAS_GEFont> GetFWLFont() = 0;
  virtual float GetLineHeight(const CFWL_ThemePart& pThemePart) const = 0;
  virtual float GetScrollBarWidth() const = 0;
  virtual FX_COLORREF GetTextColor(const CFWL_ThemePart& pThemePart) const = 0;
  virtual CFX_SizeF GetSpaceAboveBelow(
      const CFWL_ThemePart& pThemePart) const = 0;

 protected:
  explicit IFWL_ThemeProvider(cppgc::Heap* pHeap);

  CFWL_WidgetTP* GetTheme(const CFWL_Widget* pWidget) const;

 private:
  cppgc::Member<CFWL_WidgetTP> m_pCheckBoxTP;
  cppgc::Member<CFWL_WidgetTP> m_pListBoxTP;
  cppgc::Member<CFWL_WidgetTP> m_pPictureBoxTP;
  cppgc::Member<CFWL_WidgetTP> m_pSrollBarTP;
  cppgc::Member<CFWL_WidgetTP> m_pEditTP;
  cppgc::Member<CFWL_WidgetTP> m_pComboBoxTP;
  cppgc::Member<CFWL_WidgetTP> m_pMonthCalendarTP;
  cppgc::Member<CFWL_WidgetTP> m_pDateTimePickerTP;
  cppgc::Member<CFWL_WidgetTP> m_pPushButtonTP;
  cppgc::Member<CFWL_WidgetTP> m_pCaretTP;
  cppgc::Member<CFWL_WidgetTP> m_pBarcodeTP;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::IFWL_ThemeProvider;

#endif  // XFA_FWL_IFWL_THEMEPROVIDER_H_
