// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSPROPERTYHOLDER_H_
#define XFA_FDE_CSS_CFDE_CSSPROPERTYHOLDER_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "xfa/fde/css/cfde_cssvalue.h"
#include "xfa/fde/css/fde_css.h"

class CFDE_CSSPropertyHolder {
 public:
  CFDE_CSSPropertyHolder();
  ~CFDE_CSSPropertyHolder();

  FDE_CSSProperty eProperty;
  bool bImportant;
  CFX_RetainPtr<CFDE_CSSValue> pValue;
};

#endif  // XFA_FDE_CSS_CFDE_CSSPROPERTYHOLDER_H_
