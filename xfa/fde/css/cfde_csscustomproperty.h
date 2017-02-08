// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSCUSTOMPROPERTY_H_
#define XFA_FDE_CSS_CFDE_CSSCUSTOMPROPERTY_H_

#include "core/fxcrt/fx_string.h"

class CFDE_CSSCustomProperty {
 public:
  CFDE_CSSCustomProperty(const CFX_WideString& name,
                         const CFX_WideString& value);
  CFDE_CSSCustomProperty(const CFDE_CSSCustomProperty& prop);
  ~CFDE_CSSCustomProperty();

  CFX_WideString name() const { return name_; }
  CFX_WideString value() const { return value_; }

 private:
  CFX_WideString name_;
  CFX_WideString value_;
};

#endif  // XFA_FDE_CSS_CFDE_CSSCUSTOMPROPERTY_H_
