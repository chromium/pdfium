// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_FWL_PANELIMP_H_
#define XFA_FWL_CORE_FWL_PANELIMP_H_

#include "xfa/fwl/core/fwl_widgetimp.h"

class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_Content;

class CFWL_PanelImp : public CFWL_WidgetImp {
 public:
  CFWL_PanelImp(const CFWL_WidgetImpProperties& properties,
                IFWL_Widget* pOuter);
  virtual ~CFWL_PanelImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual uint32_t GetClassID() const;
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR Update();
  virtual IFWL_Content* GetContent();
  virtual FWL_ERR SetContent(IFWL_Content* pContent);

 protected:
  IFWL_Content* m_pContent;
};

#endif  // XFA_FWL_CORE_FWL_PANELIMP_H_
