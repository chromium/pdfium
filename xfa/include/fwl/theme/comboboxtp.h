// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_INCLUDE_FWL_THEME_COMBOBOXTP_H_
#define XFA_INCLUDE_FWL_THEME_COMBOBOXTP_H_

#include "xfa/include/fwl/theme/widgettp.h"

class CFWL_ComboBoxTP : public CFWL_WidgetTP {
 public:
  CFWL_ComboBoxTP();
  virtual ~CFWL_ComboBoxTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart, FX_DWORD dwCapacity);

 protected:
  void DrawDropDownButton(CFWL_ThemeBackground* pParams,
                          FX_DWORD dwStates,
                          CFX_Matrix* pMatrix);
  void DrawStrethHandler(CFWL_ThemeBackground* pParams,
                         FX_DWORD dwStates,
                         CFX_Matrix* pMatrix);
};

#endif  // XFA_INCLUDE_FWL_THEME_COMBOBOXTP_H_
