// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_DATETIMEPICKERTP_H
#define _FWL_DATETIMEPICKERTP_H
class CFWL_WidgetTP;
class CFWL_DateTimePickerTP;
class CFWL_DateTimePickerTP : public CFWL_WidgetTP {
 public:
  CFWL_DateTimePickerTP();
  virtual ~CFWL_DateTimePickerTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);

 protected:
  void DrawDropDownButton(CFWL_ThemeBackground* pParams, CFX_Matrix* pMatrix);
  void initThemeData();
  struct DTPThemeData {
    FX_ARGB BoxBkColor[13][2];
  } * m_pThemeData;
};
#endif
