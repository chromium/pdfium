// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssstylerule.h"

#include "core/fxcrt/check.h"

CFX_CSSStyleRule::CFX_CSSStyleRule() = default;

CFX_CSSStyleRule::~CFX_CSSStyleRule() = default;

size_t CFX_CSSStyleRule::CountSelectorLists() const {
  return selector_.size();
}

CFX_CSSSelector* CFX_CSSStyleRule::GetSelectorList(size_t index) const {
  return selector_[index].get();
}

CFX_CSSDeclaration* CFX_CSSStyleRule::GetDeclaration() {
  return &declaration_;
}

void CFX_CSSStyleRule::SetSelector(
    std::vector<std::unique_ptr<CFX_CSSSelector>>* list) {
  DCHECK(selector_.empty());
  selector_.swap(*list);
}
