// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSSTRINGVALUE_H_
#define XFA_FDE_CSS_CFDE_CSSSTRINGVALUE_H_

#include "xfa/fde/css/cfde_cssvalue.h"

class CFDE_CSSStringValue : public CFDE_CSSValue {
 public:
  explicit CFDE_CSSStringValue(const CFX_WideString& value);
  ~CFDE_CSSStringValue() override;

  const CFX_WideString Value() const { return value_; }

 private:
  const CFX_WideString value_;
};

#endif  // XFA_FDE_CSS_CFDE_CSSSTRINGVALUE_H_
