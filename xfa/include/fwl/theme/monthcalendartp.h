// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_MONTHCALENDARTP_H
#define _FWL_MONTHCALENDARTP_H
class CFWL_WidgetTP;
class CFWL_MonthCalendarTP;
class CFWL_MonthCalendarTP : public CFWL_WidgetTP {
 public:
  CFWL_MonthCalendarTP();
  virtual ~CFWL_MonthCalendarTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual FX_DWORD SetThemeID(IFWL_Widget* pWidget,
                              FX_DWORD dwThemeID,
                              FX_BOOL bChildren = TRUE);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual FX_BOOL DrawText(CFWL_ThemeText* pParams);
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart, FX_DWORD dwCapacity);
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

 protected:
  FX_BOOL DrawTotalBK(CFWL_ThemeBackground* pParams, CFX_Matrix* pMatrix);
  FX_BOOL DrawHeadBk(CFWL_ThemeBackground* pParams, CFX_Matrix* pMatrix);
  FX_BOOL DrawLButton(CFWL_ThemeBackground* pParams, CFX_Matrix* pMatrix);
  FX_BOOL DrawRButton(CFWL_ThemeBackground* pParams, CFX_Matrix* pMatrix);
  FX_BOOL DrawDatesInBK(CFWL_ThemeBackground* pParams, CFX_Matrix* pMatrix);
  FX_BOOL DrawDatesInCircle(CFWL_ThemeBackground* pParams, CFX_Matrix* pMatrix);
  FX_BOOL DrawTodayCircle(CFWL_ThemeBackground* pParams, CFX_Matrix* pMatrix);
  FX_BOOL DrawHSeperator(CFWL_ThemeBackground* pParams, CFX_Matrix* pMatrix);
  FX_BOOL DrawWeekNumSep(CFWL_ThemeBackground* pParams, CFX_Matrix* pMatrix);
  FWLTHEME_STATE GetState(FX_DWORD dwFWLStates);
  void SetThemeData(FX_DWORD dwThemeID);
  class MCThemeData {
   public:
    FX_ARGB clrCaption;
    FX_ARGB clrSeperator;
    FX_ARGB clrDatesHoverBK;
    FX_ARGB clrDatesSelectedBK;
    FX_ARGB clrDatesCircle;
    FX_ARGB clrToday;
    FX_ARGB clrBK;
  } * m_pThemeData;
  CFX_WideString wsResource;
};
#endif
