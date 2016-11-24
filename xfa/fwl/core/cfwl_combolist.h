// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_COMBOLIST_H_
#define XFA_FWL_CORE_CFWL_COMBOLIST_H_

#include <memory>

#include "xfa/fwl/core/cfwl_listbox.h"
#include "xfa/fwl/core/cfwl_widget.h"
#include "xfa/fwl/core/cfwl_widgetproperties.h"

class CFWL_ComboList : public CFWL_ListBox {
 public:
  CFWL_ComboList(const CFWL_App* app,
                 std::unique_ptr<CFWL_WidgetProperties> properties,
                 CFWL_Widget* pOuter);

  // CFWL_ListBox.
  void OnProcessMessage(CFWL_Message* pMessage) override;

  int32_t MatchItem(const CFX_WideString& wsMatch);

  void ChangeSelected(int32_t iSel);

  void SetNotifyOwner(bool notify) { m_bNotifyOwner = notify; }

 private:
  void ClientToOuter(FX_FLOAT& fx, FX_FLOAT& fy);
  void OnDropListFocusChanged(CFWL_Message* pMsg, bool bSet);
  void OnDropListMouseMove(CFWL_MsgMouse* pMsg);
  void OnDropListLButtonDown(CFWL_MsgMouse* pMsg);
  void OnDropListLButtonUp(CFWL_MsgMouse* pMsg);
  bool OnDropListKey(CFWL_MsgKey* pKey);
  void OnDropListKeyDown(CFWL_MsgKey* pKey);

  bool m_bNotifyOwner;
};

#endif  // XFA_FWL_CORE_CFWL_COMBOLIST_H_
