// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_FORMPROXY_H_
#define XFA_FWL_CFWL_FORMPROXY_H_

#include <memory>

#include "xfa/fwl/cfwl_form.h"

class CFWL_WidgetProperties;

class CFWL_FormProxy : public CFWL_Form {
 public:
  CFWL_FormProxy(const CFWL_App* app,
                 std::unique_ptr<CFWL_WidgetProperties> properties,
                 CFWL_Widget* pOuter);
  ~CFWL_FormProxy() override;

  // CFWL_Widget
  FWL_Type GetClassID() const override;
  bool IsInstance(const WideStringView& wsClass) const override;
  void Update() override;
  void DrawWidget(CXFA_Graphics* pGraphics, const CFX_Matrix& matrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
};

#endif  // XFA_FWL_CFWL_FORMPROXY_H_
