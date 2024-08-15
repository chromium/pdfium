// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_COMBOLIST_H_
#define XFA_FWL_CFWL_COMBOLIST_H_

#include "xfa/fwl/cfwl_listbox.h"
#include "xfa/fwl/cfwl_widget.h"

namespace pdfium {

class CFWL_ComboList final : public CFWL_ListBox {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_ComboList() override;

  // CFWL_ListBox.
  void OnProcessMessage(CFWL_Message* pMessage) override;

  int32_t MatchItem(WideStringView wsMatch);
  void ChangeSelected(int32_t iSel);

 private:
  CFWL_ComboList(CFWL_App* app,
                 const Properties& properties,
                 CFWL_Widget* pOuter);

  CFX_PointF ClientToOuter(const CFX_PointF& point);
  void OnDropListFocusChanged(CFWL_Message* pMsg, bool bSet);
  void OnDropListMouseMove(CFWL_MessageMouse* pMsg);
  void OnDropListLButtonDown(CFWL_MessageMouse* pMsg);
  void OnDropListLButtonUp(CFWL_MessageMouse* pMsg);
  bool OnDropListKey(CFWL_MessageKey* pKey);
  void OnDropListKeyDown(CFWL_MessageKey* pKey);

  bool m_bNotifyOwner = true;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_ComboList;

#endif  // XFA_FWL_CFWL_COMBOLIST_H_
