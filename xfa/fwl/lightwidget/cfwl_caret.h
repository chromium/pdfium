// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_LIGHTWIDGET_CFWL_CARET_H_
#define XFA_FWL_LIGHTWIDGET_CFWL_CARET_H_

#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fxgraphics/cfx_color.h"
#include "xfa/fwl/lightwidget/cfwl_widget.h"

class CFWL_Caret : public CFWL_Widget {
 public:
  static CFWL_Caret* Create();
  FWL_ERR Initialize(const CFWL_WidgetProperties* pProperties = NULL);
  FWL_ERR ShowCaret(FX_BOOL bFlag = TRUE);
  FWL_ERR GetFrequency(uint32_t& elapse);
  FWL_ERR SetFrequency(uint32_t elapse);
  FWL_ERR SetColor(CFX_Color crFill);

 protected:
  CFWL_Caret();
  virtual ~CFWL_Caret();
};

#endif  // XFA_FWL_LIGHTWIDGET_CFWL_CARET_H_
