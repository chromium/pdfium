// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_WIDGETMGR_H_
#define XFA_FWL_CORE_IFWL_WIDGETMGR_H_

#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/fwl_error.h"
#include "core/fxcrt/include/fx_coordinates.h"

class CFX_WideString;
class CFX_Matrix;
class IFWL_Widget;

enum FWL_WGTRELATION {
  FWL_WGTRELATION_Parent = 0,
  FWL_WGTRELATION_Owner,
  FWL_WGTRELATION_FirstSibling,
  FWL_WGTRELATION_PriorSibling,
  FWL_WGTRELATION_NextSibling,
  FWL_WGTRELATION_LastSibling,
  FWL_WGTRELATION_FirstChild,
  FWL_WGTRELATION_LastChild,
  FWL_WGTRELATION_SystemForm
};

class IFWL_WidgetMgr {
 public:
  virtual ~IFWL_WidgetMgr() {}
  virtual int32_t CountWidgets(IFWL_Widget* pParent = NULL) = 0;
  virtual IFWL_Widget* GetWidget(int32_t nIndex,
                                 IFWL_Widget* pParent = NULL) = 0;
  virtual IFWL_Widget* GetWidget(IFWL_Widget* pWidget,
                                 FWL_WGTRELATION eRelation) = 0;
  virtual int32_t GetWidgetIndex(IFWL_Widget* pWidget) = 0;
  virtual FX_BOOL SetWidgetIndex(IFWL_Widget* pWidget, int32_t nIndex) = 0;
  virtual FWL_Error RepaintWidget(IFWL_Widget* pWidget,
                                  const CFX_RectF* pRect = NULL) = 0;
  virtual uint32_t GetCapability() = 0;
};

IFWL_WidgetMgr* FWL_GetWidgetMgr();

#endif  // XFA_FWL_CORE_IFWL_WIDGETMGR_H_
