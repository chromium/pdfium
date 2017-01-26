// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_TEXTPARSECONTEXT_H_
#define XFA_FXFA_APP_CXFA_TEXTPARSECONTEXT_H_

#include <utility>
#include <vector>

#include "third_party/base/stl_util.h"
#include "xfa/fde/css/cfde_cssdeclaration.h"
#include "xfa/fde/css/fde_css.h"

class CFDE_CSSComputedStyle;

class CXFA_TextParseContext {
 public:
  CXFA_TextParseContext();
  ~CXFA_TextParseContext();

  void SetDisplay(FDE_CSSDisplay eDisplay) { m_eDisplay = eDisplay; }
  FDE_CSSDisplay GetDisplay() const { return m_eDisplay; }

  void SetDecls(std::vector<const CFDE_CSSDeclaration*>&& decl) {
    decls_ = std::move(decl);
  }
  const std::vector<const CFDE_CSSDeclaration*>& GetDecls() { return decls_; }

  CFX_RetainPtr<CFDE_CSSComputedStyle> m_pParentStyle;

 protected:
  std::vector<const CFDE_CSSDeclaration*> decls_;
  FDE_CSSDisplay m_eDisplay;
};

#endif  // XFA_FXFA_APP_CXFA_TEXTPARSECONTEXT_H_
