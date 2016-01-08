// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_fwltheme.h"
#include "xfa_ffwidget.h"
#include "xfa_ffapp.h"
CXFA_FFWidget* XFA_ThemeGetOuterWidget(IFWL_Widget* pWidget) {
  IFWL_Widget* pOuter = pWidget;
  while (pOuter->GetOuter()) {
    pOuter = pOuter->GetOuter();
  }
  if (pOuter) {
    return (CXFA_FFWidget*)pOuter->GetPrivateData(pOuter);
  }
  return NULL;
}
CXFA_FWLTheme::CXFA_FWLTheme(CXFA_FFApp* pApp) : m_pApp(pApp) {
  m_pTextOut = NULL;
  m_dwCapacity = 0;
  m_fCapacity = 0;
  m_pCalendarFont = NULL;
  m_Rect.Set(0, 0, 0, 0);
  m_SizeAboveBelow.Set(0, 0);
  m_pCheckBoxTP = new CXFA_FWLCheckBoxTP;
  m_pListBoxTP = new CFWL_ListBoxTP;
  m_pPictureBoxTP = new CFWL_PictureBoxTP;
  m_pSrollBarTP = new CFWL_ScrollBarTP;
  m_pEditTP = new CXFA_FWLEditTP;
  m_pComboBoxTP = new CFWL_ComboBoxTP;
  m_pMonthCalendarTP = new CFWL_MonthCalendarTP;
  m_pDateTimePickerTP = new CFWL_DateTimePickerTP;
  m_pPushButtonTP = new CFWL_PushButtonTP;
  m_pCaretTP = new CFWL_CaretTP;
  m_pBarcodeTP = new CFWL_BarcodeTP;
  Initialize();
}
CXFA_FWLTheme::~CXFA_FWLTheme() {
  Finalize();
  delete m_pCheckBoxTP;
  delete m_pListBoxTP;
  delete m_pPictureBoxTP;
  delete m_pSrollBarTP;
  delete m_pEditTP;
  delete m_pComboBoxTP;
  delete m_pMonthCalendarTP;
  delete m_pDateTimePickerTP;
  delete m_pPushButtonTP;
  delete m_pCaretTP;
  delete m_pBarcodeTP;
}
static const FX_WCHAR* g_FWLTheme_CalFonts[] = {
    L"Arial", L"Courier New", L"DejaVu Sans",
};
FWL_ERR CXFA_FWLTheme::Initialize() {
  m_pTextOut = IFDE_TextOut::Create();
  for (int32_t i = 0; NULL == m_pCalendarFont &&
                      i < sizeof(g_FWLTheme_CalFonts) / sizeof(const FX_WCHAR*);
       i++) {
    m_pCalendarFont = IFX_Font::LoadFont(g_FWLTheme_CalFonts[i], 0, 0,
                                         m_pApp->GetFDEFontMgr());
  }
  if (NULL == m_pCalendarFont)
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    m_pCalendarFont = m_pApp->GetFDEFontMgr()->GetDefFontByCodePage(
        FX_CODEPAGE_MSWin_WesternEuropean, 0, NULL);
#else
    m_pCalendarFont = m_pApp->GetFDEFontMgr()->GetFontByCodePage(
        FX_CODEPAGE_MSWin_WesternEuropean, 0, NULL);
#endif
  FXSYS_assert(NULL != m_pCalendarFont);
  FWLTHEME_Init();
  return FWL_ERR_Succeeded;
}
FWL_ERR CXFA_FWLTheme::Finalize() {
  if (m_pTextOut) {
    m_pTextOut->Release();
    m_pTextOut = NULL;
  }
  if (m_pCalendarFont) {
    m_pCalendarFont->Release();
    m_pCalendarFont = NULL;
  }
  FWLTHEME_Release();
  return FWL_ERR_Succeeded;
}
FX_BOOL CXFA_FWLTheme::IsValidWidget(IFWL_Widget* pWidget) {
  return TRUE;
}
FX_DWORD CXFA_FWLTheme::GetThemeID(IFWL_Widget* pWidget) {
  return 0;
}
FX_DWORD CXFA_FWLTheme::SetThemeID(IFWL_Widget* pWidget,
                                   FX_DWORD dwThemeID,
                                   FX_BOOL bChildren) {
  return 0;
}
FX_BOOL CXFA_FWLTheme::DrawBackground(CFWL_ThemeBackground* pParams) {
  return GetTheme(pParams->m_pWidget)->DrawBackground(pParams);
}
FX_BOOL CXFA_FWLTheme::DrawText(CFWL_ThemeText* pParams) {
  if (pParams->m_wsText.IsEmpty()) {
    return FWL_ERR_Indefinite;
  }
  if (pParams->m_pWidget->GetClassID() == FWL_CLASSHASH_MonthCalendar) {
    CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pParams->m_pWidget);
    if (!pWidget) {
      return FWL_ERR_Indefinite;
    }
    m_pTextOut->SetStyles(pParams->m_dwTTOStyles);
    m_pTextOut->SetAlignment(pParams->m_iTTOAlign);
    m_pTextOut->SetFont(m_pCalendarFont);
    m_pTextOut->SetFontSize(FWLTHEME_CAPACITY_FontSize);
    m_pTextOut->SetTextColor(FWLTHEME_CAPACITY_TextColor);
    if ((pParams->m_iPart == FWL_PART_MCD_DatesIn) &&
        !(pParams->m_dwStates & FWL_ITEMSTATE_MCD_Flag) &&
        (pParams->m_dwStates &
         (FWL_PARTSTATE_MCD_Hovered | FWL_PARTSTATE_MCD_Selected))) {
      m_pTextOut->SetTextColor(0xFFFFFFFF);
    }
    if (pParams->m_iPart == FWL_PART_MCD_Caption) {
      if (m_pMonthCalendarTP->GetThemeID(pParams->m_pWidget) == 0) {
        m_pTextOut->SetTextColor(ArgbEncode(0xff, 0, 153, 255));
      } else {
        m_pTextOut->SetTextColor(ArgbEncode(0xff, 128, 128, 0));
      }
    }
    CFX_Graphics* pGraphics = pParams->m_pGraphics;
    CFX_RenderDevice* pRenderDevice = pGraphics->GetRenderDevice();
    if (!pRenderDevice)
      return FALSE;
    m_pTextOut->SetRenderDevice(pRenderDevice);
    CFX_Matrix mtPart = pParams->m_matrix;
    CFX_Matrix* pMatrix = pGraphics->GetMatrix();
    if (pMatrix) {
      mtPart.Concat(*pMatrix);
    }
    m_pTextOut->SetMatrix(mtPart);
    m_pTextOut->DrawLogicText(pParams->m_wsText, pParams->m_wsText.GetLength(),
                              pParams->m_rtPart);
    return TRUE;
  }
  CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pParams->m_pWidget);
  if (!pWidget) {
    return FWL_ERR_Indefinite;
  }
  CXFA_WidgetAcc* pAcc = pWidget->GetDataAcc();
  CFX_Graphics* pGraphics = pParams->m_pGraphics;
  CFX_RenderDevice* pRenderDevice = pGraphics->GetRenderDevice();
  if (!pRenderDevice)
    return FALSE;
  m_pTextOut->SetRenderDevice(pRenderDevice);
  m_pTextOut->SetStyles(pParams->m_dwTTOStyles);
  m_pTextOut->SetAlignment(pParams->m_iTTOAlign);
  m_pTextOut->SetFont(pAcc->GetFDEFont());
  m_pTextOut->SetFontSize(pAcc->GetFontSize());
  m_pTextOut->SetTextColor(pAcc->GetTextColor());
  CFX_Matrix mtPart = pParams->m_matrix;
  CFX_Matrix* pMatrix = pGraphics->GetMatrix();
  if (pMatrix) {
    mtPart.Concat(*pMatrix);
  }
  m_pTextOut->SetMatrix(mtPart);
  m_pTextOut->DrawLogicText(pParams->m_wsText, pParams->m_wsText.GetLength(),
                            pParams->m_rtPart);
  return TRUE;
}
void* CXFA_FWLTheme::GetCapacity(CFWL_ThemePart* pThemePart,
                                 FX_DWORD dwCapacity) {
  switch (dwCapacity) {
    case FWL_WGTCAPACITY_Font: {
      if (CXFA_FFWidget* pWidget =
              XFA_ThemeGetOuterWidget(pThemePart->m_pWidget)) {
        return pWidget->GetDataAcc()->GetFDEFont();
      }
    } break;
    case FWL_WGTCAPACITY_FontSize: {
      if (CXFA_FFWidget* pWidget =
              XFA_ThemeGetOuterWidget(pThemePart->m_pWidget)) {
        m_fCapacity = pWidget->GetDataAcc()->GetFontSize();
        return &m_fCapacity;
      }
    } break;
    case FWL_WGTCAPACITY_TextColor: {
      if (CXFA_FFWidget* pWidget =
              XFA_ThemeGetOuterWidget(pThemePart->m_pWidget)) {
        m_dwCapacity = pWidget->GetDataAcc()->GetTextColor();
        return &m_dwCapacity;
      }
    } break;
    case FWL_WGTCAPACITY_LineHeight: {
      if (CXFA_FFWidget* pWidget =
              XFA_ThemeGetOuterWidget(pThemePart->m_pWidget)) {
        m_fCapacity = pWidget->GetDataAcc()->GetLineHeight();
        return &m_fCapacity;
      }
    } break;
    case FWL_WGTCAPACITY_ScrollBarWidth: {
      m_fCapacity = 9;
      return &m_fCapacity;
    } break;
    case FWL_WGTCAPACITY_UIMargin: {
      CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pThemePart->m_pWidget);
      if (pWidget) {
        CXFA_LayoutItem* pItem = pWidget;
        CXFA_WidgetAcc* pWidgetAcc = pWidget->GetDataAcc();
        pWidgetAcc->GetUIMargin(m_Rect);
        if (CXFA_Para para = pWidgetAcc->GetPara()) {
          m_Rect.left += para.GetMarginLeft();
          if (pWidgetAcc->IsMultiLine()) {
            m_Rect.width += para.GetMarginRight();
          }
        }
        if (pItem->GetPrev() == NULL) {
          if (pItem->GetNext() != NULL) {
            m_Rect.height = 0;
          }
        } else if (pItem->GetNext() == NULL) {
          m_Rect.top = 0;
        } else {
          m_Rect.top = 0;
          m_Rect.height = 0;
        }
      }
      return &m_Rect;
    } break;
    case FWL_WGTCAPACITY_SpaceAboveBelow: {
      CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pThemePart->m_pWidget);
      if (pWidget) {
        CXFA_WidgetAcc* pWidgetAcc = pWidget->GetDataAcc();
        if (CXFA_Para para = pWidgetAcc->GetPara()) {
          m_SizeAboveBelow.x = para.GetSpaceAbove();
          m_SizeAboveBelow.y = para.GetSpaceBelow();
        }
      }
      return &m_SizeAboveBelow;
    } break;
    default:
      break;
  }
  if (pThemePart->m_pWidget->GetClassID() == FWL_CLASSHASH_MonthCalendar &&
      dwCapacity >= FWL_MCCAPACITY_Sun && dwCapacity <= FWL_MCCAPACITY_Today) {
    if (CXFA_FFWidget* pWidget =
            XFA_ThemeGetOuterWidget(pThemePart->m_pWidget)) {
      IXFA_AppProvider* pAppProvider = pWidget->GetAppProvider();
      m_wsResource.Empty();
      pAppProvider->LoadString(
          XFA_IDS_StringWeekDay_Sun + dwCapacity - FWL_WGTCAPACITY_MAX - 5,
          m_wsResource);
      if (!m_wsResource.IsEmpty()) {
        return &m_wsResource;
      }
    }
  }
  return GetTheme(pThemePart->m_pWidget)->GetCapacity(pThemePart, dwCapacity);
}
FX_BOOL CXFA_FWLTheme::IsCustomizedLayout(IFWL_Widget* pWidget) {
  return GetTheme(pWidget)->IsCustomizedLayout(pWidget);
}
FWL_ERR CXFA_FWLTheme::GetPartRect(CFWL_ThemePart* pThemePart) {
  CFX_RectF rect;
  return GetTheme(pThemePart->m_pWidget)->GetPartRect(pThemePart, rect);
}
FX_BOOL CXFA_FWLTheme::IsInPart(CFWL_ThemePart* pThemePart,
                                FX_FLOAT fx,
                                FX_FLOAT fy) {
  return GetTheme(pThemePart->m_pWidget)->IsInPart(pThemePart, fx, fy);
}
FX_BOOL CXFA_FWLTheme::CalcTextRect(CFWL_ThemeText* pParams, CFX_RectF& rect) {
  if (pParams->m_pWidget->GetClassID() == FWL_CLASSHASH_MonthCalendar) {
    CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pParams->m_pWidget);
    if (!pWidget) {
      return FWL_ERR_Indefinite;
    }
    if (!pParams)
      return FALSE;
    if (!m_pTextOut)
      return FALSE;
    m_pTextOut->SetFont(m_pCalendarFont);
    m_pTextOut->SetFontSize(FWLTHEME_CAPACITY_FontSize);
    m_pTextOut->SetTextColor(FWLTHEME_CAPACITY_TextColor);
    m_pTextOut->SetAlignment(pParams->m_iTTOAlign);
    m_pTextOut->SetStyles(pParams->m_dwTTOStyles);
    m_pTextOut->CalcLogicSize(pParams->m_wsText, pParams->m_wsText.GetLength(),
                              rect);
    return TRUE;
  }
  CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pParams->m_pWidget);
  if (!pWidget) {
    return FWL_ERR_Indefinite;
  }
  CXFA_WidgetAcc* pAcc = pWidget->GetDataAcc();
  m_pTextOut->SetFont(pAcc->GetFDEFont());
  m_pTextOut->SetFontSize(pAcc->GetFontSize());
  m_pTextOut->SetTextColor(pAcc->GetTextColor());
  if (!pParams)
    return FALSE;
  if (!m_pTextOut)
    return FALSE;
  m_pTextOut->SetAlignment(pParams->m_iTTOAlign);
  m_pTextOut->SetStyles(pParams->m_dwTTOStyles);
  m_pTextOut->CalcLogicSize(pParams->m_wsText, pParams->m_wsText.GetLength(),
                            rect);
  return TRUE;
}
CFWL_WidgetTP* CXFA_FWLTheme::GetTheme(IFWL_Widget* pWidget) {
  switch (pWidget->GetClassID()) {
    case FWL_CLASSHASH_CheckBox:
      return m_pCheckBoxTP;
    case FWL_CLASSHASH_ListBox:
      return m_pListBoxTP;
    case FWL_CLASSHASH_PictureBox:
      return m_pPictureBoxTP;
    case FWL_CLASSHASH_ScrollBar:
      return m_pSrollBarTP;
    case FWL_CLASSHASH_Edit:
      return m_pEditTP;
    case FWL_CLASSHASH_ComboBox:
      return m_pComboBoxTP;
    case FWL_CLASSHASH_MonthCalendar:
      return m_pMonthCalendarTP;
    case FWL_CLASSHASH_DateTimePicker:
      return m_pDateTimePickerTP;
    case FWL_CLASSHASH_PushButton:
      return m_pPushButtonTP;
    case FWL_CLASSHASH_Caret:
      return m_pCaretTP;
    case FWL_CLASSHASH_Barcode:
      return m_pBarcodeTP;
    default:
      break;
  }
  return NULL;
}
CXFA_FWLCheckBoxTP::CXFA_FWLCheckBoxTP() {}
FX_BOOL CXFA_FWLCheckBoxTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  if (pParams->m_iPart != FWL_PART_CKB_CheckBox) {
    return TRUE;
  }
  if (((pParams->m_dwStates & FWL_PARTSTATE_CKB_Mask2) ==
       FWL_PARTSTATE_CKB_Checked) ||
      ((pParams->m_dwStates & FWL_PARTSTATE_CKB_Mask2) ==
       FWL_PARTSTATE_CKB_Neutral)) {
    DrawCheckSign(pParams->m_pWidget, pParams->m_pGraphics, &pParams->m_rtPart,
                  pParams->m_dwStates, &pParams->m_matrix);
  }
  return TRUE;
}
void CXFA_FWLCheckBoxTP::DrawCheckSign(IFWL_Widget* pWidget,
                                       CFX_Graphics* pGraphics,
                                       const CFX_RectF* pRtBox,
                                       int32_t iState,
                                       CFX_Matrix* pMatrix) {
  CFX_RectF rtSign(*pRtBox);
  FX_DWORD dwColor = 0xFF000000;
  if ((iState & FWL_PARTSTATE_CKB_Mask2) == FWL_PARTSTATE_CKB_Neutral) {
    dwColor = 0xFFA9A9A9;
  }
  {
    FX_DWORD dwStyle = pWidget->GetStylesEx();
    rtSign.Deflate(rtSign.width / 4, rtSign.height / 4);
    switch (dwStyle & FWL_STYLEEXT_CKB_SignShapeMask) {
      case FWL_STYLEEXT_CKB_SignShapeCheck:
        DrawSignCheck(pGraphics, &rtSign, dwColor, pMatrix);
        break;
      case FWL_STYLEEXT_CKB_SignShapeCircle:
        DrawSignCircle(pGraphics, &rtSign, dwColor, pMatrix);
        break;
      case FWL_STYLEEXT_CKB_SignShapeCross:
        DrawSignCross(pGraphics, &rtSign, dwColor, pMatrix);
        break;
      case FWL_STYLEEXT_CKB_SignShapeDiamond:
        DrawSignDiamond(pGraphics, &rtSign, dwColor, pMatrix);
        break;
      case FWL_STYLEEXT_CKB_SignShapeSquare:
        DrawSignSquare(pGraphics, &rtSign, dwColor, pMatrix);
        break;
      case FWL_STYLEEXT_CKB_SignShapeStar:
        DrawSignStar(pGraphics, &rtSign, dwColor, pMatrix);
        break;
      default:
        break;
    }
  }
}
CXFA_FWLEditTP::CXFA_FWLEditTP() {}
CXFA_FWLEditTP::~CXFA_FWLEditTP() {}
FX_BOOL CXFA_FWLEditTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  if (FWL_PART_EDT_CombTextLine == pParams->m_iPart) {
    CXFA_FFWidget* pWidget = XFA_ThemeGetOuterWidget(pParams->m_pWidget);
    FX_ARGB cr = 0xFF000000;
    FX_FLOAT fWidth = 1.0f;
    if (CXFA_Border borderUI = pWidget->GetDataAcc()->GetUIBorder()) {
      CXFA_Edge edge = borderUI.GetEdge(0);
      if (edge) {
        cr = edge.GetColor();
        fWidth = edge.GetThickness();
      }
    }
    CFX_Color crLine(cr);
    pParams->m_pGraphics->SetStrokeColor(&crLine);
    pParams->m_pGraphics->SetLineWidth(fWidth);
    pParams->m_pGraphics->StrokePath(pParams->m_pPath, &pParams->m_matrix);
    return TRUE;
  }
  return CFWL_EditTP::DrawBackground(pParams);
}
