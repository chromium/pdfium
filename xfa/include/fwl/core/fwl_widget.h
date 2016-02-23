// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_INCLUDE_FWL_CORE_FWL_WIDGET_H_
#define XFA_INCLUDE_FWL_CORE_FWL_WIDGET_H_

#include "xfa/include/fwl/core/fwl_note.h"
#include "xfa/include/fwl/core/fwl_target.h"
#include "xfa/include/fwl/core/fwl_widgetdef.h"
#include "xfa/include/fxgraphics/fx_graphics.h"

class IFWL_ThemeProvider;
class CFWL_WidgetImpProperties;
class IFWL_DataProvider;
class IFWL_Widget;
class IFWL_WidgetDelegate;
class IFWL_Custom;
class IFWL_Proxy;
class IFWL_Form;

class IFWL_DataProvider {
 public:
  virtual ~IFWL_DataProvider() {}
  virtual FWL_ERR GetCaption(IFWL_Widget* pWidget,
                             CFX_WideString& wsCaption) = 0;
};
class IFWL_Widget : public IFWL_Target {
 public:
  FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  FWL_ERR GetGlobalRect(CFX_RectF& rect);
  FWL_ERR SetWidgetRect(const CFX_RectF& rect);
  FWL_ERR GetClientRect(CFX_RectF& rect);
  IFWL_Widget* GetParent();
  FWL_ERR SetParent(IFWL_Widget* pParent);
  IFWL_Widget* GetOwner();
  FWL_ERR SetOwner(IFWL_Widget* pOwner);
  IFWL_Widget* GetOuter();
  FX_DWORD GetStyles();
  FWL_ERR ModifyStyles(FX_DWORD dwStylesAdded, FX_DWORD dwStylesRemoved);
  FX_DWORD GetStylesEx();
  FWL_ERR ModifyStylesEx(FX_DWORD dwStylesExAdded, FX_DWORD dwStylesExRemoved);
  FX_DWORD GetStates();
  FWL_ERR SetStates(FX_DWORD dwStates, FX_BOOL bSet = TRUE);
  FWL_ERR SetPrivateData(void* module_id,
                         void* pData,
                         PD_CALLBACK_FREEDATA callback);
  void* GetPrivateData(void* module_id);
  FWL_ERR Update();
  FWL_ERR LockUpdate();
  FWL_ERR UnlockUpdate();
  FX_DWORD HitTest(FX_FLOAT fx, FX_FLOAT fy);
  FWL_ERR TransformTo(IFWL_Widget* pWidget, FX_FLOAT& fx, FX_FLOAT& fy);
  FWL_ERR TransformTo(IFWL_Widget* pWidget, CFX_RectF& rt);
  FWL_ERR GetMatrix(CFX_Matrix& matrix, FX_BOOL bGlobal = FALSE);
  FWL_ERR SetMatrix(const CFX_Matrix& matrix);
  FWL_ERR DrawWidget(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix = NULL);
  IFWL_ThemeProvider* GetThemeProvider();
  FWL_ERR SetThemeProvider(IFWL_ThemeProvider* pThemeProvider);
  FWL_ERR SetDataProvider(IFWL_DataProvider* pDataProvider);
  IFWL_WidgetDelegate* SetDelegate(IFWL_WidgetDelegate* pDelegate);
  IFWL_NoteThread* GetOwnerThread() const;
  CFX_SizeF GetOffsetFromParent(IFWL_Widget* pParent);
};
class IFWL_WidgetDelegate {
 public:
  virtual ~IFWL_WidgetDelegate() {}
  virtual int32_t OnProcessMessage(CFWL_Message* pMessage) = 0;
  virtual FWL_ERR OnProcessEvent(CFWL_Event* pEvent) = 0;
  virtual FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = NULL) = 0;
};
class CFWL_WidgetImpProperties {
 public:
  CFWL_WidgetImpProperties() {
    m_ctmOnParent.SetIdentity();
    m_rtWidget.Set(0, 0, 0, 0);
    m_dwStyles = FWL_WGTSTYLE_Child;
    m_dwStyleExes = 0;
    m_dwStates = 0;
    m_pThemeProvider = NULL;
    m_pDataProvider = NULL;
    m_pParent = NULL;
    m_pOwner = NULL;
  }
  CFX_Matrix m_ctmOnParent;
  CFX_RectF m_rtWidget;
  FX_DWORD m_dwStyles;
  FX_DWORD m_dwStyleExes;
  FX_DWORD m_dwStates;
  IFWL_ThemeProvider* m_pThemeProvider;
  IFWL_DataProvider* m_pDataProvider;
  IFWL_Widget* m_pParent;
  IFWL_Widget* m_pOwner;
};
class IFWL_Custom : public IFWL_Widget {
 public:
  static IFWL_Custom* Create(const CFWL_WidgetImpProperties& properties,
                             IFWL_Widget* pOuter);

  FWL_ERR SetProxy(IFWL_Proxy* pProxy);

 protected:
  IFWL_Custom();
};
class IFWL_Proxy {
 public:
  virtual ~IFWL_Proxy() {}
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE) = 0;
  virtual FWL_ERR Update() = 0;
};
typedef CFX_MapPtrTemplate<FX_DWORD, FX_DWORD> CFX_MapAccelerators;
FWL_ERR FWL_Accelerator_SetApp(CFX_MapAccelerators* pMapAccel);
FWL_ERR FWL_Accelerator_SetThread(CFX_MapAccelerators* pMapAccel);
FWL_ERR FWL_Accelerator_SetForm(IFWL_Form* pFrom,
                                CFX_MapAccelerators* pMapAccel);
FWL_ERR FWL_EnabelWidget(IFWL_Widget* widget, FX_BOOL bEnable);

#endif  // XFA_INCLUDE_FWL_CORE_FWL_WIDGET_H_
