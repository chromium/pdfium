// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_COMBOLIST_H_
#define XFA_FWL_CORE_IFWL_COMBOLIST_H_

#include "xfa/fwl/core/cfwl_widgetimpproperties.h"
#include "xfa/fwl/core/ifwl_listbox.h"
#include "xfa/fwl/core/ifwl_widget.h"

class IFWL_ComboList : public IFWL_ListBox {
 public:
  IFWL_ComboList(const IFWL_App* app,
                 const CFWL_WidgetImpProperties& properties,
                 IFWL_Widget* pOuter);

  // IFWL_Widget
  void Initialize() override;
  void Finalize() override;

  int32_t MatchItem(const CFX_WideString& wsMatch);
  void ChangeSelected(int32_t iSel);
  int32_t CountItems();
  void GetItemRect(int32_t nIndex, CFX_RectF& rtItem);
  void ClientToOuter(FX_FLOAT& fx, FX_FLOAT& fy);
  void SetFocus(FX_BOOL bSet);

  FX_BOOL m_bNotifyOwner;

  friend class CFWL_ComboListImpDelegate;
  friend class IFWL_ComboBox;
};

class CFWL_ComboListImpDelegate : public CFWL_ListBoxImpDelegate {
 public:
  CFWL_ComboListImpDelegate(IFWL_ComboList* pOwner);
  void OnProcessMessage(CFWL_Message* pMessage) override;

 protected:
  void OnDropListFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  int32_t OnDropListMouseMove(CFWL_MsgMouse* pMsg);
  int32_t OnDropListLButtonDown(CFWL_MsgMouse* pMsg);
  int32_t OnDropListLButtonUp(CFWL_MsgMouse* pMsg);
  int32_t OnDropListKey(CFWL_MsgKey* pKey);
  void OnDropListKeyDown(CFWL_MsgKey* pKey);
  IFWL_ComboList* m_pOwner;
};

#endif  // XFA_FWL_CORE_IFWL_COMBOLIST_H_
