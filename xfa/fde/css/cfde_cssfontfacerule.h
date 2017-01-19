// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSFONTFACERULE_H_
#define XFA_FDE_CSS_CFDE_CSSFONTFACERULE_H_

#include "xfa/fde/css/cfde_cssdeclaration.h"
#include "xfa/fde/css/cfde_cssrule.h"

class CFDE_CSSFontFaceRule : public CFDE_CSSRule {
 public:
  CFDE_CSSFontFaceRule();
  ~CFDE_CSSFontFaceRule() override;

  CFDE_CSSDeclaration* GetDeclaration() { return &m_Declaration; }

 private:
  CFDE_CSSDeclaration m_Declaration;
};

#endif  // XFA_FDE_CSS_CFDE_CSSFONTFACERULE_H_
