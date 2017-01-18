// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSENUMVALUE_H_
#define XFA_FDE_CSS_CFDE_CSSENUMVALUE_H_

#include "xfa/fde/css/cfde_cssvalue.h"

class CFDE_CSSEnumValue : public CFDE_CSSValue {
 public:
  explicit CFDE_CSSEnumValue(FDE_CSSPropertyValue value);
  ~CFDE_CSSEnumValue() override;

  FDE_CSSPropertyValue Value() const { return value_; }

 private:
  FDE_CSSPropertyValue value_;
};

#endif  // XFA_FDE_CSS_CFDE_CSSENUMVALUE_H_
