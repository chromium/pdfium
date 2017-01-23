// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssstylerule.h"

CFDE_CSSStyleRule::CFDE_CSSStyleRule() {}

CFDE_CSSStyleRule::~CFDE_CSSStyleRule() {}

size_t CFDE_CSSStyleRule::CountSelectorLists() const {
  return m_ppSelector.size();
}

CFDE_CSSSelector* CFDE_CSSStyleRule::GetSelectorList(int32_t index) const {
  return m_ppSelector[index].get();
}

CFDE_CSSDeclaration* CFDE_CSSStyleRule::GetDeclaration() {
  return &m_Declaration;
}

void CFDE_CSSStyleRule::SetSelector(
    std::vector<std::unique_ptr<CFDE_CSSSelector>>* list) {
  ASSERT(m_ppSelector.empty());

  m_ppSelector.swap(*list);
}
