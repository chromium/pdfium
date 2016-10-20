// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_COMBOEDIT_H_
#define XFA_FWL_CORE_IFWL_COMBOEDIT_H_

#include "xfa/fwl/core/cfwl_widgetimpproperties.h"
#include "xfa/fwl/core/ifwl_edit.h"
#include "xfa/fwl/core/ifwl_widget.h"

class IFWL_ComboBox;

class IFWL_ComboEdit : public IFWL_Edit {
 public:
  static IFWL_ComboEdit* Create(const CFWL_WidgetImpProperties& properties,
                                IFWL_Widget* pOuter);

  IFWL_ComboEdit(const CFWL_WidgetImpProperties& properties,
                 IFWL_Widget* pOuter);

  void ClearSelected();
  void SetSelected();
  void EndCaret();
  void FlagFocus(FX_BOOL bSet);

 protected:
  void SetComboBoxFocus(FX_BOOL bSet);
  IFWL_ComboBox* m_pOuter;
  friend class CFWL_ComboEditImpDelegate;
};

class CFWL_ComboEditImpDelegate : public CFWL_EditImpDelegate {
 public:
  CFWL_ComboEditImpDelegate(IFWL_ComboEdit* pOwner);
  void OnProcessMessage(CFWL_Message* pMessage) override;

 protected:
  IFWL_ComboEdit* m_pOwner;
};

#endif  // XFA_FWL_CORE_IFWL_COMBOEDIT_H_
