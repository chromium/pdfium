// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_BASEWIDGET_FWL_FORMPROXYIMP_H_
#define XFA_FWL_BASEWIDGET_FWL_FORMPROXYIMP_H_

#include "xfa/fwl/core/fwl_formimp.h"
#include "xfa/fwl/core/fwl_widgetimp.h"

class CFWL_WidgetImpProperties;
class CFWL_FormProxyImpDelegate;

class CFWL_FormProxyImp : public CFWL_FormImp {
 public:
  CFWL_FormProxyImp(const CFWL_WidgetImpProperties& properties,
                    IFWL_Widget* pOuter);
  ~CFWL_FormProxyImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual uint32_t GetClassID() const;
  virtual FX_BOOL IsInstance(const CFX_WideStringC& wsClass) const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FWL_ERR Update();
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);

 protected:
  friend class CFWL_FormProxyImpDelegate;
};
class CFWL_FormProxyImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_FormProxyImpDelegate(CFWL_FormProxyImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;

 protected:
  CFWL_FormProxyImp* m_pOwner;
};

#endif  // XFA_FWL_BASEWIDGET_FWL_FORMPROXYIMP_H_
