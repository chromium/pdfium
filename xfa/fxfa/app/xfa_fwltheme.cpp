// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/xfa_fwltheme.h"

#include "xfa/fde/tto/fde_textout.h"
#include "xfa/fgas/crt/fgas_codepage.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fwl/cfwl_barcode.h"
#include "xfa/fwl/cfwl_caret.h"
#include "xfa/fwl/cfwl_checkbox.h"
#include "xfa/fwl/cfwl_combobox.h"
#include "xfa/fwl/cfwl_datetimepicker.h"
#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_listbox.h"
#include "xfa/fwl/cfwl_monthcalendar.h"
#include "xfa/fwl/cfwl_picturebox.h"
#include "xfa/fwl/cfwl_pushbutton.h"
#include "xfa/fwl/cfwl_scrollbar.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fxfa/xfa_ffapp.h"
#include "xfa/fxfa/xfa_ffwidget.h"
#include "xfa/fxgraphics/cfx_color.h"

namespace {

const FX_WCHAR* const g_FWLTheme_CalFonts[] = {
    L"Arial", L"Courier New", L"DejaVu Sans",
};

const float kLineHeight = 12.0f;

}  // namespace

CXFA_FFWidget* XFA_ThemeGetOuterWidget(CFWL_Widget* pWidget) {
  CFWL_Widget* pOuter = pWidget;
  while (pOuter && pOuter->GetOuter())
    pOuter = pOuter->GetOuter();
  return pOuter ? pOuter->GetLayoutItem() : nullptr;
}

CXFA_FWLTheme::CXFA_FWLTheme(CXFA_FFApp* pApp)
    : m_pCheckBoxTP(new CFWL_CheckBoxTP),
      m_pListBoxTP(new CFWL_ListBoxTP),
      m_pPictureBoxTP(new CFWL_PictureBoxTP),
      m_pSrollBarTP(new CFWL_ScrollBarTP),
      m_pEditTP(new CFWL_EditTP),
      m_pComboBoxTP(new CFWL_ComboBoxTP),
      m_pMonthCalendarTP(new CFWL_MonthCalendarTP),
      m_pDateTimePickerTP(new CFWL_DateTimePickerTP),
      m_pPushButtonTP(new CFWL_PushButtonTP),
      m_pCaretTP(new CFWL_CaretTP),
      m_pBarcodeTP(new CFWL_BarcodeTP),
      m_pTextOut(new CFDE_TextOut),
      m_pCalendarFont(nullptr),
      m_pApp(pApp) {
  m_Rect.Reset();

  for (size_t i = 0; !m_pCalendarFont && i < FX_ArraySize(g_FWLTheme_CalFonts);
       ++i) {
    m_pCalendarFont = CFGAS_GEFont::LoadFont(g_FWLTheme_CalFonts[i], 0, 0,
                                             m_pApp->GetFDEFontMgr());
  }
  if (!m_pCalendarFont) {
    m_pCalendarFont = m_pApp->GetFDEFontMgr()->GetFontByCodePage(
        FX_CODEPAGE_MSWin_WesternEuropean, 0, nullptr);
  }

  ASSERT(m_pCalendarFont);
}

CXFA_FWLTheme::~CXFA_FWLTheme() {
  m_pTextOut.reset();
  FWLTHEME_Release();
}

void CXFA_FWLTheme::DrawBackground(CFWL_ThemeBackground* pParams) {
  GetTheme(pParams->m_pWidget)->DrawBackground(pParams);
}

void CXFA_FWLTheme::DrawText(CFWL_ThemeText* pParams) {
  if (pParams->m_wsText.IsEmpty())
    return;

  if (pParams->m_pWidget->GetClassID() == FWL_Type::MonthCalendar) {
    CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pParams->m_pWidget);
    if (!pWidget)
      return;

    m_pTextOut->SetStyles(pParams->m_dwTTOStyles);
    m_pTextOut->SetAlignment(pParams->m_iTTOAlign);
    m_pTextOut->SetFont(m_pCalendarFont);
    m_pTextOut->SetFontSize(FWLTHEME_CAPACITY_FontSize);
    m_pTextOut->SetTextColor(FWLTHEME_CAPACITY_TextColor);
    if ((pParams->m_iPart == CFWL_Part::DatesIn) &&
        !(pParams->m_dwStates & FWL_ITEMSTATE_MCD_Flag) &&
        (pParams->m_dwStates &
         (CFWL_PartState_Hovered | CFWL_PartState_Selected))) {
      m_pTextOut->SetTextColor(0xFFFFFFFF);
    }
    if (pParams->m_iPart == CFWL_Part::Caption)
      m_pTextOut->SetTextColor(ArgbEncode(0xff, 0, 153, 255));

    CFX_Graphics* pGraphics = pParams->m_pGraphics;
    CFX_RenderDevice* pRenderDevice = pGraphics->GetRenderDevice();
    if (!pRenderDevice)
      return;

    m_pTextOut->SetRenderDevice(pRenderDevice);
    CFX_Matrix mtPart = pParams->m_matrix;
    CFX_Matrix* pMatrix = pGraphics->GetMatrix();
    if (pMatrix) {
      mtPart.Concat(*pMatrix);
    }
    m_pTextOut->SetMatrix(mtPart);
    m_pTextOut->DrawLogicText(pParams->m_wsText.c_str(),
                              pParams->m_wsText.GetLength(), pParams->m_rtPart);
    return;
  }
  CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pParams->m_pWidget);
  if (!pWidget)
    return;

  CXFA_WidgetAcc* pAcc = pWidget->GetDataAcc();
  CFX_Graphics* pGraphics = pParams->m_pGraphics;
  CFX_RenderDevice* pRenderDevice = pGraphics->GetRenderDevice();
  if (!pRenderDevice)
    return;

  m_pTextOut->SetRenderDevice(pRenderDevice);
  m_pTextOut->SetStyles(pParams->m_dwTTOStyles);
  m_pTextOut->SetAlignment(pParams->m_iTTOAlign);
  m_pTextOut->SetFont(pAcc->GetFDEFont());
  m_pTextOut->SetFontSize(pAcc->GetFontSize());
  m_pTextOut->SetTextColor(pAcc->GetTextColor());
  CFX_Matrix mtPart = pParams->m_matrix;
  CFX_Matrix* pMatrix = pGraphics->GetMatrix();
  if (pMatrix)
    mtPart.Concat(*pMatrix);

  m_pTextOut->SetMatrix(mtPart);
  m_pTextOut->DrawLogicText(pParams->m_wsText.c_str(),
                            pParams->m_wsText.GetLength(), pParams->m_rtPart);
}

CFX_RectF CXFA_FWLTheme::GetUIMargin(CFWL_ThemePart* pThemePart) const {
  CFX_RectF rect;
  CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pThemePart->m_pWidget);
  if (!pWidget)
    return rect;

  CXFA_LayoutItem* pItem = pWidget;
  CXFA_WidgetAcc* pWidgetAcc = pWidget->GetDataAcc();
  rect = pWidgetAcc->GetUIMargin();
  if (CXFA_Para para = pWidgetAcc->GetPara()) {
    rect.left += para.GetMarginLeft();
    if (pWidgetAcc->IsMultiLine())
      rect.width += para.GetMarginRight();
  }
  if (!pItem->GetPrev()) {
    if (pItem->GetNext())
      rect.height = 0;
  } else if (!pItem->GetNext()) {
    rect.top = 0;
  } else {
    rect.top = 0;
    rect.height = 0;
  }
  return rect;
}

float CXFA_FWLTheme::GetCXBorderSize() const {
  return 1.0f;
}

float CXFA_FWLTheme::GetCYBorderSize() const {
  return 1.0f;
}

float CXFA_FWLTheme::GetFontSize(CFWL_ThemePart* pThemePart) const {
  if (CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pThemePart->m_pWidget))
    return pWidget->GetDataAcc()->GetFontSize();
  return FWLTHEME_CAPACITY_FontSize;
}

CFX_RetainPtr<CFGAS_GEFont> CXFA_FWLTheme::GetFont(
    CFWL_ThemePart* pThemePart) const {
  if (CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pThemePart->m_pWidget))
    return pWidget->GetDataAcc()->GetFDEFont();
  return GetTheme(pThemePart->m_pWidget)->GetFont();
}

float CXFA_FWLTheme::GetLineHeight(CFWL_ThemePart* pThemePart) const {
  if (CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pThemePart->m_pWidget))
    return pWidget->GetDataAcc()->GetLineHeight();
  return kLineHeight;
}

float CXFA_FWLTheme::GetScrollBarWidth() const {
  return 9.0f;
}

FX_COLORREF CXFA_FWLTheme::GetTextColor(CFWL_ThemePart* pThemePart) const {
  if (CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pThemePart->m_pWidget))
    return pWidget->GetDataAcc()->GetTextColor();
  return FWLTHEME_CAPACITY_TextColor;
}

CFX_SizeF CXFA_FWLTheme::GetSpaceAboveBelow(CFWL_ThemePart* pThemePart) const {
  CFX_SizeF sizeAboveBelow;
  if (CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pThemePart->m_pWidget)) {
    CXFA_WidgetAcc* pWidgetAcc = pWidget->GetDataAcc();
    if (CXFA_Para para = pWidgetAcc->GetPara()) {
      sizeAboveBelow.width = para.GetSpaceAbove();
      sizeAboveBelow.height = para.GetSpaceBelow();
    }
  }
  return sizeAboveBelow;
}

void CXFA_FWLTheme::CalcTextRect(CFWL_ThemeText* pParams, CFX_RectF& rect) {
  if (pParams->m_pWidget->GetClassID() == FWL_Type::MonthCalendar) {
    CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pParams->m_pWidget);
    if (!pWidget || !pParams || !m_pTextOut)
      return;

    m_pTextOut->SetFont(m_pCalendarFont);
    m_pTextOut->SetFontSize(FWLTHEME_CAPACITY_FontSize);
    m_pTextOut->SetTextColor(FWLTHEME_CAPACITY_TextColor);
    m_pTextOut->SetAlignment(pParams->m_iTTOAlign);
    m_pTextOut->SetStyles(pParams->m_dwTTOStyles);
    m_pTextOut->CalcLogicSize(pParams->m_wsText.c_str(),
                              pParams->m_wsText.GetLength(), rect);
  }

  CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pParams->m_pWidget);
  if (!pWidget)
    return;

  CXFA_WidgetAcc* pAcc = pWidget->GetDataAcc();
  m_pTextOut->SetFont(pAcc->GetFDEFont());
  m_pTextOut->SetFontSize(pAcc->GetFontSize());
  m_pTextOut->SetTextColor(pAcc->GetTextColor());
  if (!pParams)
    return;

  m_pTextOut->SetAlignment(pParams->m_iTTOAlign);
  m_pTextOut->SetStyles(pParams->m_dwTTOStyles);
  m_pTextOut->CalcLogicSize(pParams->m_wsText.c_str(),
                            pParams->m_wsText.GetLength(), rect);
}

CFWL_WidgetTP* CXFA_FWLTheme::GetTheme(CFWL_Widget* pWidget) const {
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
