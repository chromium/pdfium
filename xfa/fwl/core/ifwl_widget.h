// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_WIDGET_H_
#define XFA_FWL_CORE_IFWL_WIDGET_H_

#include <memory>

#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/fwl_widgetimp.h"

// FWL contains three parallel inheritance hierarchies, which reference each
// other via pointers as follows:
//
//                   m_pIface                m_pImpl
//      CFWL_Widget ----------> IFWL_Widget ----------> CFWL_WidgetImp
//           |                       |                       |
//           A                       A                       A
//           |                       |                       |
//      CFWL_...                IFWL_...                CFWL_...Imp
//

class CFWL_WidgetImp;
class CFX_Graphics;
class IFWL_App;
class IFWL_DataProvider;
class IFWL_Form;
class IFWL_ThemeProvider;
class IFWL_WidgetDelegate;

class IFWL_Widget {
 public:
  IFWL_Widget() : m_pImpl(nullptr) {}
  virtual ~IFWL_Widget();

  FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  FWL_ERR GetGlobalRect(CFX_RectF& rect);
  FWL_ERR SetWidgetRect(const CFX_RectF& rect);
  FWL_ERR GetClientRect(CFX_RectF& rect);
  IFWL_Widget* GetParent();
  FWL_ERR SetParent(IFWL_Widget* pParent);
  IFWL_Widget* GetOwner();
  FWL_ERR SetOwner(IFWL_Widget* pOwner);
  IFWL_Widget* GetOuter();
  uint32_t GetStyles();
  FWL_ERR ModifyStyles(uint32_t dwStylesAdded, uint32_t dwStylesRemoved);
  uint32_t GetStylesEx();
  FWL_ERR ModifyStylesEx(uint32_t dwStylesExAdded, uint32_t dwStylesExRemoved);
  uint32_t GetStates();
  FWL_ERR SetStates(uint32_t dwStates, FX_BOOL bSet = TRUE);
  FWL_ERR SetPrivateData(void* module_id,
                         void* pData,
                         PD_CALLBACK_FREEDATA callback);
  void* GetPrivateData(void* module_id);
  FWL_ERR Update();
  FWL_ERR LockUpdate();
  FWL_ERR UnlockUpdate();
  uint32_t HitTest(FX_FLOAT fx, FX_FLOAT fy);
  FWL_ERR TransformTo(IFWL_Widget* pWidget, FX_FLOAT& fx, FX_FLOAT& fy);
  FWL_ERR TransformTo(IFWL_Widget* pWidget, CFX_RectF& rt);
  FWL_ERR GetMatrix(CFX_Matrix& matrix, FX_BOOL bGlobal = FALSE);
  FWL_ERR SetMatrix(const CFX_Matrix& matrix);
  FWL_ERR DrawWidget(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix = NULL);
  IFWL_ThemeProvider* GetThemeProvider();
  FWL_ERR SetThemeProvider(IFWL_ThemeProvider* pThemeProvider);
  FWL_ERR SetDataProvider(IFWL_DataProvider* pDataProvider);
  IFWL_WidgetDelegate* SetDelegate(IFWL_WidgetDelegate* pDelegate);
  IFWL_App* GetOwnerApp() const;
  CFX_SizeF GetOffsetFromParent(IFWL_Widget* pParent);

  // These call into equivalent polymorphic methods of m_pImpl. There
  // should be no need to override these in subclasses.
  FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  uint32_t GetClassID() const;
  FX_BOOL IsInstance(const CFX_WideStringC& wsClass) const;
  FWL_ERR Initialize();
  FWL_ERR Finalize();

  CFWL_WidgetImp* GetImpl() const { return m_pImpl.get(); }

 protected:
  // Takes ownership of |pImpl|.
  void SetImpl(CFWL_WidgetImp* pImpl) { m_pImpl.reset(pImpl); }

 private:
  std::unique_ptr<CFWL_WidgetImp> m_pImpl;
};

#endif  // XFA_FWL_CORE_IFWL_WIDGET_H_
