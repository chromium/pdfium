// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSCOLORVALUE_H_
#define XFA_FDE_CSS_CFDE_CSSCOLORVALUE_H_

#include "xfa/fde/css/cfde_cssvalue.h"

class CFDE_CSSColorValue : public CFDE_CSSValue {
 public:
  explicit CFDE_CSSColorValue(FX_ARGB color);
  ~CFDE_CSSColorValue() override;

  FX_ARGB Value() const { return value_; }

 private:
  FX_ARGB value_;
};

#endif  // XFA_FDE_CSS_CFDE_CSSCOLORVALUE_H_
