// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSVALUE_H_
#define XFA_FDE_CSS_CFDE_CSSVALUE_H_

#include "xfa/fde/css/fde_css.h"

class CFDE_CSSValue {
 public:
  virtual ~CFDE_CSSValue();

  FDE_CSSVALUETYPE GetType() const { return m_value; }

 protected:
  explicit CFDE_CSSValue(FDE_CSSVALUETYPE type);

 private:
  FDE_CSSVALUETYPE m_value;
};

#endif  // XFA_FDE_CSS_CFDE_CSSVALUE_H_
