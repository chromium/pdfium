// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_COMBOBOXTP_H_
#define XFA_FWL_THEME_CFWL_COMBOBOXTP_H_

#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_ComboBoxTP : public CFWL_WidgetTP {
 public:
  CFWL_ComboBoxTP();
  virtual ~CFWL_ComboBoxTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart, uint32_t dwCapacity);

 protected:
  void DrawDropDownButton(CFWL_ThemeBackground* pParams,
                          uint32_t dwStates,
                          CFX_Matrix* pMatrix);
  void DrawStrethHandler(CFWL_ThemeBackground* pParams,
                         uint32_t dwStates,
                         CFX_Matrix* pMatrix);
};

#endif  // XFA_FWL_THEME_CFWL_COMBOBOXTP_H_
