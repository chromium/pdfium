// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_DATETIMEEDIT_H_
#define XFA_FWL_CORE_IFWL_DATETIMEEDIT_H_

#include "xfa/fwl/core/cfwl_message.h"
#include "xfa/fwl/core/cfwl_widgetimpproperties.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_edit.h"
#include "xfa/fwl/core/ifwl_widget.h"

class IFWL_DateTimeEdit : public IFWL_Edit {
 public:
  IFWL_DateTimeEdit(const IFWL_App* app,
                    const CFWL_WidgetImpProperties& properties,
                    IFWL_Widget* pOuter);

 protected:
  friend class CFWL_DateTimeEditImpDelegate;
};

class CFWL_DateTimeEditImpDelegate : public CFWL_EditImpDelegate {
 public:
  CFWL_DateTimeEditImpDelegate(IFWL_DateTimeEdit* pOwner);
  void OnProcessMessage(CFWL_Message* pMessage) override;

 protected:
  IFWL_DateTimeEdit* m_pOwner;

 private:
  void DisForm_OnProcessMessage(CFWL_Message* pMessage);
};

#endif  // XFA_FWL_CORE_IFWL_DATETIMEEDIT_H_
