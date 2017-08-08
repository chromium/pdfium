// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CSS_CFX_CSSPROPERTYHOLDER_H_
#define CORE_FXCRT_CSS_CFX_CSSPROPERTYHOLDER_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/css/cfx_css.h"
#include "core/fxcrt/css/cfx_cssvalue.h"

class CFX_CSSPropertyHolder {
 public:
  CFX_CSSPropertyHolder();
  ~CFX_CSSPropertyHolder();

  CFX_CSSProperty eProperty;
  bool bImportant;
  CFX_RetainPtr<CFX_CSSValue> pValue;
};

#endif  // CORE_FXCRT_CSS_CFX_CSSPROPERTYHOLDER_H_
