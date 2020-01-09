// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_MONTHCALENDARTP_H_
#define XFA_FWL_THEME_CFWL_MONTHCALENDARTP_H_

#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_MonthCalendarTP final : public CFWL_WidgetTP {
 public:
  CFWL_MonthCalendarTP();
  ~CFWL_MonthCalendarTP() override;

  // CFWL_WidgetTP
  void DrawBackground(const CFWL_ThemeBackground& pParams) override;
  void DrawText(const CFWL_ThemeText& pParams) override;

 private:
  void DrawTotalBK(const CFWL_ThemeBackground& pParams,
                   const CFX_Matrix& matrix);
  void DrawHeadBk(const CFWL_ThemeBackground& pParams,
                  const CFX_Matrix& matrix);
  void DrawLButton(const CFWL_ThemeBackground& pParams,
                   const CFX_Matrix& matrix);
  void DrawRButton(const CFWL_ThemeBackground& pParams,
                   const CFX_Matrix& matrix);
  void DrawDatesInBK(const CFWL_ThemeBackground& pParams,
                     const CFX_Matrix& matrix);
  void DrawDatesInCircle(const CFWL_ThemeBackground& pParams,
                         const CFX_Matrix& matrix);
  void DrawTodayCircle(const CFWL_ThemeBackground& pParams,
                       const CFX_Matrix& matrix);
  void DrawHSeparator(const CFWL_ThemeBackground& pParams,
                      const CFX_Matrix& matrix);
  void DrawWeekNumSep(const CFWL_ThemeBackground& pParams,
                      const CFX_Matrix& matrix);
  FWLTHEME_STATE GetState(uint32_t dwFWLStates);
};

#endif  // XFA_FWL_THEME_CFWL_MONTHCALENDARTP_H_
