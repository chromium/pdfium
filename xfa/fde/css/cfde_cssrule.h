// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSRULE_H_
#define XFA_FDE_CSS_CFDE_CSSRULE_H_

#include "xfa/fde/css/fde_css.h"

class CFDE_CSSRule {
 public:
  virtual ~CFDE_CSSRule();

  FDE_CSSRuleType GetType() const { return m_type; }

 protected:
  explicit CFDE_CSSRule(FDE_CSSRuleType type);

 private:
  FDE_CSSRuleType m_type;
};

#endif  // XFA_FDE_CSS_CFDE_CSSRULE_H_
