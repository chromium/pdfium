// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssvaluelist.h"

#include <utility>

#include "xfa/fde/css/fde_css.h"

CFDE_CSSValueList::CFDE_CSSValueList(
    std::vector<CFX_RetainPtr<CFDE_CSSValue>>& list)
    : CFDE_CSSValue(FDE_CSSPrimitiveType::List), m_ppList(std::move(list)) {}

CFDE_CSSValueList::~CFDE_CSSValueList() {}

int32_t CFDE_CSSValueList::CountValues() const {
  return m_ppList.size();
}

CFX_RetainPtr<CFDE_CSSValue> CFDE_CSSValueList::GetValue(int32_t index) const {
  return m_ppList[index];
}
