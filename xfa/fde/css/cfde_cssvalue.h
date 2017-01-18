// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSVALUE_H_
#define XFA_FDE_CSS_CFDE_CSSVALUE_H_

#include "xfa/fde/css/fde_css.h"

class CFDE_CSSValue : public CFX_Retainable {
 public:
  FDE_CSSPrimitiveType GetType() const { return m_value; }

 protected:
  explicit CFDE_CSSValue(FDE_CSSPrimitiveType type);

 private:
  FDE_CSSPrimitiveType m_value;
};

#endif  // XFA_FDE_CSS_CFDE_CSSVALUE_H_
