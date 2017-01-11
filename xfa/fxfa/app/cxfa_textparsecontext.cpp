// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_textparsecontext.h"

#include "xfa/fde/css/fde_cssdeclaration.h"

CXFA_TextParseContext::CXFA_TextParseContext()
    : m_pParentStyle(nullptr),
      m_ppMatchedDecls(nullptr),
      m_dwMatchedDecls(0),
      m_eDisplay(FDE_CSSDisplay::None) {}

CXFA_TextParseContext::~CXFA_TextParseContext() {
  if (m_pParentStyle)
    m_pParentStyle->Release();
  FX_Free(m_ppMatchedDecls);
}

void CXFA_TextParseContext::SetDecls(const CFDE_CSSDeclaration** ppDeclArray,
                                     int32_t iDeclCount) {
  if (iDeclCount <= 0 || !ppDeclArray)
    return;

  m_dwMatchedDecls = iDeclCount;
  m_ppMatchedDecls = FX_Alloc(CFDE_CSSDeclaration*, iDeclCount);
  FXSYS_memcpy(m_ppMatchedDecls, ppDeclArray,
               iDeclCount * sizeof(CFDE_CSSDeclaration*));
}
