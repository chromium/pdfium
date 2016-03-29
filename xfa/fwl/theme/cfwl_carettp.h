// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_CARETTP_H_
#define XFA_FWL_THEME_CFWL_CARETTP_H_

#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_CaretTP : public CFWL_WidgetTP {
 public:
  CFWL_CaretTP();
  virtual ~CFWL_CaretTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);

 protected:
  void DrawCaretBK(CFX_Graphics* pGraphics,
                   uint32_t dwStates,
                   const CFX_RectF* pRect,
                   CFX_Color* crFill,
                   CFX_Matrix* pMatrix = NULL);
};

#endif  // XFA_FWL_THEME_CFWL_CARETTP_H_
