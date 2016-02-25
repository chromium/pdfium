// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_INCLUDE_FWL_CORE_FWL_PANEL_H_
#define XFA_INCLUDE_FWL_CORE_FWL_PANEL_H_

#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"

class IFWL_Content;

#define FWL_CLASS_Panel L"FWL_Panel"
#define FWL_CLASSHASH_Panel 881567292

class IFWL_Panel : public IFWL_Widget {
 public:
  static IFWL_Panel* Create(CFWL_WidgetImpProperties& properties,
                            IFWL_Widget* pOuter);

  IFWL_Content* GetContent();
  FWL_ERR SetContent(IFWL_Content* pContent);

 protected:
  IFWL_Panel();
};

class IFWL_CustomPanel : public IFWL_Widget {
 public:
  static IFWL_CustomPanel* Create(CFWL_WidgetImpProperties& properties,
                                  IFWL_Widget* pOuter);

  IFWL_Content* GetContent();
  FWL_ERR SetContent(IFWL_Content* pContent);
  FWL_ERR SetProxy(IFWL_Proxy* pProxy);

 protected:
  IFWL_CustomPanel();
};

#endif  // XFA_INCLUDE_FWL_CORE_FWL_PANEL_H_
