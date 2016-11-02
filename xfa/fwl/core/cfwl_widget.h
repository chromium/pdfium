// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_WIDGET_H_
#define XFA_FWL_CORE_CFWL_WIDGET_H_

#include <memory>

#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/cfwl_widgetproperties.h"
#include "xfa/fwl/core/ifwl_widget.h"

class CFWL_Event;
class CFWL_Message;
class CFWL_Widget;
class CFWL_WidgetDelegate;
class CFWL_WidgetMgr;

class CFWL_Widget {
 public:
  CFWL_Widget(const IFWL_App*);
  virtual ~CFWL_Widget();

  IFWL_Widget* GetWidget();
  const IFWL_Widget* GetWidget() const;

  FWL_Error GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  FWL_Error SetWidgetRect(const CFX_RectF& rect);
  FWL_Error GetGlobalRect(CFX_RectF& rect);
  FWL_Error GetClientRect(CFX_RectF& rtClient);

  FWL_Error ModifyStyles(uint32_t dwStylesAdded, uint32_t dwStylesRemoved);
  uint32_t GetStylesEx();
  FWL_Error ModifyStylesEx(uint32_t dwStylesExAdded,
                           uint32_t dwStylesExRemoved);

  uint32_t GetStates();
  void SetStates(uint32_t dwStates, FX_BOOL bSet = TRUE);

  void SetLayoutItem(void* pItem);

  void Update();
  void LockUpdate();
  void UnlockUpdate();

  FWL_WidgetHit HitTest(FX_FLOAT fx, FX_FLOAT fy);

  FWL_Error DrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = nullptr);

  IFWL_WidgetDelegate* GetDelegate() const;
  void SetDelegate(IFWL_WidgetDelegate*);

 protected:
  void Initialize();

  const IFWL_App* m_pApp;
  std::unique_ptr<IFWL_Widget> m_pIface;
  CFWL_WidgetMgr* const m_pWidgetMgr;
  std::unique_ptr<CFWL_WidgetProperties> m_pProperties;
};

#endif  // XFA_FWL_CORE_CFWL_WIDGET_H_
