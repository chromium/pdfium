// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#define FWL_THEMECAPACITY_MC_HEADER_WIDTH 200
#define FWL_THEMECAPACITY_MC_HEADER_HEIGHT 30
#define FWL_THEMECAPACITY_MC_HEADER_BTN_WIDTH 18
#define FWL_THEMECAPACITY_MC_HEADER_BTN_HEIGHT 16
#define FWL_THEMECAPACITY_MC_HEADER_BTN_HMARGIN 5
#define FWL_THEMECAPACITY_MC_HEADER_BTN_VMARGIN \
  (FWL_THEMECAPACITY_MC_HEADER_HEIGHT -         \
   FWL_THEMECAPACITY_MC_HEADER_BTN_HEIGHT) /    \
      2
#define FWL_THEMECAPACITY_MC_HEADER_TEXTWIDHT 100
#define FWL_THEMECAPACITY_MC_HEADER_TEXTHEIGHT 20
#define FWL_THEMECAPACITY_MC_HEADER_TEXT_HMARGIN \
  (FWL_THEMECAPACITY_MC_HEADER_WIDTH -           \
   FWL_THEMECAPACITY_MC_HEADER_TEXTWIDHT) /      \
      2
#define FWL_THEMECAPACITY_MC_HEADER_TEXT_VMARGIN \
  (FWL_THEMECAPACITY_MC_HEADER_HEIGHT -          \
   FWL_THEMECAPACITY_MC_HEADER_TEXTHEIGHT) /     \
      2
#define FWL_THEMECAPACITY_MC_HSEP_WIDTH (FWL_THEMECAPACITY_MC_WEEK_WIDTH - 10)
#define FWL_THEMECAPACITY_MC_HSEP_HEIGHT 1
#define FWL_THEMECAPACITY_MC_VSEP_WIDTH 1
#define FWL_THEMECAPACITY_MC_VSEP_HEIGHT FWL_THEMECAPACITY_MC_WEEKNUM_HEIGHT
#define FWL_THEMECAPACITY_MC_WEEKNUM_WIDTH 26
#define FWL_THEMECAPACITY_MC_SEP_DOFFSET -4
#define FWL_THEMECAPACITY_MC_SEP_X 3
#define FWL_THEMECAPACITY_MC_SEP_Y                                         \
  (FWL_THEMECAPACITY_MC_HEADER_HEIGHT + FWL_THEMECAPACITY_MC_WEEK_HEIGHT + \
   FWL_THEMECAPACITY_MC_SEP_DOFFSET)
#define FWL_THEMECAPACITY_MC_WEEKNUM_HEIGHT \
  (6 * FWL_THEMECAPACITY_MC_DATES_CELL_HEIGHT)
#define FWL_THEMECAPACITY_MC_WEEK_WIDTH \
  (FWL_THEMECAPACITY_MC_DATES_CELL_WIDTH * 7)
#define FWL_THEMECAPACITY_MC_WEEK_HEIGHT FWL_THEMECAPACITY_MC_DATES_CELL_HEIGHT
#define FWL_THEMECAPACITY_MC_DATES_CELL_WIDTH \
  (FWL_THEMECAPACITY_MC_HEADER_WIDTH / 7)
#define FWL_THEMECAPACITY_MC_DATES_CELL_HEIGHT 16
#define FWL_THEMECAPACITY_MC_TODAY_WIDHT FWL_THEMECAPACITY_MC_HEADER_WIDTH
#define FWL_THEMECAPACITY_MC_TODAY_HEIGHT FWL_THEMECAPACITY_MC_DATES_CELL_HEIGHT
#define FWL_THEMECAPACITY_MC_TODAY_FLAG_WIDHT \
  FWL_THEMECAPACITY_MC_DATES_CELL_WIDTH
#define FWL_MC_WIDTH 200
#define FWL_MC_HEIGHT 160
CFWL_MonthCalendarTP::CFWL_MonthCalendarTP() {
  m_pThemeData = new MCThemeData;
  SetThemeData(0);
}
CFWL_MonthCalendarTP::~CFWL_MonthCalendarTP() {
  delete m_pThemeData;
}
FX_BOOL CFWL_MonthCalendarTP::IsValidWidget(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FALSE;
  return pWidget->GetClassID() == FWL_CLASSHASH_MonthCalendar;
}
FX_DWORD CFWL_MonthCalendarTP::SetThemeID(IFWL_Widget* pWidget,
                                          FX_DWORD dwThemeID,
                                          FX_BOOL bChildren) {
  if (m_pThemeData) {
    SetThemeData(FWL_GetThemeColor(dwThemeID));
  }
  return CFWL_WidgetTP::SetThemeID(pWidget, dwThemeID, bChildren);
}
FX_BOOL CFWL_MonthCalendarTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  if (!pParams)
    return FALSE;
  switch (pParams->m_iPart) {
    case FWL_PART_MCD_Border: {
      DrawBorder(pParams->m_pGraphics, &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_MCD_Edge: {
      DrawEdge(pParams->m_pGraphics, pParams->m_pWidget->GetStyles(),
               &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_MCD_Background: {
      DrawTotalBK(pParams, &pParams->m_matrix);
      break;
    }
    case FWL_PART_MCD_Header: {
      DrawHeadBk(pParams, &pParams->m_matrix);
      break;
    }
    case FWL_PART_MCD_LBtn: {
#ifdef THEME_XPSimilar
      FWLTHEME_STATE eState = GetState(pParams->m_dwStates);
      DrawArrowBtn(pParams->m_pGraphics, &pParams->m_rtPart,
                   FWLTHEME_DIRECTION_Left, eState, &pParams->m_matrix);
#else
      DrawLButton(pParams, &pParams->m_matrix);
#endif
      break;
    }
    case FWL_PART_MCD_RBtn: {
#ifdef THEME_XPSimilar
      FWLTHEME_STATE eState = GetState(pParams->m_dwStates);
      DrawArrowBtn(pParams->m_pGraphics, &pParams->m_rtPart,
                   FWLTHEME_DIRECTION_Right, eState, &pParams->m_matrix);
#else
      DrawRButton(pParams, &pParams->m_matrix);
#endif
      break;
    }
    case FWL_PART_MCD_HSeparator: {
      DrawHSeperator(pParams, &pParams->m_matrix);
      break;
    }
    case FWL_PART_MCD_DatesIn: {
      DrawDatesInBK(pParams, &pParams->m_matrix);
      break;
    }
    case FWL_PART_MCD_TodayCircle: {
      DrawTodayCircle(pParams, &pParams->m_matrix);
      break;
    }
    case FWL_PART_MCD_DateInCircle: {
      DrawDatesInCircle(pParams, &pParams->m_matrix);
      break;
    }
    case FWL_PART_MCD_WeekNumSep: {
      DrawWeekNumSep(pParams, &pParams->m_matrix);
      break;
    }
    default: {}
  }
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarTP::DrawText(CFWL_ThemeText* pParams) {
  if (!m_pTextOut)
    return FALSE;
  if ((pParams->m_iPart == FWL_PART_MCD_DatesIn) &&
      !(pParams->m_dwStates & FWL_ITEMSTATE_MCD_Flag) &&
      (pParams->m_dwStates &
       (FWL_PARTSTATE_MCD_Hovered | FWL_PARTSTATE_MCD_Selected))) {
    m_pTextOut->SetTextColor(0xFFFFFFFF);
  } else if (pParams->m_iPart == FWL_PART_MCD_Caption) {
    m_pTextOut->SetTextColor(m_pThemeData->clrCaption);
  } else {
    m_pTextOut->SetTextColor(0xFF000000);
  }
  return CFWL_WidgetTP::DrawText(pParams);
}
void* CFWL_MonthCalendarTP::GetCapacity(CFWL_ThemePart* pThemePart,
                                        FX_DWORD dwCapacity) {
  FX_BOOL bDefPro = FALSE;
  FX_BOOL bDwordVal = FALSE;
  switch (dwCapacity) {
    case FWL_WGTCAPACITY_MC_HEADER_WIDTH: {
      m_fValue = FWL_THEMECAPACITY_MC_HEADER_WIDTH;
      break;
    }
    case FWL_WGTCAPACITY_MC_HEADER_Height: {
      m_fValue = FWL_THEMECAPACITY_MC_HEADER_HEIGHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_HEADER_BTN_WIDTH: {
      m_fValue = FWL_THEMECAPACITY_MC_HEADER_BTN_WIDTH;
      break;
    }
    case FWL_WGTCAPACITY_MC_HEADER_BTN_HEIGHT: {
      m_fValue = FWL_THEMECAPACITY_MC_HEADER_BTN_HEIGHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_HEADER_BTN_HMARGIN: {
      bDwordVal = TRUE;
      m_dwValue = FWL_THEMECAPACITY_MC_HEADER_BTN_HMARGIN;
      break;
    }
    case FWL_WGTCAPACITY_MC_HEADER_BTN_VMARGIN: {
      m_fValue = FWL_THEMECAPACITY_MC_HEADER_BTN_VMARGIN;
      break;
    }
    case FWL_WGTCAPACITY_MC_HEADER_TEXTWIDHT: {
      m_fValue = FWL_THEMECAPACITY_MC_HEADER_TEXTWIDHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_HEADER_TEXTHEIGHT: {
      m_fValue = FWL_THEMECAPACITY_MC_HEADER_TEXTHEIGHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_HEADER_TEXT_HMARGIN: {
      m_fValue = FWL_THEMECAPACITY_MC_HEADER_TEXT_HMARGIN;
      break;
    }
    case FWL_WGTCAPACITY_MC_HEADER_TEXT_VMARGIN: {
      m_fValue = FWL_THEMECAPACITY_MC_HEADER_TEXT_VMARGIN;
      break;
    }
    case FWL_WGTCAPACITY_MC_HSEP_WIDTH: {
      m_fValue = FWL_THEMECAPACITY_MC_HSEP_WIDTH;
      break;
    }
    case FWL_WGTCAPACITY_MC_HSEP_HEIGHT: {
      m_fValue = FWL_THEMECAPACITY_MC_HSEP_HEIGHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_VSEP_WIDTH: {
      m_fValue = FWL_THEMECAPACITY_MC_VSEP_WIDTH;
      break;
    }
    case FWL_WGTCAPACITY_MC_VSEP_HEIGHT: {
      m_fValue = FWL_THEMECAPACITY_MC_VSEP_HEIGHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_WEEKNUM_WIDTH: {
      m_fValue = FWL_THEMECAPACITY_MC_WEEKNUM_WIDTH;
      break;
    }
    case FWL_WGTCAPACITY_MC_WEEKNUM_HEIGHT: {
      m_fValue = FWL_THEMECAPACITY_MC_WEEKNUM_HEIGHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_WEEK_WIDTH: {
      m_fValue = FWL_THEMECAPACITY_MC_WEEK_WIDTH;
      break;
    }
    case FWL_WGTCAPACITY_MC_WEEK_HEIGHT: {
      m_fValue = FWL_THEMECAPACITY_MC_WEEK_HEIGHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_SEP_DOFFSET: {
      m_fValue = FWL_THEMECAPACITY_MC_SEP_DOFFSET;
      break;
    }
    case FWL_WGTCAPACITY_MC_SEP_X: {
      m_fValue = FWL_THEMECAPACITY_MC_SEP_X;
      break;
    }
    case FWL_WGTCAPACITY_MC_SEP_Y: {
      m_fValue = FWL_THEMECAPACITY_MC_SEP_Y;
      break;
    }
    case FWL_WGTCAPACITY_MC_DATES_CELL_WIDTH: {
      m_fValue = FWL_THEMECAPACITY_MC_DATES_CELL_WIDTH;
      break;
    }
    case FWL_WGTCAPACITY_MC_DATES_CELL_HEIGHT: {
      m_fValue = FWL_THEMECAPACITY_MC_DATES_CELL_HEIGHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_TODAY_WIDHT: {
      m_fValue = FWL_THEMECAPACITY_MC_TODAY_WIDHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_TODAY_HEIGHT: {
      m_fValue = FWL_THEMECAPACITY_MC_TODAY_HEIGHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_TODAY_FLAG_WIDHT: {
      m_fValue = FWL_THEMECAPACITY_MC_TODAY_FLAG_WIDHT;
      break;
    }
    case FWL_WGTCAPACITY_MC_WIDTH: {
      m_fValue = FWL_MC_WIDTH;
      break;
    }
    case FWL_WGTCAPACITY_MC_HEIGHT: {
      m_fValue = FWL_MC_HEIGHT;
      break;
    }
    case FWL_MCCAPACITY_Sun: {
      wsResource = L"Sun";
      return &wsResource;
    }
    case FWL_MCCAPACITY_Mon: {
      wsResource = L"Mon";
      return &wsResource;
    }
    case FWL_MCCAPACITY_Tue: {
      wsResource = L"Tue";
      return &wsResource;
    }
    case FWL_MCCAPACITY_Wed: {
      wsResource = L"Wed";
      return &wsResource;
    }
    case FWL_MCCAPACITY_Thu: {
      wsResource = L"Thu";
      return &wsResource;
    }
    case FWL_MCCAPACITY_Fri: {
      wsResource = L"Fri";
      return &wsResource;
    }
    case FWL_MCCAPACITY_Sat: {
      wsResource = L"Sat";
      return &wsResource;
    }
    case FWL_MCCAPACITY_January: {
      wsResource = L"January";
      return &wsResource;
    }
    case FWL_MCCAPACITY_February: {
      wsResource = L"February";
      return &wsResource;
    }
    case FWL_MCCAPACITY_March: {
      wsResource = L"March";
      return &wsResource;
    }
    case FWL_MCCAPACITY_April: {
      wsResource = L"April";
      return &wsResource;
    }
    case FWL_MCCAPACITY_May: {
      wsResource = L"May";
      return &wsResource;
    }
    case FWL_MCCAPACITY_June: {
      wsResource = L"June";
      return &wsResource;
    }
    case FWL_MCCAPACITY_July: {
      wsResource = L"July";
      return &wsResource;
    }
    case FWL_MCCAPACITY_August: {
      wsResource = L"August";
      return &wsResource;
    }
    case FWL_MCCAPACITY_September: {
      wsResource = L"September";
      return &wsResource;
    }
    case FWL_MCCAPACITY_October: {
      wsResource = L"October";
      return &wsResource;
    }
    case FWL_MCCAPACITY_November: {
      wsResource = L"November";
      return &wsResource;
    }
    case FWL_MCCAPACITY_December: {
      wsResource = L"December";
      return &wsResource;
    }
    case FWL_MCCAPACITY_Today: {
      wsResource = L"Today";
      return &wsResource;
    }
    default: { bDefPro = TRUE; }
  }
  if (!bDefPro) {
    if (bDwordVal) {
      return &m_dwValue;
    }
    return &m_fValue;
  }
  return CFWL_WidgetTP::GetCapacity(pThemePart, dwCapacity);
}
FWL_ERR CFWL_MonthCalendarTP::Initialize() {
  InitTTO();
  return CFWL_WidgetTP::Initialize();
}
FWL_ERR CFWL_MonthCalendarTP::Finalize() {
  FinalizeTTO();
  return CFWL_WidgetTP::Finalize();
}
FX_BOOL CFWL_MonthCalendarTP::DrawTotalBK(CFWL_ThemeBackground* pParams,
                                          CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.Create();
  CFX_RectF rtTotal(pParams->m_rtPart);
  path.AddRectangle(rtTotal.left, rtTotal.top, rtTotal.width, rtTotal.height);
  pParams->m_pGraphics->SaveGraphState();
  CFX_Color clrBK(m_pThemeData->clrBK);
  pParams->m_pGraphics->SetFillColor(&clrBK);
  pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  pParams->m_pGraphics->RestoreGraphState();
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarTP::DrawHeadBk(CFWL_ThemeBackground* pParams,
                                         CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.Create();
  CFX_RectF rtHead = pParams->m_rtPart;
  path.AddRectangle(rtHead.left, rtHead.top, rtHead.width, rtHead.height);
  pParams->m_pGraphics->SaveGraphState();
  CFX_Color clrHeadBK(m_pThemeData->clrBK);
  pParams->m_pGraphics->SetFillColor(&clrHeadBK);
  pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  pParams->m_pGraphics->RestoreGraphState();
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarTP::DrawLButton(CFWL_ThemeBackground* pParams,
                                          CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.Create();
  CFX_RectF rtLBtn;
  rtLBtn = pParams->m_rtPart;
  path.AddRectangle(rtLBtn.left, rtLBtn.top, rtLBtn.width, rtLBtn.height);
  pParams->m_pGraphics->SaveGraphState();
  CFX_Color clrLBtnEdge(ArgbEncode(0xff, 205, 219, 243));
  pParams->m_pGraphics->SetStrokeColor(&clrLBtnEdge);
  pParams->m_pGraphics->StrokePath(&path, pMatrix);
  if ((pParams->m_dwStates & FWL_PARTSTATE_MCD_Pressed) ==
      FWL_PARTSTATE_MCD_Pressed) {
    CFX_Color clrLBtnFill(ArgbEncode(0xff, 174, 198, 242));
    pParams->m_pGraphics->SetFillColor(&clrLBtnFill);
    pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  } else {
    CFX_Color clrLBtnFill(ArgbEncode(0xff, 227, 235, 249));
    pParams->m_pGraphics->SetFillColor(&clrLBtnFill);
    pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  }
  path.Clear();
  path.MoveTo(rtLBtn.left + rtLBtn.Width() / 3 * 2,
              rtLBtn.top + rtLBtn.height / 4);
  path.LineTo(rtLBtn.left + rtLBtn.Width() / 3, rtLBtn.top + rtLBtn.height / 2);
  path.LineTo(rtLBtn.left + rtLBtn.Width() / 3 * 2,
              rtLBtn.bottom() - rtLBtn.height / 4);
  CFX_Color clrFlag(ArgbEncode(0xff, 50, 104, 205));
  pParams->m_pGraphics->SetStrokeColor(&clrFlag);
  pParams->m_pGraphics->StrokePath(&path, pMatrix);
  pParams->m_pGraphics->RestoreGraphState();
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarTP::DrawRButton(CFWL_ThemeBackground* pParams,
                                          CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.Create();
  CFX_RectF rtRBtn;
  rtRBtn = pParams->m_rtPart;
  path.AddRectangle(rtRBtn.left, rtRBtn.top, rtRBtn.width, rtRBtn.height);
  pParams->m_pGraphics->SaveGraphState();
  CFX_Color clrRBtnEdge(ArgbEncode(0xff, 205, 219, 243));
  pParams->m_pGraphics->SetStrokeColor(&clrRBtnEdge);
  pParams->m_pGraphics->StrokePath(&path, pMatrix);
  if ((pParams->m_dwStates & FWL_PARTSTATE_MCD_Pressed) ==
      FWL_PARTSTATE_MCD_Pressed) {
    CFX_Color clrRBtnFill(ArgbEncode(0xff, 174, 198, 242));
    pParams->m_pGraphics->SetFillColor(&clrRBtnFill);
    pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  } else {
    CFX_Color clrRBtnFill(ArgbEncode(0xff, 227, 235, 249));
    pParams->m_pGraphics->SetFillColor(&clrRBtnFill);
    pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  }
  path.Clear();
  path.MoveTo(rtRBtn.left + rtRBtn.Width() / 3, rtRBtn.top + rtRBtn.height / 4);
  path.LineTo(rtRBtn.left + rtRBtn.Width() / 3 * 2,
              rtRBtn.top + rtRBtn.height / 2);
  path.LineTo(rtRBtn.left + rtRBtn.Width() / 3,
              rtRBtn.bottom() - rtRBtn.height / 4);
  CFX_Color clrFlag(ArgbEncode(0xff, 50, 104, 205));
  pParams->m_pGraphics->SetStrokeColor(&clrFlag);
  pParams->m_pGraphics->StrokePath(&path, pMatrix);
  pParams->m_pGraphics->RestoreGraphState();
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarTP::DrawHSeperator(CFWL_ThemeBackground* pParams,
                                             CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.Create();
  CFX_RectF rtHSep;
  rtHSep = pParams->m_rtPart;
  path.MoveTo(rtHSep.left, rtHSep.top + rtHSep.height / 2);
  path.LineTo(rtHSep.right(), rtHSep.top + rtHSep.height / 2);
  pParams->m_pGraphics->SaveGraphState();
  CFX_Color clrHSep(m_pThemeData->clrSeperator);
  pParams->m_pGraphics->SetStrokeColor(&clrHSep);
  pParams->m_pGraphics->StrokePath(&path, pMatrix);
  pParams->m_pGraphics->RestoreGraphState();
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarTP::DrawWeekNumSep(CFWL_ThemeBackground* pParams,
                                             CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.Create();
  CFX_RectF rtWeekSep;
  rtWeekSep = pParams->m_rtPart;
  path.MoveTo(rtWeekSep.left, rtWeekSep.top);
  path.LineTo(rtWeekSep.left, rtWeekSep.bottom());
  pParams->m_pGraphics->SaveGraphState();
  CFX_Color clrHSep(m_pThemeData->clrSeperator);
  pParams->m_pGraphics->SetStrokeColor(&clrHSep);
  pParams->m_pGraphics->StrokePath(&path, pMatrix);
  pParams->m_pGraphics->RestoreGraphState();
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarTP::DrawDatesInBK(CFWL_ThemeBackground* pParams,
                                            CFX_Matrix* pMatrix) {
  pParams->m_pGraphics->SaveGraphState();
  if (pParams->m_dwStates & FWL_PARTSTATE_MCD_Selected) {
    CFX_Path path;
    path.Create();
    CFX_RectF rtSelDay;
    rtSelDay = pParams->m_rtPart;
    path.AddRectangle(rtSelDay.left, rtSelDay.top, rtSelDay.width,
                      rtSelDay.height);
    CFX_Color clrSelDayBK;
    clrSelDayBK = m_pThemeData->clrDatesSelectedBK;
    pParams->m_pGraphics->SetFillColor(&clrSelDayBK);
    pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  } else if (pParams->m_dwStates & FWL_PARTSTATE_MCD_Hovered) {
    CFX_Path path;
    path.Create();
    CFX_RectF rtSelDay;
    rtSelDay = pParams->m_rtPart;
    path.AddRectangle(rtSelDay.left, rtSelDay.top, rtSelDay.width,
                      rtSelDay.height);
    CFX_Color clrSelDayBK;
    clrSelDayBK = m_pThemeData->clrDatesHoverBK;
    pParams->m_pGraphics->SetFillColor(&clrSelDayBK);
    pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  }
  pParams->m_pGraphics->RestoreGraphState();
  return FALSE;
}
FX_BOOL CFWL_MonthCalendarTP::DrawDatesInCircle(CFWL_ThemeBackground* pParams,
                                                CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.Create();
  CFX_RectF rtSelDay;
  rtSelDay = pParams->m_rtPart;
  path.AddRectangle(rtSelDay.left, rtSelDay.top, rtSelDay.width,
                    rtSelDay.height);
  pParams->m_pGraphics->SaveGraphState();
  CFX_Color clrSelDayBK;
  clrSelDayBK = m_pThemeData->clrDatesCircle;
  pParams->m_pGraphics->SetStrokeColor(&clrSelDayBK);
  pParams->m_pGraphics->StrokePath(&path, pMatrix);
  pParams->m_pGraphics->RestoreGraphState();
  return TRUE;
}
FX_BOOL CFWL_MonthCalendarTP::DrawTodayCircle(CFWL_ThemeBackground* pParams,
                                              CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.Create();
  CFX_RectF rtTodayCircle;
  rtTodayCircle = pParams->m_rtPart;
  path.AddRectangle(rtTodayCircle.left, rtTodayCircle.top, rtTodayCircle.width,
                    rtTodayCircle.height);
  pParams->m_pGraphics->SaveGraphState();
  CFX_Color clrTodayCircle;
  clrTodayCircle = m_pThemeData->clrDatesCircle;
  pParams->m_pGraphics->SetStrokeColor(&clrTodayCircle);
  pParams->m_pGraphics->StrokePath(&path, pMatrix);
  pParams->m_pGraphics->RestoreGraphState();
  return TRUE;
}
FWLTHEME_STATE CFWL_MonthCalendarTP::GetState(FX_DWORD dwFWLStates) {
  if (dwFWLStates & FWL_PARTSTATE_MCD_Hovered) {
    return FWLTHEME_STATE_Hover;
  } else if (dwFWLStates & FWL_PARTSTATE_MCD_Pressed) {
    return FWLTHEME_STATE_Pressed;
  }
  return FWLTHEME_STATE_Normal;
}
void CFWL_MonthCalendarTP::SetThemeData(FX_DWORD dwThemeID) {
  if (dwThemeID == 0) {
    m_pThemeData->clrCaption = ArgbEncode(0xff, 0, 153, 255);
    m_pThemeData->clrSeperator = ArgbEncode(0xff, 141, 161, 239);
    m_pThemeData->clrDatesHoverBK = ArgbEncode(0xff, 193, 211, 251);
    m_pThemeData->clrDatesSelectedBK = ArgbEncode(0xff, 173, 188, 239);
    m_pThemeData->clrDatesCircle = ArgbEncode(0xff, 103, 144, 209);
    m_pThemeData->clrToday = ArgbEncode(0xff, 0, 0, 0);
    m_pThemeData->clrBK = ArgbEncode(0xff, 255, 255, 255);
  } else {
    m_pThemeData->clrCaption = ArgbEncode(0xff, 128, 128, 0);
    m_pThemeData->clrSeperator = ArgbEncode(0xff, 128, 128, 64);
    m_pThemeData->clrDatesHoverBK = ArgbEncode(0xff, 217, 220, 191);
    m_pThemeData->clrDatesSelectedBK = ArgbEncode(0xff, 204, 208, 183);
    m_pThemeData->clrDatesCircle = ArgbEncode(0xff, 128, 128, 0);
    m_pThemeData->clrToday = ArgbEncode(0xff, 0, 0, 0);
    m_pThemeData->clrBK = ArgbEncode(0xff, 255, 255, 255);
  }
}
