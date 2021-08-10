// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_LISTBOXTP_H_
#define XFA_FWL_THEME_CFWL_LISTBOXTP_H_

#include "fxjs/gc/heap.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_ListBoxTP final : public CFWL_WidgetTP {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_ListBoxTP() override;

  // CFWL_WidgetTP:
  void DrawBackground(const CFWL_ThemeBackground& pParams) override;

 private:
  CFWL_ListBoxTP();

  void DrawListBoxItem(CFGAS_GEGraphics* pGraphics,
                       Mask<CFWL_PartState> dwStates,
                       const CFX_RectF& rtItem,
                       const CFX_RectF* pData,
                       const CFX_Matrix& matrix);
};

#endif  // XFA_FWL_THEME_CFWL_LISTBOXTP_H_
