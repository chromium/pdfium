// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_COMBOBOXPROXY_H_
#define XFA_FWL_CFWL_COMBOBOXPROXY_H_

#include <memory>

#include "xfa/fwl/cfwl_formproxy.h"

class CFWL_ComboBox;

class CFWL_ComboBoxProxy : public CFWL_FormProxy {
 public:
  CFWL_ComboBoxProxy(CFWL_ComboBox* pCombobBox,
                     const CFWL_App* app,
                     std::unique_ptr<CFWL_WidgetProperties> properties,
                     CFWL_Widget* pOuter);
  ~CFWL_ComboBoxProxy() override;

  // CFWL_FormProxy
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CXFA_Graphics* pGraphics,
                    const CFX_Matrix& matrix) override;

  void Reset() { m_bLButtonUpSelf = false; }

 private:
  void OnLButtonDown(CFWL_Message* pMsg);
  void OnLButtonUp(CFWL_Message* pMsg);
  void OnFocusChanged(CFWL_Message* pMsg, bool bSet);

  bool m_bLButtonDown;
  bool m_bLButtonUpSelf;
  CFWL_ComboBox* m_pComboBox;
};

#endif  // XFA_FWL_CFWL_COMBOBOXPROXY_H_
