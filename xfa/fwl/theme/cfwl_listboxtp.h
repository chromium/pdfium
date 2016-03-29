// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_LISTBOXTP_H_
#define XFA_FWL_THEME_CFWL_LISTBOXTP_H_

#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_ListBoxTP : public CFWL_WidgetTP {
 public:
  CFWL_ListBoxTP();
  virtual ~CFWL_ListBoxTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

 protected:
  void DrawListBoxItem(CFX_Graphics* pGraphics,
                       uint32_t dwStates,
                       const CFX_RectF* prtItem,
                       void* pData = NULL,
                       CFX_Matrix* pMatrix = NULL);
};

#endif  // XFA_FWL_THEME_CFWL_LISTBOXTP_H_
