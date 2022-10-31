// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_CARETTP_H_
#define XFA_FWL_THEME_CFWL_CARETTP_H_

#include "fxjs/gc/heap.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_CaretTP final : public CFWL_WidgetTP {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_CaretTP() override;

  // CFWL_WidgetTP
  void DrawBackground(const CFWL_ThemeBackground& pParams) override;

 private:
  CFWL_CaretTP();

  void DrawCaretBK(CFGAS_GEGraphics* pGraphics,
                   Mask<CFWL_PartState> dwStates,
                   const CFX_RectF& rect,
                   const CFX_Matrix& matrix);
};

#endif  // XFA_FWL_THEME_CFWL_CARETTP_H_
