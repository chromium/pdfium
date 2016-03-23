// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_PANEL_H_
#define XFA_FWL_CORE_IFWL_PANEL_H_

#include "xfa/fwl/core/fwl_widgetimp.h"
#include "xfa/fwl/core/ifwl_widget.h"

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

#endif  // XFA_FWL_CORE_IFWL_PANEL_H_
