// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_TEXTPARSECONTEXT_H_
#define XFA_FXFA_APP_CXFA_TEXTPARSECONTEXT_H_

#include "xfa/fde/css/fde_css.h"

class CFDE_CSSDeclaration;
class IFDE_CSSComputedStyle;

class CXFA_TextParseContext {
 public:
  CXFA_TextParseContext();
  ~CXFA_TextParseContext();

  void SetDisplay(FDE_CSSDisplay eDisplay) { m_eDisplay = eDisplay; }
  FDE_CSSDisplay GetDisplay() const { return m_eDisplay; }

  void SetDecls(const CFDE_CSSDeclaration** ppDeclArray, int32_t iDeclCount);
  const CFDE_CSSDeclaration** GetDecls() {
    return const_cast<const CFDE_CSSDeclaration**>(m_ppMatchedDecls);
  }
  uint32_t CountDecls() const { return m_dwMatchedDecls; }

  IFDE_CSSComputedStyle* m_pParentStyle;

 protected:
  CFDE_CSSDeclaration** m_ppMatchedDecls;
  uint32_t m_dwMatchedDecls;
  FDE_CSSDisplay m_eDisplay;
};

#endif  // XFA_FXFA_APP_CXFA_TEXTPARSECONTEXT_H_
