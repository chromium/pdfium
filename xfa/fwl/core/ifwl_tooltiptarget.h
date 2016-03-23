// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_TOOLTIPTARGET_H_
#define XFA_FWL_CORE_IFWL_TOOLTIPTARGET_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/fwl_error.h"

class CFX_Graphics;
class IFWL_Widget;

class IFWL_ToolTipTarget {
 public:
  virtual ~IFWL_ToolTipTarget() {}
  virtual IFWL_Widget* GetWidget() = 0;
  virtual FX_BOOL IsShowed() = 0;
  virtual FWL_ERR DrawToolTip(CFX_Graphics* pGraphics,
                              const CFX_Matrix* pMatrix,
                              IFWL_Widget* pToolTip) = 0;
  virtual FX_BOOL UseDefaultTheme() = 0;
  virtual FWL_ERR GetCaption(CFX_WideString& wsCaption) = 0;
  virtual FWL_ERR GetToolTipSize(CFX_SizeF& sz) = 0;
  virtual FWL_ERR GetToolTipPos(CFX_PointF& pt) { return FWL_ERR_Indefinite; }
};

FWL_ERR FWL_AddToolTipTarget(IFWL_ToolTipTarget* pTarget);
FWL_ERR FWL_RemoveToolTipTarget(IFWL_ToolTipTarget* pTarget);
FWL_ERR FWL_SetToolTipInitialDelay(int32_t iDelayTime);
FWL_ERR FWL_SetToolTipAutoPopDelay(int32_t iDelayTime);

#endif  // XFA_FWL_CORE_IFWL_TOOLTIPTARGET_H_
